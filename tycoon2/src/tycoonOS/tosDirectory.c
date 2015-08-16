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
  tosDirectory.c 1.0 final  07-OCT-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Runtime support for directories.
*/

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef rt_LIB_Win32_i386
  #include <windows.h>
  #include <direct.h>
#else
  #include <unistd.h>
  #include <dirent.h>
  #include <sys/param.h>
  #include <sys/unistd.h>
#endif


#include "tosDirectory.h"
#include "tos.h"
#include "tosLog.h"
#include "tosError.h"
#include "tosString.h"
#include "tosFilename.h"


int tosDirectory_realpath(char *pszPath,
                          char *pszResolvedPath,
                          int   n)
  /*
   * Return 0 on success, or -1 on failure.
   * On failiure, pszResolvedPath contains the name of the offending
   * file object.
   */
{
  int   res = 0;
  char *pszResult;
  char  systemPath[tosFilename_MAXLEN];

  if (n < tosFilename_MAXLEN) {
     errno = ERANGE;
     res   = -1;
  }
  else {
     tosFilename_normalize(pszPath, systemPath, tosFilename_MAXLEN);
#ifdef rt_LIB_Win32_i386
     pszResult = _fullpath(pszResolvedPath, systemPath, tosFilename_MAXLEN);
#else
     pszResult = realpath(systemPath, pszResolvedPath);
#endif
     if (pszResult == NULL) res = -1;
  }

  #ifdef tosDirectory_DEBUG
     tosLog_debug("tosDirectory",
                  "realpath",
                  "Expand path %s to %s, res=(%d:%d:%d)",
                  systemPath,
                  pszResolvedPath,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;                  
}


int tosDirectory_chroot(char *pszRoot)
{
  int res = 0;
  char systemPath[tosFilename_MAXLEN];

  tosFilename_normalize(pszRoot, systemPath, tosFilename_MAXLEN);

#ifdef rt_LIB_Win32_i386
  tosLog_error("tosDirectory",
               "chroot",
               "Not implemented on Win32 API");

  SetLastError(120); /* not implemented */
  errno = EWIN32API;
  res   = -1;

#else
  res = chroot(systemPath);
#endif

  #ifdef tosDirectory_DEBUG
     tosLog_debug("tosDirectory",
                  "chroot",
                  "Change the root directory to %s, res=(%d:%d:%d)",
                  systemPath,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}


int tosDirectory_create(char *pszPath)
{
  int res;
  char systemPath[tosFilename_MAXLEN];
  tosFilename_normalize(pszPath, systemPath, tosFilename_MAXLEN);

#ifdef rt_LIB_Win32_i386
  res = _mkdir(systemPath);
#else
  res = mkdir(systemPath,
              (S_ISUID | S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH));
#endif

  #ifdef tosDirectory_DEBUG
     tosLog_debug("tosDirectory",
                  "create",
                  "Create directory %s, res=(%d:%d:%d)",
                  systemPath,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}


int tosDirectory_remove(char *pszPath)
{
  int res;
  char systemPath[tosFilename_MAXLEN];

  tosFilename_normalize(pszPath, systemPath, tosFilename_MAXLEN);

#ifdef rt_LIB_Win32_i386
  res = _rmdir(systemPath);
#else
  res = rmdir(systemPath);
#endif

  #ifdef tosDirectory_DEBUG
     tosLog_debug("tosDirectory",
                  "remove",
                  "Remove directory %s, res=(%d:%d:%d)",
                  systemPath,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}


/*== Directory streams ====================================================*/

int tosDirectory_sizeOfT(void) {
#ifdef rt_LIB_Win32_i386
   return sizeof(tosDirectory_T);
#else
   return sizeof(tosDirectory_T) + MAXNAMLEN + 1;
#endif
}

int tosDirectory_open(tosDirectory_T *pDir, char *dirName)
{
  int res = 0;

#ifdef rt_LIB_Win32_i386
  struct _stat buf;

  if (pDir == NULL) {
     errno = EINVAL;
     res   = -1;
  }
  else {
     pDir->searchHandle = 0;
     
     tosFilename_normalize(dirName, pDir->searchPath, tosFilename_MAXLEN);
     if (_stat(pDir->searchPath, &buf) < 0) {
        res = -1;
     }
     else {
        if ((buf.st_mode & S_IFMT) != S_IFDIR) {
           errno = ENOTDIR;
           res   = -1;
        }
        else {
           if (pDir->searchPath[strlen(pDir->searchPath)-1] == '\\')
              strcat(pDir->searchPath, "*");
           else
              strcat(pDir->searchPath, "\\*");
        }
     }
  }

#else

  if (pDir == NULL) {
     errno = EINVAL;
     res   = -1;
  }
  else {
     tosFilename_normalize(dirName, pDir->searchPath, tosFilename_MAXLEN);
     pDir->searchHandle = opendir(dirName);
     if (!(pDir->searchHandle)) res = -1;
  }
#endif

  #ifdef tosDirectory_DEBUG
     tosLog_debug("tosDirectory",
                  "open",
                  "Open search handle for path %s, handle=%d, res=(%d:%d:%d)",
                  pDir->searchPath,
                  pDir->searchHandle,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}


static int __readDir(tosDirectory_T *pDir, char **foundFile)
{
  int res = 0;
 
#ifdef rt_LIB_Win32_i386
  *foundFile = NULL;
  if (pDir->searchHandle == NULL) {
    pDir->searchHandle = FindFirstFile(pDir->searchPath, &(pDir->lastEntry));
    if (pDir->searchHandle == INVALID_HANDLE_VALUE)
       res = -1;
  }
  else
    if (FindNextFile(pDir->searchHandle, &(pDir->lastEntry)) != 1)
       res = -1;

  /* ERROR_NO_MORE_FILES is not an error ... */
  if ((res != 0) && (tosError_getCodeDetail() != ERROR_NO_MORE_FILES))
     errno = EWIN32API;
  else
     *foundFile = tosString_strdup(pDir->lastEntry.cFileName);

#else

  #if defined(rt_LIB_Linux_i386) || defined(_POSIX_PTHREAD_SEMANTICS)
      struct dirent * pp;
      *foundFile = NULL;
      res = readdir_r(pDir->searchHandle, &(pDir->lastEntry), &pp);
      if ((res == 0) && (pp != &(pDir->lastEntry)))
         res = -1;
      if ((res == 0) && (pDir->lastEntry.d_name != NULL))
         *foundFile = tosString_strdup(pDir->lastEntry.d_name);

  #elif defined (rt_LIB_HPUX_PARISC) && defined(tmthread_DRAFT_INTERFACE)
      *foundFile = NULL;
      res = readdir_r(pDir->searchHandle, &(pDir->lastEntry));
      if ((res == 0) && (pDir->lastEntry.d_name != NULL))
         *foundFile = tosString_strdup(pDir->lastEntry.d_name);

  #else
      int olderrno = errno;
      errno = 0;
      *foundFile = NULL;
      if (readdir_r(pDir->searchHandle, &(pDir->lastEntry)) == NULL) {
         if (errno != 0)
            res = -1;
         else
            errno = olderrno;
      }
      else {
         errno = olderrno;
         *foundFile = tosString_strdup(pDir->lastEntry.d_name);
      }
  #endif
#endif

  return res;
}


char * tosDirectory_read(tosDirectory_T *pDir)
{
  int   res = 0;
  char *foundFile;
  do
    {
        res = __readDir(pDir, &foundFile);
        if (foundFile == NULL) {
           #ifdef tosDirectory_DEBUG
              if (res == 0)
                 tosLog_debug("tosDirectory",
                              "read",
                              "No more entries, handle=%d",
                              pDir->searchHandle);
              else
                 tosLog_debug("tosDirectory",
                              "read",
                              "Return with error, handle=%d, res=(%d:%d:%d)",
                              pDir->searchHandle,
                              res,
                              tosError_getCode(),
                              tosError_getCodeDetail());
           #endif
           return NULL;
        }
    }
  while((strcmp(foundFile, ".") == 0) || (strcmp(foundFile, "..") == 0));

  #ifdef tosDirectory_DEBUG
     tosLog_debug("tosDirectory",
                  "read",
                  "Found entry %s, path=%s, handle=%d, res=(%d:%d:%d)",
                  foundFile,
                  pDir->searchPath,
                  pDir->searchHandle,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return foundFile;
}


int tosDirectory_close(tosDirectory_T *pDir)
{
  int res = 0;

#ifdef rt_LIB_Win32_i386
  BOOL rc = 0;
  if (pDir != NULL) {
     if ((pDir->searchHandle != NULL) && (pDir->searchHandle != INVALID_HANDLE_VALUE))
        rc = FindClose(pDir->searchHandle);
     if (!rc) {
        res   = -1;
        errno = EWIN32API;
     }
  }
#else
  res = closedir(pDir->searchHandle);
#endif

  #ifdef tosDirectory_DEBUG
     tosLog_debug("tosDirectory",
                  "close",
                  "Close search handle for %s, handle=%d, res=(%d:%d:%d)",
                  pDir->searchPath,
                  pDir->searchHandle,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}

