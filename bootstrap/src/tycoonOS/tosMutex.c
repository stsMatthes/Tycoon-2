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
  tosMutex.h 1.0 final  08-AUG-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Mutual exclusion locks (mutexes) prevent multiple threads
  from simultaneously executing critical sections of code
  which access shared data.
  That is, mutexes are used to serialize the execution of
  threads.

*/

#include <stdlib.h>
#include <errno.h>

#ifdef rt_LIB_Win32_i386
  #include <windows.h>
#else
  #include <pthread.h>
#endif

#if defined(rt_LIB_Solaris2_SPARC)
  #include <thread.h>
#endif


#include "tosMutex.h"
#include "tos.h"
#include "tosLog.h"
#include "tosError.h"


/*== Init / Destroy =======================================================*/

int tosMutex_init(tosMutex_T *mx)
{
  int res = 0;

  #ifdef rt_LIB_Win32_i386
     *mx = CreateMutex(NULL, FALSE, NULL);
     if (*mx == NULL)
        res = EWIN32API;
  #else
     #if defined(rt_LIB_HPUX_PARISC) && defined(tmthread_DRAFT_INTERFACE)
        res = pthread_mutex_init(mx, pthread_mutexattr_default);
     #else
        res = pthread_mutex_init(mx, NULL);
     #endif
  #endif

  /* POSIX Thread library: Return value is errno */
  if (res) { errno = res; res = -1; }

  #ifdef tosMutex_DEBUG
     tosLog_debug("tosMutex",
                  "init",
                  "Create mutex %u, res=(%d:%d:%d)",
                  *mx,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}


int tosMutex_destroy(tosMutex_T *mx)
{
  int res = 0;

  #ifdef rt_LIB_Win32_i386
     BOOL rc = CloseHandle(*mx);
     if (!rc)
        res = EWIN32API;
  #else
     res = pthread_mutex_destroy(mx);
  #endif

  /* POSIX Thread library: Return value is errno */
  if (res) { errno = res; res = -1; }

  #ifdef tosMutex_DEBUG
     tosLog_debug("tosMutex",
                  "destroy",
                  "Destroy mutex %u, res=(%d:%d:%d)",
                  *mx,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}


/*== Locking ==============================================================*/

int tosMutex_lock(tosMutex_T *mx)
{
  int res = 0;

  #ifdef rt_LIB_Win32_i386
     DWORD rc;

     #ifdef tosMutex_DEBUG
        tosLog_debug("tosMutex",
                     "lock",
                     "Waiting for mutex lock, mutex=%u",
                     *mx);
     #endif

     rc = WaitForSingleObject(*mx, INFINITE);
     if (rc == WAIT_FAILED)
        res = EWIN32API;

  #else
     #ifdef tosMutex_DEBUG
        tosLog_debug("tosMutex",
                     "lock",
                     "Waiting for mutex lock, mutex=%u",
                     *mx);
     #endif
     res = pthread_mutex_lock(mx);
  #endif

  /* POSIX Thread library: Return value is errno */
  if (res) { errno = res; res = -1; }

  #ifdef tosMutex_DEBUG
     tosLog_debug("tosMutex",
                  "lock",
                  "Getting mutex lock, mutex=%u, res=(%d:%d:%d)",
                  *mx,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}


int tosMutex_trylock(tosMutex_T *mx)
{
  int res = 0;

  #ifdef rt_LIB_Win32_i386
     DWORD rc = WaitForSingleObject(*mx, 0);
     if (rc == WAIT_FAILED)
        res = EWIN32API;
  #else
     res = pthread_mutex_trylock(mx);
  #endif

  /* POSIX Thread library: Return value is errno */
  if (res) { errno = res; res = -1; }

  #ifdef tosMutex_DEBUG
     tosLog_debug("tosMutex",
                  "trylock",
                  "Try lock for mutex %u, res=(%d:%d:%d)",
                  *mx,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}


int tosMutex_unlock(tosMutex_T *mx)
{
  int res = 0;

  #ifdef rt_LIB_Win32_i386
     BOOL rc = ReleaseMutex(*mx);
     if (!rc)
        res = EWIN32API;
  #else
     res = pthread_mutex_unlock(mx);
  #endif

  /* POSIX Thread library: Return value is errno */
  if (res) { errno = res; res = -1; }

  #ifdef tosMutex_DEBUG
     tosLog_debug("tosMutex",
                  "unlock",
                  "Release lock for mutex %u, res=(%d:%d:%d)",
                  *mx,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}

