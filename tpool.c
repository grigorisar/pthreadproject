#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#include "tpool.h"

static const size_t num_threads = 4;
static const size_t num_items   = 100;




void worker(void *arg) {
    int *val = arg;
    int  old = *val;
    int msec = 0, trigger = 10; /* 10ms */
    clock_t before = clock();

    // pthread_mutex_lock(&(tm->work_mutex));
    for (int i = 0; i < 100000; ++i)
    {
        old += old * 73425 * 36428 * 73254 * 6342;
    }

    *val += 1000;
    printf("tid=%p, old=%d, val=%d\n", pthread_self(), old, *val);
    // printf("Time taken %d seconds %d milliseconds \n", msec/1000, msec%1000);

    // if (*val%2)
        // usleep(100000);

    // pthread_mutex_unlock(&(tm->work_mutex));

}

static void *helper(void *context)
{
    return (thread_func_t)context;
}

int main(int argc, char **argv)
{
    tpool_t *tm;
    int     *vals;
    size_t   i;
    char     c;


    tm   = tpool_create(num_threads);
    vals = calloc(num_items, sizeof(*vals));

    char * buffer = 0;
    long length;
    FILE * f = fopen ("test.txt", "rb");    //find the file with the instructions

    if (f)
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
    }

   

    if (buffer) {
        int init_size = strlen(buffer);
        // printf("%d\n", init_size);

        char *a1;
        char *a2;
        char *a3;

        int msec       = 0, trigger = 10; /* 10ms */
        clock_t before = clock();

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

        if ( strcmp(a1, "worker") == 0 ) tpool_add_work(tm, worker, a2);
        // printf("Added work\n");


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

            if ( strcmp(a1, "worker") == 0 ) tpool_add_work(tm, worker, a2);
            // printf("Added work\n");



            // printf(" \t %s \t %s \t %s\n", a1, a2, a3);

           
        } 

    }

    while (1) {
        if (getchar()) {

            free(vals);
            tpool_destroy(tm);    
            free(buffer);
            return 0;

        }
    }
}
