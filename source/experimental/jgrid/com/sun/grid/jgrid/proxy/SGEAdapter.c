/* 
 * File:   SGEException.c
 * Author: dant
 *
 * Created on May 25, 2003, 10:38 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include "com_sun_grid_proxy_SGEAdapter.h"
#include "sge_gdi.h"

/*
 * 
 */
int main (int argc, char** argv) {    
    return (EXIT_SUCCESS);
}

/*
 * Class:     com_sun_grid_proxy_SGEAdapter
 * Method:    setupGDI
 * Signature: ()Ljava/lang/Object;
 */
JNIEXPORT void JNICALL Java_com_sun_grid_proxy_SGEAdapter_setupGDI (JNIEnv* env, jobject thisObj) {
  /* GENERAL SGE SETUP */
  int setup_ok = sge_gdi_setup("JGrid Compute Proxy");
	
  switch(setup_ok) {
  case AE_OK:
    fprintf(stdout, "Grid Engine GDI Setup: ok\n");
    break;
  case  AE_ALREADY_SETUP:
    fprintf(stderr, "Grid Engine GDI Setup: already setup\n");
    throw_exception(env, "Grid Engine GDI Setup: already setup.");
    break;
  case AE_UNKNOWN_PARAM:
    fprintf(stderr, "Grid Engine GDI Setup: unknown parameter\n");
    throw_exception(env, "Grid Engine GDI Setup: unknown parameter.");
    break;
  case AE_QMASTER_DOWN:
    fprintf(stderr, "Grid Engine GDI Setup: qmaster not alive\n");
    throw_exception(env, "Grid Engine GDI Setup: qmaster not alive.");
    break;
  default:
    fprintf(stderr, "Grid Engine GDI Setup: unknown error\n");
    throw_exception(env, "Grid Engine GDI Setup: unknown error.");
    break;
  }
}

/*
 * Class:     com_sun_grid_proxy_SGEAdapter
 * Method:    submitJob
 * Signature: (Lcom/sun/grid/Job;)Ljava/lang/Object;
 */
JNIEXPORT void JNICALL Java_com_sun_grid_proxy_SGEAdapter_submitJob (JNIEnv* env, jobject thisObj, jstring skel, jstring jobId) {
  char error_header[256];
  lListElem* job = NULL;
	lList* answer = NULL;
	
  /* create list of JB_Type with 1 element */
  jobs = lCreateElemList("JGrid Job", JB_Type, 1);
  /* set job to defaults */
  answer = cull_parse_job_parameter (NULL, &job);
	lAppendElem(jobs, job);
 
  /* submit (add) the job */
  answer = sge_gdi(SGE_JOB_LIST, SGE_GDI_ADD | SGE_GDI_RETURN_NEW_VERSION, &jobs, NULL, NULL);

  // printf("================    SUBMITTED JOB   ================\n");
  // lWriteListTo(jobs, stdout);
  // printf("================ END of JOB LIST ================\n");
  
  sprintf(error_header, "failed to submit job \"%s\" :", name);
  check_answer_status(env, answer, "nativeSubmitJob", error_header);

  if(env->ExceptionCheck() == JNI_TRUE) {
    delete sgeJob;
    return;
  }
  
  sgeJob->freeSGEVars();
  sgeJob->resolveId();

  if(env->ExceptionCheck() == JNI_TRUE) {
    delete sgeJob;
    return;
  }

  int job_id = sgeJob->getId();
  delete sgeJob;

  int ret = 0;
  ret = callSetIntMethod(env, job, "setID", job_id);
  if(!ret)
    throw_exception(env, "Update job id failed : error executing setID method.");
}

/*
 * Class:     com_sun_grid_proxy_SGEAdapter
 * Method:    shutdownGDI
 * Signature: ()Ljava/lang/Object;
 */
JNIEXPORT void JNICALL Java_com_sun_grid_proxy_SGEAdapter_shutdownGDI (JNIEnv* env, jobject thisObj) {
  /* GENERAL SGE SHUTDOWN */
  int shutdown_ok = sge_gdi_shutdown();
	
  switch(shutdown_ok) {
  case AE_OK:
    fprintf(stdout, "Grid Engine GDI Shutdown: ok\n");
    break;
  case AE_QMASTER_DOWN:
    fprintf(stderr, "Grid Engine GDI Shutdown: qmaster not alive\n");
    throw_exception(env, "Grid Engine GDI Shutdown: qmaster not alive.");
    break;
  default:
    fprintf(stderr, "Grid Engine GDI Shutdown: unknown error\n");
    throw_exception(env, "Grid Engine GDI Shutdown: unknown error.");
    break;
  }
}
