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
  tosReal.h 1.0 final  11-OCT-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Runtime support for floating point values
*/

#ifndef TOSREAL_H
#define TOSREAL_H


#include "tos.h"


#ifdef __cplusplus
extern "C" {
#endif


/*== Conversion ===========================================================*/

extern double tosReal_fromInt(int x);
extern int    tosReal_asInt(double d);

extern double tosReal_fromLong(Long x);
extern Long   tosReal_asLong(double d);

extern double tosReal_fromString(const char *string);


/*== Constants ============================================================*/

extern Float  tosReal_floatNaN(void);
extern Double tosReal_doubleNaN(void);


/*== Arithmetic / Order ===================================================*/

extern int tosReal_order(double x, double y);

extern double tosReal_add(double x, double y);
extern double tosReal_sub(double x, double y);
extern double tosReal_mul(double x, double y);
extern double tosReal_div(double x, double y);

extern double tosReal_sqrt(double x);
extern double tosReal_ln(double x);
extern double tosReal_sin(double x);
extern double tosReal_cos(double x);
extern double tosReal_tan(double x);
extern double tosReal_asin(double x);
extern double tosReal_acos(double x);
extern double tosReal_atan(double x);

extern double tosReal_pow(double x, double y);
extern double tosReal_expE(double x);


#ifdef __cplusplus
}
#endif

#endif /* TOSREAL_H */

