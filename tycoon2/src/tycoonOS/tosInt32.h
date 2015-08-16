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
  tosInt32.h 1.0 final  11-OCT-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Runtime support for class Int32
  (*### will be obsolete after bootstrap *)
*/

#ifndef TOSINT32_H
#define TOSINT32_H

#ifdef __cplusplus
extern "C" {
#endif


/*== Conversion ===========================================================*/

extern int tosInt32_fromInt(int x);
extern int tosInt32_asInt(int x);

extern int tosInt32_fromString(const char *string);


/*== Arithmetic / Order ===================================================*/

extern int tosInt32_order(int x, int y);

extern int tosInt32_add(int x, int y);
extern int tosInt32_sub(int x, int y);
extern int tosInt32_mul(int x, int y);
extern int tosInt32_div(int x, int y);
extern int tosInt32_mod(int x, int y);


#ifdef __cplusplus
}
#endif

#endif /* TOSINT32_H */

