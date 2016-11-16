#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int val = 42;

int main() {
    int pid, status;
    pid = fork();
    usleep(1000 * 150);
    if (pid == 0) {//child
        val = 18951;
        fprintf(stderr, "Child pid:%d: val:%d\n", getpid(), val);
        usleep(1000 * 500);
    }
    else if (pid > 0) {//parent
        fprintf(stderr, "ParentExecuting. Child has been created %d\n", pid);
        fprintf(stderr, "Parent pid:%d: val:%d\n", getpid(), val);
        wait(&status);
        fprintf(stderr, "ParentExecuting. Child finished execution. value=:%d\n", val);
    }
    return 0;
}
