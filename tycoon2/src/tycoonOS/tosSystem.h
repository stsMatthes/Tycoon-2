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
  tosSystem.h 1.0 final  10-OCT-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Detection of base host operating system parameters.
*/

#ifndef TOSSYSTEM_H
#define TOSSYSTEM_H


#include "tos.h"


#ifdef __cplusplus
extern "C" {
#endif


/*== Host machine identification ==========================================*/

/*
 * Following constants must be the same as in TL2/Standard/Tycoon.tc
 */

#define tosSystem_ID_INVALID      0
#define tosSystem_ID_SOLARIS2     1
#define tosSystem_ID_LINUX        2
#define tosSystem_ID_HPUX         3
#define tosSystem_ID_NEXTSTEP     4
#define tosSystem_ID_WINNT        5
#define tosSystem_ID_WIN95        6
#define tosSystem_ID_WIN98        7
#define tosSystem_ID_MACOSX       8
#define tosSystem_ID_MACOSX_386   9
#define tosSystem_ID_UNKNOWN      10

/*
 * tosSystem_getID
 * ---------------
 * Returns the current system id (one of the constants above).
 *
 * Return          - Current system id
 * Ressources      - none
 */

extern Int tosSystem_getID(void);


/*
 * tosSystem_getName
 * -----------------
 * Returns the current system name. The string ist stored in a global
 * static buffer, no need for deallocation. ´tosSystem_NAME_SIZE´ is the
 * maximum size of the returned string.
 *
 * Return          - Host name depending of current system id
 * Ressources      - none
 */

#define tosSystem_NAME_SIZE 16

extern String tosSystem_getName(void);


/*== Byte order ===========================================================*/

#define tosSystem_BIG_ENDIAN     0
#define tosSystem_LITTLE_ENDIAN  1


/* static detection */

#if defined(rt_LIB_Nextstep_i386) || defined(rt_LIB_Linux_i386) || defined(rt_LIB_Win32_i386) || defined(rt_LIB_Darwin_i386)
  #define tosSystem_BYTEORDER tosSystem_LITTLE_ENDIAN
#else
  #define tosSystem_BYTEORDER tosSystem_BIG_ENDIAN
#endif


/* dynamic detection */

extern Int tosSystem_getByteOrder(void);

extern Int tosSystem_getByteOrderOf(Int systemId);


/*== Initializing =========================================================*/

extern void tosSystem_init(void);


#ifdef __cplusplus
}
#endif

#endif /* TOSSYSTEM_H */

