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
  tosSecurity.h 1.0        11-JAN-1999  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Support for a very basicly security support.

  Status: No ready for win32
*/

#ifndef TOSSECURITY_H
#define TOSSECURITY_H


#ifdef rt_LIB_Win32_i386
  #include <windows.h>
#else
  #include <sys/types.h>
  #include <unistd.h>
#endif


#include "tos.h"


#ifdef __cplusplus
extern "C" {
#endif

    
/*== Basic types ==========================================================*/

typedef Int tosSecurity_SID; /* Internal security descritpor */


/*== Basic type conversion ================================================*/

extern Int tosSecurity_uidToString(tosSecurity_SID *uid,
                                   char            *res,
                                   Int             *sizeOfRes);

extern Int tosSecurity_gidToString(tosSecurity_SID *gid,
                                   char            *res,
                                   Int             *sizeOfRes);

extern Int tosSecurity_uidFromString(char            *uname,
                                     tosSecurity_SID *resUid,
                                     Int             *sizeOfSID);

extern Int tosSecurity_gidFromString(char            *gname,
                                     tosSecurity_SID *resGid,
                                     Int             *sizeOfSID);


/*== Current access id tokens =============================================*/

extern Int tosSecurity_getCurrentUID(tosSecurity_SID *resUid,
                                     Int             *sizeOfSID);

extern Int tosSecurity_getCurrentGID(tosSecurity_SID *resUid,
                                     Int             *sizeOfSID);

extern Int tosSecurity_setCurrentUID(tosSecurity_SID *uid);
extern Int tosSecurity_setCurrentGID(tosSecurity_SID *gid);


/*== Change file access tokens ============================================*/

extern Int tosSecurity_chown(const char      *path,
                             tosSecurity_SID *owner,
                             tosSecurity_SID *group);


/*== Init =================================================================*/

extern void tosSecurity_init(void);


#ifdef __cplusplus
}
#endif

#endif /* TOSSECURITY_H */

