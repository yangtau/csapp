#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void sigint_handler(int sig) {
    printf("pid: %d, ppid: %d\n   signal: %d", getpid(), getppid(), sig);
}

int main() {
    signal(SIGINT, sigint_handler);
    pid_t pid;
    if ((pid = fork()) < 0) {
        printf("main: fork error.\n");
        exit(-1);
    }
    if (pid == 0) {
        char *argv[] = {"/bin/sleep", "30", NULL}; 
        extern char **environ;
        if (execvp("/bin/sleep", argv)<0) {
            printf("child: execvp error\n");
        }
        exit(0);
    }
    for(;;);
    return 0;
}


