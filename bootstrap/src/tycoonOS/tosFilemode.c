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
  tosFilemode.c 1.0 final  07-OCT-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Runtime support for file-mode informations.
*/

#include <sys/types.h>
#include <sys/stat.h>

#include "tosFilemode.h"
#include "tos.h"
#include "tosLog.h"
#include "tosError.h"
#include "tosFilename.h"

#ifdef rt_LIB_Win32_i386
#include <io.h>
#endif

/*== Get / set the file mode ==============================================*/

int tosFilemode_get(char *pszPath, tosFilemode_T *mode)
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

  if (res == 0)
     *mode = buf.st_mode;
  else
     *mode = 0;

  #ifdef tosFilemode_DEBUG
     tosLog_debug("tosFilemode",
                  "get",
                  "Getting file mode for %s, mode=%d, res=(%d:%d:%d)",
                  systemPath,
                  *mode,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}


int tosFilemode_set(char *pszPath, tosFilemode_T mode)
{
  int res;
  char systemPath[tosFilename_MAXLEN];

  tosFilename_normalize(pszPath, systemPath, tosFilename_MAXLEN);
  res = chmod(systemPath, mode);

  #ifdef tosFilemode_DEBUG
     tosLog_debug("tosFilemode",
                  "set",
                  "Setting file mode for %s, mode=%d, res=(%d:%d:%d)",
                  systemPath,
                  mode,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}

