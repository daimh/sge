#include <time.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "drmaa.h"

#define CELL "default"
#define WD "/tmp"
#define CMD "/tmp/sleeper.sh"
#define CATEGORY "test"

int handle_code(int code, char *msg, int r, int t);
void *run(void *arg);

int main(int argc, char **argv) {
    int ret = DRMAA_ERRNO_SUCCESS;
    char error[DRMAA_ERROR_STRING_BUFFER + 1];
    
    int runs = 20;
    int threads = 50;
    int run_count = 0;
    int thread_count = 0;
    pthread_t *ids = NULL;
    
    ids = (pthread_t *)malloc(sizeof (pthread_t) * threads);
    
    ret = drmaa_init(CELL, error, DRMAA_ERROR_STRING_BUFFER);
    if (handle_code(ret, error, -1, 0) == 1) {
        exit(1);
    }
    
    for (run_count = 0; run_count < runs; run_count++) {
        printf("-1 0 STARTING RUN %d %ld\n", run_count, time(NULL));
        
        for (thread_count = 0; thread_count < threads; thread_count++) {
            int *arg = (int *)malloc(sizeof (int) * 2);
            
            arg[0] = run_count;
            arg[1] = thread_count;
            
            if (pthread_create(&ids[thread_count], NULL, run, arg) != 0) {
                printf("%d %d EXCEPTION: Couldn't create thread %ld\n", run_count,
                thread_count, time(NULL));
            }
        }
        
        for (thread_count = 0; thread_count < threads; thread_count++) {
            pthread_join(ids[thread_count], NULL);
        }
        
        printf("-1 0 ENDING RUN %d %ld\n", run_count, time(NULL));
    }
    
    ret = drmaa_exit(error, DRMAA_ERROR_STRING_BUFFER);
    handle_code(ret, error, -1, 0);
    
    return 0;
}

int handle_code(int code, char *msg, int r, int t) {
    if (code != DRMAA_ERRNO_SUCCESS) {
        printf("%d %d EXCEPTION: %s %ld\n", r, t, msg, time(NULL));
        return 1;
    }
    else {
        return 0;
    }
}

void *run(void *arg) {
    int ret = DRMAA_ERRNO_SUCCESS;
    char error[DRMAA_ERROR_STRING_BUFFER + 1];
    
    drmaa_job_template_t *jt = NULL;
    int run = ((int *)arg)[0];
    int thread = ((int *)arg)[1];
    char jobid[DRMAA_JOBNAME_BUFFER + 1];
    int queued = 1;
    int running = 0;
    int status = -1;
    
    sge_free(&arg);
    
    ret = drmaa_allocate_job_template(&jt, error, DRMAA_ERROR_STRING_BUFFER);
    if (handle_code(ret, error, run, thread) == 1) {
        return NULL;
    }
    
    ret = drmaa_set_attribute(jt, DRMAA_REMOTE_COMMAND, CMD, error,
    DRMAA_ERROR_STRING_BUFFER);
    if (handle_code(ret, error, run, thread) == 1) {
        return NULL;
    }
    
    ret = drmaa_set_attribute(jt, DRMAA_WD, WD, error,
    DRMAA_ERROR_STRING_BUFFER);
    if (handle_code(ret, error, run, thread) == 1) {
        return NULL;
    }
    
    ret = drmaa_set_attribute(jt, DRMAA_JOB_CATEGORY, CATEGORY, error,
    DRMAA_ERROR_STRING_BUFFER);
    if (handle_code(ret, error, run, thread) == 1) {
        return NULL;
    }
    
    printf("%d %d SETUP complete %ld\n", run, thread, time(NULL));
    
    ret = drmaa_run_job(jobid, DRMAA_JOBNAME_BUFFER, jt, error,
    DRMAA_ERROR_STRING_BUFFER);
    if (handle_code(ret, error, run, thread) == 1) {
        return NULL;
    }
    
    printf("%d %d SUBMITTED jobid: %s %ld\n", run, thread, jobid, time(NULL));
    
    ret = drmaa_delete_job_template(jt, error, DRMAA_ERROR_STRING_BUFFER);
    handle_code(ret, error, run, thread);
    
    while (queued) {
        ret = drmaa_wait(jobid, NULL, 0, NULL, 2, NULL, error,
        DRMAA_ERROR_STRING_BUFFER);
        
        if (ret != DRMAA_ERRNO_EXIT_TIMEOUT) {
            if (handle_code(ret, error, run, thread) == 1) {
                return NULL;
            }
        }
        else {
            printf ("%d %d TIMEOUT jobid: %s %ld\n", run, thread, jobid, time (NULL));
        }
        
        ret = drmaa_job_ps(jobid, &status, error, DRMAA_ERROR_STRING_BUFFER);
        if (handle_code(ret, error, run, thread) == 1) {
            return NULL;
        }
        
        queued = (status == DRMAA_PS_QUEUED_ACTIVE) ||
        (status == DRMAA_PS_SYSTEM_ON_HOLD) ||
        (status == DRMAA_PS_SYSTEM_ON_HOLD) ||
        (status == DRMAA_PS_USER_ON_HOLD) ||
        (status == DRMAA_PS_USER_SYSTEM_ON_HOLD);
    }
    
    printf("%d %d RUNNING jobid: %s %ld\n", run, thread, jobid, time(NULL));
    
    running = 1;
    
    while (running == 1) {
        ret = drmaa_wait(jobid, NULL, 0, NULL, 60, NULL, error,
                         DRMAA_ERROR_STRING_BUFFER);

        if (ret != DRMAA_ERRNO_EXIT_TIMEOUT) {
            if (handle_code(ret, error, run, thread) == 1) {
                return NULL;
            }
            
            running = 0;
            
            printf("%d %d FINISHED jobid: %s %ld\n", run, thread, jobid,
            time(NULL));
        }
        else {
            printf ("%d %d TIMEOUT jobid: %s %ld\n", run, thread, jobid, time (NULL));

            ret = drmaa_job_ps(jobid, &status, error, DRMAA_ERROR_STRING_BUFFER);
            
            if (handle_code(ret, error, run, thread) == 1) {
                return NULL;
            }
            
            if (status != DRMAA_PS_RUNNING) {
                running = 0;
                
                printf("%d %d HUNG jobid: %s %ld\n", run, thread, jobid, time(NULL));
            }
        }
    }
    
    return NULL;
}
