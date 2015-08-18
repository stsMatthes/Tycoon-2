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
  tosFilename.c 1.0 final  30-JUL-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  System independent filename support.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tos.h"
#include "tosLog.h"
#include "tosError.h"
#include "tosString.h"
#include "tosFilename.h"


static void tosFilename_getSlashDotLen(const char * pszFilename,
                                              int * pSlash,
                                              int * pDot,
                                              int * pLen)
{
  int slash, dot, len;

  len = strlen(pszFilename);
  slash = len;
  while ((pszFilename[slash] != '\\') &&
         (pszFilename[slash] != '/')  &&
         (pszFilename[slash] != ':')  &&
         (slash != 0))
    slash--;

  /* First dot beginning from delimiter */
  dot = slash;
  do {
     dot++;
  }
  while ((pszFilename[dot] != '.') && (dot != len));

  *pSlash = slash;
  *pDot   = dot;
  *pLen   = len;
}


/*== Parsing Components ===================================================*/

void tosFilename_split(const char * pszFilename,
                             char * pszPath,
                             char * pszName,
                             char * pszExtension)
{
  int slash, dot, len;

  if (pszPath      != NULL) *pszPath      = '\0';
  if (pszName      != NULL) *pszName      = '\0';
  if (pszExtension != NULL) *pszExtension = '\0';

  tosFilename_getSlashDotLen(pszFilename, &slash, &dot, &len);

  if ((pszFilename[slash] == '\\')  ||
      (pszFilename[slash] == '/' )  ||
      (pszFilename[slash] == ':' ))
  {
     if (pszPath != NULL)
        tosString_extract(pszPath, 0, slash+1, pszFilename);
     if (pszName != NULL)
        tosString_extract(pszName, slash+1, dot-(slash-1), pszFilename);
  }
  else
     if (pszName != NULL)
        tosString_extract(pszName, 0, dot-slash, pszFilename);

  if ((pszFilename[dot] == '.') && (pszExtension != NULL))
     tosString_extract(pszExtension, dot+1, len-dot, pszFilename);

  #ifdef tosFilename_DEBUG
     tosLog_debug("tosFilename", "split", "Filename=%s", pszFilename);
     if (pszPath != NULL)
        tosLog_debug("tosFilename", "split", "-> path=%s", pszPath);
     if (pszName != NULL)
        tosLog_debug("tosFilename", "split", "-> name=%s", pszName);
     if (pszExtension != NULL)
        tosLog_debug("tosFilename", "split", "-> extension=%s", pszExtension);
  #endif
}


void tosFilename_getPath(const char * pszFilename, char * pszPath)
{
  tosFilename_split(pszFilename, pszPath, NULL, NULL);
}

void tosFilename_getName(const char * pszFilename, char * pszName)
{
  tosFilename_split(pszFilename, NULL, pszName, NULL);
}

void tosFilename_getExtension(const char * pszFilename, char * pszExtension)
{
  tosFilename_split(pszFilename, NULL, NULL, pszExtension);
}


/*== Composing Filenames ==================================================*/

void tosFilename_new(char * pszFilename,
                     const char * pszPath,
                     const char * pszName,
                     const char * pszExtension)
{
  int len;
  strcpy(pszFilename, pszPath);
  len = strlen(pszFilename);
  if ((len > 0) &&
      (pszFilename[len-1] != '\\') &&
      (pszFilename[len-1] != '/')  &&
      (pszFilename[len-1] != ':'))
     strcat(pszFilename, "/");

  strcat(pszFilename, pszName);
  if (pszExtension[0] != '\0') {
     strcat(pszFilename, ".");
     strcat(pszFilename, pszExtension);
  }

  #ifdef tosFilename_DEBUG
     tosLog_debug("tosFilename",
                  "new",
                  "p=%s, n=%s, e=%s -> %s",
                  pszPath, pszName, pszExtension, pszFilename);
  #endif
}


Bool tosFilename_setPath(char * pszFilename,
                         const char * pszPath)
{
  char p[tosFilename_MAXLEN];
  char n[tosFilename_MAXLEN];
  char e[tosFilename_MAXLEN];
  Bool change;

  tosFilename_split(pszFilename, p, n, e);
  change = (strcmp(p, pszPath) != 0);
  if (change)
     tosFilename_new(pszFilename, pszPath, n, e);

  return change;
}


Bool tosFilename_setName(char * pszFilename,
                         const char * pszName)
{
  char p[tosFilename_MAXLEN];
  char n[tosFilename_MAXLEN];
  char e[tosFilename_MAXLEN];
  Bool change;

  tosFilename_split(pszFilename, p, n, e);
  change = (strcmp(n, pszName) != 0);
  if (change)
     tosFilename_new(pszFilename, p, pszName, e);

  return change;
}


Bool tosFilename_setExtension(char * pszFilename,
                              const char * pszExtension)
{
  char p[tosFilename_MAXLEN];
  char n[tosFilename_MAXLEN];
  char e[tosFilename_MAXLEN];
  Bool change;

  tosFilename_split(pszFilename, p, n, e);
  change = (strcmp(e, pszExtension) != 0);
  if (change)
     tosFilename_new(pszFilename, p, n, pszExtension);

  return change;
}


/*== Conversion ===========================================================*/

void tosFilename_toWin(const char * pszUnixFilename,
                             char * pszWinFilename,
                             int    size)
{
  int i = 0;
  while (pszUnixFilename[i]) {
     if (i == size) break;
     pszWinFilename[i] = pszUnixFilename[i];
     if (pszWinFilename[i] == '/') pszWinFilename[i] = '\\';
        i++;
  }
  pszWinFilename[i] = 0;

  #ifdef tosFilename_DEBUG
     tosLog_debug("tosFilename",
                  "toWin",
                  "%s -> %s",
                  pszUnixFilename, pszWinFilename);
  #endif
}


void tosFilename_toUnix(const char * pszWinFilename,
                              char * pszUnixFilename,
                              int    size)
{
  int i = 0;
  while (pszWinFilename[i]) {
     if (i == size) break;
     pszUnixFilename[i] = pszWinFilename[i];
     if (pszUnixFilename[i] == '\\') pszUnixFilename[i] = '/';
        i++;
  }
  pszUnixFilename[i] = 0;

  #ifdef tosFilename_DEBUG
     tosLog_debug("tosFilename",
                  "toUnix",
                  "%s -> %s",
                  pszWinFilename, pszUnixFilename);
  #endif
}


void tosFilename_normalize(const char * pszFilename,
                                 char * pszSystemFilename,
                                 int    size)
{
#ifdef rt_LIB_Win32_i386
  tosFilename_toWin(pszFilename, pszSystemFilename, size);
#else
  tosFilename_toUnix(pszFilename, pszSystemFilename, size);
#endif
}


/*== Temporary filenames ==================================================*/

int tosFilename_tempName(const char *dir, 
                         const char *pfx,
                               char *tmpName,
                               int   size)
{
  int   res = 0;
  char  systemDir[tosFilename_MAXLEN];
  char  systemPfx[tosFilename_MAXLEN];
  char *systemRes;

  *tmpName = 0;  
  tosFilename_normalize(dir, systemDir, tosFilename_MAXLEN);
  tosFilename_normalize(pfx, systemPfx, tosFilename_MAXLEN);

  /* tempnam conforming to SVID 3, BSD 4.3 */
#ifdef rt_LIB_Win32_i386
  systemRes = _tempnam(dir, pfx);
#else
  systemRes = tempnam(dir, pfx);
#endif

  if (systemRes) {
     strncpy(tmpName, systemRes, size);
     systemRes[size-1] = 0;
     free(systemRes);
  }
  else res = -1;

  #ifdef tosFilename_DEBUG
     tosLog_debug("tosFilename",
                  "tempName",
                  "Generating temporary filename %s, res=(%d:%d:%d)",
                  systemRes,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}


int tosFilename_tempNameSize(void)
{
  return L_tmpnam;
}

