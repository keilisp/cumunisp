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

// Forward Declarations

struct lval;
struct lenv;
typedef struct lval lval;
typedef struct lenv lenv;

// Enumeration of possible lval types
enum { LVAL_NUM, LVAL_SYM, LVAL_SEXPR, LVAL_QEXPR, LVAL_ERR, LVAL_FUN };

typedef lval *(*lbuiltin)(lenv *, lval *);

// lval Struct
struct lval {
  int type;

  double num;
  char *err;
  char *sym;
  lbuiltin fun;

  // Count and Pointer to a list of "lval"
  int count;
  struct lval **cell;
};

// lval fun constructor
lval *lval_fun(lbuiltin func) {
  lval *v = malloc(sizeof(lval));
  v->type = LVAL_FUN;
  v->cell = NULL;
  v->fun = func;
  return v;
}

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
lval *lval_err(char *fmt, ...) {
  lval *v = malloc(sizeof(lval));
  v->type = LVAL_ERR;

  // Create a va list and initialize it
  va_list va;
  va_start(va, fmt);

  // Allocate 512 bytes of space
  v->err = malloc(512);

  // printf the error string with a maximum of 511 characters
  vsnprintf(v->err, 511, fmt, va);

  // Reallocate to number of bytes actually used
  v->err = realloc(v->err, strlen(v->err) + 1);

  // Cleanup va list
  va_end(va);

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

    // DO nothing for fun type
  case LVAL_FUN:
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

    // lval fun type
  case LVAL_FUN:
    printf("<function>");
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

// Copying the environment
lval *lval_copy(lval *v) {
  lval *x = malloc(sizeof(lval));
  x->type = v->type;

  switch (v->type) {
  // Copy Functions and Numbers directly
  case LVAL_FUN:
    x->fun = v->fun;
    break;
  case LVAL_NUM:
    x->num = v->num;
    break;

  // Copy Strings using malloc and strcpy
  case LVAL_ERR:
    x->err = malloc(sizeof(strlen(v->err) + 1));
    strcpy(x->err, v->err);
    break;
  case LVAL_SYM:
    x->sym = malloc(sizeof(strlen(v->sym) + 1));
    strcpy(x->sym, v->sym);
    break;

  // Copy Lists by copying each sub-expression
  case LVAL_SEXPR:
  case LVAL_QEXPR:
    x->count = v->count;
    x->cell = malloc(sizeof(lval *) * x->count);
    for (int i = 0; i < x->count; i++) {
      x->cell[i] = lval_copy(v->cell[i]);
    }
    break;
  }

  return x;
}

// Environment
struct lenv {
  int count;
  char **syms;
  lval **vals;
};

// New environment
lenv *lenv_new(void) {
  lenv *e = malloc(sizeof(lenv));
  e->count = 0;
  e->syms = NULL;
  e->vals = NULL;
  return e;
}

// Deletion for environment
void lenv_del(lenv *e) {
  for (int i = 0; i < e->count; i++) {
    free(e->syms[i]);
    lval_del(e->vals[i]);
  }

  free(e->syms);
  free(e->vals);
  free(e);
}

// Get from environment
lval *lenv_get(lenv *e, lval *k) {
  // Iterate over all items in environment
  for (int i = 0; i < e->count; i++) {
    // Check if the stored string matches the symbel string
    // If it does, return a copy of the value
    if (strcmp(e->syms[i], k->sym) == 0) {
      return lval_copy(e->vals[i]);
    }
  }
  // If no symbol found return error
  return lval_err("Unbound symbol! '%s'", k->sym);
}

// Put into environment
void lenv_put(lenv *e, lval *k, lval *v) {
  // Iterate over all items in environment
  // This is to see if variable already exists
  for (int i = 0; i < e->count; i++) {
    // If variable is found delete itema at that position
    // And replace with variable supplied by user
    if (strcmp(e->syms[i], k->sym) == 0) {
      lval_del(e->vals[i]);
      e->vals[i] = lval_copy(v);
      return;
    }
  }

  // If no existing entry found allocate space for new entry
  e->count++;
  e->vals = realloc(e->vals, sizeof(lval *) * e->count);
  e->syms = realloc(e->syms, sizeof(lval *) * e->count);

  // Copy contents of lval and symbol string into new location
  e->vals[e->count - 1] = lval_copy(v);
  e->syms[e->count - 1] = malloc(strlen(k->sym) + 1);
  strcpy(e->syms[e->count - 1], k->sym);
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

lval *lval_eval_sexpr(lenv *e, lval *v);

lval *lval_eval(lenv *e, lval *v) {
  if (v->type == LVAL_SYM) {
    lval *x = lenv_get(e, v);
    lval_del(v);
    return x;
  }
  // Evaluate Sexpressions
  if (v->type == LVAL_SEXPR) {
    return lval_eval_sexpr(e, v);
  }
  return v;
}

char *ltype_name(int t) {
  switch (t) {
  case LVAL_FUN:
    return "Function";
  case LVAL_NUM:
    return "Number";
  case LVAL_ERR:
    return "Error";
  case LVAL_SYM:
    return "Symbol";
  case LVAL_SEXPR:
    return "S-Expression";
  case LVAL_QEXPR:
    return "Q-Expression";
  default:
    return "Unknown";
  }
}

// Macroses
#define LASSERT(args, cond, fmt, ...)                                          \
  if (!(cond)) {                                                               \
    lval *err = lval_err(fmt, ##__VA_ARGS__);                                  \
    lval_del(args);                                                            \
    return err;                                                                \
  }

#define LASSERT_TYPE(func, args, index, expect)                                \
  LASSERT(args, args->cell[index]->type == expect,                             \
          "Function '%s' passed incorrect type for argunment %i. Got: %s, "    \
          "Expected: %s!",                                                     \
          func, index, ltype_name(args->cell[index]->type),                    \
          ltype_name(expect));

#define LASSERT_NUM(func, args, num)                                           \
  LASSERT(args, args->count == num,                                            \
          "Function '%s', passed incorrect number of arguments. Got: %i, "     \
          "Expected: %i!",                                                     \
          func, args->count, num);

#define LASSERT_NOT_EMPTY(func, args, index)                                   \
  LASSERT(args, args->cell[index]->count != 0,                                 \
          "Function '%s' passed {} for argument %i!" func, index);

lval *builtin_op(lenv *e, lval *a, char *op) {
  // Ensure all args are numbers
  for (int i = 0; i < a->count; i++) {
    if (a->cell[i]->type != LVAL_NUM) {
      lval_del(a);
      LASSERT_TYPE(op, a, i, LVAL_NUM);
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

lval *builtin_head(lenv *e, lval *a) {
  // Check Error Conditions
  LASSERT_NUM("head", a, 1);
  LASSERT_TYPE("head", a, 0, LVAL_QEXPR);
  LASSERT_NOT_EMPTY("head", a, 0);

  // Otherwise take first arg
  lval *v = lval_take(a, 0);

  // Delete all elems that are not head and return
  while (v->count > 1) {
    lval_del(lval_pop(v, 1));
  }
  return v;
}

lval *builtin_tail(lenv *e, lval *a) {
  // Check Error Conditions
  LASSERT_NUM("tail", a, 1);
  LASSERT_TYPE("tail", a, 0, LVAL_QEXPR);
  LASSERT_NOT_EMPTY("tail", a, 0);

  // Otherwise take first arg
  lval *v = lval_take(a, 0);

  // Delete firs elem and return
  lval_del(lval_pop(v, 0));
  return v;
}

lval *builtin_eval(lenv *e, lval *a) {
  LASSERT_NUM("eval", a, 1);
  LASSERT_TYPE("eval", a, 0, LVAL_QEXPR);

  lval *x = lval_take(a, 0);
  x->type = LVAL_SEXPR;
  return lval_eval(e, x);
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

lval *builtin_join(lenv *e, lval *a) {
  for (int i = 0; i < a->count; i++) {
    LASSERT_TYPE("join", a, i, LVAL_QEXPR);
  }

  lval *x = lval_pop(a, 0);
  while (a->count) {
    x = lval_join(x, lval_pop(a, 0));
  }

  lval_del(a);
  return x;
}

lval *builtin_list(lenv *e, lval *a) {
  a->type = LVAL_QEXPR;
  return a;
}

lval *builtin_cons(lenv *e, lval *a) {
  LASSERT_NUM("cons", a, 2);
  LASSERT_TYPE("cons", a, 1, LVAL_QEXPR);

  // Create list of the entire expr
  lval *x = builtin_list(e, a);
  // Pop the qexpr
  lval *y = lval_pop(a, 1);
  // Append cell[0] to the qexpr
  lval *list = lval_join(x, y);

  return list;
}

lval *builtin_len(lenv *e, lval *a) {
  LASSERT_NUM("len", a, 1);
  LASSERT_TYPE("len", a, 0, LVAL_QEXPR);

  // Just return cell[0] count
  return lval_num(a->cell[0]->count);
}

lval *builtin_init(lenv *e, lval *a) {
  // Check Error Conditions
  LASSERT_NUM("init", a, 1);
  LASSERT_TYPE("init", a, 0, LVAL_QEXPR);
  LASSERT_NOT_EMPTY("init", a, 0);

  // Take first argument
  lval *v = lval_take(a, 0);

  // Delete all elems that are not init and return
  while (v->count > 1) {
    lval_del(lval_pop(v, 0));
  }

  return v;
}

lval *lval_eval_sexpr(lenv *e, lval *v) {
  // Evaluate children
  for (int i = 0; i < v->count; i++) {
    v->cell[i] = lval_eval(e, v->cell[i]);
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

  // Ensure first element is a function after evaluation
  lval *f = lval_pop(v, 0);
  if (f->type != LVAL_FUN) {
    lval_del(v);
    lval_del(f);
    return lval_err("First elements is not a function!");
  }

  // If so call function to get result
  lval *result = f->fun(e, v);
  lval_del(f);
  return result;
}

lval *builtin_add(lenv *e, lval *a) { return builtin_op(e, a, "+"); }
lval *builtin_sub(lenv *e, lval *a) { return builtin_op(e, a, "-"); }
lval *builtin_mul(lenv *e, lval *a) { return builtin_op(e, a, "*"); }
lval *builtin_div(lenv *e, lval *a) { return builtin_op(e, a, "/"); }
lval *builtin_rem(lenv *e, lval *a) { return builtin_op(e, a, "rem"); }
lval *builtin_pow(lenv *e, lval *a) { return builtin_op(e, a, "pow"); }
lval *builtin_min(lenv *e, lval *a) { return builtin_op(e, a, "min"); }
lval *builtin_max(lenv *e, lval *a) { return builtin_op(e, a, "max"); }

// Add builitins functions
void lenv_add_builtin(lenv *e, char *name, lbuiltin func) {
  lval *k = lval_sym(name);
  lval *v = lval_fun(func);
  lenv_put(e, k, v);
  lval_del(k);
  lval_del(v);
}

lval *builtin_def(lenv *e, lval *a) {
  LASSERT_TYPE("def", a, 0, LVAL_QEXPR);

  // First argument is symbol list
  lval *syms = a->cell[0];

  // Ensure all elements of first list are symbol
  for (int i = 0; i < syms->count; i++) {
    LASSERT(a, (syms->cell[i]->type == LVAL_SYM),
            "Function 'def' cannot define non-symbol. "
            "Got %s, Expected %s.",
            ltype_name(syms->cell[i]->type), ltype_name(LVAL_SYM));
  }

  // Check correct number of symbols and values
  LASSERT(a, syms->count == a->count - 1,
          "Function 'def' cannot define incorrect number of values to symbols! "
          "Got: %i, Expected: %i",
          syms->count, a->count - 1);

  // Assign copies of values to symbols
  for (int i = 0; i < syms->count; i++) {
    lenv_put(e, syms->cell[i], a->cell[i + 1]);
  }

  lval_del(a);
  return lval_sexpr();
}

void lenv_add_builtins(lenv *e) {
  // List Functions
  lenv_add_builtin(e, "list", builtin_list);
  lenv_add_builtin(e, "head", builtin_head);
  lenv_add_builtin(e, "tail", builtin_tail);
  lenv_add_builtin(e, "eval", builtin_eval);
  lenv_add_builtin(e, "join", builtin_join);
  lenv_add_builtin(e, "cons", builtin_cons);
  lenv_add_builtin(e, "init", builtin_init);
  lenv_add_builtin(e, "len", builtin_len);

  // Mathematical Functions
  lenv_add_builtin(e, "+", builtin_add);
  lenv_add_builtin(e, "add", builtin_add);
  lenv_add_builtin(e, "-", builtin_sub);
  lenv_add_builtin(e, "sub", builtin_sub);
  lenv_add_builtin(e, "*", builtin_mul);
  lenv_add_builtin(e, "mul", builtin_mul);
  lenv_add_builtin(e, "/", builtin_div);
  lenv_add_builtin(e, "div", builtin_div);
  lenv_add_builtin(e, "rem", builtin_rem);
  lenv_add_builtin(e, "pow", builtin_pow);
  lenv_add_builtin(e, "min", builtin_min);
  lenv_add_builtin(e, "max", builtin_max);

  // Variable Functions
  lenv_add_builtin(e, "def", builtin_def);
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
      symbol   : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ ;  \
      sexpr    : '(' <expr>* ')' ; \
      qexpr    : '{' <expr>* '}' ; \
      expr     : <number> | <symbol> | <sexpr> | <qexpr> ;  \
      cumunisp    : /^/ <expr>* /$/ ;             \
    ",
            Number, Symbol, Sexpr, Qexpr, Expr, Cumunisp);

  /* symbol   : '+' | '-' | '*' | '/' | '%' | '^'     \ */
  /*          | \"add\" | \"sub\" | \"mul\" | \"div\" | \"rem\" | \"pow\" \
   */
  /*          | \"max\" | \"min\"   \ */
  /*          | \"list\"| \"head\"| \"tail\"| \"join\"| \"eval\"| \"cons\" |
   * \"len\" | \"init\" ;  \ */

  // Print Version and Exit Information
  puts("Cumunisp Version 0.0.0.0.1");
  puts("Press Ctrl+c to Exit\n");

  lenv *e = lenv_new();
  lenv_add_builtins(e);

  while (1) {
    // Output prompt
    char *input = readline("cumunisp> ");
    // Add input to history
    add_history(input);

    // Attempt to Parse the user Input
    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Cumunisp, &r)) {

      lval *result = lval_eval(e, lval_read(r.output));
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

  lenv_del(e);
  // Undefine and delete Parsers
  mpc_cleanup(4, Number, Symbol, Sexpr, Qexpr, Expr, Cumunisp);
  return 0;
}
