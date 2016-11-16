#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/time.h>
#include <math.h>
#include <sys/wait.h>

void produce (char *name, char *message) {
    // printf("%s \n %s \n", name, message);
    FILE *fp;
    fp = fopen("/dev/fifo", "w+");

    struct timeval clock_time;
    gettimeofday(&clock_time, NULL);
    long long unsigned times = (long long unsigned int)clock_time.tv_sec;

    const int n = snprintf(NULL, 0, "%lu", times);
    char timeArray[n+1];
    snprintf(timeArray, n+1, "%lu",times);

    int length = strlen(timeArray) + strlen(message) + 3;

    char* my_buf = malloc(length * sizeof(char));
    strcpy(my_buf, "0");
    strcat(my_buf, ",");
    strcat(my_buf, timeArray);
    strcat(my_buf, ",");
    strcat(my_buf, message);

    int len = strlen(my_buf);
    my_buf[len] = '\0';

printf("[%s][%u][%llu]\t%s\n", name, 0, times, message);

    fprintf(fp, my_buf);
    fclose(fp);
}

void consume (char *name) {
    // printf("%s \n", name);
    FILE *fp;
    char buff[255];
    fp = fopen("/dev/fifo", "r");
    fgets(buff, 255, (FILE *)fp);
    fclose(fp);
}

void print_usage() {
    printf("Usage: producer_user p (if Producer) || c (if Consumer) \n");
}

int main(int argc, char *argv[]) {
    int option = 0;
    int pflag = 0, cflag = 0, rate = 0;
    char *name, *message;

    //Specifying the expected options
    //The two options l and b expect numbers as argument
    while ((option = getopt(argc, argv,"pcn:m:r:")) != -1) {
        switch (option) {
             case 'p' : 
                    if (cflag == 1) {
                        print_usage();
                        exit(1);
                    }
                    pflag = 1;
                    break;
             case 'c' : 
                    if (pflag == 1) {
                        print_usage();
                        exit(1);
                    }
                    cflag = 1;
                    break;
            case 'n' : 
                    if (cflag || pflag) {
                        name = optarg;
                    } else {
                        print_usage();
                        exit(1);
                    }                    
                    break;
             case 'm' : 
                    if (pflag && !cflag) {
                        message = optarg;
                    } else {
                        print_usage();
                        exit(1);
                    }                    
                    break;
             case 'r' : 
                    if (cflag || pflag) {
                        rate = atoi(optarg);
                    } else {
                        print_usage();
                        exit(1);
                    }                    
                    break;
             default: print_usage(); 
                 exit(EXIT_FAILURE);
        }
    }

    if (rate == 0) {
        print_usage(); 
        exit(EXIT_FAILURE);
    }

    double sleepDuration = floor((double)1000/rate);
    while (1) {
        if (pflag) {
            produce(name, message);      
        } else if (cflag) {
            consume(name);
        }
        usleep(sleepDuration * 1000); // usleep takes microseconds
    }

    return 0;
}