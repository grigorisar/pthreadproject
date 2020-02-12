#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#include "tpool.h"

static const size_t num_threads = 4;
static const size_t num_items   = 100;


void printor(void *arg) {
    // int *val   =  arg;
    int  value = atoi(arg);
    // printf("Printor Value :%d\n",value );
    
    for (int i = 0; i < 100; ++i) {
        value++;
    }
    // printf("%d\n", value);
}

void worker(void *arg) {
    int value = atoi(arg);
    int *val  = arg;
    int  old  = *val;
    // int msec  = 0, trigger = 10; /* 10ms */
    // printf("Worker Value :%d\n",old );


    // pthread_mutex_lock(&(tm->work_mutex));
    for (int i = 0; i < 100000; ++i)
    {
        old += value * 73425 * 36428 * 73254 * 6342;
    }

    *val += 1000;

    // printf("tid=%p, old=%d, val=%d\n", pthread_self(), old, *val);
    // printf("Time taken %d seconds %d milliseconds \n", msec/1000, msec%1000);

    // if (*val%2)
        // usleep(100000);

    // pthread_mutex_unlock(&(tm->work_mutex));

}

void factorial(void *arg){
    int n = atoi(arg)+1000;
    int i;
    unsigned long long fact = 1;
    // shows error if the user enters a negative integer
    if (n < 0)
        printf("Error! Factorial of a negative number doesn't exist.");
    else {
        for (i = 1; i <= n; ++i) {
            fact *= i;
        }
        // printf("Factorial of %d ", n);
    }
}

void pie(void *arg) {
    int digits = atoi(arg);
    int r[digits + 1];
    int i, k;
    int b, d;
    int c = 0;

    for (i = 0; i < digits; i++) {
        r[i] = 2000;
    }
    // printf("Pie digits: ");
    for (k = digits; k > 0; k -= 14) {
        d = 0;

        i = k;
        for (;;) {
            d += r[i] * 10000;
            b = 2 * i - 1;

            r[i] = d % b;
            d /= b;
            i--;
            if (i == 0) break;
            d *= i;
        }
        // printf("%.4d", c + d / 10000);
        c = d % 10000;
    }
    // printf("\n");
}


static void *helper(void *context)
{
    return (thread_func_t)context;
}

int main(int argc, char **argv)
{
    struct timespec ts1, ts2;
    tpool_t *tm;
    int     *vals;
    size_t   i;
    char     c;

    // printf("%d\n", argc);

    if (argc ==  1) {
        printf("Enter file name as argument\n");
        return -1;
    }


    tm   = tpool_create(num_threads);
    


    // printf("\nActive Threads :%d\n",tm->thread_cnt);
    // printf("Scheduling:ROUND ROBIN \tWorks :9\nLIGHT WORKLOAD\n");


    char * buffer = 0;
    long length;
    FILE * f = fopen (argv[1], "r");    //find the file with the instructions
    

    if (f )
    {
      fseek (f, 0, SEEK_END);
      length = ftell (f);
      fseek (f, 0, SEEK_SET);
      buffer = malloc (length + 1);
      if (buffer)
      {
        fread (buffer, 1, length, f);
      }
      fclose (f);
      buffer[length] = '\0';
    } else {
        printf("File not found in directory\n");
        return -1;
    }

   
    clock_gettime(CLOCK_REALTIME, &ts1);

    if (buffer != NULL) {
        int init_size = strlen(buffer);
        // printf("%d\n", init_size);

        char *a1;
        char *a2;
        char *a3;

        int msec       = 0, trigger = 10; /* 10ms */
        // clock_t before = clock();

        // printf("%s\n", buffer);

        const char s[10] = ",";

        char *function_name  = strtok(buffer, s);
        a1=function_name;

        char *function_arg   = strtok(NULL, s);
        a2=function_arg;

        char *timestamp      = strtok(NULL, s);
        a3=timestamp;

        int timer = atoi(timestamp);
        // printf("Sleeping for %d\n", timer);

        

        sleep(timer);                               //timer is an int that subtracts the current timestamp for the previous one
                                                    //and then we use sleep(int)
        int value2 = atoi(a2);
        if ( strcmp(a1, "worker") == 0 ) {
            tpool_add_work(tm, worker,a2);
            // printf("Added work worker A2: %d\n",value2);
        }else if (strcmp(a1, "printor") == 0){
            tpool_add_work(tm, printor, a2);
            // printf("Added work printor A2:%d\n", value2);
        }else if (strcmp(a1, "pie") == 0){
            tpool_add_work(tm, pie, a2);
            // printf("Added work pie A2:%d\n", value2);
        }else if (strcmp(a1, "factorial") == 0){
            printf("HEAVY WORKLOAD\n");
            vals = calloc(atoi(a2), sizeof(*vals));
            for (int i = 0; i < 100000; ++i) {
                tpool_add_work(tm, factorial, a2);
            }
            // printf("Added work factorial A2:%d\n", value2);
        }
        // printf("%s %s %s\n", a1, a2, a3);
      
        // Keep printing tokens while one of the 
        // delimiters present in buffer. 
        while ( a1 != NULL ) {

            function_name = strtok(NULL, s);
            a1=function_name;
            if (a1 == NULL ) break;
            function_arg  = strtok(NULL, s);
            a2=function_arg;
            timestamp     = strtok(NULL, s);
            a3=timestamp;

            timer = atoi(timestamp);
            // printf("Sleeping for %d\n", timer);
            sleep(timer);

            if ( strcmp(a1, "worker") == 0 ) {
                tpool_add_work(tm, worker,a2);
                // printf("Added work worker A2: %d\n",value2);
            }else if (strcmp(a1, "printor") == 0){
                tpool_add_work(tm, printor, a2);
                // printf("Added work printor A2:%d\n", value2);
            }else if (strcmp(a1, "pie") == 0){
                tpool_add_work(tm, pie, a2);
                // printf("Added work pie A2:%d\n", value2);
            }else if (strcmp(a1, "factorial") == 0){
                vals = calloc(atoi(a2), sizeof(*vals));
                for (int i = 0; i < 100000; ++i) {
                    tpool_add_work(tm, factorial, a2);
                }
                // printf("Added work factorial A2:%d\n", value2);
                // printf(" \t %s \t %s \t %s\n", a1, a2, a3);       
            }
        }
        tpool_wait(tm);
        // for (int i = 0; i < 100; ++i)
        //  {
        //      printf("%d\n", vals[i] );
        //  } 

        tpool_destroy(tm);    
  
        clock_gettime(CLOCK_REALTIME, &ts2);
        if (ts2.tv_nsec < ts1.tv_nsec) {
            ts2.tv_nsec += 1000000000;
            ts2.tv_sec--;
        }
        printf("%ld.%09ld\n", (long)(ts2.tv_sec - ts1.tv_sec),
        ts2.tv_nsec - ts1.tv_nsec);
    } else {
        return -1;
    }
    
    // while (1) {
    //     if (getchar()) {

    //         free(vals);

    //         tpool_destroy(tm);    
    //         free(buffer);
    //         return 0;

    //     }
    // }
}
