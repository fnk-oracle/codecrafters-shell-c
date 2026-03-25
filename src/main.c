#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include<unistd.h>

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
      printf("%s: command not found\n", command);
    }

    
  }
  
  
  return 0;
}
