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

  tvm.h 1.14 98/05/18 Andreas Gawecki, Marc Weikard

  Tycoon Virtual Machine Interface

*/

#ifndef TVM_H
#define TVM_H

#include "tsp.h"
#include "tyc.h"

#ifdef __cplusplus
extern "C" {
#endif


#define TVM_OP(op, n, sp) tvm_Op_ ## op,
typedef enum {

  #include "opcodes.h"

  tvm_Op_nOps
} tvm_Opcode;
#undef TVM_OP

#define TVM_BUILTIN(class,sel,op,args) tvm_Builtin_ ## class ## _ ## op,
typedef enum {

  #include "builtins.h"

  tvm_Builtin_nBuiltins
} tvm_Builtin;
#undef TVM_BUILTIN


extern void tvm_init(void);
extern void tvm_enumRootPtr(tsp_VisitPtr visitRootPtr);
extern void tvm_enumAmbiguousRootPtr(tsp_VisitPtr visitAmbiguousPtr);

extern void tvm_clearCache(Bool fInit);
extern void tvm_raise(void * pException);

extern tsp_OID tvm_run(void);

extern void tvm_raiseTypeError(tsp_OID pObject, tsp_ClassId wClassId);


#ifdef tvm_DEBUG
  extern void tvm_setTrace(Bool f);
  extern void tvm_setBreak(Bool f);
  extern void tvm_setBP(char * cn, char * sn);
  extern void tvm_breakPoint(void);
  extern void tvm_debugSend(tyc_ClassId idClass, tyc_SelectorId idSelector);
  extern void tvm_debugSuperSend(tyc_ClassId idClass, tyc_ClassId idSuperClass,
                                 tyc_SelectorId idSelector);
#else
  extern void tvm_setTrace(void);
  extern void tvm_setBreak(void);
  extern void tvm_setBP(void);
  extern void tvm_breakPoint(void);
  extern void tvm_debugSend(void);
  extern void tvm_debugSuperSend(void);
#endif


#ifdef __cplusplus
}
#endif

#endif
