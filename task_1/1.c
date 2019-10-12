#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
int main() {
    char* command = malloc(2097152);
    for(;;) {
        fgets(command, 2097151, stdin);
        int child_pid = fork();
        if(child_pid) {
            int return_value;
            waitpid(child_pid, &return_value, 0);
            printf("Exit status: %d\n", WEXITSTATUS(return_value));
            continue;
        }
        char** tokens = malloc(1005000);
        tokens[0] = strtok(command, " \n");
        for(int i = 1; tokens[i] = strtok(NULL, " \n"); i++);
        execvp(tokens[0], tokens);
        printf("Incorrect command!\n");
        exit(1);
    }
}