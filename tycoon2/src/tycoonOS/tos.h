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
  tos.h 1.0 final  28-JUL-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Base definitions, initialization, ...

  ============================================================================
  Copyright (c) 1996 Higher-Order GmbH, Hamburg. All rights reserved.

  HOX general naming conventions:

  Exported names: prefixed with the name of exporting "*.h" file.

  Functions: lowercase, capitalized subwords
    rtthread_raise, tsp_open, tsp_newWeakRef

  types: capitalized
    rtthread_T

  macros: all upper case except optional lower case type prefix

  enumeration constants: type name "prefix", capitalized, capitalized subwords:
    tsp_Format_Real

  Type prefixes:

  prefix     type                            examples
  ------     ----                            --------
  f          Bool ("flag")                   fValid, fFound, fFull
  b          Byte                            b, bCode
  ch         Char                            ch, chIn, chLast
  w          Word                            wSize
  i          Word, Int, int (counter, index) i, iBuffer, iStack
  n          Word, Int, int ("# of...")      n, nBytes, nParams
  sz         Zero terminated string          sz, szName, szFileName
  p          Pointer                         pw, pbBuffer, pStack, pszFileName
  a          Vector ("Array")                a, aNames, awTable
  ============================================================================
*/

#ifndef TOS_H
#define TOS_H

#include "tosDBG.h"                /* Debug flags      */
#include "tosTYPES.h"              /* Data types       */


#ifdef __cplusplus
extern "C" {
#endif


/*== Base entry point for initialize TOS ==================================*/

extern void tos_init(void);


#ifdef __cplusplus
}
#endif

#endif /* TOS_H */
