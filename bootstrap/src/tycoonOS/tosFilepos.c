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
  tosFilepos.c 1.0 final  26-OCT-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Runtime support for file position pointer.
*/


#ifdef rt_LIB_Win32_i386
  #include <stdio.h>
  #include <io.h>
#else
  #include <sys/types.h>
  #include <unistd.h>
#endif

#include "tosFilepos.h"
#include "tos.h"
#include "tosLog.h"
#include "tosError.h"


static tosFilepos_T __tos_lseek(int fd, tosFilepos_T pos, int whence)
{
#if defined(rt_LIB_Win32_i386) && defined(_MSC_VER)
  return _lseeki64(fd, pos, whence);
  /* ### how to reach __llseek using BCC ? */
#else
  /* check for overflow: */
  if (((Long) ((long) pos)) != pos) {
     tosLog_error("tosFilepos", "__lseek", "File position pointer overflow");
     return (Long) -1;
  }
  return lseek(fd, (long) pos, whence);
#endif
}


tosFilepos_T tosFilepos_seek(int fd, tosFilepos_T pos)
{
  tosFilepos_T p = __tos_lseek(fd, pos, SEEK_SET);

  #ifdef tosFilepos_DEBUG
     tosLog_debug("tosFilepos",
                  "seek",
                  "Setting file pointer to absolute position %u, res=(%u:%d:%d)",
                  pos,
                  p,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return p;
}


tosFilepos_T tosFilepos_seekEnd(int fd, tosFilepos_T pos)
{
  tosFilepos_T p = __tos_lseek(fd, pos, SEEK_END);

  #ifdef tosFilepos_DEBUG
     tosLog_debug("tosFilepos",
                  "seekEnd",
                  "Setting file pointer relative to end of file, pos=%u, res=(%u:%d:%d)",
                  pos,
                  p,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return p;
}


tosFilepos_T tosFilepos_seekCur(int fd, tosFilepos_T pos)
{
  tosFilepos_T p = __tos_lseek(fd, pos, SEEK_CUR);

  #ifdef tosFilepos_DEBUG
     tosLog_debug("tosFilepos",
                  "seekCur",
                  "Setting file pointer relative to current, pos=%u, res=(%u:%d:%d)",
                  pos,
                  p,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return p;
}

