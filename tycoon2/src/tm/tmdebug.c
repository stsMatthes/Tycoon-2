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

  $File: //depot/tycoon2/stsmain/tycoon2/src/tm/tmdebug.c $ $Revision: #4 $ $Date: 2003/11/01 $ Marc Weikard

  Simple Debugging
  
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "tos.h"
#include "tosLog.h"
#include "tosDate.h"
#include "tosFile.h"
#include "tosError.h"

#include "tyc.h"
#include "tvm.h"
#include "tsp.h"
#include "tmthread.h"


typedef struct tmdebug_OpList{
  char * pszName;
  Word nArgs;
} tmdebug_OpList;

#define TVM_OP(op, n, sp) { # op , n},

static tmdebug_OpList opcodes[] = {
  #include "opcodes.h"
  {NULL, 0}
};

#undef TVM_OP


#ifdef tmdebug_TEST

Byte abTest[] = {
  tvm_Op_local, 2,
  tvm_Op_short, 1, 1,
  tvm_Op_literal8, 0,
  tvm_Op_arg, 0,
  tvm_Op_send, 0, 1,
  tvm_Op_nil,
  tvm_Op_jump8, 254,
  tvm_Op_ifTrue16, 247, 255,
  tvm_Op_return
};

#if defined(tsp_size)
  #undef tsp_size
#endif
#define tsp_size(p) 19

#endif

FILE * tmdebug_stdout;
FILE * tmdebug_stderr;

void disassemble(Byte * pCode, FILE * out)
{
  Word nArgs, ip = 0, nBytes = tsp_size(pCode);
  Byte bOpcode;

  for(; ip < nBytes; ip++) {
    if((bOpcode = *pCode++) >= tvm_Op_nOps) {
      fprintf(out, "%d: \t illegal: %d\n", ip, bOpcode);
      continue;
    }
    switch(bOpcode)
      {
      case tvm_Op_ifTrue8:
      case tvm_Op_ifFalse8:
      case tvm_Op_jump8:
	fprintf(out, "%d: \t %s ", ip, opcodes[bOpcode].pszName); 
	fprintf(out, "%d\n", ip + (signed char)*pCode++);
        ip += 1;
        break;
      case tvm_Op_ifTrue16:
      case tvm_Op_ifFalse16:
      case tvm_Op_jump16:
        {
        Int v = *pCode++;
        v |= *pCode++ << 8;
	fprintf(out, "%d: \t %s ", ip, opcodes[bOpcode].pszName); 
	fprintf(out, "%d\n", ip + v); 	
        ip += 2;
        }
        break;
      default:
        nArgs = opcodes[bOpcode].nArgs;
        switch(nArgs)
          {
          case 0:
	    fprintf(out, "%d: \t %s\n", ip, opcodes[bOpcode].pszName);
            break;
          case 1:
	    fprintf(out, "%d: \t %s %d\n",
		    ip++, opcodes[bOpcode].pszName, *pCode++);
            break;
          case 2:
            {
            Short v = *pCode++;
            v |= *pCode++ << 8;
#ifndef tmdebug_TEST
            if(bOpcode <= tvm_Op_sendTail) {
              char * pSelector = tyc_SELECTOR(v);
	      fprintf(out, "%d \t %s %s\n",
		      ip, opcodes[bOpcode].pszName, pSelector);
            }
            else {
	      fprintf(out, "%d \t %s %d\n", ip, opcodes[bOpcode].pszName, v);
            }
#else
	    fprintf(out, "%d \t %s %d\n", ip, opcodes[bOpcode].pszName, v);
#endif
            ip += 2;
            }
            break;
          default:
	    fprintf(out, "%d \t %s invalid no. of arguments: %d\n",
		   ip, opcodes[bOpcode].pszName, nArgs);
            ip += nArgs;
          }
      }
  }
}

#ifndef tmdebug_TEST

void tmdebug_backTrace(tyc_Thread * pThread, FILE * out)
{
  Word iFrame = 0; 
  Byte * ip;
  tyc_StackFrame * fp;
  tsp_OID * sp, * pStack;
  
  assert(pThread);
  
  pStack = pThread->pStack;
  sp = pThread->sp;
  fp = pThread->fp;
  ip = pThread->ip;
  fprintf(out, "\n- BackTrace -\n");
  while(fp) {
    char * pszClassName = fp->pCode->pClass->pszName;
    char * pszSelectorName = fp->pCode->method.pSelector;
    fprintf(out, "%d: %s.%s\n", iFrame, pszClassName, pszSelectorName);
    /* source code output */
    fp = fp->parent.fp;
    if(fp->parent.fp == NULL)
      return;                  /* skip barrier frame */
    iFrame++;
  }
  fprintf(out, "\n");
}


/* VM debugging support */

Bool tmdebug_fShowBackTrace   = FALSE;
Bool tmdebug_fShowStoreGrow   = FALSE;
Bool tmdebug_fShowObjectCount = FALSE;
Bool tmdebug_fCountOnGc       = FALSE;


static char * aExceptionNames[] = {
  "Type",
  "DoesNotUnderstand",
  "MustBeBoolean",
  "DLLOpen",
  "DLLCall",
  "DivisionByZero",
  "IndexOutOfBounds",
  NULL
};

static Word wExceptionMask = 0;


Int * pOldTable, * pNewTable;


static void printTime(void)
{
  char s[100];
  struct tosDate_tm tm;

  tosDate_fromTime(&tm, tosDate_time());
  tosDate_normalize(&tm);
  tosDate_format(&tm, "%e/%b/%Y:%T", s, 100);

  printf("%s", s);
}


static void countObject(tsp_OID p)
{
  pNewTable[tsp_classId(p)]++;
}

static void collectObjects(void)
{
  /* clear table */
  memset(pNewTable, 0, tsp_MAX_CLASSIDS * sizeof(Word));
  /* sweep store */
  tsp_scanObjects(countObject);
}


static void showTables(void)
{
  Word n;
  /* calc difference */
  for(n = 0; n < tsp_MAX_CLASSIDS; n++) {
    pNewTable[n] = pOldTable[n] - pNewTable[n];
  }
  /* output */
  for(n = 0; n < tsp_MAX_CLASSIDS; n++) {
    if(pNewTable[n] != 0) {
      printf("%s:\t%+d\n", tyc_CLASS(n)->pszName, pNewTable[n]);
    }
  }
}

static void swapTables(void)
{
  Int * p = pOldTable;
  pOldTable = pNewTable;
  pNewTable = p;
}

static void initTables(void)
{
  /* allocate and init object count tables */
  pOldTable = malloc(tsp_MAX_CLASSIDS * sizeof(Word));
  pNewTable = malloc(tsp_MAX_CLASSIDS * sizeof(Word));
  if(!pOldTable || !pNewTable) {
    tosLog_error("tmdebug", "initTables", "Not enough memory");
    tosError_ABORT();
  }
  /* start initial count */
  collectObjects();
  swapTables();
}


void tmdebug_showBackTrace(tyc_Thread * pThread, Word wClassId)
{
  if(wClassId >= tyc_ClassId_TypeError &&
     wClassId <= tyc_ClassId_IndexOutOfBounds) {
    Word wIndex = wClassId - tyc_ClassId_TypeError;
    if((wExceptionMask & (1 << wIndex))) {
      printf("\n### DEBUG: Backtrace\n");
      tmdebug_backTrace(pThread, tmdebug_stdout);
      printf("\n###\n");
    }
  }
}


void tmdebug_showStoreGrow(Word wSize, Int wDirection)
{
  printf("\n### DEBUG: Storegrow\n");
  printf("Time: ");
  printTime();
  printf("\nExpanding store by %d pages. Growing ", wSize);
  switch(wDirection)
    {
    case 0:
      printf("down.\n");
      break;
    case 1:
      printf("up.\n");
      break;
    case 2:
      printf("between.\n");
      break;
    default:
      printf("somewhere.\n");
      break;
    }
  printf("\n###\n");
}


void tmdebug_showObjectCount(void)
{
  printf("\n### DEBUG: Objectcount\n");
  collectObjects();
  swapTables();
  showTables();
  printf("\n###\n");
}


/* low level thread state output */

/*
1. diplay threadlist
   - thread status
   - backtrace
   - locked mutexes
2. scan for objects using locked mutexes
*/


static tsp_OID mutexOID;


static void showThreadState(tyc_Thread * pThread, FILE * out)
{
  int i;
  char header[256];
  sprintf(header, "\n\"%s\" Thread (Ox%x)\n", pThread->name, (Word)pThread);
  fprintf(out, "%s", header);
  for(i = strlen(header); i > 2; i--) { 
    fprintf(out, "-");
  }
  fprintf(out, "\n\n- State -\n");
  /* check TL-2 state */
  if(IS_TERMINATED(pThread)) {
    /* thread is terminated */
    fprintf(out, "Terminated. Return value: 0x%x.", (Word)pThread->value);
  }
  else {
    /* thread is running. check internal state */
    if(IS_BLOCKED(pThread)) {
      if(DOES_CCALL(pThread)) {
	/* thread is blocked in an external call */
	Byte * ip = pThread->ip;
	Word id = (ip[2] << 8) | ip[1];
	fprintf(out, "Blocked in ccall. Method called: %s.", tyc_SELECTOR(id));
      }
      else {
	if(IS_WAITING(pThread)) {
	  /* thread is waiting for a condition */
	  tsp_OID * sp = pThread->sp;
	  fprintf(out, "Waiting for condition. ");
	  if(tsp_classId(sp[0]) == tyc_ClassId_Mutex) {
	    fprintf(out, "Associated mutex for wait: Ox%x.", (Word)sp[0]);
	  }
	  else if(tsp_classId(sp[1]) == tyc_ClassId_Mutex) {
	    fprintf(out, "Associated mutex for timedWait: Ox%x.", (Word)sp[1]);
	  }
	  else {
	    fprintf(out, "Cannot determine associated mutex.");
	  }
	}
	else if(IS_SUSPENDED(pThread)) {
	  fprintf(out, "Suspended by debug event, type=%08x.", SUSPEND_FLAGS(pThread));
	}
	else {
	  /* thread is blocked on a mutex */
	  tsp_OID * sp = pThread->sp;
	  fprintf(out, "Blocked on mutex. Mutex is: Ox%x.", (Word)sp[0]);
	}
      }
    }
    else {
      /* thread is running */
    }
    if(TEST_CANCEL(pThread)) {
      /* cancellation request is pending */
      fprintf(out, " Cancellation request pending.");
    }
  }
  fprintf(out, "\n");
}


static void findResources(tsp_OID p)
{
  tsp_OID * pp = p;
  /* assume that all objects not defined by the vm are arrays */
  if(tyc_CLASSID(p) >= tyc_ClassId_nPredefined) {
    Word n, nSlots = tsp_size(p) / sizeof(tsp_OID);
    for(n = 0; n < nSlots; n++) {
      if(*pp++ == mutexOID) {
	tsp_printObject(p);
	break;
      }
    }
  }
}


static void showResources(tyc_Thread * pThread, FILE * out)
{
  tsp_WeakRef * pWeakRef = tsp_weakRefs();
  fprintf(out, "\n- Resources -\n");
  while(pWeakRef) {
    tsp_OID * pObject = pWeakRef->p;
    if((tyc_CLASSID(pObject) == tyc_ClassId_Mutex) &&
       (((tyc_Mutex*)pObject)->pOwner == pThread)) {
      fprintf(out, "Mutex Ox%x", (Word)pObject);
      /* find resources using this mutex */
      mutexOID = pObject;
      tsp_scanObjects(findResources);
      fprintf(out, "\n");
    }
    pWeakRef = pWeakRef->pNext;
  }
}


void tmdebug_systemInfo(FILE * out)
{
  tyc_Thread * pThread;
  /* gain access to critical data */
  tmthread_criticalLock();
  /* start output */
  fprintf(out, "\n\nTycoon-System State\n");
  fprintf(out, "-------------------\n");
  pThread = tyc_pRoot->pThread;
  tsp_setPrintDepth(2, 15);
  while(pThread) {
    showThreadState(pThread, out);
    tmdebug_backTrace(pThread, out);
    showResources(pThread, out);
    pThread = pThread->pNext;
  }
  /* release lock */
  tmthread_criticalUnlock();
}


void tmdebug_init()
{
  char * pszDebug = getenv("HOX_DEBUG");

#if defined(__BORLANDC__)
  // hack for borlandC
  tmdebug_stdout = fdopen(tosFile_stdout(), "w");
  tmdebug_stderr = fdopen(tosFile_stderr(), "w");
#else
  tmdebug_stdout = stdout;
  tmdebug_stderr = stderr;
#endif

  if(pszDebug) {
    if(strcmp(pszDebug, "all") == 0) {
      /* turn on all options */
      tmdebug_fShowBackTrace   = TRUE;
      tmdebug_fShowStoreGrow   = TRUE;
      tmdebug_fShowObjectCount = TRUE;
      wExceptionMask = 0xffffffff;
      initTables();
      return;
    }
    if(strstr(pszDebug, "BackTrace") != NULL) {
      tmdebug_fShowBackTrace = TRUE;
      /* build exception mask */
      if(strstr(pszDebug, "AllExceptions") != NULL) {
        wExceptionMask = 0xffffffff;
      }
      else {
        Word n;
        for(n = 0; aExceptionNames[n] != NULL; n++) {
          if(strstr(pszDebug, aExceptionNames[n]) != NULL) {
            wExceptionMask |= 1 << n;
          }
        }
      }
    }
    if(strstr(pszDebug, "StoreGrow") != NULL) {
      tmdebug_fShowStoreGrow = TRUE;
    }
    if(strstr(pszDebug, "ObjectCount") != NULL) {
      tmdebug_fShowObjectCount = TRUE;
      if(strstr(pszDebug, "CountOnGC") != NULL) {
        tmdebug_fCountOnGc = TRUE;
      }
      initTables();
    }
  }
  return;
}


#endif

#ifdef tmdebug_TEST

int main()
{
  disassemble(abTest);
  return 0;
}

#endif

