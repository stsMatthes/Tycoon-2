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
  tosFilename.h 1.0 final  30-JUL-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  System independent filename support.
*/

#ifndef TOSFILENAME_H
#define TOSFILENAME_H


#ifdef rt_LIB_Win32_i386
  #include <stdlib.h>
#else
  #include <sys/param.h>
#endif


#include "tos.h"


#ifdef __cplusplus
extern "C" {
#endif


/*== Constants ============================================================*/

/** tosFilename_MAXLEN
 *
 *  Maximum size of a complete filename.
 */

#ifdef rt_LIB_Win32_i386
  #define tosFilename_MAXLEN  _MAX_PATH
#else
  #define tosFilename_MAXLEN  MAXPATHLEN
#endif


/*== Parsing Components ===================================================*/

extern void tosFilename_split(const char * pszFilename,
                                    char * pszPath,
                                    char * pszName,
                                    char * pszExtension);

extern void tosFilename_getPath(const char * pszFilename,
                                      char * pszPath);

extern void tosFilename_getName(const char * pszFilename,
                                      char * pszName);

extern void tosFilename_getExtension(const char * pszFilename,
                                           char * pszExtension);


/*== Composing Filenames ==================================================*/

extern void tosFilename_new(char * pszFilename,
                            const char * pszPath,
                            const char * pszName,
                            const char * pszExtension);

extern Bool tosFilename_setPath(char * pszFilename,
                                const char * pszPath);

extern Bool tosFilename_setName(char * pszFilename,
                                const char * pszName);

extern Bool tosFilename_setExtension(char * pszFilename,
                                     const char * pszExtension);


/*== Conversion ===========================================================*/

extern void tosFilename_toWin(const char * pszUnixFilename,
                                    char * pszWinFilename,
                                    int    size);

extern void tosFilename_toUnix(const char * pszWinFilename,
                                     char * pszUnixFilename,
                                     int    size);

extern void tosFilename_normalize(const char * pszFilename,
                                        char * pszSystemFilename,
                                        int    size);
  /* one of toUnix or toWin */
  

/*== Temporary filenames ==================================================*/

extern int tosFilename_tempName(const char *dir, 
                                const char *pfx,
                                      char *tmpName,       /* Result      */
                                      int   size);         /* Result-size */

extern int tosFilename_tempNameSize(void);


#ifdef __cplusplus
}
#endif

#endif /* TOSFILENAME_H */
