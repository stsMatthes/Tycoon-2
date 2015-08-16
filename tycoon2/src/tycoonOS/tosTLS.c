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
  tosTLS.c 1.0 final  08-AUG-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Management of dynamic, thread local storage (TLS).
*/


#include <errno.h>
#include <stdlib.h>

#ifdef rt_LIB_Win32_i386
  #include <windows.h>
#else
  #include <pthread.h>
#endif

#if defined(rt_LIB_Solaris2_SPARC)
  #include <thread.h>
#endif


#include "tosTLS.h"
#include "tos.h"
#include "tosLog.h"
#include "tosError.h"


/*== Key management =======================================================*/

int tosTLS_keyCreate(tosTLS_T *key)
{
  int res = 0;
  
  #ifdef rt_LIB_Win32_i386
     *key = TlsAlloc();
     if (*key == TLS_OUT_OF_INDEXES)
        res = EWIN32API;
  #else
     #if defined(rt_LIB_HPUX_PARISC) && defined(tmthread_DRAFT_INTERFACE)
         res = pthread_keycreate(key, NULL);
     #else
         res = pthread_key_create(key, NULL);
     #endif
  #endif

  if (res) { errno = res; res = -1; }

  #ifdef tosTLS_DEBUG
     tosLog_debug("tosTLS",
                  "keyCreate",
                  "Create thread local store, key=%u, res=(%d:%d:%d)",
                   (unsigned) *key,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}


int tosTLS_keyDelete(tosTLS_T key)
{
  int res = 0;
  
  #ifdef rt_LIB_Win32_i386
     BOOL rc = TlsFree(key);
     if (!rc)
        res = EWIN32API;
  #else
     res = pthread_key_delete(key);
  #endif

  if (res) { errno = res; res = -1; }

  #ifdef tosTLS_DEBUG
     tosLog_debug("tosTLS",
                  "keyDelete",
                  "Delete thread local store, key=%u, res=(%d:%d:%d)",
                  (unsigned) key,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}


/*== Value management =====================================================*/

int tosTLS_setValue(tosTLS_T key, void * val)
{
  int res = 0;
  
  #ifdef rt_LIB_Win32_i386
     BOOL rc = TlsSetValue(key, val);
     if (!rc)
        res = EWIN32API;
  #else
     res = pthread_setspecific(key, val);
  #endif

  if (res) { errno = res; res = -1; }

  #ifdef tosTLS_DEBUG
     tosLog_debug("tosTLS",
                  "setValue",
                  "Set thread local store value, key=%u, value=%u, res=(%d:%d:%d)",
                  (unsigned) key,
                  (unsigned) val,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}


void * tosTLS_getValue(tosTLS_T key)
{
  int res = 0;
  
  #ifdef rt_LIB_Win32_i386
     LPVOID val = TlsGetValue(key);
     if (val == NULL) {
        /* valid NULL value ? */
        if (GetLastError() != NO_ERROR)
           res = EWIN32API;
     }
  #else
     void * val = NULL;
     #if defined(rt_LIB_HPUX_PARISC) && defined(tmthread_DRAFT_INTERFACE)
        res = pthread_getspecific(key, &val);
     #else
        val = pthread_getspecific(key);
     #endif
        
  #endif

  if (res) { errno = res; val = NULL; }

  #ifdef tosTLS_DEBUG
     tosLog_debug("tosTLS",
                  "getValue",
                  "Get thread local store value, key=%u, value=%u, res=(%d:%d:%d)",
                  (unsigned) key,
                  (unsigned) val,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return val;
}

