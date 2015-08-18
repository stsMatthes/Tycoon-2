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
  tosString.h 1.0 final  30-JUL-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Runtime support for some String utilities.
*/

#ifndef TOSSTRING_H
#define TOSSTRING_H

#ifdef __cplusplus
extern "C" {
#endif


/*== String Utilities =====================================================*/

extern char *tosString_strdup(const char *s);

extern char *tosString_extract(char *dest, int pos, int len, const char *source);


/*== String Utilities for Tycoon-2 ========================================*/

extern int tosString_locateSubString(char *s, char *sub, int startingAt);

extern int tosString_locateLastSubString(char *s, char *sub, int before);

extern int tosString_locateSomeChar(char *s, char *chars, int startingAt);

extern int tosString_locateSomeCharBefore(char *s, char *chars, int before);


#ifdef __cplusplus
}
#endif

#endif /* TOSSTRING_H */

