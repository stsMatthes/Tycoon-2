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
  $File: //depot/tycoon2/stsmain/bootstrap/src/tycoonOS/tosMemory.c $ $Revision: #3 $ $Date: 2003/10/02 $ Andreas Gawecki, Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Direct main memory access
*/

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>


#include "tos.h"
#include "tosMemory.h"


void tosMemory_formatString(char *buffer, char *fmt, int value)
  {
  sprintf(buffer, fmt, value);
  }

int tosMemory_isNull(int base)
  {
  return base == 0;
  }

/*== Get memory values ====================================================*/

int tosMemory_peekInt(int base, int offs)
  {
  return *((int *) (base+offs));
  }

char *tosMemory_peekString(int base, int offs)
  {
  return (char *) (base+offs);
  }

char tosMemory_peekChar(int base, int offs)
  {
  return *((char *) (base+offs));
  }

unsigned char tosMemory_peekUnsignedChar(int base, int offs)
  {
  return *((unsigned char *) (base+offs));
  }

short tosMemory_peekShort(int base, int offs)
  {
  return *((short *) (base+offs));
  }

double tosMemory_peekReal(int base, int offs)
  {
  return *((double *) (base+offs));
  }

Long tosMemory_peekLong(int base, int offs)
  {
  return *((Long *) (base+offs));
  }


/*== Set memory values ====================================================*/

void tosMemory_pokeInt(int base, int offs, int value)
  {
  *((int *) (base+offs)) = value;
  }

void tosMemory_pokeString(int base, int offs, char * value)
  {
  strcpy((char *) (base+offs), value);
  }

void tosMemory_pokeChar(int base, int offs, char value)
  {
  *((char *) (base+offs)) = value;
  }

void tosMemory_pokeUnsignedChar(int base, int offs, unsigned char value)
  {
  *((unsigned char *) (base+offs)) = value;
  }

void tosMemory_pokeShort(int base, int offs, short value)
  {
  *((short *) (base+offs)) = value;
  }

void tosMemory_pokeReal(int base, int offs, double value)
  {
  *((double *) (base+offs)) = value;
  }

void tosMemory_pokeLong(int base, int offs, Long value)
  {
  *((Long *) (base+offs)) = value;
  }


/*== Conversion ===========================================================*/

int tosMemory_IntToInt32(int value)
  {
  return value;
  }

