#include <stdio.h>
#include <stdlib.h>
#include "drmaa.h"

#define WD "/tmp"

int handle_code(int code, char *msg);

int main(int argc, char **argv) {
    int ret = DRMAA_ERRNO_SUCCESS;
    char error[DRMAA_ERROR_STRING_BUFFER + 1];
    drmaa_job_template_t *jt = NULL;
    char jobid[DRMAA_JOBNAME_BUFFER + 1];

    if (argc != 2) {
       printf("Usage: %s path_to_script\n", argv[0]);
       exit(1);
    }
    
    ret = drmaa_init("", error, DRMAA_ERROR_STRING_BUFFER);
    if (handle_code(ret, error) == 1) {
        exit(1);
    }
    
    ret = drmaa_allocate_job_template(&jt, error, DRMAA_ERROR_STRING_BUFFER);
    if (handle_code(ret, error) == 1) {
        return 1;
    }
    
    ret = drmaa_set_attribute(jt, DRMAA_REMOTE_COMMAND, argv[1], error,
    DRMAA_ERROR_STRING_BUFFER);
    if (handle_code(ret, error) == 1) {
        return 1;
    }
    
    ret = drmaa_set_attribute(jt, DRMAA_WD, WD, error,
    DRMAA_ERROR_STRING_BUFFER);
    if (handle_code(ret, error) == 1) {
        return 1;
    }
    
    ret = drmaa_set_attribute(jt, DRMAA_NATIVE_SPECIFICATION, "-b n -cwd", error,
    DRMAA_ERROR_STRING_BUFFER);
    if (handle_code(ret, error) == 1) {
        return 1;
    }
    
    ret = drmaa_run_job(jobid, DRMAA_JOBNAME_BUFFER, jt, error,
    DRMAA_ERROR_STRING_BUFFER);
    if (handle_code(ret, error) == 1) {
        return 1;
    }
    
    ret = drmaa_exit(error, DRMAA_ERROR_STRING_BUFFER);
    handle_code(ret, error);
    
    printf ("OK\n");
    
    return 0;
}

int handle_code(int code, char *msg) {
    if (code != DRMAA_ERRNO_SUCCESS) {
        printf("EXCEPTION: %s\n", msg);
        return 1;
    }
    else {
        return 0;
    }
}
