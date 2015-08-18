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
  tosTLS.h 1.0 final  08-AUG-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Management of dynamic, thread local storage (TLS).
*/

#ifndef TOSTLS_H
#define TOSTLS_H


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


/*== Data type for local memory key =======================================*/

#ifdef rt_LIB_Win32_i386
  #define tosTLS_T DWORD
#else
  #define tosTLS_T pthread_key_t
#endif


/*== Key management =======================================================*/

/*
 * tosTLS_keyCreate
 * ----------------
 * Creates a new entry (key) in the thread specific data area for all
 * threads of the current process (initialized with NULL). Per-thread
 * values for this key can be set/retrieved with the setValue- / getValue-
 * functions.
 *
 * Return          - zero or -1 on error (see tosError_getCode)
 * Ressources      - allocates system specific ressources
 */

extern int tosTLS_keyCreate(tosTLS_T *key);


/*
 * tosTLS_keyDelete
 * ----------------
 * Deletes an entry (key) in the thread specific data area for all threads
 * of the current process. You have to cleanup any value holded by this
 * key, before call this function.
 *
 * Return          - zero or -1 on error (see tosError_getCode)
 * Ressources      - frees system specific ressources
 */

extern int tosTLS_keyDelete(tosTLS_T key);



/*== Value management =====================================================*/

/*
 * tosTLS_setValue
 * ---------------
 * Sets the value of the thread specific data entry 'key' to the value 'val'.
 *
 * Return          - zero or -1 on error (see tosError_getCode)
 * Ressources      - references to Heap objects
 */

extern int tosTLS_setValue(tosTLS_T key, void *val);


/*
 * tosTLS_getValue
 * ---------------
 * Returns the value of the thread specific data entry associated with
 * 'key' in the calling thread. Don't use with undefined 'key' values.
 *
 * Return          - zero or -1 on error (see tosError_getCode)
 * Ressources      - references to Heap objects
 */

extern void * tosTLS_getValue(tosTLS_T key);


#ifdef __cplusplus
}
#endif

#endif /* TOSTLS_H */

