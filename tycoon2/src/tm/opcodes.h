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

  $File: //depot/tycoon2/stsmain/tycoon2/src/tm/opcodes.h $ $Revision: #6 $ $Date: 2003/10/14 $ Marc Weikard

  Tycoon Virtual Machine Opcodes
  
*/


  /* format: opcode name, size of argument in bytes, sp change (s = special) */

  /* send message */
  TVM_OP(send,      2,  0 /*s*/)
  TVM_OP(send0,     2,  0)
  TVM_OP(send1,     2, -1)
  TVM_OP(send2,     2, -2)
  TVM_OP(send3,     2, -3)
  TVM_OP(send4,     2, -4)
  TVM_OP(send5,     2, -5)  
  TVM_OP(sendSuper, 2,  0 /*s*/)
  TVM_OP(sendTail,  2,  0 /*s*/)
  
  /* load operands */
  TVM_OP(global,    1, 1)
  TVM_OP(literal8,  1, 1)
  TVM_OP(literal16, 2, 1)
  TVM_OP(arg,       1, 1)
  TVM_OP(local,     1, 1)
  TVM_OP(char,      1, 1)
  TVM_OP(byte,      1, 1)
  TVM_OP(short,     2, 1)
  TVM_OP(true,      0, 1)
  TVM_OP(false,     0, 1)
  TVM_OP(nil,       0, 1)
  TVM_OP(minusOne,  0, 1)
  TVM_OP(zero,      0, 1)
  TVM_OP(one,       0, 1)
  TVM_OP(two,       0, 1)
  
  /* store operands */
  TVM_OP(storeLocal, 1, 0)
  TVM_OP(storeArg, 1, 0)

  /* access captured mutable bindings */
  TVM_OP(cellNew,   0,  0)
  TVM_OP(cellLoad,  0,  0)
  TVM_OP(cellStore, 0, -1)

  /* control */
  TVM_OP(return,    0,  0 /*s*/)
  TVM_OP(ifTrue8,   1, -1 /*s*/)
  TVM_OP(ifTrue16,  2, -1 /*s*/)
  TVM_OP(ifFalse8,  1, -1 /*s*/)
  TVM_OP(ifFalse16, 2, -1 /*s*/)
  TVM_OP(jump8,     1,  0 /*s*/)
  TVM_OP(jump16,    2,  0 /*s*/)

  /* stack manipulation */
  TVM_OP(drop,   1,  0 /*s*/)
  TVM_OP(adjust, 1,  0 /*s*/)
  TVM_OP(pop,    0, -1)

  /* create closure */
  TVM_OP(closure, 1, 0 /*s*/)

  /* syncronize threads */
  TVM_OP(sync, 0, 0)

  /* illegal opcode */ 
  TVM_OP(terminateThread, 0, 0)

  /* special sends */
  TVM_OP(sendAdd,         2, -1)
  TVM_OP(sendSub,         2, -1)
  TVM_OP(sendLessOrEqual, 2, -1)
  TVM_OP(sendEqual,       2, -1)
  TVM_OP(sendNotEqual,    2, -1)
  TVM_OP(sendFun0Apply,   2,  0)
  TVM_OP(sendFun1Apply,   2, -1)
  TVM_OP(sendIsNil,       2,  0)
  TVM_OP(sendIsNotNil,    2,  0)

  TVM_OP(makeArray, 1, 0 /* s */)

  /* components */
  TVM_OP(referenceArgument, 1, 1)
  TVM_OP(takeFromArgument,  1, 1)
  TVM_OP(referenceLocal,    1, 1)
  TVM_OP(takeFromLocal,     1, 1)
  TVM_OP(moveToLocal,       1, 0)
  TVM_OP(moveToArgument,    1, 0)
  TVM_OP(componentCellNew,  0, 0)
  TVM_OP(cellReference,     0, 0)
  TVM_OP(takeFromCell,      0, 0)
  TVM_OP(moveToCell,        0, -1)
  TVM_OP(componentPop,      0, -1)
  TVM_OP(componentAdjust,   0, 0)
