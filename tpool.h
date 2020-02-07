
#ifndef __TPOOL_H__
#define __TPOOL_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

struct tpool;
typedef struct tpool tpool_t;

typedef void (*thread_func_t)(void *arg);

tpool_t *tpool_create(size_t num);
void tpool_destroy(tpool_t *tm);

bool tpool_add_work(tpool_t *tm, thread_func_t func, void *arg);
void tpool_wait(tpool_t *tm);

//DEFAULT IS MANY TO MANY
#endif /* __TPOOL_H__ */

static const bool manytomany_flag = false;
static const bool manytoone_flag = true;
static const bool onetoone_flag = false;
//ONE TO ONE OBFUSCATED


struct tpool_work {
    thread_func_t      func;
    void              *arg;
    struct tpool_work *next;
};

typedef struct tpool_work tpool_work_t;

struct tpool {
    pthread_attr_t attr;
    tpool_work_t    *work_first;
    tpool_work_t    *work_last;
    pthread_mutex_t  work_mutex;
    pthread_cond_t   work_cond;
    pthread_cond_t   working_cond;
    size_t           working_cnt;
    size_t           thread_cnt;
    bool             stop;
};

static tpool_work_t *tpool_work_create(thread_func_t func, void *arg){
    tpool_work_t *work;

    if (func == NULL)
        return NULL;

    work       = malloc(sizeof(*work));             //EDIT
    work->func = func;
    work->arg  = arg;
    work->next = NULL;
    return work;
}



static void tpool_work_destroy(tpool_work_t *work){
    if (work == NULL)
        return;
    free(work);
}

static tpool_work_t *tpool_work_get(tpool_t *tm){
    tpool_work_t *work;

    if (tm == NULL)
        return NULL;

    work = tm->work_first;
    if (work == NULL)
        return NULL;

    if (work->next == NULL) {                   //if there is no work left set both pointers to null 
        tm->work_first = NULL;
        tm->work_last  = NULL;
    } else {
        tm->work_first = work->next;
    }

    return work;
}

static void *tpool_worker(void *arg)
{
    tpool_t      *tm = arg;
    tpool_work_t *work;

    while (1) {                                    //we keep a certain number of threads alive this way unlees we signal the pool to stop


        // printf("Waiting for work, thread: %p\n", pthread_self());
        pthread_mutex_lock(&(tm->work_mutex));      //all threads without work wait until the first one in that queue gets work


        while (tm->work_first == NULL && !tm->stop)                 //if there is no work and if we have not stopped the pool

            pthread_cond_wait(&(tm->work_cond), &(tm->work_mutex)); //the thread waits for available work

        if (tm->stop)                                               //if the pool stops the thread break from endless loop and reduce the working count
            break;

        work = tpool_work_get(tm);                                  //get the first in the work queue or null if nothing exists

        // while (tm->working_cnt > 0 );

        tm->working_cnt++;
        printf("Working, thread: %p\n", pthread_self());
        
        if (manytomany_flag) pthread_mutex_unlock(&(tm->work_mutex));                    //let the next thread wait for work, this is the key to switch from many-to-many to many-to-one

        if (work != NULL) {
            work->func(work->arg);
            tpool_work_destroy(work);
        }

        if (manytomany_flag) pthread_mutex_lock(&(tm->work_mutex));    //lock thread to change working condition
        
        tm->working_cnt--;
        printf("Finished Working, thread: %p\n", pthread_self());

        if (!tm->stop && tm->working_cnt == 0 && tm->work_first == NULL)
            pthread_cond_signal(&(tm->working_cond));
        pthread_mutex_unlock(&(tm->work_mutex));
    }
    tm->thread_cnt--;
    pthread_cond_signal(&(tm->working_cond));
    pthread_mutex_unlock(&(tm->work_mutex));
    return NULL;
}



bool tpool_add_work(tpool_t *tm, thread_func_t func, void *arg){
    tpool_work_t *work;

    if (tm == NULL)
        return false;

    work = tpool_work_create(func, arg);
    if (work == NULL)
        return false;

    pthread_mutex_lock(&(tm->work_mutex));
    if (tm->work_first == NULL) {
        tm->work_first = work;
        tm->work_last  = tm->work_first;
    } else {
        tm->work_last->next = work;
        tm->work_last       = work;
    }

    pthread_cond_broadcast(&(tm->work_cond));                       //call to all the threads that work is available

    pthread_mutex_unlock(&(tm->work_mutex));                        //let first thread in our queue get the the work that was added

    return true;
}

tpool_t *tpool_create(size_t num){              //Create thread pool
    tpool_t   *tm;

    pthread_t  thread;
    size_t     i;

    if (num == 0)
        num = 2;

    tm             = malloc( sizeof(*tm));  //TODO its scientifically calloc
    tm->thread_cnt = num;

    pthread_mutex_init(&(tm->work_mutex), NULL);            
    pthread_cond_init(&(tm->work_cond), NULL);
    pthread_cond_init(&(tm->working_cond), NULL);

    tm->work_first = NULL;
    tm->work_last  = NULL;

    pthread_attr_setschedpolicy(&(tm->attr), SCHED_RR);           //set schedule police SCHED_FIFO is FIFO, SCHED_RR is for Round Robin
                                                    //When threads executing with the scheduling policy SCHED_FIFO, SCHED_RR, or SCHED_SPORADIC 
                                                    // are waiting on a mutex, they shall acquire the mutex in priority order when the mutex is unlocked.
                                                    // https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_attr_setschedpolicy.html
     if (!onetoone_flag){
        for (i=0; i<num; i++) {
            pthread_create(&thread, &(tm->attr), tpool_worker, tm);
            pthread_detach(thread);
        }
     }else{
        pthread_create(&thread, &(tm->attr), tpool_worker, tm);
        pthread_join(thread,NULL);
     }
 
    return tm;
}

void tpool_add_threads(tpool_t *tm, size_t num){        //ADDITION unstable don't use it

    pthread_t thread;

    for (int i = 0; i < num; ++i) {
        pthread_create(&thread, NULL, tpool_worker, tm);
        pthread_detach(thread);        
    }

}


void tpool_destroy(tpool_t *tm){            //Destroy thread pool
    tpool_work_t *work;
    tpool_work_t *work2;

    if (tm == NULL)
        return;

    pthread_mutex_lock(&(tm->work_mutex));
    work = tm->work_first;
    while (work != NULL) {
        work2 = work->next;
        tpool_work_destroy(work);
        work = work2;
    }
    tm->stop = true;
    pthread_cond_broadcast(&(tm->work_cond));
    pthread_mutex_unlock(&(tm->work_mutex));

    tpool_wait(tm);

    pthread_mutex_destroy(&(tm->work_mutex));
    pthread_cond_destroy(&(tm->work_cond));
    pthread_cond_destroy(&(tm->working_cond));

    free(tm);
}

void tpool_wait(tpool_t *tm)
{
    if (tm == NULL)
        return;

    pthread_mutex_lock(&(tm->work_mutex));
    while (1) {
        if ((!tm->stop && tm->working_cnt != 0) || (tm->stop && tm->thread_cnt != 0)) {
            pthread_cond_wait(&(tm->working_cond), &(tm->work_mutex));
        } else {
            break;
        }
    }
    pthread_mutex_unlock(&(tm->work_mutex));
}
