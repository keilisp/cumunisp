#include "mpc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// If we are compiling on Windows compile these functions
#ifdef _WIN32
/* #include <string.h> */

static char buffer[2048];

char *readline(char *prompt) {
  fputs(prompt, stdout);
  fgets(buffer, 2048, stdin);
  char *cpy = malloc(strlen(buffer) + 1);
  strcpy(cpy, buffer);
  cpy[strlen(cpy) - 1] = '\0';
  return cpy;
}

void add_history(char *unused) {}

#else
#include <editline/readline.h>
#endif

// New lval Struct
typedef struct lval {
  int type;
  double num;
  char *err;
  char *sym;
  // Count and Pointer to a list of "lval"
  int count;
  struct lval **cell;
} lval;
// Enumeration of possible lval types
enum { LVAL_NUM, LVAL_SYM, LVAL_SEXPR, LVAL_QEXPR, LVAL_ERR };

// Create a pointer to a new Number type lval
lval *lval_num(double x) {
  lval *v = malloc(sizeof(lval));
  v->type = LVAL_NUM;
  v->num = x;
  return v;
}

// A pointer to a new empty Qexpr lval
lval *lval_qexpr(void) {
  lval *v = malloc(sizeof(lval));
  v->type = LVAL_QEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}

// Create a pointer to a new Error type lval
lval *lval_err(char *m) {
  lval *v = malloc(sizeof(lval));
  v->type = LVAL_ERR;
  v->err = malloc(strlen(m) + 1);
  strcpy(v->err, m);
  return v;
}

// Create a pointer to a new Symbol type lval
lval *lval_sym(char *s) {
  lval *v = malloc(sizeof(lval));
  v->type = LVAL_SYM;
  v->sym = malloc(strlen(s) + 1);
  strcpy(v->sym, s);
  return v;
}

// A point to a new empty Sexpr lval
lval *lval_sexpr(void) {
  lval *v = malloc(sizeof(lval));
  v->type = LVAL_SEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}

// Function that calls free() for every malloc to prevent memory leaks
void lval_del(lval *v) {
  switch (v->type) {
  // Do nothing for number type
  case LVAL_NUM:
    break;

  // For Err or Sym free the string data
  case LVAL_ERR:
    free(v->err);
    break;
  case LVAL_SYM:
    free(v->sym);
    break;

    // If Qexpr or Sexpr then delete all elems inside
  case LVAL_QEXPR:
  case LVAL_SEXPR:
    for (int i = 0; i < v->count; i++) {
      lval_del(v->cell[i]);
    }

    // Then also free the memory allocated to conatin the pointers
    free(v->cell);
    break;
  }
  free(v);
}
lval *lval_read_num(mpc_ast_t *t) {
  errno = 0;
  double x = strtod(t->contents, NULL);
  return errno != ERANGE ? lval_num(x) : lval_err("Invalid number!");
}

lval *lval_add(lval *v, lval *x) {
  v->count++;
  v->cell = realloc(v->cell, sizeof(lval *) * v->count);
  v->cell[v->count - 1] = x;
  return v;
}

lval *
lval_read(mpc_ast_t *t) { // If Symbol or Number return conversion to that type
  if (strstr(t->tag, "number")) {
    return lval_read_num(t);
  }
  if (strstr(t->tag, "symbol")) {
    return lval_sym(t->contents);
  }

  // If root (>) or sexpr then crete empty list
  lval *x = NULL;
  if (strstr(t->tag, ">")) {
    x = lval_sexpr();
  }
  if (strstr(t->tag, "sexpr")) {
    x = lval_sexpr();
  }

  if (strstr(t->tag, "qexpr")) {
    x = lval_qexpr();
  }

  // Fill this list with any valid wxpression contained within
  for (int i = 0; i < t->children_num; i++) {
    if (strcmp(t->children[i]->contents, "(") == 0) {
      continue;
    }
    if (strcmp(t->children[i]->contents, ")") == 0) {
      continue;
    }
    if (strcmp(t->children[i]->contents, "{") == 0) {
      continue;
    }
    if (strcmp(t->children[i]->contents, "}") == 0) {
      continue;
    }
    if (strcmp(t->children[i]->tag, "regex") == 0) {
      continue;
    }
    x = lval_add(x, lval_read(t->children[i]));
  }
  return x;
}

void lval_print(lval *v);

// Function to loop over all the sub-expresession and prints them
void lval_expr_print(lval *v, char open, char close) {
  putchar(open);
  for (int i = 0; i < v->count; i++) {
    // Print Value contained within
    lval_print(v->cell[i]);

    // Don't print trailing space if last element
    if (i != (v->count - 1)) {
      putchar(' ');
    }
  }
  putchar(close);
}

// Print an "lval"
void lval_print(lval *v) {
  // In the case the type is a number print it
  switch (v->type) {
  case LVAL_NUM:
    printf("%g", v->num);
    break;
    // In the case the type is an error
  case LVAL_ERR:
    printf("Error: %s", v->err);
    break;
  case LVAL_SYM:
    printf("%s", v->sym);
    break;
  case LVAL_SEXPR:
    lval_expr_print(v, '(', ')');
    break;
  case LVAL_QEXPR:
    lval_expr_print(v, '{', '}');
    break;
  }
}

// Print an "lval" followed by a newline
void lval_println(lval *v) {
  lval_print(v);
  putchar('\n');
}

lval *lval_pop(lval *v, int i) {
  // Find the item at "i"
  lval *x = v->cell[i];

  // Shift memory after the item at "i" over the top
  memmove(&v->cell[i], &v->cell[i + 1], sizeof(lval *) * (v->count - i - 1));

  // Decrease the count of items in the list
  v->count--;

  // Reallocate the memory user
  v->cell = realloc(v->cell, sizeof(lval *) * v->count);
  return x;
}
lval *lval_take(lval *v, int i) {
  lval *x = lval_pop(v, i);
  lval_del(v);
  return x;
}

lval *lval_eval_sexpr(lval *v);

lval *lval_eval(lval *v) {
  // Evaluate Sexpressions
  if (v->type == LVAL_SEXPR) {
    return lval_eval_sexpr(v);
  }
  return v;
}

lval *builtin_op(lval *a, char *op) {
  // Ensure all args are numbers
  for (int i = 0; i < a->count; i++) {
    if (a->cell[i]->type != LVAL_NUM) {
      lval_del(a);
      return lval_err("Cannot operate on non-number");
    }
  }
  // Pop the firse element
  lval *x = lval_pop(a, 0);

  // If no args and sub then perform unary negation
  if ((strcmp(op, "-") == 0) && a->count == 0) {
    x->num = -x->num;
  }

  // While there are still elems remaining
  while (a->count > 0) {
    // Pop the next elem
    lval *y = lval_pop(a, 0);

    // Perform operation
    if (strcmp(op, "+") == 0 || strcmp(op, "add") == 0) {
      x->num += y->num;
    }
    if (strcmp(op, "-") == 0 || strcmp(op, "sub") == 0) {
      x->num -= y->num;
    }
    if (strcmp(op, "*") == 0 || strcmp(op, "mul") == 0) {
      x->num *= y->num;
    }
    if (strcmp(op, "/") == 0 || strcmp(op, "div") == 0) {
      if (y->num == 0) {
        lval_del(x);
        lval_del(y);
        x = lval_err("Division by zero!");
        break;
      }
      x->num /= y->num;
    }
    if (strcmp(op, "%") == 0 || strcmp(op, "rem") == 0) {
      x->num = fmod(x->num, y->num);
    }
    if (strcmp(op, "^") == 0 || strcmp(op, "pow") == 0) {
      x->num = pow(x->num, y->num);
    }
    if (strcmp(op, "max") == 0) {
      x->num = fmax(x->num, y->num);
    }
    if (strcmp(op, "min") == 0) {
      x->num = fmin(x->num, y->num);
    }
    // Delete elem now finished with
    lval_del(y);
  }
  // Delete input expression and return result
  lval_del(a);
  return x;
}

#define LASSERT(args, cond, err)                                               \
  if (!(cond)) {                                                               \
    lval_del(args);                                                            \
    return lval_err(err);                                                      \
  }

lval *builtin_head(lval *a) {
  // Check Error Conditions
  LASSERT(a, a->count == 1, "Function 'head' passed to many arguments!");
  LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
          "Function 'head' passed incorrect type!");
  LASSERT(a, a->cell[0]->count != 0,
          "Function 'head' passed empty quote expression");

  // Otherwise take first arg
  lval *v = lval_take(a, 0);

  // Delete all elems that are not head and return
  while (v->count > 1) {
    lval_del(lval_pop(v, 1));
  }
  return v;
}

lval *builtin_tail(lval *a) {
  // Check Error Conditions
  LASSERT(a, a->count == 1, "Function 'tail' passed to many arguments!");
  LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
          "Function 'tail' passed incorrect type!");
  LASSERT(a, a->cell[0]->count != 0,
          "Function 'tail' passed empty quote expression");

  // Otherwise take first arg
  lval *v = lval_take(a, 0);

  // Delete firs elem and return
  lval_del(lval_pop(v, 0));
  return v;
}

lval *builtin_eval(lval *a) {
  LASSERT(a, a->count == 1, "Function 'eval' passed too many arguments!");
  LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
          "Function 'eval' passed incorrect type!");

  lval *x = lval_take(a, 0);
  x->type = LVAL_SEXPR;
  return lval_eval(x);
}

lval *lval_join(lval *x, lval *y) {
  // For each cell in 'y' add it to 'x'
  while (y->count) {
    x = lval_add(x, lval_pop(y, 0));
  }
  // Then delete the empty 'y' and return 'x'
  lval_del(y);
  return x;
}

lval *builtin_join(lval *a) {
  for (int i = 0; i < a->count; i++) {
    LASSERT(a, a->cell[i]->type == LVAL_QEXPR,
            "Function 'join' passed incorrect type.")
  }

  lval *x = lval_pop(a, 0);
  while (a->count) {
    x = lval_join(x, lval_pop(a, 0));
  }

  lval_del(a);
  return x;
}

lval *builtin_list(lval *a) {
  a->type = LVAL_QEXPR;
  return a;
}

lval *builtin_cons(lval *a) {
  LASSERT(a, a->cell[1]->type == LVAL_QEXPR,
          "Function 'cons' passed incorrect type!");
  LASSERT(a, a->count == 2, "Function 'cons' passed too many arguments");

  // Create list of the entire expr
  lval *x = builtin_list(a);
  // Pop the qexpr
  lval *y = lval_pop(a, 1);
  // Append cell[0] to the qexpr
  lval *list = lval_join(x, y);

  return list;
}

lval *builtin_len(lval *a) {
  LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
          "Function 'len' passed incorrect type!");
  LASSERT(a, a->count == 1, "Function 'len' passed too many arguments");
  // Just return cell[0] count
  return lval_num(a->cell[0]->count);
}

lval *builtin_init(lval *a) {
  // Check Error Conditions
  LASSERT(a, a->count == 1, "Function 'init' passed to many arguments!");
  LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
          "Function 'init' passed incorrect type!");
  LASSERT(a, a->cell[0]->count != 0,
          "Function 'init' passed empty quote expression");

  // Get the length of qexpr
  lval *x = builtin_len(a);
  int len = (int)x->num;
  // Take only last elem
  lval *v = lval_take(a->cell[0], len - 1);

  return v;
}

lval *builtin(lval *a, char *func) {
  if (strcmp("list", func) == 0) {
    return builtin_list(a);
  }
  if (strcmp("head", func) == 0) {
    return builtin_head(a);
  }
  if (strcmp("tail", func) == 0) {
    return builtin_tail(a);
  }
  if (strcmp("join", func) == 0) {
    return builtin_join(a);
  }
  if (strcmp("eval", func) == 0) {
    return builtin_eval(a);
  }
  if (strcmp("cons", func) == 0) {
    return builtin_cons(a);
  }
  if (strcmp("len", func) == 0) {
    return builtin_len(a);
  }
  if (strcmp("init", func) == 0) {
    return builtin_init(a);
  }
  if (strcmp("add", func) == 0) {
    return builtin_op(a, func);
  }
  if (strcmp("sub", func) == 0) {
    return builtin_op(a, func);
  }
  if (strcmp("mul", func) == 0) {
    return builtin_op(a, func);
  }
  if (strcmp("div", func) == 0) {
    return builtin_op(a, func);
  }
  if (strcmp("rem", func) == 0) {
    return builtin_op(a, func);
  }
  if (strcmp("pow", func) == 0) {
    return builtin_op(a, func);
  }
  if (strcmp("min", func) == 0) {
    return builtin_op(a, func);
  }
  if (strcmp("max", func) == 0) {
    return builtin_op(a, func);
  }
  if (strstr("+-*/%^", func)) {
    return builtin_op(a, func);
  }

  lval_del(a);
  return lval_err("Unknown Function");
}

lval *lval_eval_sexpr(lval *v) {
  // Evaluate children
  for (int i = 0; i < v->count; i++) {
    v->cell[i] = lval_eval(v->cell[i]);
  }

  // Error checking
  for (int i = 0; i < v->count; i++) {
    if (v->cell[i]->type == LVAL_ERR) {
      return lval_take(v, i);
    }
  }

  // Empty Expression
  if (v->count == 0) {
    return v;
  }

  // Single Expression
  if (v->count == 1) {
    return lval_take(v, 0);
  }

  // Ensuyre Firse Element is Symbol
  lval *f = lval_pop(v, 0);
  if (f->type != LVAL_SYM) {
    lval_del(f);
    lval_del(v);
    return lval_err("S-expression does not start with symbol");
  }

  // Vall builtin with operator
  lval *result = builtin(v, f->sym);
  lval_del(f);
  return result;
}
int main() {
  // Create some Parsers
  mpc_parser_t *Number = mpc_new("number");
  mpc_parser_t *Symbol = mpc_new("symbol");
  mpc_parser_t *Sexpr = mpc_new("sexpr");
  mpc_parser_t *Qexpr = mpc_new("qexpr");
  mpc_parser_t *Expr = mpc_new("expr");
  mpc_parser_t *Cumunisp = mpc_new("cumunisp");

  // Define Parsers
  mpca_lang(MPCA_LANG_DEFAULT,
            "                                                     \
      number   : /-?[0-9]+(\\.[0-9]*)?/ ;                             \
      symbol   : '+' | '-' | '*' | '/' | '%' | '^'     \
               | \"add\" | \"sub\" | \"mul\" | \"div\" | \"rem\" | \"pow\" \
               | \"max\" | \"min\"   \
               | \"list\"| \"head\"| \"tail\"| \"join\"| \"eval\"| \"cons\" | \"len\" | \"init\" ;  \
      sexpr    : '(' <expr>* ')' ; \
      qexpr    : '{' <expr>* '}' ; \
      expr     : <number> | <symbol> | <sexpr> | <qexpr> ;  \
      cumunisp    : /^/ <expr>* /$/ ;             \
    ",
            Number, Symbol, Sexpr, Qexpr, Expr, Cumunisp);

  // Print Version and Exit Information
  puts("Cumunisp Version 0.0.0.0.1");
  puts("Press Ctrl+c to Exit\n");

  while (1) {
    // Output prompt
    char *input = readline("cumunisp> ");
    // Add input to history
    add_history(input);

    // Attempt to Parse the user Input
    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Cumunisp, &r)) {

      lval *result = lval_eval(lval_read(r.output));
      lval_println(result);
      lval_del(result);
      mpc_ast_delete(r.output);
    } else {
      // Otherwise print the Error
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }

    // Free retrieved input
    free(input);
  }

  // Undefine and delete Parsers
  mpc_cleanup(4, Number, Symbol, Sexpr, Qexpr, Expr, Cumunisp);
  return 0;
}
