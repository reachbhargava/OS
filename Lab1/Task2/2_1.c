#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int val = 42;

/**
 * function to be called by thread
 */
void *changeValue() {
    val = 18951;
    fprintf(stderr, "Child %lu val: %d\n", (unsigned long) pthread_self(), val);
    usleep(1000 * 500);
    return NULL;
}


int main() {
    pthread_t thread;

    /* create a second thread which executes changeValue() */
    if (pthread_create(&thread, NULL, changeValue, &val)) {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }


    usleep(1000 * 150);

    //not clear in exercise where it belongs, would be more sensible before usleep
    fprintf(stderr, "Parent %lu has val: %d\n", (unsigned long) pthread_self(), val);

    /* wait for the second thread to finish */
    if (pthread_join(thread, NULL)) {
        fprintf(stderr, "Error joining thread\n");
        return 2;
    }

    fprintf(stderr, "Parent %d has val: %d after join\n", (int) pthread_self(), val);

    return 0;
}
