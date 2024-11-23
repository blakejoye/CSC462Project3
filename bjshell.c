#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MAX_ARGS 256

int main(int argc, char *argv[]) {
    char prompt[20] = "bjshell$ "; // Default prompt
    char input[1024];
    char *args[MAX_ARGS];
    int background;
    int pid;

    if (argc > 1) {
        if (strcmp(argv[1], "-") == 0) {
            prompt[0] = '\0';  // No prompt if argument is "-"
        } else {
            snprintf(prompt, sizeof(prompt), "%s ", argv[1]);  // Custom prompt based on the argument
        }
    }

    while (1) {
        background = 0;
        if (prompt[0]) {
            printf("%s", prompt);
            fflush(stdout); // Ensure the prompt is displayed immediately
        }

        if (!fgets(input, sizeof(input), stdin)) {
            if (prompt[0]) printf("\n"); // Print a newline on EOF if prompt is not disabled
            break; // Exit loop on EOF
        }

        if (input[strlen(input) - 1] == '\n') input[strlen(input) - 1] = '\0'; // Remove trailing newline

        // Handle empty input gracefully
        if (strlen(input) == 0) continue;

        // Parse input into arguments
        int count = 0;
        char *token = strtok(input, " ");
        while (token != NULL) {
            if (strcmp(token, "&") == 0 && !token[1]) {
                background = 1;
                break;
            }
            args[count++] = token;
            token = strtok(NULL, " ");
        }
        args[count] = NULL;

        if (args[0]) {
            if (strcmp(args[0], "exit") == 0) break; // Exit the shell on "exit" command

            pid = fork();
            if (pid == 0) {
                // Child process
                execvp(args[0], args);
                perror(args[0]); // Exec never returns unless there is an error
                _exit(EXIT_FAILURE);
            } else if (pid > 0) {
                // Parent process
                if (!background) {
                    int status;
                    waitpid(pid, &status, 0); // Wait for the child if not a background process
                } else {
                    printf("[Started in background] PID: %d\n", pid);
                }
            } else {
                perror("fork");
            }
        }
    }
    return 0;
}
