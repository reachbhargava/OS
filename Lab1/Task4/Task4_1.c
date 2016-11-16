#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>

//message queue name
#define SM_NAME "/DEEDS_lab1_mq"

int main() {
    pid_t childpid;
    int status = 0;
    int my_value = 42;

    //msg queue attributes
    struct mq_attr msgq_attr;
    msgq_attr.mq_flags = 0;
    msgq_attr.mq_maxmsg = 10;
    msgq_attr.mq_msgsize = 50;
    msgq_attr.mq_curmsgs = 0;

    childpid = fork();
    usleep(150000);
    if (childpid == -1) {
        perror("Cannot proceed. fork() error");
        return 1;
    }
    else if (childpid == 0) // child
    {
        // create and open the message queue in parent
        mqd_t mqd = mq_open(SM_NAME, O_RDONLY);
        fprintf(stderr, "Child - mq opened \n");

        if (mqd == (mqd_t) -1) {
            perror("mq_open()");
            exit(EXIT_FAILURE);
        }

        // get queue attributes; returns 0 if OK, -1 on error
        /*if (mq_getattr(mqd, &msgq_attr) == -1) {
            perror("mq_getattr()");
            exit(EXIT_FAILURE);
        }*/

        char *str_buf;

        str_buf = (char *) malloc((size_t) msgq_attr.mq_msgsize);    // remove size_t

        if (str_buf == 0) {
            perror("malloc(): Out of memory");
            exit(EXIT_FAILURE);
        }

        //char str_buf[msgq_attr.mq_msgsize];
        unsigned int msg_prio = 0;
        my_value = 18951;
        fprintf(stderr, "Child process ID : %d, Value of my_value : %d\n", getpid(), my_value);

        // receive message from queue
        // returns length of received message, -1 on error
        if (mq_receive(mqd, str_buf, msgq_attr.mq_msgsize, &msg_prio) == (ssize_t) -1) {
            perror("mq_receive()");
            exit(EXIT_FAILURE);
        }
        fprintf(stderr, "Child - message received \n");

        usleep(500000);
        fprintf(stderr, "The message from parent : %s \n", str_buf);
        free(str_buf);

        // close queue; returns: 0 if OK, -1 on error
        if (mq_close(mqd) == -1) {
            perror("mq_close()");
            exit(EXIT_FAILURE);
        }
        fprintf(stderr, "Child - mq closed \n");

        return 0;
    }
    else                    // parent
    {
        // create and open the message queue in parent
        mqd_t mqd = mq_open(SM_NAME, O_CREAT | O_RDWR, (S_IRWXU | S_IRWXG | S_IRWXO), &msgq_attr);
        fprintf(stderr, "Parent - mq created \n");

        if (mqd == (mqd_t) -1) {
            perror("mq_open()");
            exit(EXIT_FAILURE);
        }

        char str_buf[msgq_attr.mq_msgsize];
        unsigned int msg_prio = 0;
        sprintf(str_buf, "Hi, I am your parent. My PID=%d and my_value=%d", getpid(), my_value);
        fprintf(stderr, "Parent process ID : %d, Value of my_value : %d\n", getpid(), my_value);
        fprintf(stderr, "Child with process ID %d was created by parent with PID %d \n", childpid, getpid());

        // send message to queue; returns 0 if OK, -1 on error
        if (mq_send(mqd, str_buf, strlen(str_buf) + 1, msg_prio) == -1) {
            perror("mq_send()");
            exit(EXIT_FAILURE);
        }
        fprintf(stderr, "Parent - message sent \n");

        wait(&status);
        fprintf(stderr, "Child with process ID %d has terminated. Value of my_value : %d\n", childpid, my_value);
        fprintf(stderr, "Parent process : %d, terminating now.\n", getpid());

        if (mq_close(mqd) == -1) {
            perror("mq_close()");
            exit(EXIT_FAILURE);
        }
        fprintf(stderr, "Parent - mq closed \n");

        // remove queue; returns 0 if OK, -1 on error
        if (mq_unlink(SM_NAME) == -1) {
            perror("mq_unlink()");
            exit(EXIT_FAILURE);
        }
        fprintf(stderr, "Parent - mq removed \n");

        return 0;
    }
    return 0;
}