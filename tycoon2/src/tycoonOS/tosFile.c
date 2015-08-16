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
  tosFile.c 1.0 final  07-OCT-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Runtime support for filesystem operations.
*/

#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef rt_LIB_Win32_i386
  #include <io.h>
  #include <windows.h>
#else
  #include <unistd.h>
#endif

#include "tosFile.h"
#include "tos.h"
#include "tosLog.h"
#include "tosError.h"
#include "tosSystem.h"
#include "tosFilename.h"
#include "tosFilemode.h"


/*== Standard files =======================================================*/

#if !defined(__BORLANDC__)
#if !defined(STDIN_FILENO)
  #define STDIN_FILENO  fileno(stdin)
#endif
#if !defined(STDOUT_FILENO)
  #define STDOUT_FILENO fileno(stdout)
#endif
#if !defined(STDERR_FILENO)
  #define STDERR_FILENO fileno(stderr)
#endif
#else
#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
#endif

int tosFile_stdin(void)  { return STDIN_FILENO;  }
int tosFile_stdout(void) { return STDOUT_FILENO; }
int tosFile_stderr(void) { return STDERR_FILENO; }


/*== Open / Close =========================================================*/

int tosFile_open(char *pszPath, int flags)
{
  int res;
  int os_flags = 0;
  tosFilemode_T os_mode = tosFilemode_DEFAULT;
  char systemPath[tosFilename_MAXLEN];

  if (flags & TYC_O_RDONLY) os_flags |= O_RDONLY;
  if (flags & TYC_O_WRONLY) os_flags |= O_WRONLY;
  if (flags & TYC_O_RDWR  ) os_flags |= O_RDWR  ;
  if (flags & TYC_O_APPEND) os_flags |= O_APPEND;
  if (flags & TYC_O_CREAT ) os_flags |= O_CREAT ;
  if (flags & TYC_O_EXCL  ) os_flags |= O_EXCL  ;
  if (flags & TYC_O_TRUNC ) os_flags |= O_TRUNC ;

  tosFilename_normalize(pszPath, systemPath, tosFilename_MAXLEN);

#ifdef rt_LIB_Win32_i386
  os_flags |= O_BINARY;
  if (flags & TYC_O_SYNC)
     tosLog_error("tosFile",
                  "open",
                  "O_SYNC file mode not implemented on Windows NT");
#elif rt_LIB_Darwin_PPC
  if (flags & TYC_O_SYNC  ) os_flags |= O_FSYNC  ;
#else
  if (flags & TYC_O_SYNC  ) os_flags |= O_SYNC  ;
#endif

  res = open(systemPath, os_flags, os_mode);

  #ifdef tosFile_DEBUG
     tosLog_debug("tosFile",
                  "open",
                  "Open file %s, handle=%d, res=(%d:%d)",
                  systemPath,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}


int tosFile_sync(int fd)
{
#ifdef rt_LIB_Win32_i386
  int res = _commit(fd);
#else
  int res = fsync(fd);
#endif

  #ifdef tosFile_DEBUG
     tosLog_debug("tosFile",
                  "sync",
                  "Synchronize file, handle=%d, res=(%d:%d:%d)",
                  fd,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}


int tosFile_close(int fd)
{
  int res = close(fd);
  #ifdef tosFile_DEBUG
     tosLog_debug("tosFile",
                  "close",
                  "Close file, handle=%d, res=(%d:%d:%d)",
                  fd,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}


/*== File I/O =============================================================*/


int tosFile_read1(int fd, void * pBuffer, int start, int n)
{
  return read(fd, ((char*)pBuffer)+start, n);
}


int tosFile_write1(int fd, const void * pBuffer, int start, int n)
{
  /*
   * Clear errno in case not all bytes can be written and
   * errno is not set by the systems 'write' function
   */

  int result;
  tosError_reset();
  result = write(fd, ((const char *)pBuffer)+start, n);

  if (result != n && result >= 0) {
    return -1;
  }
  return result;
}


int tosFile_readChar(int fd)
{
  /*
   * To distinguish error and eof conditions:
   * return -1 on error or eof. set errno to 0 if eof.
   */

  char buf[1];
  int result = tosFile_read1(fd, buf, 0, 1);
  if (result <= 0) {
    if (result == 0) tosError_reset();
    return -1;
  }
  return (int) (unsigned char) buf[0];
}


int tosFile_writeChar(int fd, char ch)
{
  char buf[1];
  buf[0] = ch;
  return tosFile_write1(fd, buf, 0, 1);
}


/*== File information =====================================================*/

Bool tosFile_exists(char *pszPath)
{
  int res;
  char systemPath[tosFilename_MAXLEN];

#ifdef rt_LIB_Win32_i386
  struct _stat buf;
  tosFilename_normalize(pszPath, systemPath, tosFilename_MAXLEN);
  res = _stat(systemPath, &buf);
#else
  struct stat buf;
  tosFilename_normalize(pszPath, systemPath, tosFilename_MAXLEN);
  res = stat(systemPath, &buf);
#endif

  #ifdef tosFile_DEBUG
     tosLog_debug("tosFile",
                  "exists",
                  "Check existence of %s, res=(%d:%d:%d)",
                  systemPath,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  if (res == 0)
     return TRUE;
  else
     return FALSE;
}


Bool tosFile_isCharacterDevice(int fd)
{
  if (isatty(fd))
     return TRUE;
  else
     return FALSE;
}


/*== File operations ======================================================*/

int tosFile_copy(char *from, char *to)
{
  int  res;
  char systemPathFrom[tosFilename_MAXLEN];
  char systemPathTo  [tosFilename_MAXLEN];

#ifdef rt_LIB_Win32_i386
  BOOL rc;
#else
  FILE *ifp, *ofp;
  int c;
#endif

  tosFilename_normalize(from, systemPathFrom, tosFilename_MAXLEN);
  tosFilename_normalize(to,   systemPathTo  , tosFilename_MAXLEN);

#ifdef rt_LIB_Win32_i386
  /* Copy file in overwrite mode */
  rc = CopyFile(systemPathFrom, systemPathTo, FALSE);
  if (!rc) {
     errno = EWIN32API;
     res   = -1;
  }
#else
  ifp = fopen(systemPathFrom, "r");
  if (!ifp) 
     res = -1;
  else {
     ofp = fopen(systemPathTo, "w");
     if (!ofp) {
        fclose(ifp);
        res = -1;
     }
     else {
        while((c = getc(ifp)) != EOF)
           putc(c, ofp);
        if (fclose(ifp) == EOF) {
           fclose(ofp);
           res = -1;
        }
        else
           if (fclose(ofp) == EOF)
              res = -1;
           else
              res = 0;
     }
  }
#endif

  #ifdef tosFile_DEBUG
     tosLog_debug("tosFile",
                  "copy",
                  "Copy file %s to %s, res=(%d:%d:%d)",
                  systemPathFrom,
                  systemPathTo,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}


int tosFile_rename(char *from, char *to)
{
  int  res;
  char systemPathFrom[tosFilename_MAXLEN];
  char systemPathTo  [tosFilename_MAXLEN];
#ifdef rt_LIB_Win32_i386
  BOOL rc;
#endif

  tosFilename_normalize(from, systemPathFrom, tosFilename_MAXLEN);
  tosFilename_normalize(to,   systemPathTo  , tosFilename_MAXLEN);

#ifdef rt_LIB_Win32_i386
  /* Unix-Semantics: Overwrite, copy over disc drives */
  if (tosSystem_getID() == tosSystem_ID_WINNT)
     rc = MoveFileEx(systemPathFrom, systemPathTo,
                       MOVEFILE_COPY_ALLOWED
                     + MOVEFILE_REPLACE_EXISTING
                     + MOVEFILE_WRITE_THROUGH);
  else {
     /* Au weia, ... */
     rc = DeleteFile(systemPathTo);
     rc = MoveFile(systemPathFrom, systemPathTo);
  }

  if (rc) {
     res = 0;
  } else {
     errno = EWIN32API;
     res   = -1;
  }
#else
  res = rename(systemPathFrom, systemPathTo);
#endif

  #ifdef tosFile_DEBUG
     tosLog_debug("tosFile",
                  "rename",
                  "Rename file %s to %s, res=(%d:%d:%d)",
                  systemPathFrom,
                  systemPathTo,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}


int tosFile_remove(char *path)
{
  int  res;
  char systemPath[tosFilename_MAXLEN];

  tosFilename_normalize(path, systemPath, tosFilename_MAXLEN);
  res = remove(systemPath);

  #ifdef tosFile_DEBUG
     tosLog_debug("tosFile",
                  "remove",
                  "Remove file %s, res=(%d:%d:%d)",
                  systemPath,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}


/*== Symbolic links =======================================================*/

int tosFile_mkSymLink(char *name1, char *name2)
{
  int  res;
  char systemName1[tosFilename_MAXLEN];
  char systemName2[tosFilename_MAXLEN];

  tosFilename_normalize(name1, systemName1, tosFilename_MAXLEN);
  tosFilename_normalize(name2, systemName2, tosFilename_MAXLEN);

#ifdef rt_LIB_Win32_i386
  /* Waiting until Windows 2000 */
  tosLog_error("tosFile",
               "mkSymLink",
               "Symbolic links not implemented, wait until NT 5.0");

  SetLastError(120); /* not implemented */
  errno = EWIN32API;
  res   = -1;
#else
  res = symlink(systemName1, systemName2);
#endif

  #ifdef tosFile_DEBUG
     tosLog_debug("tosFile",
                  "mkSymLink",
                  "Create symolic link from %s to %s, res=(%d:%d:%d)",
                  systemName1,
                  systemName2,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}

