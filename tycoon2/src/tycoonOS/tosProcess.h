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
  tosProcess.h 1.0 final  12-AUG-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Support for process control. The return value in case of errors
  is -1 if not stated otherwise. For error codes see tosError.

*/


#ifndef TOSPROCESS_H
#define TOSPROCESS_H

#include "tos.h"


#ifdef __cplusplus
extern "C" {
#endif


/*== Process identification ===============================================*/

extern Int tosProcess_getpid(void);
extern Int tosProcess_getppid(void);


/*== Process informations =================================================*/

extern Long tosProcess_getUserCPUtime(void);
  /*
   * Returns CPU time used while executing instructions in the user space
   * of the calling process and all of its child processes in milliseconds.
   */

extern Long tosProcess_getKernelCPUtime(void);
  /*
   * Returns CPU time used by the kernel on behalf of the calling process
   * and all of its child processes in milliseconds.
   */


/*== Process control and execution ========================================*/

extern Int tosProcess_kill(int pid, int value);
  /*
   * value:   Unix    -- Signal, send to pid
   *          Windows -- Exit-Code value for pid
   */

extern Int tosProcess_signal(int pid, int value);
  /*
   * value:   Unix    -- Signal, send to pid
   *          Linux   -- Don´t send SIGUSR1 or SIGUSR2 (linuxthreads)
   *          Windows -- Not implemented
   */

extern Int tosProcess_execute(const char *cmdname, char *const argv[]);

extern Int tosProcess_system (const char *cmdname);


#ifdef __cplusplus
}
#endif

#endif /* TOSPROCESS_H */

