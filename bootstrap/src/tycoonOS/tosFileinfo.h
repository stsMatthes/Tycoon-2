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
  tosFileinfo.h 1.0 final  15-OCT-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Runtime support for file informations.
*/

#ifndef TOSFILEINFO_H
#define TOSFILEINFO_H

#include <sys/types.h>
#include <sys/stat.h>

#include "tos.h"


#ifdef __cplusplus
extern "C" {
#endif


/*== Constants / Types ====================================================*/

#ifdef rt_LIB_Win32_i386
  typedef struct _stati64 tosFileinfo_Buffer;
#else
  typedef struct stat tosFileinfo_Buffer;
#endif

extern Int tosFileinfo_bufferSize(void);

extern Int tosFileinfo_getBuffer(char *path, tosFileinfo_Buffer *buf);


/*== File informations ====================================================*/

extern Bool tosFileinfo_isFile        (tosFileinfo_Buffer *buf);
extern Bool tosFileinfo_isDirectory   (tosFileinfo_Buffer *buf);
extern Bool tosFileinfo_isSymbolicLink(tosFileinfo_Buffer *buf);

extern Long tosFileinfo_lastModified    (tosFileinfo_Buffer *buf);
extern Long tosFileinfo_lastAccessed    (tosFileinfo_Buffer *buf);
extern Long tosFileinfo_lastStatusChange(tosFileinfo_Buffer *buf);
extern Long tosFileinfo_size            (tosFileinfo_Buffer *buf);


#ifdef __cplusplus
}
#endif

#endif /* TOSFILEINFO_H */

