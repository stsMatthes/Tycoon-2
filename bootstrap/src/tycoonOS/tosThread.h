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
  tosThread.h 1.0 final  12-AUG-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Basic Thread support.
*/

#ifndef TOSTHREAD_H
#define TOSTHREAD_H

#ifdef rt_LIB_Win32_i386
  #include <windows.h>
#else
  #include <pthread.h>
#endif

#if defined(rt_LIB_Solaris2_SPARC)
  #include <thread.h>
#endif

#include "tos.h"


#ifdef __cplusplus
extern "C" {
#endif


/*== Data type for threads and thread declarations ========================*/

/*
 * Various datatypes:
 *
 * T          - ID of a thread
 * FUNC_T     - Function prototype of a thread function
 * FUNC_RET   - Return type of a thread function
 * FUNC_DECL  - Deklaration prefix of a thread function
 */

#ifdef rt_LIB_Win32_i386

  #define tosThread_T         HANDLE
  #define tosThread_FUNC_RET  unsigned
  #define tosThread_FUNC_DECL unsigned __stdcall

  typedef unsigned (__stdcall *tosThread_FUNC_T) (void *);

#else

  #define tosThread_T         pthread_t
  #define tosThread_FUNC_RET  void *
  #define tosThread_FUNC_DECL void *

  typedef void *(*tosThread_FUNC_T) (void *);

#endif


/*== Starting / Termination of threads ====================================*/

/*
 * tosThread_create
 * ----------------
 * Creates a new thread with system depending default attributes. The
 * thread is created by executing 'threadFunction' with 'pArg' as its
 * sole argument.
 * As a result 'pTid' contains the new thread handle. On Unix systems
 * this function also detach (pthread_detach) the newly created thread.
 *
 * Return          - zero or -1 on error (see tosError_getCode)
 * Ressources      - allocates system specific ressources
 */

extern int tosThread_create(tosThread_T      *pTid,
                            tosThread_FUNC_T  threadFunction,
                            void             *pArg);


/*
 * tosThread_exit
 * --------------
 * Terminates the currently running thread with exit-code 'exitCode'.
 *
 * Return          - never
 * Ressources      - release system specific ressources
 */

extern void tosThread_exit(void *exitCode);


/*
 * tosThread_mainWaitedExit
 * ------------------------
 * Terminates the main program with waiting until all child threads
 * are terminated.
 *
 * Return          - never
 * Ressources      - release system specific ressources
 */

extern void tosThread_mainWaitedExit(void);


/*
 * tosThread_kill
 * --------------
 * Terminates the thread ´tid´. On Unix systems the signal SIGUSR2
 * or SIGKILL (Linux) is send to the thread. On the Win32 API this
 * function has a unsafe implementation in releasing all ressources
 * occupied by ´tid´.
 *
 * Return          - zero or -1 on error (see tosError_getCode)
 * Ressources      - release system specific ressources
 */

extern int tosThread_kill(tosThread_T tid);


/*== Various utilities ====================================================*/

/*
 * tosThread_sleep
 * ---------------
 * Makes the current thread sleep until ´sec´ seconds have elapsed or
 * a signal arrives which is not ignored. Only the current thread is
 * sleeping, not the whole process.
 *
 * Return          - nothing
 * Ressources      - none
 */

extern void tosThread_sleep(Word sec);


/*== Initializing =========================================================*/

extern void tosThread_init();


#ifdef __cplusplus
}
#endif

#endif /* TOSTHREAD_H */

