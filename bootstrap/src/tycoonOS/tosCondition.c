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
  tosCondition.c 1.0 final  09-AUG-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Condition variables
*/

#include <stdlib.h>
#include <errno.h>

#ifdef rt_LIB_Win32_i386
  /*
   * We need special NT 4.0 functions, detection of the correct
   * operating system version will be dynamic.
   */
  #ifdef _WIN95
     #undef _WIN95
  #endif
  #ifdef _WIN32_WINDOWS
     #undef _WIN32_WINDOWS
  #endif
  #ifndef  _WINNT
    #define  _WINNT
  #endif
  #define  _WIN32_WINNT 0x0400
  #include <windows.h>
#else
  #include <pthread.h>
  #include <sys/time.h>
#endif

#if defined(rt_LIB_Solaris2_SPARC)
  #include <thread.h>
#endif


#include "tosCondition.h"
#include "tos.h"
#include "tosMutex.h"
#include "tosError.h"
#include "tosLog.h"
#include "tosSystem.h"


/*== Init / Destroy =======================================================*/

int tosCondition_initSingle(tosCondition_Single *cond)
{
  int res = 0;
  
  #ifdef rt_LIB_Win32_i386
     *cond = CreateEvent(NULL, FALSE, FALSE, NULL);
     if (*cond == NULL)
        res = EWIN32API;
  #else
     #if defined(rt_LIB_HPUX_PARISC) && defined(tmthread_DRAFT_INTERFACE)
         res = pthread_cond_init(cond, pthread_condattr_default);   
     #else
         res = pthread_cond_init(cond, NULL);
     #endif
  #endif

  /* POSIX Thread library: Return value is errno */
  if (res) { errno = res; res = -1; }

  #ifdef tosCondition_DEBUG
     tosLog_debug("tosCondition",
                  "initSingle",
                  "Create single condition, cond=%u, res=(%d:%d:%d)",
                  *cond,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}


int tosCondition_initBroadcast(tosCondition_Broadcast *cond)
{
  int res = 0;

  #ifdef rt_LIB_Win32_i386
     *cond = CreateEvent(NULL, TRUE, FALSE, NULL);
     if (*cond == NULL)
        res = EWIN32API;
  #else
     #if defined(rt_LIB_HPUX_PARISC) && defined(tmthread_DRAFT_INTERFACE)
         res = pthread_cond_init(cond, pthread_condattr_default);   
     #else
         res = pthread_cond_init(cond, NULL);
     #endif
  #endif

  /* POSIX Thread library: Return value is errno */
  if (res) { errno = res; res = -1; }

  #ifdef tosCondition_DEBUG
     tosLog_debug("tosCondition",
                  "initBroadcast",
                  "Create broadcast condition, cond=%u, res=(%d:%d:%d)",
                  *cond,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}


int tosCondition_destroy(tosCondition_T *cond)
{
  int res = 0;

  #ifdef rt_LIB_Win32_i386
     BOOL rc = CloseHandle(*cond);
     if (!rc)
        res = EWIN32API;
  #else
     res = pthread_cond_destroy(cond);
  #endif

  /* POSIX Thread library: Return value is errno */
  if (res) { errno = res; res = -1; }

  #ifdef tosCondition_DEBUG
     tosLog_debug("tosCondition",
                  "destroy",
                  "Destroy condition, cond=%u, res=(%d:%d:%d)",
                  *cond,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}


/*== Waiting / Sending ====================================================*/

int tosCondition_signal(tosCondition_Single *cond)
{
  int res = 0;

  #ifdef rt_LIB_Win32_i386
     BOOL rc = PulseEvent(*cond);
     if (!rc)
        res = EWIN32API;
  #else
     res = pthread_cond_signal(cond);
  #endif

  /* POSIX Thread library: Return value is errno */
  if (res) { errno = res; res = -1; }

  #ifdef tosCondition_DEBUG
     tosLog_debug("tosCondition",
                  "signal",
                  "Signal condition, cond=%u, res=(%d:%d:%d)",
                  *cond,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}


int tosCondition_broadcast(tosCondition_Broadcast *cond)
{
  int res = 0;

  #ifdef rt_LIB_Win32_i386
     BOOL rc = PulseEvent(*cond);
     if (!rc)
        res = EWIN32API;
  #else
     res = pthread_cond_broadcast(cond);
  #endif

  /* POSIX Thread library: Return value is errno */
  if (res) { errno = res; res = -1; }

  #ifdef tosCondition_DEBUG
     tosLog_debug("tosCondition",
                  "broadcast",
                  "Broadcast condition, cond=%u, res=(%d:%d:%d)",
                  *cond,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}


int tosCondition_wait(tosCondition_T *cond, tosMutex_T *mx)
{
  int res = 0;

  #ifdef rt_LIB_Win32_i386
     /*
      * Unsafe non-atomic release of the mutex on Windows 95/98 systems
      * or safe, atomic release and wait function on Windows NT
      */
     DWORD rc = 0;
     if (tosSystem_getID() != tosSystem_ID_WINNT) {
        #ifdef tosCondition_DEBUG
           tosLog_debug("tosCondition",
                        "wait",
                        "Enter unsafe (Win95/98) condition wait, cond=%u, mutex=%u",
                        *cond,
                        *mx);
        #endif
        tosLog_error("tosCondition", "wait", "Warning: Unsafe condition wait on Windows 95/98");
        tosMutex_unlock(mx);
        rc = WaitForSingleObject(*cond, INFINITE);
     }
     else {

        #ifdef tosCondition_DEBUG
           tosLog_debug("tosCondition",
                        "wait",
                        "Enter condition wait, cond=%u, mutex=%u",
                        *cond,
                        *mx);
        #endif
        rc = SignalObjectAndWait(*mx, *cond, INFINITE, FALSE);
     }
     tosMutex_lock(mx);
     if ((rc == 1) || (rc == WAIT_FAILED))
        res = EWIN32API;
#else
  #ifdef tosCondition_DEBUG
     tosLog_debug("tosCondition",
                  "wait",
                  "Enter condition wait, cond=%u, mutex=%u",
                  *cond,
                  *mx);
  #endif
  res = pthread_cond_wait(cond, mx);
#endif

  /* POSIX Thread library: Return value is errno */
  if (res) { errno = res; res = -1; }

  #ifdef tosCondition_DEBUG
     tosLog_debug("tosCondition",
                  "wait",
                  "Leave condition wait, cond=%u, res=(%d:%d:%d)",
                  *cond,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}


int tosCondition_timedwait(tosCondition_T *cond, tosMutex_T *mx, Long mseconds)
{
  int res = 0;

  #ifdef rt_LIB_Win32_i386
     /*
      * Unsafe non-atomic release of the mutex on Windows 95/98 systems
      * or safe, atomic release and wait function on Windows NT
      */
     DWORD rc = 0;
     if (tosSystem_getID() != tosSystem_ID_WINNT) {
        #ifdef tosCondition_DEBUG
           tosLog_debug("tosCondition",
                        "timedwait",
                        "Enter unsafe (Win95/98) condition timed wait, cond=%u, mutex=%u, msecs=%u",
                        *cond,
                        *mx,
                        mseconds);
        #endif
        tosLog_error("tosCondition", "timedwait", "Warning: Unsafe condition timed wait on Windows 95/98");
        tosMutex_unlock(mx);
        rc = WaitForSingleObject(*cond, (DWORD) mseconds);
     }
     else {

        #ifdef tosCondition_DEBUG
           tosLog_debug("tosCondition",
                        "timedwait",
                        "Enter condition timed wait, cond=%u, mutex=%u, msecs=%u",
                        *cond,
                        *mx,
                        mseconds);
        #endif
        rc = SignalObjectAndWait(*mx, *cond, (DWORD) mseconds, FALSE);
     }

     tosMutex_lock(mx);
     if ((rc == 1) || (rc == WAIT_FAILED))
        res = EWIN32API;
#else

  struct timespec absTime;
  absTime.tv_sec  = mseconds / (Long)1000;
  absTime.tv_nsec = mseconds % (Long)1000;

  #ifdef tosCondition_DEBUG
     tosLog_debug("tosCondition",
                  "timedwait",
                  "Enter condition wait, cond=%u, mutex=%u, msecs=%u",
                  *cond,
                  *mx,
                  mseconds);
  #endif

  /* What other than ETIME ? */
  res = pthread_cond_timedwait(cond, mx, &absTime);

#endif

  /* POSIX Thread library: Return value is errno */
  if (res) { errno = res; res = -1; }

  #ifdef tosCondition_DEBUG
     tosLog_debug("tosCondition",
                  "timedwait",
                  "Leave condition timed wait, cond=%u, res=(%d:%d:%d)",
                  *cond,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}

