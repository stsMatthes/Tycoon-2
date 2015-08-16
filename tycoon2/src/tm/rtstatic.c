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
  Copyright (c) 1996 Higher-Order GmbH, Hamburg. All rights reserved.

  $File: //depot/tycoon2/stsmain/tycoon2/src/tm/rtstatic.c $ $Revision: #3 $ $Date: 2003/10/01 $ Andreas Gawecki

  Support Dynamic Lookup in Statically Linked Objects

*/

#include "tos.h"
#include "tosDll.h"

#include "rtstatic.h"


/* 1. Declare external symbols: */

#define LIB(lib,ext) 
#define SYM(sym) extern void sym();
#define ENDLIB 

#include "static.h"

#undef LIB
#undef SYM
#undef ENDLIB

/* 2. Build symbol table: */

#define LIB(lib,ext) static rtstatic_Sym lib ## _syms[] = {
#define SYM(sym) { #sym, (void *) sym },
#define ENDLIB   { NULL, NULL }};

#include "static.h"

#undef LIB
#undef SYM
#undef ENDLIB

/* 3. Build library table: */

#define LIB(lib,ext) { #lib "." #ext, lib ## _syms },
#define SYM(sym) 
#define ENDLIB 

rtstatic_Lib rtstatic_aStaticLibs[] = {

  #include "static.h"

  { NULL, NULL }
  };

#undef LIB
#undef SYM
#undef ENDLIB

