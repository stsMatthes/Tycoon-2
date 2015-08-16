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
  tos.c 1.0 final  28-JUL-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Base definitions, initialization, ...
*/


#include "tos.h"
#include "tosSystem.h"
#include "tosDate.h"
#include "tosThread.h"
#include "tosNet.h"
#include "tosSecurity.h"


/*== Initialize all modules ===============================================*/

void tos_init(void)
{
  static Bool has_been_called = FALSE;
  
  if (!has_been_called) {
     has_been_called = TRUE;

     tosSystem_init();
     tosDate_init();
     tosThread_init();
     tosNet_init();
     tosSecurity_init();
  }
}

