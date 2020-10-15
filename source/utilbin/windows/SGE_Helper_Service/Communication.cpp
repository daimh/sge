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
 *  Portions of this software are Copyright (c) 2011 Univa Corporation
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#include <winsock2.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

#include "Job.h"
#include "JobList.h"
#include "JobStart.h"
#include "Communication.h"
#include "SGE_Helper_Service.h"
#include "Logging.h"

// defines
#define COMMAND_SIZE 32*1024

// global variables
extern C_JobList g_JobList;
extern BOOL      g_bAcceptJobs;

/****** C_Communication::C_Communication() *************************************
*  NAME
*     C_Communication::C_Communication() -- constructor
*
*  SYNOPSIS
*     C_Communication::C_Communication()
*
*  FUNCTION
*     Initializes the C_Communication object.
*
*  NOTES
*******************************************************************************/
C_Communication::C_Communication()
{
   m_ListenSocket = INVALID_SOCKET;
}

/****** C_Communication::InitComm() ********************************************
*  NAME
*     C_Communication::InitComm() -- constructor, initializes the communication
*
*  SYNOPSIS
*     C_Communication::InitComm()
*
*  FUNCTION
*     Initializes the communication, i.e. sets up the server that listens
*     on a specific port for incoming connections.
*
*  RESULT
*     int - 0  if communication was successfully initialized,
*          >0  WSAGetLastError()
*
*  NOTES
*******************************************************************************/
int C_Communication::InitComm()
{
   WSADATA            wsaData;
   struct sockaddr_in saddr;
   int                protofamily = AF_INET;
   char               ipaddr[]    = "127.0.0.1";
   int                res         = 0;
   int                length      = sizeof(saddr);
  
   try {
      res = WSAStartup(MAKEWORD(2,2), &wsaData);
      if(res != 0) {
         throw 1;
      }

      m_ListenSocket = socket(protofamily, SOCK_STREAM, 0);
      if(m_ListenSocket == INVALID_SOCKET) {
         throw 2;
      }

      saddr.sin_family      = protofamily;
      saddr.sin_port        = 0;
      saddr.sin_addr.s_addr = inet_addr(ipaddr);

      if(bind(m_ListenSocket, (struct sockaddr*)&saddr, sizeof(saddr))!=0) {
         throw 3;
      }

      if(getsockname(m_ListenSocket, (struct sockaddr*)&saddr, &length) == -1) {
         throw 4;
      }
      WriteToLogFile("server port: %d", ntohs(saddr.sin_port));
      if(WritePortToRegistry(ntohs(saddr.sin_port)) != 0) {
         throw 5;
      }

      if(listen(m_ListenSocket, 5)!=0) {
         throw 6;
      }
   }
   catch(int where) {
      if(where > 1) {
         res = WSAGetLastError();
      }
      if(where > 2) {
         closesocket(m_ListenSocket);
         m_ListenSocket = INVALID_SOCKET;
      }      
      if(where == 5) {
         res = 1;         
      }
   }
   return res;
}

/****** C_Communication::~C_Communication() ************************************
*  NAME
*     C_Communication::~C_Communication() -- destructor, cleans up communication
*
*  SYNOPSIS
*     C_Communication::~C_Communication()
*
*  FUNCTION
*     Stops the server from listening on a port, closes the socket where the
*     server listened, cleans up windows sockets.
*
*  NOTES
*******************************************************************************/
C_Communication::~C_Communication()
{
   ExitComm();
}

/****** C_Communication::DoComm() const ****************************************
*  NAME
*     C_Communication::DoComm() const -- does the communication
*
*  SYNOPSIS
*     int C_Communication::DoComm() const
*
*  FUNCTION
*     Does the communication with sge_shepherd. I.e. whenever a sge_shepherd
*     connects to the server, it receives the request, does what is requested
*     and then shuts down the connection.
*
*  RESULT
*     int - 0 if command was successfully received and processed, 1 else
*
*  NOTES
*******************************************************************************/
int C_Communication::DoComm() const
{
   int                ret = 1;
   int                idx = 0;
   SOCKET             comm_sock;
   int                remote_saddr_len;
   struct sockaddr_in remote_saddr;
   char               szMessage[1024];
   char               command[COMMAND_SIZE];
   const int          commandsize = sizeof(command);
   C_Job              Job;
   C_Job              *pJob = NULL;
   en_request_type    request_type;

   remote_saddr_len = sizeof(remote_saddr);
   comm_sock = accept(m_ListenSocket, 
                     (struct sockaddr*)&remote_saddr, &remote_saddr_len);
   if(comm_sock == INVALID_SOCKET) {
      return WSAGetLastError();
   }

   ZeroMemory(command, COMMAND_SIZE);
   while((ret=recv(comm_sock, command+idx, commandsize-idx, 0))>0) {
      idx += ret;
      if(command[idx-1] == EOF) {
         break;
      }
   }
   if(ret == -1) {
      ret = WSAGetLastError();
      ShutdownSocket(&comm_sock);
      return ret;
   }
   ret = 0;

   WriteToLogFile("Received new request, will parse it now.");
   request_type = Job.ParseCommand(command);
   WriteToLogFile("Finished parsing the new request.");

   switch(request_type) {
      case req_job_start:
         {
            BOOL   bAlreadyInList = TRUE;
            HANDLE hThread;

            WriteToLogFile("Got a job start request.");

            if(g_bAcceptJobs) {
               Job.m_comm_sock = comm_sock;

               sprintf(szMessage, "Starting job %lu.%lu %s", 
                  Job.m_job_id, Job.m_ja_task_id,
                  Job.m_pe_task_id ? Job.m_pe_task_id : "<null>");
               WriteToLogFile(szMessage);

               // try to put job in job list
               pJob = new C_Job(Job);
               bAlreadyInList = g_JobList.AddJobToList(pJob);
            }

            if (bAlreadyInList == FALSE) {
               WriteToLogFile("New job, starting thread");
               // job is not already in job list, send ACK and start worker thread
               hThread = CreateThread(NULL, 0, JobStarterThread, 0, 0, 0);
               CloseHandle(hThread);
            } else {
               WriteToLogFile("Existing job, send NACK and delete job object");
               // job is already in job list, send NACK and delete job object
               strcpy(szMessage, "NAK");
               szMessage[strlen(szMessage)+1] = (char)EOF;
               send(comm_sock, szMessage, (int)strlen(szMessage)+2, 0);
               ShutdownSocket(&comm_sock);
               delete pJob;
               pJob = NULL;
            }
         }
         break;

      case req_send_job_usage:
         WriteToLogFile("Got a send usage request.");
         pJob = g_JobList.RemoveJobFromList(Job);
         if(pJob) {
            pJob->m_comm_sock = comm_sock;
            SendJobUsage(*pJob);
            delete pJob;
            pJob = NULL;
            sprintf(szMessage, "Sending usage of job %lu.%lu %s",
                        Job.m_job_id, Job.m_ja_task_id,
                        Job.m_pe_task_id ? Job.m_pe_task_id : "<null>");
         } else {
            sprintf(szMessage, "Warning: Job %lu.%lu %s not found!",
                        Job.m_job_id, Job.m_ja_task_id,
                        Job.m_pe_task_id ? Job.m_pe_task_id : "<null>");
         }
         WriteToLogFile(szMessage);
         ShutdownSocket(&comm_sock);
         break;

      case req_forward_signal:
         WriteToLogFile("Got a forward signal request.");
         // lock access to job list
         WaitForSingleObject(g_JobList.m_hJobListMutex, INFINITE);
         pJob = g_JobList.FindJobInList(Job);
         if(pJob
            && pJob->m_JobStatus != js_Finished
            && pJob->m_JobStatus != js_Failed
            && pJob->m_JobStatus != js_Deleted) {
            BOOL bRet = FALSE;
            char szAnswer[100];
            int  sent;

            if(pJob->m_hProcess != INVALID_HANDLE_VALUE
               && pJob->m_hJobObject != INVALID_HANDLE_VALUE) {
               bRet = TerminateJobObject(pJob->m_hJobObject, 0);
               if(bRet) {
                  pJob->m_JobStatus = js_Deleted;
               }
            }
            ReleaseMutex(g_JobList.m_hJobListMutex);

            sprintf(szAnswer, "%s", bRet ? "ACK" : "NAK");
            szAnswer[strlen(szAnswer)+1] = (char)EOF;
            sent = send(comm_sock, szAnswer, (int)strlen(szAnswer)+2, 0);
            if(sent >= 0) {
               ret = 0;
            }
         } else {
	         ReleaseMutex(g_JobList.m_hJobListMutex);
         }
         ShutdownSocket(&comm_sock);
         break;
   }
   return ret;
}

/****** C_Communication::ExitComm() ********************************************
*  NAME
*     C_Communication::ExitComm() - cleans up communication
*
*  SYNOPSIS
*     C_Communication::ExitComm()
*
*  FUNCTION
*     Stops the server from listening on a port, closes the socket where the
*     server listened, cleans up windows sockets.
*
*  RESULT
*     int: 0 if communication cleanup succeeded, else 1
*
*  NOTES
*******************************************************************************/
int C_Communication::ExitComm()
{
   int ret = 1;

   if(m_ListenSocket != INVALID_SOCKET) {
      closesocket(m_ListenSocket);
      m_ListenSocket = INVALID_SOCKET;
   }
   ret = WSACleanup();
   return ret;
}

/****** C_Communication::SendJobUsage() const **********************************
*  NAME
*     C_Communication::SendJobUsage() const -- sends job usage of a specific job
*                                              over the socket
*
*  SYNOPSIS
*     int C_Communication::SendJobUsage(const C_Job &Job) const
*
*  FUNCTION
*     Sends the usage data of a specific job over the socket to sge_shepherd.
*
*  INPUTS
*     const C_Job &Job - the job object that contains the usage data
*
*  RESULT
*     int - 0 if job usage data was successfully sent, 1 else
*
*  NOTES
*******************************************************************************/
int C_Communication::SendJobUsage(const C_Job &Job) const
{
   int sent;
   int ret = 1;
   char job_usage[1000];
   int  status;

   status = 0 + (((BYTE)Job.dwExitCode) << 8);

   sprintf(job_usage, "%ld %ld %ld %ld %d%c", 
      Job.lKernelSec, Job.lKernelUSec, Job.lUserSec, Job.lUserUSec, status, (char)EOF);
   job_usage[strlen(job_usage)+1] = (char)EOF;
   sent = send(Job.m_comm_sock, job_usage, (int)strlen(job_usage)+2, 0);
   if (sent >= 0) {
      ret = 0;
   }
   return ret;
}

/****** C_Communication::SendExitStatus() const ********************************
*  NAME
*     C_Communication::SendExitStatus() const -- sends exit status of a specific
*                                                job over the socket
*
*  SYNOPSIS
*     int C_Communication::SendExitStatus(const C_Job &Job) const
*
*  FUNCTION
*     Sends the exit status of a specific job over the socket to sge_shepherd.
*
*  INPUTS
*     const C_Job &Job - the job object that contains the exit status
*
*  RESULT
*     int - 0 if job exit status was successfully sent, 1 else
*
*  NOTES
*******************************************************************************/
int C_Communication::SendExitStatus(const C_Job &Job) const
{
   int sent;
   int ret = 1;
   char job_result[1000];

   sprintf(job_result, "%d %lu %s%c", 
      (int)Job.m_JobStatus, 
      Job.dwExitCode, 
      Job.szError ? Job.szError : "No error",
      (char)EOF);
   sent = send(Job.m_comm_sock, job_result, (int)strlen(job_result), 0);
   if (sent >= 0) {
      ret = 0;
   }
   return ret;
}

/****** C_Communication::ShutdownSocket() const ********************************
*  NAME
*     C_Communication::ShutdownSocket() const -- gracefully shuts down a socket
*
*  SYNOPSIS
*     int C_Communication::ShutdownSocket(SOCKET *pSock) const
*
*  FUNCTION
*     Gracefully shuts down a socket, i.e. it tells the sge_shepherd that it
*     will shut down a socket (using shutdown()) and then closes the socket.
*     This makes sure that no data is lost.
*
*  INPUTS
*     SOCKET *pSock - the socket that is to be shut down
*
*  RESULT
*     int - 0 if socket was shut down, 1 else
*
*  NOTES
*******************************************************************************/
int C_Communication::ShutdownSocket(SOCKET *pSock) const
{
   shutdown(*pSock, SD_BOTH);
   closesocket(*pSock);
   *pSock = -1;
   return 0;
}

/****** C_Communication::WritePortToRegistry() const ***************************
*  NAME
*     C_Communication::WritePortToRegistry() const -- Writes the port where the
*                                                     SGE Helper Service
*                                                     listens to the registry
*
*  SYNOPSIS
*     int C_Communication::WritePortToRegistry(DWORD dwPort) const
*
*  FUNCTION
*     Write the port where the SGE Helper Service listens for connections
*     to the registry to  the key
*     "SOFTWARE\\Sun Microsystems\\N1 Grid Engine\\Helper Service";
*     
*  INPUTS
*     DWORD dwPort - Number of the listening port
*
*  RESULT
*     int - 0 if port was successfully written to registry,
*           1 else.
*******************************************************************************/
int C_Communication::WritePortToRegistry(DWORD dwPort) const
{
   char szKey[] = "SOFTWARE\\Sun Microsystems\\N1 Grid Engine";
   char tmpSzKey[] = "SOFTWARE\\Sun Microsystems\\N1 Grid Engine\\Helper Service";
   HKEY tmpHRegKey;
   HKEY hRegKey;
   long res, res1;

   res = RegCreateKeyEx(HKEY_LOCAL_MACHINE, szKey,
      0, NULL,
      REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
      NULL, &hRegKey,
      NULL);

   if(res == ERROR_SUCCESS) {
      RegCloseKey(hRegKey);

      res1 = RegCreateKeyEx(HKEY_LOCAL_MACHINE, tmpSzKey,
         0, NULL,
         REG_OPTION_VOLATILE, KEY_ALL_ACCESS,
         NULL, &tmpHRegKey,
         NULL);

      if(res1 == ERROR_SUCCESS) {
         RegSetValueEx(tmpHRegKey, "Port", NULL, REG_DWORD, (BYTE*)&dwPort, sizeof(dwPort));
         RegCloseKey(tmpHRegKey);

         return 0;
      }
   }
   return 1;
}

