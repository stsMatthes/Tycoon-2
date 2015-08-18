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
  tosError.h 1.0 final  30-JUL-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Error handling of TycoonOS layer.
*/

#ifndef TOSERROR_H
#define TOSERROR_H

#include <assert.h>
#include <stdlib.h>

#include "tos.h"


#ifdef __cplusplus
extern "C" {
#endif


/*== tycoonOS specific error values =======================================*/

#define EWIN32API        6000
#define EWINSOCKET       6001


/*== TVM and tycoonOS error management ====================================*/

#define tosError_ABORT() assert(FALSE); abort();
#define tosError_reset() tosError_setContext(0, 0);

extern int tosError_getCode(void);
extern int tosError_getCodeDetail(void);

extern void tosError_setContext(int errCode, int errCodeDetail);


/*== Context-free TL2 error text utilities ================================*/

extern void tosError_getText(int   errCode,
                             int   errCodeDetail,
                             char *msg,
                             int   msgSize);

extern void tosError_log(int errCode, int errCodeDetail);


#ifdef __cplusplus
}
#endif

#endif /* TOSERROR_H */

