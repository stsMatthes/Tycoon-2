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
  tosStdio.c 1.0 final  06-OCT-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Standard file I/O for TSP / TVM.

  You can also use <stdio.h>, but than you can go into trouble
  whith future filename-conventions or default open-modes.
*/

#include <stdio.h>
#include <errno.h>

#ifdef rt_LIB_Win32_i386
  #include <io.h>
  #include <windows.h>
#else
  #include <unistd.h>
#endif


#include "tosStdio.h"
#include "tos.h"
#include "tosLog.h"
#include "tosError.h"
#include "tosFilename.h"


/*== Open / Close =========================================================*/

tosStdio_T *tosStdio_open(const char *szFileName, const char *szMode)
{
  tosStdio_T *res;
  char systemFileName[tosFilename_MAXLEN];

  tosFilename_normalize(szFileName, systemFileName, tosFilename_MAXLEN);
  res = fopen(systemFileName, szMode);

  #ifdef tosStdio_DEBUG
     tosLog_debug("tosStdio",
                  "open",
                  "Open file, name=%s, mode=%s, fileno=%d, err=%d",
                  systemFileName,
                  szMode,
                  fileno(res),
                  tosError_getCode());
  #endif
  return res;
}


/*== Positioning ==========================================================*/

int tosStdio_truncate(tosStdio_T *file, int truncPos)
{
  int res = 0;

#ifdef rt_LIB_Win32_i386

  HANDLE hFile;
  BOOL   rc;

  fflush(file);
  fseek(file, truncPos, SEEK_SET);

  if ((hFile = (HANDLE)_get_osfhandle(fileno(file))) != (HANDLE)-1) {
     rc = SetEndOfFile(hFile);
     if (!rc)
        res = EWIN32API;
  }

#else
  res = ftruncate(fileno(file), truncPos);
#endif

  if (res) { errno = res; res = -1; }

  #ifdef tosStdio_DEBUG
     tosLog_debug("tosStdio",
                  "truncate",
                  "Truncating file, fileno=%d, pos=%d, res=(%d:%d:%d)",
                  fileno(file),
                  truncPos,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif

  return res;
}

