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
  tosReal.c 1.0 final  11-OCT-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Runtime support for floating point values
*/

#include <math.h>
#include <stdlib.h>

#include "tos.h"
#include "tosReal.h"
#include "tosLog.h"


/*== Conversion ===========================================================*/

double tosReal_fromInt(int x)
  {
  return (double) x;
  }

int tosReal_asInt(double d)
  {
  return (int) d;
  }

double tosReal_fromLong(Long x)
  {
  return (double) x;
  }

Long tosReal_asLong(double d)
  {
  return (Long) d;
  }

double tosReal_fromString(const char *string)
  {
  return atof(string);
  }


/*== Constants ============================================================*/

Float tosReal_floatNaN(void)
{
  Float f;
  *((int *)&f) = 0x7fffffffL;  /* avoid conversion */
  return f;
}


Double tosReal_doubleNaN(void)
{
  Double d;

#ifdef rt_LIB_Win32_i386
  *((Long *)&d) = 0xffffffffffffffffi64;   /* avoid conversion */
#else
  *((Long *)&d) = 0xffffffffffffffffLL;    /* avoid conversion */
#endif

  return d;
}


/*== Arithmetic / Order ===================================================*/

int tosReal_order(double x, double y)
  {
  return x < y ? -1 : x > y ? 1 : 0;
  }

double tosReal_add(double x, double y)
  {
  return x + y;
  }

double tosReal_sub(double x, double y)
  {
  return x - y;
  }

double tosReal_mul(double x, double y)
  {
  return x * y;
  }

double tosReal_div(double x, double y)
  {
  if (y == 0.0)
     tosLog_error("tosReal", "div", "Division by zero");

  return x / y;
  }

double tosReal_sqrt(double x)
  {
  return sqrt(x);
  }

double tosReal_ln(double x)
  {
  return log(x);
  }

double tosReal_sin(double x)
  {
  return sin(x);
  }

double tosReal_cos(double x)
  {
  return cos(x);
  }

double tosReal_tan(double x)
  {
  return tan(x);
  }

double tosReal_asin(double x)
  {
  return asin(x);
  }

double tosReal_acos(double x)
  {
  return acos(x);
  }

double tosReal_atan(double x)
  {
  return atan(x);
  }

double tosReal_pow(double x, double y)
  {
  return pow(x,y);
  }

double tosReal_expE(double x)
  {
  return exp(x);
  }

