#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>

int main(int argc, char *argv[]) {
  // Flush after every printf
  setbuf(stdout, NULL);

  // TODO: Uncomment the code below to pass the first stage
  while (1){

    printf("$ ");

    char command[1024];
    if(fgets(command, 1024, stdin) == NULL){
      break;
    }

    command[strcspn(command, "\n")] = '\0';

    if(strcmp(command, "exit") == 0){
      break;
    }

    else if(strncmp(command, "echo ", 5) == 0){
      printf("%s\n", command + 5);
    }

    else if(strncmp(command, "type ", 5) == 0){
      char *arg = command +5;

      //1. Checking Builtins first
      if(strcmp(arg, "echo") == 0 || strcmp(arg, "exit") == 0 || strcmp(arg, "type") == 0){
        printf("%s is a shell builtin\n", arg);
      } 
      else{
        //2. Check PATH for executables
        char *path_env = getenv("PATH");
        char *path_copy = strdup(path_env);
        char *dir = strtok(path_copy, ":");
        int found = 0;

        while(dir != NULL){
          char full_path[1024];
          snprintf(full_path, sizeof(full_path), "%s/%s", dir, arg);

          if (access(full_path, X_OK) == 0){
            printf("%s is %s\n", arg, full_path);
            found = 1;
            break;
          }
          dir = strtok(NULL, ":");
        }

        if(!found){
          printf("%s: not found\n", arg);
        }

       
      }
    }


    else{
      // 1. Prepare arguments for execvp
      char *args[64];
      int i = 0;
      char *token = strtok(command, " ");

      while(token != NULL){
        args[i++] = token;
        token = strtok(NULL, " ");
      }

      args[i] = NULL; //Must be NULL terminated

      //2. Try to runt the program
      pid_t pid = fork();
      if(pid == 0){
        //In the CHILD process
          if(execvp(args[0], args) == -1){
            //If it fails to find/run the program, THEN print the error
            printf("%s: command not found\n", args[0]);
            exit(1);
          }

      }
      else if (pid > 0){
        //In the PARENT process
        wait(NULL);
      }
      else{
        perror("fork");
      }
    }

    
  }
  
  
  return 0;
}
