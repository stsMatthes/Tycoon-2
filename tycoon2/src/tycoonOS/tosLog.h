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
  tosLog.h 1.0 final  29-JUL-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  System message logging
*/

#ifndef TOSLOG_H
#define TOSLOG_H

#include <stdio.h>


#ifdef __cplusplus
extern "C" {
#endif


/*== Output files =========================================================*/

extern void tosLog_redirectError(FILE *newFile);
extern void tosLog_redirectDebug(FILE *newFile);


/*== Log messages =========================================================*/


/*
 * Output error messages (default to stderr).
 */
extern void tosLog_error(char *module, char *fun, char *msg, ...);


/*
 * Output debug messages (default into tycoon2.dbg file).
 */
extern void tosLog_debug(char *module, char *fun, char *msg, ...);



#ifdef __cplusplus
}
#endif

#endif /* TOSLOG_H */

