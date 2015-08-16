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
  tosTYPES.h 1.0 final  28-JUL-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Base data types.
*/

#ifndef TOSTYPES_H
#define TOSTYPES_H


#ifdef __cplusplus
extern "C" {
#endif


/*== OS specific definitions ==============================================*/

#if defined(rt_LIB_Win32_i386)
  #define tosTYPES_LONG           __int64
  #define tosTYPES_ULONG unsigned __int64
#else
  #define tosTYPES_LONG           long long
  #define tosTYPES_ULONG unsigned long long
#endif


/*== Base definitions =====================================================*/

#ifndef NULL
  #define NULL ((void*)0)
#endif


/*== Base types ===========================================================*/

typedef unsigned char Byte;
typedef unsigned char Char;

#undef FALSE
#undef TRUE

typedef enum {FALSE, TRUE} Bool;

typedef char * String;


/*== Signed values ========================================================*/

typedef signed short   Short;
typedef int            Int;
typedef tosTYPES_LONG  Long;


/*== Unsigned values ======================================================*/

typedef unsigned char   QWord;
typedef unsigned short  HWord;
typedef unsigned int    Word;
typedef tosTYPES_ULONG  DWord;


/*== Real numbers =========================================================*/

typedef float Float;
typedef double Double;
typedef Double Real;


#ifdef __cplusplus
}
#endif

#endif /* TOSTYPES_H */

