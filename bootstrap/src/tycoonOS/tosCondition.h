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
  tosCondition.h 1.0 final  30-JUL-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Condition variables
*/

#ifndef TOSCONDITION_H
#define TOSCONDITION_H


#if defined(rt_LIB_Win32_i386)
  #include <windows.h>
#else
  #include <pthread.h>
#endif

#if defined(rt_LIB_Solaris2_SPARC)
  #include <thread.h>
#endif

#include "tos.h"
#include "tosMutex.h"


#ifdef __cplusplus
extern "C" {
#endif


/*== Data type for condition variable =====================================*/

#ifdef rt_LIB_Win32_i386
  #define tosCondition_T HANDLE
#else
  #define tosCondition_T pthread_cond_t
#endif


/*
 *  Single    <: T
 *  Broadcast <: T
 */
typedef tosCondition_T tosCondition_Single;
typedef tosCondition_T tosCondition_Broadcast;


/*== Init / Destroy =======================================================*/

extern int tosCondition_initSingle   (tosCondition_Single    *cond);
extern int tosCondition_initBroadcast(tosCondition_Broadcast *cond);

extern int tosCondition_destroy(tosCondition_T *cond);


/*== Waiting / Sending ====================================================*/

extern int tosCondition_signal   (tosCondition_Single    *cond);
extern int tosCondition_broadcast(tosCondition_Broadcast *cond);

extern int tosCondition_wait(tosCondition_T *cond, tosMutex_T *mx);

extern int tosCondition_timedwait(tosCondition_T *cond,
                                  tosMutex_T     *mx,
                                  Long            mseconds);


#ifdef __cplusplus
}
#endif

#endif /* TOSCONDITION_H */
