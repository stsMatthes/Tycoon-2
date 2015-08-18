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
  $File: //depot/tycoon2/stsmain/bootstrap/src/tycoonOS/tosInt32.c $ $Revision: #3 $ $Date: 2003/10/02 $  Andreas Gawecki, Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Runtime support for class Int32
  (*### probably mostly obsolete *)
*/

#include <stdlib.h>

#include "tos.h"
#include "tosInt32.h"
#include "tosLog.h"


/*== Conversion ===========================================================*/

int tosInt32_fromInt(int x)
  {
  return x;
  }

int tosInt32_asInt(int x)
  {
  return x;
  }

int tosInt32_fromString(const char *string)
  {
  return atoi(string);
  }


/*== Arithmetic / Order ===================================================*/

int tosInt32_order(int x, int y)
  {
  return x < y ? -1 : x > y ? 1 : 0;
  }

int tosInt32_add(int x, int y)
  {
  return x + y;
  }

int tosInt32_sub(int x, int y)
  {
  return x - y;
  }

int tosInt32_mul(int x, int y)
  {
  return x * y;
  }

int tosInt32_div(int x, int y)
  {
  if (y == 0)
     tosLog_error("tosInt32", "div", "Division by zero");

  return x / y;
  }

int tosInt32_mod(int x, int y)
  {
  if (y == 0)
     tosLog_error("tosInt32", "mod", "Remainder by zero");

  return x % y;
  }

