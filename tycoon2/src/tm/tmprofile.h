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
  Copyright (c) 1997 Higher-Order GmbH, Hamburg. All rights reserved.

  $File: //depot/tycoon2/stsmain/tycoon2/src/tm/tmprofile.h $ $Revision: #3 $ $Date: 2003/10/01 $ Marc Weikard

  Tycoon Machine Profiler
  
*/

#ifndef TMPROFILE_H
#define TMPROFILE_H

#include "tos.h"
#include "tyc.h"

#ifdef __cplusplus
extern "C" {
#endif


#ifdef tvm_PROFILE

extern Bool tmprofile_fTakeSample;
extern Bool tmprofile_fStarted;

extern Word tmprofile_nMicroSeconds;

extern void tmprofile_takeSample(tyc_Thread * pThread);

extern void tmprofile_reset(void);
extern void tmprofile_result(void);
extern void tmprofile_start(void);
extern void tmprofile_stop(void);

extern void tmprofile_setResolution(Word nMicroSeconds);
extern void tmprofile_setMode(Bool fAccumulate);

#else

extern void tmprofile_reset(void);
extern void tmprofile_result(void);
extern void tmprofile_start(void);
extern void tmprofile_stop(void);

extern void tmprofile_setResolution(void);
extern void tmprofile_setMode(void);

#endif


#ifdef __cplusplus
}
#endif

#endif
