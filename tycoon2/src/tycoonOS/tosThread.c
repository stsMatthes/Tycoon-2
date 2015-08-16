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
  tosThread.c 1.0 final  12-AUG-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Basic Thread support.
*/


#include <stdlib.h>
#include <errno.h>

#ifdef rt_LIB_Win32_i386
  #include <windows.h>
  #include <process.h>
#else
  #include <pthread.h>
  #include <unistd.h>
  #include <signal.h>
#endif

#if   defined(rt_LIB_Solaris2_SPARC)
#include <thread.h>
#endif


#include "tosThread.h"
#include "tos.h"
#include "tosLog.h"
#include "tosError.h"
#include "tosMutex.h"
#include "tosCondition.h"


/*== Starting / Termination of threads ====================================*/

int tosThread_create(tosThread_T      *pTid,
                     tosThread_FUNC_T  threadFunction,
                     void             *pArg)
{
  int res = 0;
  
  #ifdef rt_LIB_Win32_i386
     unsigned tadr;
     *pTid = (tosThread_T) _beginthreadex(NULL,
                                          0,
                                          threadFunction,
                                          pArg,
                                          0,
                                          &tadr);

     /* We want Win32 API error (not errno/doserror) */
     if (*pTid == 0)
        res = EWIN32API;

  #else
     #if defined(rt_LIB_HPUX_PARISC) && defined(tmthread_DRAFT_INTERFACE)
        res = pthread_create(pTid,
                             pthread_attr_default,
                             threadFunction,
                             pArg);

        /* detach thread (if not already detached) */
        pthread_detach(pTid);
     #else
        res = pthread_create(pTid,
                             NULL,
                             threadFunction,
                             pArg);

        /* detach thread (if not already detached) */
        pthread_detach(*pTid);
     #endif
  #endif

  if (res) { errno = res; res = -1; }

  #ifdef tosThread_DEBUG
     tosLog_debug("tosThread",
                  "create",
                  "Create new thread, tid=%u, res=(%d:%d:%d)",
                  (unsigned) *pTid,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}


void tosThread_exit(void *exitCode)
{
  #ifdef rt_LIB_Win32_i386
     HANDLE tHandle;
  #endif

  #ifdef tosThread_DEBUG
     if (exitCode == NULL)
        tosLog_debug("tosThread",
                     "exit",
                     "Terminating thread with exit code 0");
     else
        tosLog_debug("tosThread",
                     "exit",
                     "Terminating thread with exit code %d",
                     *(unsigned*)exitCode);
  #endif

  #ifdef rt_LIB_Win32_i386
     /*
      * Close thread handle, if exists
      */
     tHandle = GetCurrentThread();
     if (tHandle != (HANDLE)(-1L)) {
        CloseHandle(tHandle);
        #ifdef tosThread_DEBUG
           tosLog_debug("tosThread",
                        "exit",
                        "Handle closed: %u, win32err=%d",
                        tHandle,
                        tosError_getCodeDetail());
        #endif
     }

     if (exitCode == NULL)
        _endthreadex(0);
     else
        _endthreadex(*(DWORD*)exitCode);

  #else
     pthread_exit(exitCode);
  #endif
}


void tosThread_mainWaitedExit(void)
{
#ifdef rt_LIB_HPUX_PARISC       /* no matter what interface is used */
  /* main thread must not terminate */
  tosMutex_T     mmx;
  tosCondition_T ccx;

  tosMutex_init(&mmx);
  tosCondition_init(&ccx);

  /* wait forever */
  tosMutex_lock(&mmx);
  while(1) {
    tosCondition_wait(&ccx, &mmx);
  }
#else
  /*
   * Other systems: explicit wait not needed,
   * main program termination waits until the
   * last thread will terminate...
   */
  tosThread_exit(NULL);
#endif
}


int tosThread_kill(tosThread_T tid)
{
  int res = 0;
  
  #ifdef rt_LIB_Win32_i386
     BOOL rc;

     tosLog_error("tosThread",
                  "kill",
                  "Warning: Usafe implementation of killing threads in Win32 API");

     rc = TerminateThread(tid, 1);
     if (!rc)
        res = EWIN32API;
  #else
     #if defined(rt_LIB_HPUX_PARISC) && defined(tmthread_DRAFT_INTERFACE)
        res = 0;
     #elif defined(rt_LIB_Linux_i386)
        res = pthread_kill(tid, SIGKILL);
     #else
        res = pthread_kill(tid, SIGUSR2);
     #endif
  #endif

  if (res) { errno = res; res = -1; }

  #ifdef tosThread_DEBUG
     tosLog_debug("tosThread",
                  "kill",
                  "Killing thread, tid=%u, res=(%d:%d:%d)",
                  (unsigned) tid,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}


/*== Various utilities ====================================================*/

void tosThread_sleep(Word sec)
{
  #ifdef tosThread_DEBUG
     tosLog_debug("tosThread",
                  "sleep",
                  "Sleeping %d seconds ...",
                  sec);
  #endif

  #ifdef rt_LIB_Win32_i386
     /* Miliseconds */
     Sleep((DWORD)sec * 1000);
  #else
     sleep((unsigned)sec);
  #endif

  #ifdef tosThread_DEBUG
     tosLog_debug("tosThread",
                  "sleep",
                  "... wakeup, err=%d",
                  tosError_getCode());
  #endif
}


/*== Initializing =========================================================*/


/*
 * Per-thread signal handling for thread cancelation.
 * Not on Win32 plattforms, not on Linux because of linuxthreads
*/
#if !defined(rt_LIB_Win32_i386) && !defined(rt_LIB_Linux_i386)

static void handler_sigusr2()
{
  return;
}

static void initSignals(void)
{
  struct sigaction sa;
  sigset_t signalMask;

  /* block SIGPIPE and SIGVTALRM */
  sigemptyset(&signalMask);
  sigaddset(&signalMask, SIGPIPE);
  sigaddset(&signalMask, SIGPROF);
  
  /* set main thread signal mask. inherited by all child threads */
  #if defined(rt_LIB_HPUX_PARISC) && defined(tmthread_DRAFT_INTERFACE)
      pthread_sigmask(SIG_BLOCK, &signalMask, NULL);
  #endif
      
  /* catch USR2 signal to interrupt blocking system calls (for cancellation) */
  sa.sa_handler = handler_sigusr2;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);
  sigaddset(&sa.sa_mask, SIGUSR2);
  sigaction(SIGUSR2, &sa, NULL);
  return;
}

#endif


void tosThread_init()
{
#ifdef rt_LIB_Solaris2_SPARC
  thr_setconcurrency(5);
#endif
#if !defined(rt_LIB_Win32_i386) && !defined(rt_LIB_Linux_i386)
  initSignals();
#endif

}

