#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
typedef struct {
    int ptc[2];//parent to child
    int ctp[2];//child to parent
} dpipe_t;
int main() {
    dpipe_t p;
    pipe(p.ctp);
    pipe(p.ptc);
    int cpid = fork();
    char data, buff;
    if(cpid) {
        close(p.ctp[1]);
        close(p.ptc[0]);
        for(char data = 'a'; data <= 'z'; data++) {
            printf("Parent sent: %c\n", data);
            if(data == 'f')
                printf("(to pay respects)\n");
            write(p.ptc[1], &data, 1);
            read(p.ctp[0], &buff, 1);
            printf("Parent recieved: %c\n", buff);
            sleep(1);
        }
        printf("Parent exiting, child should die shortly\n"); //sigpipe kills
    }
    else {
        close(p.ctp[0]);
        close(p.ptc[1]);
        for(char data = 'e'; data <= 'h'; data++) {
            printf("Child sent: %c\n", data);
            write(p.ctp[1], &data, 1);
            read(p.ptc[0], &buff, 1);
            printf("Child recieved: %c\n", buff);
            sleep(1);
        }
        printf("Child exiting, parent should die shortly\n");
    }
    return 0;
}