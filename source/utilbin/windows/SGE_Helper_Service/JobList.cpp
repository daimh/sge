/*___INFO__MARK_BEGIN__*/
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the Sun Industry Standards Source License Version 1.2
 *
 *  Sun Microsystems Inc., March, 2001
 *
 *
 *  Sun Industry Standards Source License Version 1.2
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.2 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://gridengine.sunsource.net/Gridengine_SISSL_license.html
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *   Copyright: 2001 by Sun Microsystems, Inc.
 *
 *   All Rights Reserved.
 *
 * Portions of this software are Copyright (c) 2011 Univa Corporation
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#include <winsock2.h>
#include "Job.h"
#include "JobList.h"
#include "Logging.h"

/****** C_JobList::C_JobList() ************************************************
*  NAME
*     C_JobList::C_JobList() -- constructor
*
*  SYNOPSIS
*     C_JobList::C_JobList()
*
*  FUNCTION
*     Initializes C_JobList object.
*
*  NOTES
*******************************************************************************/
C_JobList::C_JobList()
{
   m_hJobListMutex = CreateMutex(NULL, FALSE, "ListMutex");
   m_pFirst = NULL;
}

C_JobList::~C_JobList()
{
   C_Job *pNext = NULL;
   C_Job *pJob  = m_pFirst;

   while (pJob != NULL) {
      pNext = pJob->next;
      delete pJob;
      pJob = pNext; 
   }
   CloseHandle(m_hJobListMutex);
}

/****** C_JobList::AddJobToList() *********************************************
*  NAME
*     C_JobList::AddJobToList() -- adds new job to the global job list
*
*  SYNOPSIS
*     POSITION C_JobList::AddJobToList(C_Job *pJob)
*
*  FUNCTION
*     Adds the new job to the global job list. If the job (identified by
*     job_id, ja_task_id, pe_task_id) already exists in the job list,
*     function fails.
*
*  INPUTS
*     C_Job *pJob - the new job object that is to be added to the list
*
*  RESULT
*     BOOL - true if the new job object already existed in the list,
*            false if it is really new.
*
*  NOTES
*******************************************************************************/
BOOL C_JobList::AddJobToList(C_Job *pJob)
{
   C_Job    *pExistingJob = NULL;
   BOOL     bAlreadyInList = TRUE;

   // lock access to job list
   WaitForSingleObject(m_hJobListMutex, INFINITE);
   pExistingJob = FindJobInList(*pJob);
   if(pExistingJob == NULL) {
      bAlreadyInList = FALSE;
      AddTail(pJob);
   }

   // unlock access to job list
   ReleaseMutex(m_hJobListMutex);

   return bAlreadyInList;
}

void C_JobList::AddTail(C_Job *pJob)
{
   C_Job *pPrevElem = NULL;
   C_Job *pCurElem = m_pFirst;

   // search last element in list
   while (pCurElem != NULL) {
      pPrevElem = pCurElem;
      pCurElem = pCurElem->next;
   }

   // in any case, we insert the last elem, so let it point to NULL.
   pJob->next = NULL;
   if (pPrevElem != NULL) {
      pPrevElem->next = pJob;
   } else {
      m_pFirst = pJob;
   }
}

/****** C_JobList::RemoveJobFromList() *****************************************
*  NAME
*     C_JobList::RemoveJobFromList() -- removes job from the job list
*
*  SYNOPSIS
*     C_Job *C_JobList::RemoveJobFromList(C_Job &Job)
*
*  FUNCTION
*     Removes a job object from the job list. 
*
*  INPUTS
*     C_Job &pJob - a temporary job object that contains the job_id, ja_task_id
*                   and pe_task_id of the job object that is to be removed from
*                   the list.
*
*  RESULT
*     C_Job* - pointer to the job object that was removed from the list.
*              NULL if the job object was not found in the list.
*
*  NOTES
*******************************************************************************/
C_Job *C_JobList::RemoveJobFromList(C_Job &Job)
{
   C_Job    *pJob = NULL;
   C_Job    *pPrev = NULL;

   // lock access to job list
   WaitForSingleObject(m_hJobListMutex, INFINITE);

   // get job from job list
   pJob = m_pFirst;
   while (pJob != NULL) {
      if(pJob->m_job_id == Job.m_job_id
         && pJob->m_ja_task_id == Job.m_ja_task_id
         && ((pJob->m_pe_task_id==NULL && Job.m_pe_task_id==NULL)
            || strcmp(pJob->m_pe_task_id, Job.m_pe_task_id)==0)
         && (pJob->m_JobStatus == js_Finished 
            || pJob->m_JobStatus == js_Failed
            || pJob->m_JobStatus == js_Deleted)) {
         if (pPrev != NULL) {
            pPrev->next = pJob->next;
         } else {
            m_pFirst = pJob->next;
         }
         break;
      }
      pPrev = pJob;
      pJob = pJob->next;
   }
   // unlock access to job list
   ReleaseMutex(m_hJobListMutex);

   return pJob;
}

/****** C_JobList::FindJobInList() ********************************************
*  NAME
*     C_JobList::FindJobInList() -- searches job in job list
*
*  SYNOPSIS
*     C_Job *C_JobList::FindJobInList(C_Job &Job)
*
*  FUNCTION
*     Searches a job object in the list
*
*  INPUTS
*     C_Job &pJob - a temporary job object that contains the job_id, ja_task_id
*                   and pe_task_id of the job object that is to be searched in 
*                   the list.
*
*  RESULT
*     C_Job* - pointer to a copy of the job object that was found in the list.
*              Delete this copy if you finished using it!
*              NULL if the job object was not found in the list.
*
*  NOTES
*    MT-Note: Lock list before calling this function!
*******************************************************************************/
C_Job *C_JobList::FindJobInList(C_Job &Job)
{
   C_Job *pJob = m_pFirst;

   // get job from job list that is not already executed
   while (pJob != NULL) {
      if(pJob->m_job_id == Job.m_job_id
         && pJob->m_ja_task_id == Job.m_ja_task_id
         && ((pJob->m_pe_task_id==NULL && Job.m_pe_task_id==NULL)
            || strcmp(pJob->m_pe_task_id, Job.m_pe_task_id)==0)) {
         break;
      }
      pJob = pJob->next;
   }
   return pJob;
}

/****** C_JobList::GetFirstJobInReceivedState() ********************************
*  NAME
*     C_JobList::GetFirstJobInReceivedState() -- searches first job in 
*                                                js_Revceived state
*
*  SYNOPSIS
*     C_Job *C_JobList::GetFirstJobInReceivedState(C_Job &Job)
*
*  FUNCTION
*     Searches the first job in js_Received state in the list
*
*  RESULT
*     C_Job* - pointer to the first job object in js_Received state in the list.
*              NULL if no object in js_Received state was found in the list.
*
*  NOTES
*    MT-Note: Lock list before calling this function!
*******************************************************************************/
C_Job* C_JobList::GetFirstJobInReceivedState()
{
   C_Job    *pJob = m_pFirst;

   while (pJob != NULL) {
      if (pJob->m_JobStatus == js_Received) {
         break;
      }
      pJob = pJob->next;
   }
   return pJob;
}

BOOL C_JobList::IsEmpty()
{
   return m_pFirst == NULL;
}
