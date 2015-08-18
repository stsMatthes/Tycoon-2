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
  tosMemory.h 1.0 final  13-OCT-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Direct main memory access
*/

#ifndef TOSMEMORY_H
#define TOSMEMORY_H


#include "tos.h"


#ifdef __cplusplus
extern "C" {
#endif


extern void tosMemory_formatString(char *buffer,
                                   char *fmt,
                                   int value);

extern int tosMemory_isNull(int base);


/*== Get memory values ====================================================*/

extern int           tosMemory_peekInt         (int base, int offs);
extern char         *tosMemory_peekString      (int base, int offs);
extern char          tosMemory_peekChar        (int base, int offs);
extern unsigned char tosMemory_peekUnsignedChar(int base, int offs);
extern short         tosMemory_peekShort       (int base, int offs);
extern double        tosMemory_peekReal        (int base, int offs);
extern Long          tosMemory_peekLong        (int base, int offs);


/*== Set memory values ====================================================*/

extern void tosMemory_pokeInt         (int base, int offs, int value);
extern void tosMemory_pokeString      (int base, int offs, char * value);
extern void tosMemory_pokeChar        (int base, int offs, char value);
extern void tosMemory_pokeUnsignedChar(int base, int offs, unsigned char value);
extern void tosMemory_pokeShort       (int base, int offs, short value);
extern void tosMemory_pokeReal        (int base, int offs, double value);
extern void tosMemory_pokeLong        (int base, int offs, Long value);


/*== Conversion ===========================================================*/

extern int tosMemory_IntToInt32(int value);


#ifdef __cplusplus
}
#endif

#endif /* TOSMEMORY_H */

