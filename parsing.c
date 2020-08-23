#include "mpc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* #include <stdio.h> */
/* #include <stdlib.h> */

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

int power(long base, long exp) {
  int count, result = 1;

  for (count = 1; count <= exp; count++) {
    result *= base;
  }
  return result;
}

int max(long x, long y) {
  if (x > y) {
    return x;
  } else {
    return y;
  }
}

int min(long x, long y) {
  if (x < y) {
    return x;
  } else {
    return y;
  }
}

// Arithmetic operators
long eval_op(long x, char *op, long y) {
  if (strcmp(op, "+") == 0 || strcmp(op, "add") == 0) {
    return x + y;
  }
  if (strcmp(op, "-") == 0 || strcmp(op, "sub") == 0) {
    return x - y;
  }
  if (strcmp(op, "*") == 0 || strcmp(op, "mul") == 0) {
    return x * y;
  }
  if (strcmp(op, "/") == 0 || strcmp(op, "div") == 0) {
    return x / y;
  }
  if (strcmp(op, "%") == 0 || strcmp(op, "rem") == 0) {
    return x % y;
  }
  if (strcmp(op, "^") == 0 || strcmp(op, "pow") == 0) {
    return power(x, y);
  }
  if (strcmp(op, "max") == 0) {
    return max(x, y);
  }
  if (strcmp(op, "min") == 0) {
    return min(x, y);
  }

  return 0;
}

long eval(mpc_ast_t *t) {

  // If tagged as number return it directly
  printf("T hn: %d \n", t->children_num);
  if (strstr(t->tag, "number")) {
    printf("atoi: %i \n", atoi(t->contents));
    /* long x = atoi(t->contents); */
    return atoi(t->contents);
  }

  // The operator is always second child
  char *op = t->children[1]->contents;
  printf("Op : %s \n", op);

  // Store the third child in 'x'
  long x = eval(t->children[2]);

  // TODO: impl (node | branch | leaces)_number_check for negating num
  // If operator '-' have only one argument, negates it
  if (t->children_num == 4 && strcmp(op, "-") == 0) {
    return x * -1;
  }
  printf("x : %li\n", x);

  // Iterate the remaining children
  int i = 3;

  while (strstr(t->children[i]->tag, "expr")) {
    x = eval_op(x, op, eval(t->children[i]));
    printf("x : %li\n", x);
    i++;
  }

  return x;
}

int main(int argc, char **argv) {
  // Create some Parsers
  mpc_parser_t *Number = mpc_new("number");
  mpc_parser_t *Operator = mpc_new("operator");
  mpc_parser_t *Expr = mpc_new("expr");
  mpc_parser_t *Cumunisp = mpc_new("cumunisp");

  // Define Parsers
  mpca_lang(MPCA_LANG_DEFAULT,
            "                                                     \
      number   : /-?[0-9]+(\\.[0-9]*)?/ ;                             \
      operator : '+' | '-' | '*' | '/' | '%' | '^' | /add/ | /sub/ | /mul/ | /div/ | /rem/ | /pow/ | /max/ | /min/ ;                  \
      expr     : <number> | '(' <operator> <expr>+ ')' ;  \
      cumunisp    : /^/ <operator> <expr>+ /$/ ;             \
    ",
            Number, Operator, Expr, Cumunisp);

  // Polish Notation
  /* expr     : <number> | '(' <operator> <expr>+ ')' ; \ */
  /* cumunisp : /^/ <operator> <expr>+ /$/ ; \ */

  // Normal Notation(broken)
  /* expr     : <number> | <operator> <expr>+ | '(' <expr> <operator> <expr>+
   * ')' ;             \ */
  /* cumunisp : /^/ <expr> <operator> <expr>+ /$/ ; \ */

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

      // LOGS
      // Load AST from output
      /* mpc_ast_t *a = r.output; */
      /* printf("Tag: %s\n", a->tag); */
      /* printf("Contents: %s\n", a->contents); */
      /* printf("Number of children %i\n", a->children_num); */

      // Get first Child
      /* mpc_ast_t *c0 = a->children[0]; */
      /* printf("First Child tag: %s\n", c0->tag); */
      /* printf("First Child contents: %s\n", c0->contents); */
      /* printf("First Child number of Children: %i\n\n", c0->children_num);
       */

      // If successed print the result
      /* mpc_ast_print(r.output); */
      long result = eval(r.output);
      printf("Result: \n");
      printf("%li\n", result);
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
  mpc_cleanup(4, Number, Operator, Expr, Cumunisp);
  return 0;
}
