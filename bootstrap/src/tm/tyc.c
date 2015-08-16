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

  $File: //depot/tycoon2/stsmain/bootstrap/src/tm/tyc.c $ $Revision: #3 $ $Date: 2003/10/02 $ Marc Weikard

  Tycoon Objects
  
*/

#include <string.h>

#include "tm.h"
#include "tvm.h"
#include "tsp.h"
#include "tyc.h"


/* persistent root object */
tyc_Root * tyc_pRoot;

/* arguments supplied */
tsp_OID * tyc_pArgv;

/* single BlockingCallInterrupt object */
tsp_OID tyc_pBlockingCCallException;
/* single ThreadCancelled object */
tsp_OID tyc_pThreadCancelledException;
/* single CommitError object */
tsp_OID tyc_pCommitError;


void tyc_init(void)
{
  Int i, j, n;
  tsp_OID pArg;
  /* set globals */
  tyc_pRoot = tsp_root();
  tsp_inhibitGc();
  /* init argv */
  tyc_pArgv = tsp_newArray(tyc_ClassId_Array, tm_nArguments-1);
  for(i = j = 0; i < tm_nArguments; i++) {
    if (i == 1) {
      /* ignore store parameter */
      continue;
    }
    n = strlen(tm_pArguments[i]) + 1;
    pArg = tsp_newByteArray(tyc_ClassId_String, n);
    strcpy(pArg, tm_pArguments[i]);
    tyc_pArgv[j++] = pArg;
  }
  /* create a single BlockingCallInterrupt object */
  tyc_pBlockingCCallException =
    tsp_newArray(tyc_ClassId_BlockingCallInterrupt, 0);
  /* create a single ThreadCancelledException object */
  tyc_pThreadCancelledException =
    tsp_newArray(tyc_ClassId_ThreadCancelled, 0);
  /* create a single CommitError object */
  tyc_pCommitError = tsp_newArray(tyc_ClassId_CommitError, 0);
  tsp_allowGc();
  /* init virtual machine */
  tvm_init();
}

void tyc_enumRootPtr(tsp_VisitPtr visitRootPtr)
{
  tvm_enumRootPtr(visitRootPtr);
  /* enumerate OIDs */
  visitRootPtr((tsp_OID*)&tyc_pRoot);
  visitRootPtr((tsp_OID*)&tyc_pArgv);
  visitRootPtr((tsp_OID*)&tyc_pBlockingCCallException);
  visitRootPtr((tsp_OID*)&tyc_pThreadCancelledException);
  visitRootPtr((tsp_OID*)&tyc_pCommitError);
}

void tyc_enumAmbiguousRootPtr(tsp_VisitPtr visitAmbiguousPtr)
{
  tvm_enumAmbiguousRootPtr(visitAmbiguousPtr);
}


tsp_OID * tyc_boxInt(Int value)
{
  tsp_OID * pNew = tsp_newStruct(tyc_ClassId_Int);
  ((tyc_Int*)pNew)->value = value;
  return pNew;
}

tsp_OID * tyc_boxLong(Long value)
{
  tsp_OID * pNew = tsp_newStruct(tyc_ClassId_Long);
  ((tyc_Long*)pNew)->value = value;
  return pNew;
}

tsp_OID * tyc_boxReal(Real value)
{
  tsp_OID * pNew = tsp_newStruct(tyc_ClassId_Real);
  ((tyc_Real*)pNew)->value = value;
  return pNew;
}


