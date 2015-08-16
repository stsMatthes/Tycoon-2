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

  $File: //depot/tycoon2/stsmain/tycoon2/src/tm/rtstatic.h $ $Revision: #3 $ $Date: 2003/10/01 $ Andreas Gawecki

  Support Dynamic Lookup in Statically Linked Objects

*/

#ifndef RTSTATIC_H
#define RTSTATIC_H

#ifdef __cplusplus
extern "C" {
#endif


typedef struct rtstatic_Sym {
  char * pszSym;
  void * pValue;
} rtstatic_Sym;
  
typedef struct rtstatic_Lib {
  char * pszName;
  rtstatic_Sym * aSyms;
} rtstatic_Lib;
  
extern rtstatic_Lib rtstatic_aStaticLibs[];


#ifdef __cplusplus
}
#endif

#endif








