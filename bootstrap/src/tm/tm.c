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

  tm.c 1.16 98/05/18 Andreas Gawecki, Marc Weikard

  Tycoon Machine Main

*/

#include <stdio.h>
#include <string.h>

#include "tos.h"
#include "tosLog.h"
#include "tosError.h"

#include "tyc.h"
#include "tsp.h"
#include "tmthread.h"
#include "tmdebug.h"
#include "tm.h"


int     tm_nArguments;
char ** tm_pArguments;
char *  tm_pszProg;


int tm_main(int argc, char *argv[])
{
  tsp_ErrorCode wErrorCode;

  tm_nArguments = argc;
  tm_pArguments = argv;
  tm_pszProg = argv[0];

  tos_init();

#ifdef BOOTSTRAP
  if(argc <= 1) {
    fprintf(stderr, "Usage: %s store [arguments] || %s -bootstrap dump\n",
            argv[0], argv[0]);
    exit(-1);
  }
  if(strcmp(argv[1], "-bootstrap") == 0) {
    return tsp_createFromFile(argv[2]);
  }
  else {
    if ((wErrorCode = tsp_open(argv[1]))) {
      tosLog_error("tm", "main", "Cannot open store %s: %s", argv[1], tsp_errorCode(wErrorCode));
      tosError_ABORT();
    }
    rtdll_begin(rtstatic_aStaticLibs);

    /* init multithreading support (mutexes and variables) */
    tmthread_init();

    tyc_init();
    tmdebug_init();
    tsp_setEnumRootPtr(tyc_enumRootPtr);
    tsp_setEnumAmbiguousRootPtr(tyc_enumAmbiguousRootPtr);

    tmthread_restart();
  }
#else
  if(argc <= 1) {
    fprintf(stderr, "Usage: %s store [arguments]\n", argv[0]);
    exit(-1);
  }
  if ((wErrorCode = tsp_open(argv[1]))) {
    tosLog_error("tm", "main", "Cannot open store %s: %s", argv[1], tsp_errorCode(wErrorCode));
    tosError_ABORT();
  }
  rtdll_begin(rtstatic_aStaticLibs);

  /* init multithreading support (mutexes and variables) */
  tmthread_init();

  tyc_init();
  tmdebug_init();
  tsp_setEnumRootPtr(tyc_enumRootPtr);
  tsp_setEnumAmbiguousRootPtr(tyc_enumAmbiguousRootPtr);

  tmthread_restart();
#endif

  return 0;
}

