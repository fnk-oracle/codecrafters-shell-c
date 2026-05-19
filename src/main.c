#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <stdbool.h>

int main(int argc, char *argv[]) {
  // Flush after every printf
  setbuf(stdout, NULL);

  while (1) {
    printf("$ ");

    char command[1024];
    if (fgets(command, 1024, stdin) == NULL) {
      break;
    }

    command[strcspn(command, "\n")] = '\0';

    // ---------------------------------------------------------
    // NEW PARSER: Handles single quotes and preserves spaces
    // ---------------------------------------------------------
    char *args[64];
    int args_count = 0;
    char *ptr = command;

    // Skip initial leading spaces
    while (*ptr && isspace((unsigned char)*ptr)) ptr++;

    while (*ptr && args_count < 63) {
      char buffer[1024] = {0};
      int buf_idx = 0;
      bool in_single_quotes = false;

      // Extract a single token
      while (*ptr) {
        if (*ptr == '\'') {
          in_single_quotes = !in_single_quotes; // Toggle state
          ptr++; // Skip the quote character itself
        } else if (!in_single_quotes && isspace((unsigned char)*ptr)) {
          // Break token if we hit a space outside of quotes
          break;
        } else {
          buffer[buf_idx++] = *ptr;
          ptr++;
        }
      }

      // Save token if it contains text or was an empty quoted string ''
      if (buf_idx > 0 || in_single_quotes) {
        args[args_count++] = strdup(buffer);
      }

      // Skip spaces separating the arguments
      while (*ptr && isspace((unsigned char)*ptr)) ptr++;
    }
    args[args_count] = NULL; // Array must be NULL terminated

    // If nothing was entered, continue back to the prompt
    if (args_count == 0) {
      continue;
    }
    // ---------------------------------------------------------

    // 1. EXIT BUILT-IN
    if (strcmp(args[0], "exit") == 0) {
      for (int i = 0; i < args_count; i++) free(args[i]);
      break;
    }

    // 2. ECHO BUILT-IN
    else if (strcmp(args[0], "echo") == 0) {
      for (int i = 1; i < args_count; i++) {
        printf("%s", args[i]);
        if (i < args_count - 1) {
          printf(" ");
        }
      }
      printf("\n");
    }

    // 3. TYPE BUILT-IN
    else if (strcmp(args[0], "type") == 0) {
      if (args_count < 2) {
        printf("type: missing operand\n");
      } else {
        char *arg = args[1];

        // Checking Builtins first
        if (strcmp(arg, "echo") == 0 || strcmp(arg, "exit") == 0 || strcmp(arg, "type") == 0 || strcmp(arg, "pwd") == 0) {
          printf("%s is a shell builtin\n", arg);
        } 
        else {
          // Check PATH for executables
          char *path_env = getenv("PATH");
          int found = 0;

          if (path_env != NULL) {
            char *path_copy = strdup(path_env);
            char *dir = strtok(path_copy, ":");

            while (dir != NULL) {
              char full_path[1024];
              snprintf(full_path, sizeof(full_path), "%s/%s", dir, arg);

              if (access(full_path, X_OK) == 0) {
                printf("%s is %s\n", arg, full_path);
                found = 1;
                break;
              }
              dir = strtok(NULL, ":");
            }
            free(path_copy);
          }

          if (!found) {
            printf("%s: not found\n", arg);
          }
        }
      }
    }

    // 4. PWD BUILT-IN
    else if (strcmp(args[0], "pwd") == 0) {
      char cwd[1024];
      if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
      } else {
        perror("getcwd() error");
      }
    }

    // 5. EXTERNAL COMMAND EXECUTION
    else {
      pid_t pid = fork();
      if (pid == 0) {
        if (execvp(args[0], args) == -1) {
          printf("%s: command not found\n", args[0]);
          exit(1);
        }
      } 
      else if (pid > 0) {
        wait(NULL);
      } 
      else {
        perror("fork");
      }
    }

    // Clean up memory allocated by strdup
    for (int i = 0; i < args_count; i++) {
      free(args[i]);
    }
  }

  return 0;
}
