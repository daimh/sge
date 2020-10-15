/*___INFO__MARK_BEGIN__*/
/***************************************************************************
 *
 *  The contents of this file are made available subject to the terms of the
 *  Apache Software License 2.0 ('The License').
 *  You may not use this file except in compliance with The License.
 *  You may obtain a copy of The License at
 *  http://www.apache.org/licenses/LICENSE-2.0.html
 *
 *  Copyright (c) 2011 Univa Corporation.
 *
 ***************************************************************************/
/*___INFO__MARK_END__*/

#include <winsock2.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

extern BOOL g_bDoLogging;

/****** WriteToLogFile() *******************************************************
*  NAME
*     WriteToLogFile() -- writes a message to a log file
*
*  SYNOPSIS
*     int WriteToLogFile(const char *szMessage, ...)
*
*  FUNCTION
*     Writes a message to a log file. For debugging purposes only.
*
*  INPUTS
*     const char *szMessage - the format string of the message
*     ...                   - the parameters of the message (see printf())
*
*  RESULT
*     int - 0 if message was written to log file, 1 else
*           2 if logging is disabled
*
*  NOTES
*******************************************************************************/
int WriteToLogFile(const char *szMessage, ...)
{
   int         ret = 1;
   FILE        *fp = NULL;
   SYSTEMTIME  sysTime;
   va_list     args;
   char        Buffer[4096];
   DWORD       dwLastError;
   static BOOL g_bFirstTime = TRUE;
   static char g_szTraceFile[5000];

   if (g_bDoLogging == FALSE) {
      return 2;
   }
   // We do not want to change LastError in here.
   dwLastError = GetLastError();
   GetLocalTime(&sysTime);

   // If we don't have a trace file yet, create it's name.
   if (g_bFirstTime == TRUE) {
      g_bFirstTime = FALSE;
      // The trace file name is $TEMP\SGE_Helper_Service.log
      // e.g. "C:\Temp\SGE_Helper_Service.log"
      strcpy(Buffer, "C:\\TEMP");
//      GetEnvironmentVariable("TEMP", Buffer, 4095);
      _snprintf(g_szTraceFile, 4999, "%s\\SGE_Helper_Service.log", Buffer);
   }

   fp = fopen(g_szTraceFile, "a+");
   if (fp == NULL) {
      // Create a "panic" trace file
      sprintf(g_szTraceFile, "c:\\Temp\\SGE_Helper_Service.%02d%02d%02d.log", 
         sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
      fp = fopen(g_szTraceFile, "a+");
   }
   if(fp != NULL) {
      va_start(args, szMessage);
      _vsnprintf(Buffer, 4095, szMessage, args);
      fprintf(fp, "%02d:%02d:%02d [SGE_Helper_Service] %s\n",
         sysTime.wHour, sysTime.wMinute, sysTime.wSecond, Buffer);
      fflush(fp);
      fclose(fp);
      ret = 0;
   }
   SetLastError(dwLastError);

   return ret;
}
