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
  $File: //depot/tycoon2/stsmain/tycoon2/src/tycoonOS/tosString.c $ $Revision: #5 $ $Date: 2003/11/02 $  Andreas Gawecki, Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Runtime support for some String utilities.
*/

#include <stdio.h>
#include <string.h>

#include "tosString.h"
#include "tosError.h"
#include "tosLog.h"


/*== String Utilities =====================================================*/

char * tosString_strdup(const char * s)
{
#if defined(rt_LIB_Win32_i386) && defined(_MSC_VER)
  return _strdup(s);

#elif defined(rt_LIB_Nextstep_i386)
  char * ns = (char*) malloc(strlen(s) + 1);
  if(!ns) {
    tosLog_error("tosString", "strdup", "Out of memory");
    tosError_ABORT();
  }
  return strcpy(ns, s);

#else
  return strdup(s);

#endif
}


char * tosString_extract(char *dest, int pos, int len, const char *source)
{
  char *res = strncpy(dest, &(source[pos]), len);
  dest[len] = '\0';
  return res;
}


/*== String Utilities for Tycoon-2 ========================================*/

int tosString_locateSubString(char *s, char *sub, int startingAt)
{
  char * p = strstr(s+startingAt, sub);
  return p ? p-s : -1;
}

int tosString_locateLastSubString(char *s, char *sub, int before)
{
  int n = strlen(sub);
  int i = strlen(s)-n;

  if (i < 0)
    return -1;

  if (i > before)
    i = before;

  for(;i>=0;i--)
    if (strncmp(s+i,sub,n) == 0)
      break;
  return i;
}

int tosString_locateSomeChar(char *s, char *chars, int startingAt)
{
  int i = strlen(s);
  if (i > startingAt)
    i = startingAt;
  if (i < 0) i = 0;
  return i + strcspn(s+i, chars);
}


int tosString_locateSomeCharBefore(char *s, char *chars, int before)
{
  int i = strlen(s);
  if (i > before)
    i = before;
  if (i < 0)
    i = 0;
  for(;--i>=0;)
    if (strchr(chars,s[i]))
      break;
  return i;
}

