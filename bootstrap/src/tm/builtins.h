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

  builtins.h 1.39 98/11/13 Marc Weikard

  Tycoon Virtual Machine Builtins

*/

  /* */
  TVM_BUILTIN(Builtin,"",fail,0)
  
  /* */
  TVM_BUILTIN(Int,"+",add,1)
  TVM_BUILTIN(Int,"-",sub,1)
  TVM_BUILTIN(Int,"*",mult,1)
  TVM_BUILTIN(Int,"/",div,1)
  TVM_BUILTIN(Int,"%",mod,1)
  TVM_BUILTIN(Int,"=",equal,1)
  TVM_BUILTIN(Int,"<",less,1)
  TVM_BUILTIN(Int,"<=",lessOrEqual,1)
  TVM_BUILTIN(Int,">",greater,1)
  TVM_BUILTIN(Int,">=",greaterOrEqual,1)
  TVM_BUILTIN(Int,"|",or,1)
  TVM_BUILTIN(Int,"&",and,1)
  TVM_BUILTIN(Int,"xor",xor,1)
  TVM_BUILTIN(Int,"not",not,0)
  TVM_BUILTIN(Int,">>",shiftRight,1)
  TVM_BUILTIN(Int,"<<",shiftLeft,1)
  TVM_BUILTIN(Int,"asChar",asChar,0)

  /* */
  TVM_BUILTIN(Object,"==",equal,1)
  TVM_BUILTIN(Object,"class",class,0)
  TVM_BUILTIN(Object,"superComponent",superComponent,0)
  TVM_BUILTIN(Object,"isImmutable",isImmutable,0)
  TVM_BUILTIN(Object,"setImmutable",setImmutable,0)
  TVM_BUILTIN(Object,"true",true,0)
  TVM_BUILTIN(Object,"false",false,0)
  TVM_BUILTIN(Object,"nil",nil,0)  
  TVM_BUILTIN(Object,"nil@",nilAt,0)
  TVM_BUILTIN(Object,"fetch@",fetchAt,0)
  TVM_BUILTIN(Object,"shallowCopy",shallowCopy,0)
  TVM_BUILTIN(Object,"__hollowCopy",hollowCopy,0)
  TVM_BUILTIN(Object,"_typeCast",_typeCast,1)
  TVM_BUILTIN(Object,"_hash",_hash,0)
  TVM_BUILTIN(Object,"_setHash",_setHash,1)
  TVM_BUILTIN(Object,"_perform",_perform,2)
  TVM_BUILTIN(Object,"_perform@",_performAt,2)
  TVM_BUILTIN(Object,"__basicSize",_basicSize,0)
  TVM_BUILTIN(Object,"__basicAt",_basicAt,1)
  TVM_BUILTIN(Object,"__basicAtPut",_basicAtPut,2)
  TVM_BUILTIN(Object,"__doesNotUnderstand",__doesNotUnderstand,2)

  /* */
  TVM_BUILTIN(Class,"_set",_set,3)
     
  /* */
  TVM_BUILTIN(String,"[]",at,1)
  TVM_BUILTIN(String,"size",size,0)

  /* */
  TVM_BUILTIN(SymbolClass,"_new",_new,1)

  /* */
  TVM_BUILTIN(MutableString,"[]:=",atPut,2)
  TVM_BUILTIN(MutableString,"replace",replace,4)
  TVM_BUILTIN(MutableStringClass,"_new1",_new1,1)

  /* */
  TVM_BUILTIN(Array,"[]",at,1)
  TVM_BUILTIN(Array,"size",size,0)

  /* */
  TVM_BUILTIN(MutableArray,"[]:=",atPut,2)
  TVM_BUILTIN(MutableArray,"replace",replace,4)
  TVM_BUILTIN(MutableArrayClass,"_new1",_new1,1)

  /* */
  TVM_BUILTIN(ByteArray,"[]",at,1)
  TVM_BUILTIN(ByteArray,"[]:=", atPut,2)
  TVM_BUILTIN(ByteArray,"size",size,0)
  TVM_BUILTIN(ByteArrayClass,"_new1",_new1,1)

  /* */
  TVM_BUILTIN(ShortArray,"[]",at,1)
  TVM_BUILTIN(ShortArray,"[]:=", atPut,2)
  TVM_BUILTIN(ShortArray,"size",size,0)
  TVM_BUILTIN(ShortArrayClass,"_new1",_new1,1)

  /* */
  TVM_BUILTIN(IntArray,"[]",at,1)
  TVM_BUILTIN(IntArray,"[]:=", atPut,2)
  TVM_BUILTIN(IntArray,"size",size,0)
  TVM_BUILTIN(IntArrayClass,"_new1",_new1,1)

  /* */
  TVM_BUILTIN(LongArray,"[]",at,1)
  TVM_BUILTIN(LongArray,"[]:=", atPut,2)
  TVM_BUILTIN(LongArray,"size",size,0)
  TVM_BUILTIN(LongArrayClass,"_new1",_new1,1)

  /* */
  TVM_BUILTIN(Char,"asInt",asInt,0)
  TVM_BUILTIN(Char,"=",equal,1)

  /* */
  TVM_BUILTIN(DLL,"__open",__open,1)
  TVM_BUILTIN(DLL,"__close",__close,1)

  /* */
  TVM_BUILTIN(Exception,"_raise",_raise,0)

  /* */
  TVM_BUILTIN(Fun0,"[]",apply,0)
  TVM_BUILTIN(Fun1,"[]",apply,1)
  TVM_BUILTIN(Fun2,"[]",apply,2)
  TVM_BUILTIN(Fun3,"[]",apply,3)
  TVM_BUILTIN(Fun4,"[]",apply,4)
  TVM_BUILTIN(Fun5,"[]",apply,5)
  TVM_BUILTIN(Fun6,"[]",apply,6)
  TVM_BUILTIN(Fun7,"[]",apply,7)
  TVM_BUILTIN(Fun8,"[]",apply,8)
  TVM_BUILTIN(Fun9,"[]",apply,9)
  TVM_BUILTIN(Fun10,"[]",apply,10)

  /* */
  TVM_BUILTIN(Tycoon,"errno",errno,0)
  TVM_BUILTIN(Tycoon,"backTrace",backTrace,0)
  TVM_BUILTIN(Tycoon,"_rollback",_rollback,0)
  TVM_BUILTIN(Tycoon,"_commit",_commit,0)
  TVM_BUILTIN(Tycoon,"builtinArgv",builtinArgv,0)
  TVM_BUILTIN(Tycoon,"_platformCode",_platformCode,0)

  /* */
  TVM_BUILTIN(ConcreteClass,"_new",_new,0)

  /* */
  TVM_BUILTIN(EagerDFA,"_builtinTransition",transition,3)

  /* */
  TVM_BUILTIN(Root,"_newClassId",newClassId,2)
  TVM_BUILTIN(Root,"_flushAll",flushAll,0)
  TVM_BUILTIN(Root,"_flushClass",flushClass,1)
  TVM_BUILTIN(Root,"_flushSingle",flushSelector,2)

  /* */
  TVM_BUILTIN(CStructClass,"_new",_new,0)

  /* */
  TVM_BUILTIN(ThreadClass,"_new",_new,0)
  TVM_BUILTIN(ThreadClass,"this",this,0)
  TVM_BUILTIN(ThreadClass,"sleep",sleep,1)
  TVM_BUILTIN(ThreadClass,"testCancel",testCancel,0)
  TVM_BUILTIN(Thread,"__init",__init,0)
  TVM_BUILTIN(Thread,"cancel",cancel,0)

  /* */
  TVM_BUILTIN(WeakRefClass,"_new",_new,0)
  TVM_BUILTIN(WeakRef,"__init",__init,0)

  /* */
  TVM_BUILTIN(Mutex,"__init",__init,0)
  TVM_BUILTIN(Mutex,"__finalize",__finalize,0)
  TVM_BUILTIN(Mutex,"lock",lock,0)
  TVM_BUILTIN(Mutex,"tryLock",tryLock,0)
  TVM_BUILTIN(Mutex,"unlock",unlock,0)

  /* */
  TVM_BUILTIN(Condition,"__init",__init,0)
  TVM_BUILTIN(Condition,"__finalize",__finalize,0)
  TVM_BUILTIN(Condition,"wait",wait,1)
  TVM_BUILTIN(Condition,"timedWait",timedWait,2)
  TVM_BUILTIN(Condition,"signal",signal,0)

  /* */
  TVM_BUILTIN(BroadcastingCondition,"__init",__init,0)
  TVM_BUILTIN(BroadcastingCondition,"signal",broadcast,0)

  /* */
  TVM_BUILTIN(Finalizer,"__fetchWeaks",__fetchWeaks,0)

  /* */
  TVM_BUILTIN(BlockingExternalMethodCaller,"__suspendBlockingCall",__suspend,1)
  TVM_BUILTIN(BlockingExternalMethodCaller,"__resumeBlockingCall",__resume,1)

  /* component extension */
#if !defined(TVM_BUILTIN_C)
#define TVM_BUILTIN_C(class,selector,op,args,sorts) \
   TVM_BUILTIN(class,selector,op,args)
#endif

  TVM_BUILTIN_C(AtArray,"[]:=",moveTo,2,2)  /* /RC */
  TVM_BUILTIN_C(AtArray,"[]@",takeFrom,1,0)
  TVM_BUILTIN_C(AtArrayClass,"_new1@",_new1At,1,0)

  TVM_BUILTIN_C(FunC,"[]@",apply,0,0)
  TVM_BUILTIN_C(FunCR,"[]",apply,1,1)
  TVM_BUILTIN_C(FunRC,"[]@",apply,1,0)
  TVM_BUILTIN_C(FunCC,"[]@",apply,1,1)
  TVM_BUILTIN_C(FunCRR,"[]",apply,2,1)
  TVM_BUILTIN_C(FunRCR,"[]",apply,2,2)
  TVM_BUILTIN_C(FunCCR,"[]",apply,2,3)
  TVM_BUILTIN_C(FunRRC,"[]@",apply,2,0)
  TVM_BUILTIN_C(FunCRC,"[]@",apply,2,1)
  TVM_BUILTIN_C(FunRCC,"[]@",apply,2,2)
  TVM_BUILTIN_C(FunCCC,"[]@",apply,2,3)

  /* debugging */
  TVM_BUILTIN(X_TL2Debugger,"_fetchStoppedThread",_fetchStoppedThread,0)
  TVM_BUILTIN(Object,"isTracedComponent",isTracedComponent,0)
  TVM_BUILTIN(Object,"isTracedComponent:=",setIsTracedComponent,1)
  TVM_BUILTIN(Thread,"traceFlags",traceFlags,0)
  TVM_BUILTIN(Thread,"traceFlags:=",setTraceFlags,1)
  TVM_BUILTIN(Thread,"debugEvent",debugEvent,0)
  TVM_BUILTIN(Thread,"inspectSendSelector",inspectSendSelector,0)
  TVM_BUILTIN(Thread,"inspectReturnReceiver",inspectReturnReceiver,0)
  TVM_BUILTIN(Thread,"inspectReturnComponent",inspectReturnComponent,0)
  TVM_BUILTIN(Thread,"inspectStack",inspectStack,1)
  TVM_BUILTIN(Thread,"inspectActiveObject",inspectActiveObject,0)
  TVM_BUILTIN(Thread,"resume",resume,0)

