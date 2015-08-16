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
  tosDirectory.h 1.0 final  07-OCT-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Runtime support for directories.
*/

#ifndef TOSDIRECTORY_H
#define TOSDIRECTORY_H


#include "tos.h"


#ifdef rt_LIB_Win32_i386

  #include <windows.h>

  typedef struct {
            char            searchPath[_MAX_PATH];
            HANDLE          searchHandle;
            WIN32_FIND_DATA lastEntry;
          } tosDirectory_T;

#else

  #include <dirent.h>

  typedef struct {
            char   searchPath[MAXNAMLEN];
            DIR   *searchHandle;
            struct dirent lastEntry;
          } tosDirectory_T;

#endif


#ifdef __cplusplus
extern "C" {
#endif


extern int tosDirectory_realpath(char *pszPath,
                                 char *pszResolvedPath,
                                 int   n);

extern int tosDirectory_chroot(char *pszRoot); /* Only on Unix */

extern int tosDirectory_create(char *pszPath);
extern int tosDirectory_remove(char *pszPath);


/*== Directory streams ====================================================*/

extern int   tosDirectory_sizeOfT(void);
extern int   tosDirectory_open   (tosDirectory_T *pDir, char *dirName);
extern char *tosDirectory_read   (tosDirectory_T *pDir);
extern int   tosDirectory_close  (tosDirectory_T *pDir);


#ifdef __cplusplus
}
#endif

#endif /* TOSDIRECTORY_H */
