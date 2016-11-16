#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

//name of the shared memory
#define SM_NAME "/DEEDS_lab1_shm"
//size of the buffers
#define MAX_LEN 1024

int val = 42;

struct region {
    /* Defines structure of shared memory */
    int len;
    char parent[MAX_LEN];
    char child[MAX_LEN];
};

int main() {

    struct region *rptr;

    int fd = shm_open(SM_NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        return -1;
    }
    int err = ftruncate(fd, sizeof(struct region));
    if (err == -1) {
        shm_unlink(SM_NAME);
        return -2;
    }

    int pid, status;


    pid = fork();

    //map memory
    rptr = mmap(NULL, sizeof(struct region),
                PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (rptr == MAP_FAILED) {
        shm_unlink(SM_NAME);
        return -3;
    }

    if (pid != 0) {
        sprintf(rptr->parent, "Parent %d sends to child %d value %d\n", getpid(), pid, val);
    }
    //sleep now
    usleep(1000 * 150);
    if (pid == 0) {
        val = 18951;
        fprintf(stderr, "Child pid:%d: val:%d\n", getpid(), val);

        usleep(1000 * 500);

        //is this wanted??
        //while(rptr->parent[0] == 0);//wait for parent to write

        fprintf(stderr, "Message from parent: '%s'\n", rptr->parent);
        //send back
        sprintf(rptr->child, "Child %d sends to Parent  value %d", getpid(), val);
        //unmap
        munmap(rptr, sizeof(struct region));
    }
    else if (pid > 0) {
        fprintf(stderr, "ParentExecuting. Child has been created %d\n", pid);
        fprintf(stderr, "Parent pid:%d: val:%d\n", getpid(), val);

        //should wait here somehow before I guess but its not clear stated
        fprintf(stderr, "Got Message from Child: '%s'\n", rptr->child);
        wait(&status);
        fprintf(stderr, "ParentExecuting. Child finished execution. value=:%d\n", val);
        munmap(rptr, sizeof(struct region));
        shm_unlink(SM_NAME);
    }
    return 0;
}
