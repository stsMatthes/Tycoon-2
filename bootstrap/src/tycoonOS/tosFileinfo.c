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
  tosFileinfo.c 1.0 final  15-OCT-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Runtime support for file informations.
*/

#include <sys/types.h>
#include <sys/stat.h>

#include "tosFileinfo.h"
#include "tos.h"
#include "tosLog.h"
#include "tosError.h"
#include "tosFilename.h"


/*== Constants / Types ====================================================*/

Int tosFileinfo_bufferSize(void)
{
  return (int) sizeof(tosFileinfo_Buffer);
}


Int tosFileinfo_getBuffer(char *path, tosFileinfo_Buffer *buf)
{
  int res;
  char systemPath[tosFilename_MAXLEN];

  tosFilename_normalize(path, systemPath, tosFilename_MAXLEN);

#ifdef rt_LIB_Win32_i386
  res = _stati64(systemPath, buf);
#else
  res = stat(systemPath, buf);
#endif

  #ifdef tosFileinfo_DEBUG
     tosLog_debug("tosFileinfo",
                  "get",
                  "Getting fileinfo buffer for %s, res=(%d:%d:%d)",
                  systemPath,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}


/*== File informations ====================================================*/


Bool tosFileinfo_isDirectory(tosFileinfo_Buffer *buf)
{
  return (buf->st_mode & S_IFMT) == S_IFDIR ? TRUE : FALSE;
}

Bool tosFileinfo_isFile(tosFileinfo_Buffer *buf)
{
  return (buf->st_mode & S_IFMT) != S_IFDIR ? TRUE : FALSE;
}

Bool tosFileinfo_isSymbolicLink(tosFileinfo_Buffer *buf)
{
#ifdef rt_LIB_Win32_i386
  return FALSE;
#else
  return buf->st_mode == S_IFLNK ? TRUE : FALSE;
#endif
}

Long tosFileinfo_lastModified(tosFileinfo_Buffer *buf)
{
  return (Long) buf->st_mtime;
}

Long tosFileinfo_lastAccessed(tosFileinfo_Buffer *buf)
{
  return (Long) buf->st_atime;
}

Long tosFileinfo_lastStatusChange(tosFileinfo_Buffer *buf)
{
  return (Long) buf->st_ctime;
}

Long tosFileinfo_size(tosFileinfo_Buffer *buf)
{
  return (Long) buf->st_size;
}

