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
  tosFilemode.h 1.0 final  07-OCT-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Runtime support for file-mode informations.
*/

#ifndef TOSFILEMODE_H
#define TOSFILEMODE_H

#include <sys/types.h>
#include <sys/stat.h>

#include "tos.h"


#ifdef __cplusplus
extern "C" {
#endif


/*== Constants / Types ====================================================*/

#if   defined(rt_LIB_Nextstep_i386)
  typedef unsigned short tosFilemode_T;

#elif defined(rt_LIB_Win32_i386)
  typedef unsigned short tosFilemode_T;

#else
  typedef mode_t tosFilemode_T;
#endif


#ifdef rt_LIB_Win32_i386
  #define tosFilemode_DEFAULT  _S_IREAD | _S_IWRITE
#else
  #define tosFilemode_DEFAULT  S_IFREG | S_IRWXU | S_IRGRP | S_IROTH
  /*
  (4+2)*8*8 + 4*8 + 4; ==   S_IRUSER | S_IWUSER | S_IRGRP | S_ROTH
  */
#endif


/*== Get / set the file mode ==============================================*/

extern int tosFilemode_get(char *pszPath, tosFilemode_T *mode);
extern int tosFilemode_set(char *pszPath, tosFilemode_T  mode);


#ifdef __cplusplus
}
#endif

#endif /* TOSFILEMODE_H */

