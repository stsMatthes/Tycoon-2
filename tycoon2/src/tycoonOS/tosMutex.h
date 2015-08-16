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
  tosMutex.h 1.0 final  30-JUL-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Mutual exclusion locks (mutexes) prevent multiple threads
  from simultaneously executing critical sections of code
  which access shared data.
  That is, mutexes are used to serialize the execution of
  threads.

*/

#ifndef TOSMUTEX_H
#define TOSMUTEX_H


#ifdef rt_LIB_Win32_i386
  #include <windows.h>
#else
  #include <pthread.h>
#endif

#if defined(rt_LIB_Solaris2_SPARC)
  #include <thread.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif


/*== Data type for mutex ==================================================*/

#ifdef rt_LIB_Win32_i386
  #define tosMutex_T HANDLE
#else
  #define tosMutex_T pthread_mutex_t
#endif


/*== Init / Destroy =======================================================*/

extern int tosMutex_init   (tosMutex_T *mx);
extern int tosMutex_destroy(tosMutex_T *mx);


/*== Locking ==============================================================*/

extern int tosMutex_lock   (tosMutex_T *mx);
extern int tosMutex_trylock(tosMutex_T *mx);
extern int tosMutex_unlock (tosMutex_T *mx);


#ifdef __cplusplus
}
#endif

#endif /* TOSMUTEX_H */

