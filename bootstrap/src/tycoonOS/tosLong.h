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
  tosLong.h 1.0 final  16-OCT-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Runtime support for class Long
*/

#ifndef TOSLONG_H
#define TOSLONG_H


#include "tos.h"


#ifdef __cplusplus
extern "C" {
#endif


/*== Conversion ===========================================================*/

extern Long tosLong_fromInt(int x);
extern int tosLong_asInt(Long x);


/*== Arithmetic / Order ===================================================*/

extern int tosLong_order(Long x, Long y);

extern Long tosLong_add(Long x, Long y);
extern Long tosLong_sub(Long x, Long y);
extern Long tosLong_mul(Long x, Long y);
extern Long tosLong_and(Long x, Long y);
extern Long tosLong_div(Long x, Long y);
extern Long tosLong_mod(Long x, Long y);


#ifdef __cplusplus
}
#endif

#endif /* TOSLONG_H */

