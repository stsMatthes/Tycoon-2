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

  $File: //depot/tycoon2/stsmain/tycoon2/src/tm/classids.h $ $Revision: #3 $ $Date: 2003/10/01 $ Marc Weikard

  Tycoon Predefined ClassIds
  
*/

  /* Class-ID Redefinition for classes without packages */

  /* package, class_Id, tsp_layout, descriptor */

  /* used by tsp */
  TYC_CLASSID(BUILTIN, Unused,     Array,      NULL)
  TYC_CLASSID(BUILTIN, WeakRef,    WeakRef,    NULL)
  TYC_CLASSID(BUILTIN, Array,      Array,      NULL) 
  TYC_CLASSID(BUILTIN, String,     ByteArray,  NULL)
  TYC_CLASSID(BUILTIN, ByteArray,  ByteArray,  NULL)
  TYC_CLASSID(BUILTIN, ShortArray, ShortArray, NULL)
  TYC_CLASSID(BUILTIN, IntArray,   IntArray,   NULL)
  TYC_CLASSID(BUILTIN, LongArray,  LongArray,  NULL)
  TYC_CLASSID(BUILTIN, Thread,     Thread,     NULL) /* "ioiiiiiiooooooooo" */
  TYC_CLASSID(BUILTIN, TVMStack,   Stack,      NULL)

  TYC_CLASSID(BUILTIN, MutableArray,  Array,     NULL)
  TYC_CLASSID(BUILTIN, MutableString, ByteArray, NULL)
  TYC_CLASSID(BUILTIN, Symbol,        ByteArray, NULL)

  /* boxed values */
  TYC_CLASSID(BUILTIN, True,  Struct, "i")
  TYC_CLASSID(BUILTIN, False, Struct, "i")
  TYC_CLASSID(BUILTIN, Bool,  Struct, "i")

  TYC_CLASSID(BUILTIN, Char, Struct, "C")
  TYC_CLASSID(BUILTIN, Int,  Struct, "i")
  TYC_CLASSID(BUILTIN, Long, Struct, "w")
  TYC_CLASSID(BUILTIN, Real, Struct, "d")

  TYC_CLASSID(BUILTIN, Nil, Array, NULL)

  /* dynamic classIds */
     /* TYC_CLASSID(BUILTIN, Fun0,  Array, NULL) */
     /* TYC_CLASSID(BUILTIN, Fun1,  Array, NULL) */
     /* TYC_CLASSID(BUILTIN, Fun2,  Array, NULL) */
     /* Fun3...Fun10 must be consecutive classids! */
  TYC_CLASSID(BUILTIN, Fun3,  Array, NULL)
  TYC_CLASSID(BUILTIN, Fun4,  Array, NULL)
  TYC_CLASSID(BUILTIN, Fun5,  Array, NULL)
  TYC_CLASSID(BUILTIN, Fun6,  Array, NULL)
  TYC_CLASSID(BUILTIN, Fun7,  Array, NULL)
  TYC_CLASSID(BUILTIN, Fun8,  Array, NULL)
  TYC_CLASSID(BUILTIN, Fun9,  Array, NULL)
  TYC_CLASSID(BUILTIN, Fun10, Array, NULL)

  /* predefined TVM objects */
  TYC_CLASSID(BUILTIN, EmptyList,        Array, NULL)
  TYC_CLASSID(BUILTIN, List,             Array, NULL) /* "oo" */
  TYC_CLASSID(BUILTIN, Class,            Array, NULL) /* "ooooooooiioooooo" */
  TYC_CLASSID(TL2 , MethodDictionary, Array, NULL) /* "ooo" */

  TYC_CLASSID(BUILTIN, DLL,              Array, NULL) /* "ooooo" WR, Int32, Path, err, errDetail */

  TYC_CLASSID(TL2, Selector,            Struct, "oii")

  TYC_CLASSID(TL2 , Method,              Struct, "ooooooiiii")
  TYC_CLASSID(TL2 , CompiledMethod,      Struct, "ooooooiiiioioooiio")
  TYC_CLASSID(TL2 , BuiltinMethod,       Struct, "ooooooiiiioioooiioii")
  TYC_CLASSID(TL2 , ExternalMethod,      Struct, "ooooooiiiiooooii")
  TYC_CLASSID(TL2 , SlotMethod,          Struct, "ooooooiiiioi")  
  TYC_CLASSID(TL2 , SlotAccessMethod,    Struct, "ooooooiiiioi")  
  TYC_CLASSID(TL2 , SlotUpdateMethod,    Struct, "ooooooiiiioi") 
  TYC_CLASSID(TL2 , PoolMethod,          Struct, "ooooooiiiio")  
  TYC_CLASSID(TL2 , PoolAccessMethod,    Struct, "ooooooiiiio")  
  TYC_CLASSID(TL2 , PoolUpdateMethod,    Struct, "ooooooiiiio")  
  TYC_CLASSID(TL2 , CSlotAccessMethod,   Struct, "ooooooiiiioiC") 
  TYC_CLASSID(TL2 , CSlotUpdateMethod,   Struct, "ooooooiiiioiC")
  TYC_CLASSID(TL2 , SlotReferenceMethod, Struct, "ooooooiiiioi")  
  TYC_CLASSID(TL2 , SlotTakeFromMethod,  Struct, "ooooooiiiioi")  
  TYC_CLASSID(TL2 , SlotMoveToMethod,    Struct, "ooooooiiiioi") 
  TYC_CLASSID(TL2 , CSlotReferenceMethod,Struct, "ooooooiiiioiC")  
  TYC_CLASSID(TL2 , CSlotTakeFromMethod, Struct, "ooooooiiiioiC")  
  TYC_CLASSID(TL2 , CSlotMoveToMethod,   Struct, "ooooooiiiioiC") 
  TYC_CLASSID(TL2 , CompiledFun,         Struct, "ooooooiiiioioooiiooi")
  TYC_CLASSID(BUILTIN, Function,            Array,  NULL)       
  TYC_CLASSID(TL2 , DeferredMethod,      Struct, "ooooooiiii")
  TYC_CLASSID(TL2 , UnimplementedMethod, Struct, "ooooooiiii")
  TYC_CLASSID(TL2 , Slot, 		Struct, "ooooiio")

  /* exceptions raised by TVM */
  TYC_CLASSID(BUILTIN, TypeError,             Array, NULL) /* "oo" *//* debug range */
  TYC_CLASSID(BUILTIN, DoesNotUnderstand,     Array, NULL) /* "oo" */
  TYC_CLASSID(BUILTIN, MustBeBoolean,         Array, NULL) /* "o" */
  TYC_CLASSID(BUILTIN, DLLOpenError,          Array, NULL) /* "o" */
  TYC_CLASSID(BUILTIN, DLLCallError,          Array, NULL) /* "oo" */
  TYC_CLASSID(BUILTIN, DivisionByZero,        Array, NULL) /* "o" */
  TYC_CLASSID(BUILTIN, IndexOutOfBounds,      Array, NULL) /* "oo" *//* debug range */
  TYC_CLASSID(BUILTIN, BlockingCallInterrupt, Array, NULL) /* "" */
  TYC_CLASSID(BUILTIN, ThreadCancelled,       Array, NULL) /* "" */
  TYC_CLASSID(BUILTIN, CommitError,           Array, NULL) /* "" */
  TYC_CLASSID(BUILTIN, WrongSignature,        Array, NULL) /* "oooo" */
  TYC_CLASSID(BUILTIN, FetchBoundComponent,   Array, NULL) /* "o" */
  TYC_CLASSID(BUILTIN, PerformArityMismatch,  Array, NULL) /* "ooo" */
  TYC_CLASSID(BUILTIN, WriteToImmutable,      Array, NULL) /* "o" */
  TYC_CLASSID(BUILTIN, CyclicComponent,       Array, NULL) /* "oo" */

  /* special classIds */
  TYC_CLASSID(BUILTIN, Root,   Struct, "ooooooooiiooio")
  TYC_CLASSID(BUILTIN, Tycoon, Array, NULL)
  TYC_CLASSID(TL2 , Debugger, Array, NULL)
  TYC_CLASSID(BUILTIN, Object, Array, NULL)

  /* multithreading extensions */
  TYC_CLASSID(BUILTIN, Mutex,                 Struct, "io")
  TYC_CLASSID(BUILTIN, Condition,             Struct, "i")
  TYC_CLASSID(BUILTIN, BroadcastingCondition, Struct, "i")

  /* component extension */
  TYC_CLASSID(BUILTIN, AtArray,       Array,     NULL)

     /* order of FunXXX is important (computed classids) */
  TYC_CLASSID(BUILTIN, Fun0,	       Array, NULL)
  TYC_CLASSID(BUILTIN, FunC,	       Array, NULL)
  TYC_CLASSID(BUILTIN, Fun1,	       Array, NULL)
  TYC_CLASSID(BUILTIN, FunCR,	       Array, NULL)
  TYC_CLASSID(BUILTIN, FunRC,	       Array, NULL)
  TYC_CLASSID(BUILTIN, FunCC,	       Array, NULL)
  TYC_CLASSID(BUILTIN, Fun2,	       Array, NULL)
  TYC_CLASSID(BUILTIN, FunCRR,	       Array, NULL)
  TYC_CLASSID(BUILTIN, FunRCR,	       Array, NULL)
  TYC_CLASSID(BUILTIN, FunCCR,	       Array, NULL)
  TYC_CLASSID(BUILTIN, FunRRC,	       Array, NULL)
  TYC_CLASSID(BUILTIN, FunCRC,	       Array, NULL)
  TYC_CLASSID(BUILTIN, FunRCC,	       Array, NULL)
  TYC_CLASSID(BUILTIN, FunCCC,	       Array, NULL)

