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

  $File: //depot/tycoon2/stsmain/tycoon2/src/tm/tmprofile.c $ $Revision: #3 $ $Date: 2003/10/01 $ Marc Weikard

  Tycoon Machine Profiler

*/

#include <stdio.h>
#include <string.h>

#include "tos.h"
#include "tosLog.h"

#include "tyc.h"
#include "tmthread.h"
#include "tmprofile.h"


#ifdef tvm_PROFILE

#define nENTRIES (1024 * 64)

#define KEY(id, sel)  (((id) << 16) ^ (sel))
#define ID(k)         (((k) >> 16) & 0x0000ffffL)
#define SEL(k)        ((k) & 0x0000ffffL)
#define HASH(id, sel) ((((id) << 4) + (sel)) & (nENTRIES - 1))


Bool tmprofile_fTakeSample = FALSE;
Bool tmprofile_fStarted    = FALSE;
Bool tmprofile_fAccumulate = TRUE;

Word tmprofile_nMicroSeconds = 100000;  /* sample resolution */


typedef struct TableEntry {
  Word wKey;
  Word wCount;
} TableEntry;

static TableEntry aHashTable[nENTRIES];
static Bool        aRecTable[nENTRIES]; /* flags to detect recursion */

static Word nEvents, nMiss, nUnrecorded;


static int compare(const void * l, const void * r)
{
  Int wDiff;
  TableEntry * p1 = (TableEntry*)l, * p2 = (TableEntry*)r;
  /* sort entries by count, class name, selector name */
  if((wDiff = p2->wCount - p1->wCount) == 0 && p1->wCount != 0) {
    if((wDiff = strcmp(tyc_CLASS(ID(p1->wKey))->pszName,
                       tyc_CLASS(ID(p2->wKey))->pszName)) == 0) {
      wDiff = strcmp(tyc_SELECTOR(SEL(p1->wKey)), tyc_SELECTOR(SEL(p2->wKey)));
    }
  }
  return wDiff;
}


static void dumpTable(void)
{
  /* print header */
  fflush(stdout);
  fprintf(stdout, "\nTVM-Profiler Statistical Output");
  fprintf(stdout, "\n-------------------------------\n");
  fprintf(stdout, "   events recorded: %d\n", nEvents);
  fprintf(stdout, "       events lost: %d\n", nUnrecorded);
  fprintf(stdout, "additional lookups: %d\n", nMiss);
  fprintf(stdout, "\nMethod Calls");
  fprintf(stdout, "\n------------\n");
  fflush(stdout);
  /* sort entries */
  qsort(&aHashTable, nENTRIES, sizeof(TableEntry), compare);
  /* print entries */
  {
    Word n;
    Int nFill;
    char * pClassName, * pSelectorName;
    for(n = 0; n < nENTRIES; n++) {
      if(aHashTable[n].wKey != 0) {
        pClassName = tyc_CLASS(ID(aHashTable[n].wKey))->pszName;
        pSelectorName = tyc_SELECTOR(SEL(aHashTable[n].wKey));
        fprintf(stdout, "%s.%s", pClassName, pSelectorName);
        for(nFill = 30 - (strlen(pClassName) + strlen(pSelectorName));
            nFill > 0; nFill--)
          putchar(' ');
        fprintf(stdout,"\t%d events (%.2f%%)\n", aHashTable[n].wCount,
                (float)100 * (float)aHashTable[n].wCount / (float)nEvents);
        fflush(stdout);
      }
    }
  }
  return;
}


static void initTable(void)
{
  /* clear tables */
  memset(aHashTable, 0, sizeof(TableEntry) * nENTRIES);
  memset(aRecTable, 0, sizeof(Bool) * nENTRIES);
  /* reset counter */
  nEvents = nMiss = nUnrecorded = 0;
}

static void clearRecTable(void)
{
  memset(aRecTable, FALSE, sizeof(Bool) * nENTRIES);
}


static void accumulate(tyc_ClassId idClass, tyc_SelectorId idSelector)
{
  TableEntry * pEntry;
  Bool * pFlag;
  Word n, wKey  = KEY(idClass, idSelector), wHash = HASH(idClass, idSelector);
  /* hash table with linear probing */
  for(n = nENTRIES; n > 0; n--) {
    wHash &= (nENTRIES - 1);
    pEntry = &aHashTable[wHash], pFlag = &aRecTable[wHash];
    if(pEntry->wKey == wKey) {
      /* entry found */
      if(!tmprofile_fAccumulate || !(*pFlag)) {
        pEntry->wCount ++;
        *pFlag = TRUE;
      }
      return;
    }
    if(pEntry->wKey == 0) {
      /* empty slot */
      pEntry->wKey = wKey;
      pEntry->wCount = 1;
      *pFlag = TRUE;
      return;
    }
    wHash++;
    nMiss++;
  }
  /* no free entry */
  nUnrecorded++;
}


void tmprofile_takeSample(tyc_Thread * pThread)
{
  if(tmthread_timerExpired()) {
    tyc_StackFrame * fp = pThread->fp;
    /* one more sample */
    nEvents++;
    /* if accumulating clear recursion flags */
    if(tmprofile_fAccumulate) {
      clearRecTable();
    }
    /* loop through stack frames */
    while(fp) {
      tyc_ClassId idClass = tyc_CLASSID(fp->pReceiver);
      tyc_SelectorId idSelector = fp->pCode->idSelector;
      /* register activated method */
      accumulate(idClass, idSelector);
      /* if we are not accumulating stop right now */
      if(!tmprofile_fAccumulate) {
        break;
      }
      /* goto next frame */
      fp = fp->parent.fp;
      if(fp->parent.fp == NULL)
        break;                   /* skip barrier frame */
    }
    /* restart timer for next sample */
    tmthread_notifyTimerThread();
  }
  return;
}


void tmprofile_reset(void)
{
  initTable();
}

void tmprofile_result(void)
{
  dumpTable();
}

void tmprofile_start(void)
{
  tmthread_startTimer();
}

void tmprofile_stop(void)
{
  tmthread_stopTimer();
}

void tmprofile_setResolution(Word nMicroSeconds)
{
  tmprofile_nMicroSeconds = nMicroSeconds;
}

void tmprofile_setMode(Bool fAccumulate)
{
  tmprofile_fAccumulate = fAccumulate;
}

#else /* tvm_PROFILE */

/* profiling support was not configured at compile time.
   create dummy functions to print an error message. */

#define PROFILE_ERROR(fun) \
  void fun(void) \
  { \
    tosLog_error("tmprofile",\
                 "PROFILE",\
                 "TMPROFILE error: Machine not configured for profiling");\
  }

PROFILE_ERROR(tmprofile_reset)
PROFILE_ERROR(tmprofile_result)
PROFILE_ERROR(tmprofile_start)
PROFILE_ERROR(tmprofile_stop)
PROFILE_ERROR(tmprofile_setResolution)
PROFILE_ERROR(tmprofile_setMode)

#endif /* tvm_PROFILE */

