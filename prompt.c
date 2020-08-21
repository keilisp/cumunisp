#include <stdio.h>
#include <stdlib.h>

// If we are compiling on Windows compile these functions
#ifdef _WIN32
#include <string.h>

static char *[2048];

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

int main(int argc, char **argv) {
  // Print Version and Exit Information
  puts("Cumunisp Version 0.0.0.0.1");
  puts("Press Ctrl+c to Exit\n");

  while (1) {
    // Output prompt
    char *input = readline("cumunisp> ");

    // Add input to history
    add_history(input);

    // Echo input back to user
    printf("No you're a %s\n", input);

    // Free retrieved input
    free(input);
  }

  return 0;
}
