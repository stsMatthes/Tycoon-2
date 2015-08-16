/*
 * This file is part of the Tycoon-2 system.
 *
 * The Tycoon-2 system is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation (Version 2).
 *
 * The Tycoon-2 system is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public
 * License along with the Tycoon-2 system; see the file LICENSE.
 * If not, write to AB 4.02, Softwaresysteme, TU Hamburg-Harburg
 * D-21071 Hamburg, Germany. http://www.sts.tu-harburg.de
 * 
 * Copyright (c) 1996-1998 Higher-Order GmbH, Hamburg. All rights reserved.
 *
 */
/*
  $File: //depot/tycoon2/stsmain/bootstrap/src/tycoonOS/tosProcess.c $ $Revision: #3 $ $Date: 2003/10/02 $ Andreas Gawecki, Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Support for process control. The return value in case of errors
  is -1 if not stated otherwise. For error codes see tosError.
*/

#include <errno.h>

#ifdef rt_LIB_Win32_i386
  #include <process.h>
  #include <windows.h>
#else
  #include <sys/types.h>
  #include <sys/times.h>
  #include <limits.h>
  #include <unistd.h>
  #include <signal.h>
  #include <pthread.h>
#endif


#include "tosProcess.h"
#include "tos.h"
#include "tosLog.h"
#include "tosError.h"
#include "tosDate.h"
#include "tosFilename.h"


/*== Process identification ===============================================*/

Int tosProcess_getpid(void)
{
  int res;

  #ifdef rt_LIB_Win32_i386
    res = _getpid();
  #else
    res = getpid();
  #endif

  #ifdef tosProcess_DEBUG
     tosLog_debug("tosProcess",
                  "getpid",
                  "Get current process id %d, res=(%d:%d)",
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}


Int tosProcess_getppid(void)
{
  int res;

  #ifdef rt_LIB_Win32_i386
    tosLog_error("tosProcess",
                 "getppid",
                 "Not implemented on Win32 API");

    SetLastError(120); /* not implemented */
    errno = EWIN32API;
    res   = -1;
  #else
    res = getppid();
  #endif

  #ifdef tosProcess_DEBUG
     tosLog_debug("tosProcess",
                  "getppid",
                  "Get parent process id %d, res=(%d:%d)",
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}


/*== Process informations =================================================*/

Long tosProcess_getUserCPUtime(void)
{
  Long res;

#ifdef rt_LIB_Win32_i386

  HANDLE   hProcess;
  BOOL     rc;

  FILETIME creationTime;  /* when the process was created              */
  FILETIME exitTime;      /* when the process exited                   */
  FILETIME kernelTime;    /* time the process has spent in kernel mode */
  FILETIME userTime;      /* time the process has spent in user mode   */

  LARGE_INTEGER resValue;

  /* Get pseudohandle of current process (no CloseHandle needed...) */
  hProcess = GetCurrentProcess();
  if (hProcess != NULL) {
     rc = GetProcessTimes(hProcess, &creationTime, &exitTime, &kernelTime, &userTime);
     if (rc) {
        resValue.LowPart = userTime.dwLowDateTime;
        resValue.HighPart = userTime.dwHighDateTime;
        res = resValue.QuadPart * 10000; /* 100 nano-second-units to milliseconds */
     }
     else {
        errno = EWIN32API;
        res = 0;
     }
  }
  else {
     errno = EWIN32API;
     res = 0;
  }

#else
  struct tms rusage;
  times (&rusage);
  res = ((double)(rusage.tms_utime + rusage.tms_cutime)
         * 1000.0 / tosDate_clocksPerSecond());
#endif

  return res;
}


Long tosProcess_getKernelCPUtime(void)
{
  Long res;

#ifdef rt_LIB_Win32_i386

  HANDLE   hProcess;
  BOOL     rc;

  FILETIME creationTime;  /* when the process was created              */
  FILETIME exitTime;      /* when the process exited                   */
  FILETIME kernelTime;    /* time the process has spent in kernel mode */
  FILETIME userTime;      /* time the process has spent in user mode   */

  LARGE_INTEGER resValue;

  /* Get pseudohandle of current process (no CloseHandle needed...) */
  hProcess = GetCurrentProcess();
  if (hProcess != NULL) {
     rc = GetProcessTimes(hProcess, &creationTime, &exitTime, &kernelTime, &userTime);
     if (rc) {
        resValue.LowPart = kernelTime.dwLowDateTime;
        resValue.HighPart = kernelTime.dwHighDateTime;
        res = resValue.QuadPart * 10000; /* 100 nano-second-units to milliseconds */
     }
     else {
        errno = EWIN32API;
        res = 0;
     }
  }
  else {
     errno = EWIN32API;
     res = 0;
  }

#else
  struct tms rusage;
  times (&rusage);
  res = ((double)(rusage.tms_stime + rusage.tms_cstime)
         * 1000.0 / tosDate_clocksPerSecond());
#endif

  return res;
}


/*== Process control and execution ========================================*/

Int tosProcess_kill(int pid, int value)
{
  int res = 0;

#ifdef rt_LIB_Win32_i386
  HANDLE hProcess;
  BOOL rc;

  /* First get handle, kill and close handle */
  hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, (DWORD)pid);
  if (hProcess != NULL) {
     rc = TerminateProcess(hProcess, (UINT)value);
     if (!rc) {
        errno = EWIN32API;
        res = -1;
     }
     CloseHandle(hProcess);
  }
  else {
     errno = EWIN32API;
     res = -1;
  }

#else
  res = kill(pid, value);
#endif

  #ifdef tosProcess_DEBUG
     tosLog_debug("tosProcess",
                  "kill",
                  "Kill process, pid=%d, val=%d, res=(%d:%d:%d)",
                  pid,
                  value,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}

Int tosProcess_signal(int pid, int value)
{
  int res;

  #ifdef rt_LIB_Win32_i386
    tosLog_error("tosProcess",
                 "signal",
                 "Not implemented on Win32 API");

    SetLastError(120); /* not implemented */
    errno = EWIN32API;
    res   = -1;
  #elif defined(rt_LIB_Linux_i386)
    if ((value == SIGUSR1) || (value == SIGUSR2)) {
       tosLog_error("tosProcess",
                    "signal",
                    "Don´t use SIGUSR1 or SIGUSR2 under Linux (because of linuxthreads)");
       errno = ENOTSUP;
       res   = -1;
    }
    else
       res = kill(pid, value);
  #else
    res = kill(pid, value);
  #endif

  #ifdef tosProcess_DEBUG
     tosLog_debug("tosProcess",
                  "signal",
                  "Sent signal %d to process id %d, res=(%d:%d:%d)",
                  value,
                  pid,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}


Int tosProcess_execute(const char *cmdname, char *const argv[])
{
  int res = 0;
  char realCMD[tosFilename_MAXLEN];

  tosFilename_normalize(cmdname, realCMD, tosFilename_MAXLEN);
  res = execvp(realCMD, argv);

  #ifdef tosProcess_DEBUG
     tosLog_debug("tosProcess",
                  "execute",
                  "Start process %s, res=(%d:%d:%d)",
                  realCMD,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}


Int tosProcess_system(const char *cmdname)
{
  int res = 0;
  char realCMD[tosFilename_MAXLEN];

  tosFilename_normalize(cmdname, realCMD, tosFilename_MAXLEN);
  res = system(realCMD);

  #ifdef tosProcess_DEBUG
     tosLog_debug("tosProcess",
                  "system",
                  "Start process %s, res=(%d:%d:%d)",
                  realCMD,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}

