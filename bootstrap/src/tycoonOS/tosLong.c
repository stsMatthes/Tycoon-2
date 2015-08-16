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
  tosLong.c 1.0 final  16-OCT-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Runtime support for class Long
*/

#ifdef rt_LIB_Solaris2_SPARC
  #include <stdlib.h>
#endif

#include "tos.h"
#include "tosLong.h"
#include "tosLog.h"


/*== Conversion ===========================================================*/

Long tosLong_fromInt(int x)
  {
  return (Long) x;
  }

int tosLong_asInt(Long x)
  {
  return (int) x;
  }


/*== Arithmetic / Order ===================================================*/

int tosLong_order(Long x, Long y)
  {
  return x < y ? -1 : x > y ? 1 : 0;
  }

Long tosLong_add(Long x, Long y)
  {
  return x + y;
  }

Long tosLong_sub(Long x, Long y)
  {
  return x - y;
  }

Long tosLong_mul(Long x, Long y)
  {
  return x * y;
  }

Long tosLong_and(Long x, Long y)
  {
  return x & y;
  }


Long tosLong_div(Long x, Long y)
  {
  if (y == 0)
     tosLog_error("tosLong", "div", "Division by zero");

#if   defined(rt_LIB_Solaris2_SPARC)
  return lldiv(x,y).quot;
#elif defined(rt_LIB_Linux_i386)
  return (unsigned long long)x / (unsigned long long)y;
#else
  return x / y;
#endif
  }


Long tosLong_mod(Long x, Long y)
  {
  if (y == 0)
     tosLog_error("tosLong", "mod", "Remainder by zero");

#if   defined(rt_LIB_Solaris2_SPARC)
  return lldiv(x,y).rem;
#elif defined(rt_LIB_Linux_i386)
  return (unsigned long long)x % (unsigned long long)y;
#else
  return x % y;
#endif
  }

