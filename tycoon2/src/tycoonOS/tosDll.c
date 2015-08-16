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
  tosDll.c 1.0 final  30-JUL-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Basic Dynamic Link Library Support
*/

#include <string.h>
#include <errno.h>

#include "tos.h"
#include "tosDll.h"
#include "tosLog.h"
#include "tosError.h"
#include "tosFilename.h"


/*== OS DEFINES ===========================================================*/

#if   defined(rt_LIB_Win32_i386)

    #include <windows.h>

    #define tosDll_EXTENSION           "dll"
    #define tosDll_HANDLE              HINSTANCE
    #define tosDll_NIL                 0
    #define tosDll_OPEN(dll)           LoadLibrary(dll)
    #define tosDll_CLOSE(dll)          FreeLibrary((HMODULE)dll)
    #define tosDll_LOOKUP(s, dll, id)  s = GetProcAddress(dll, id);
    #define tosDll_ERROR(fun)          errno = EWIN32API; \
                                       tosError_log(tosError_getCode(), \
                                                    tosError_getCodeDetail());
#if defined(_MSC_VER)
    #define tosDll_STD_LIBC            "msvcrt"
#endif
#if defined(__BORLANDC__)
    #define tosDll_STD_LIBC            "cc3250mt"
#endif

#elif defined(rt_LIB_HPUX_PARISC)

    #include <dl.h>

    #define tosDll_EXTENSION           "so"
    #define tosDll_HANDLE              shl_t
    #define tosDll_NIL                 NULL
    #define tosDll_OPEN(dll)           shl_load(dll, BIND_DEFERRED, 0)
    #define tosDll_CLOSE(dll)          shl_unload(dll)
    #define tosDll_LOOKUP(s, dll, id)  s = NULL; \
                                       if(shl_findsym(&dll, id, TYPE_PROCEDURE, &s)) \
                                         s = NULL;
    #define tosDll_ERROR(fun)
    #define tosDll_STD_LIBC            "libc"

#else

    #include <dlfcn.h>

    #define tosDll_EXTENSION           "so"
    #define tosDll_HANDLE              void *
    #define tosDll_NIL                 NULL
    #define tosDll_OPEN(dll)           dlopen(dll, RTLD_LAZY)
    #define tosDll_CLOSE(dll)          dlclose(dll)
    #define tosDll_LOOKUP(s, dll, id)  s = dlsym(dll, id);
    #define tosDll_ERROR(fun)          char *msg = dlerror(); \
                                       if (msg) tosLog_error("tosDll", fun, msg)
    #define tosDll_STD_LIBC            "libc"

#endif


/*== Local utilities ======================================================*/


static void tosDll_convName(const char *pszFromPath, char *pszToPath, int n)
{
  char dllName[80];

  /* real name */
  tosFilename_normalize(pszFromPath, pszToPath, n);

  tosFilename_setExtension(pszToPath, tosDll_EXTENSION);
  tosFilename_getName(pszToPath, dllName);

  /* Aliasing of ANSI-C library name */
  if (strcmp(dllName, "libc") == 0) {
     tosFilename_setName(pszToPath, tosDll_STD_LIBC);
  }
}


/*== Open / Close =========================================================*/

void *tosDll_open(char *pszPath)
{
   tosDll_HANDLE h;
   char szRealName[tosFilename_MAXLEN];

   tosDll_convName(pszPath, szRealName, tosFilename_MAXLEN);
   h = tosDll_OPEN(szRealName);

   #ifdef tosDll_DEBUG
      tosLog_debug("tosDll",
                   "open",
                   "Open shared library, handle=%u, lib=%s",
                   (unsigned) h,
                   szRealName);
   #endif

   if (h == tosDll_NIL) {
      tosDll_ERROR("open");
      tosLog_error("tosDll", "open", "%s: Failed to open library", (char*)szRealName);
   }
   return h;
}


Int tosDll_close(void *hDLL)
{
   if (hDLL == tosDll_NIL)
      return 0;

   #ifdef tosDll_DEBUG
      tosLog_debug("tosDll",
                   "close",
                   "Close shared library, handle=%u",
                   (unsigned) hDLL);
   #endif
   return tosDll_CLOSE(hDLL);
}


/*== Lookup ===============================================================*/

void *tosDll_lookup(void *hDLL, char *pszSym)
{
   tosDll_HANDLE h = hDLL;
   void * sym;

   if (h == tosDll_NIL) {
      tosLog_error("tosDll", "lookup", "Library for symbol %s not open", pszSym);
      errno = EINVAL;
      return NULL;
   }

#if defined(__BORLANDC__)
   {
     char *pszUScSym;
     pszUScSym = malloc(strlen(pszSym)+2);
     if(pszUScSym == NULL) {
       tosLog_error("tosDll", "lookup", "cannot allocate %d bytes for _%s",
		    strlen(pszSym)+2, pszSym);
       return NULL;
     }
     pszUScSym[0] = '_';
     strcpy(pszUScSym+1, pszSym);
     pszSym = pszUScSym;
   }
#endif

   tosDll_LOOKUP(sym, h, pszSym);

   #ifdef tosDll_DEBUG
      tosLog_debug("tosDll",
                   "lookup",
                   "Lookup function in shared library, handle=%u, id=%s, procAdr=%x",
                   (unsigned) h, pszSym, sym);
   #endif

   if (!sym) {
      tosDll_ERROR("lookup");
      tosLog_error("tosDll", "lookup", "%s: lookup failed", pszSym);
   }

#if defined(__BORLANDC__)
   free(pszSym);
#endif

   return sym;
}

