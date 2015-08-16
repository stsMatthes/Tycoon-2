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

  $File: //depot/tycoon2/stsmain/tycoon2/src/tm/tmdebug.h $ $Revision: #3 $ $Date: 2003/10/01 $ Marc Weikard

  Simple Debugging Infos

*/

#ifndef TMDEBUG_H
#define TMDEBUG_H

#include <stdio.h>

#include "tyc.h"


#ifdef __cplusplus
extern "C" {
#endif


extern void tmdebug_backTrace(tyc_Thread * pThread, FILE * out);

extern Bool tmdebug_fShowBackTrace;
extern Bool tmdebug_fShowStoreGrow;
extern Bool tmdebug_fShowObjectCount;
extern Bool tmdebug_fCountOnGc;

extern void tmdebug_showBackTrace(tyc_Thread * pThread, Word wClassId);
extern void tmdebug_showStoreGrow(Word wSize, Int wDirection);
extern void tmdebug_showObjectCount(void);
extern void tmdebug_init(void);


#ifdef __cplusplus
}
#endif

#endif

