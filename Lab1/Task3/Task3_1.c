#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int my_value = 42;

int main() {
    pid_t childpid;
    int status = 0;

    int pipefd[2];

    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    childpid = fork();
    usleep(150000);
    if (childpid == -1) {
        perror("Cannot proceed. fork() error");
        return 1;
    }
    else if (childpid == 0) {//child
        char sentence[50];
        my_value = 18951;
        fprintf(stderr, "Child process ID : %d, Value of my_value : %d\n", getpid(), my_value);
        close(pipefd[1]);

        usleep(500 * 1000);
        //read pipe
        read(pipefd[0], sentence, sizeof(sentence));
        fprintf(stderr, "The message from parent : %s \n", sentence);
        close(pipefd[0]);
        return 0;
    }
    else {//parent
        char sentence[50];
        sprintf(sentence, "Hi, I am your parent. My PID=%d and my_value=%d", getpid(), my_value);
        fprintf(stderr, "Parent process ID : %d, Value of my_value : %d\n", getpid(), my_value);
        fprintf(stderr, "Child with process ID %d was created by  %d \n", childpid, getpid());
        close(pipefd[0]);          /* Close unused read end */
        //write to pipe
        write(pipefd[1], sentence, strlen(sentence) + 1);
        wait(&status);
        fprintf(stderr, "Child with process ID %d has terminated. Value of my_value : %d\n", childpid, my_value);
        fprintf(stderr, "Parent process : %d, terminating now.\n", getpid());
        close(pipefd[1]);
        return 0;
    }
    return 0;
}