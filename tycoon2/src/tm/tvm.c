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

  $File: //depot/tycoon2/stsmain/tycoon2/src/tm/tvm.c $ $Revision: #11 $ $Date: 2003/10/20 $  Andreas Gawecki, Marc Weikard

  component and debugging extensions (c) 1998 Axel Wienberg

  Tycoon Virtual Machine
  
*/

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <assert.h>
#include <setjmp.h>

#include "tos.h"
#include "tosLog.h"
#include "tosError.h"
#include "tosSystem.h"
#include "tosString.h"
#include "tosThread.h"
#include "tosMutex.h"
#include "tosCondition.h"
#include "tosProcess.h"

#include "tyc.h"
#include "tsp.h"
#include "tvm.h"
#include "rtdll.h"
#include "tmccall.h"
#include "tmthread.h"
#include "tmdebug.h"
#include "tmprofile.h"
#include "tm.h"


/* Function classids */
#define FUNC_MAXARGS 3
#define tyc_ClassId_Fun_FUNC_MAXARGS tyc_ClassId_Fun3
#define FUN_MAXARGS 10
  

/* method cache stuff: */

#define CACHE_SIZE      2*2*2*1024     /* should be power of two */
#define SUPERCACHE_SIZE     2*1024

#define CACHE_UPDATE         10000

#define HASH(c, s)         (((s) ^ ((c) << 8)) & (CACHE_SIZE - 1))
#define SUPERHASH(c, i, s) (((s) ^ (((c) + (i)) << 2)) & (SUPERCACHE_SIZE - 1))

/* Constructors: */
#define tvm_KEY(idSelector, idClass) \
  ( ((idClass) << 16) | (idSelector) )

/* Selectors: */
#define tvm_SELECTORID(key) ((key) & 0xffff) /* bit  0..15 */
#define tvm_CLASSID(key)  (((key) >> 16))    /* bit 16..31 */

#define tvm_INVALID_KEY 0

typedef struct tvm_CacheEntry {
  Word key;
  tyc_Method * pMethod;
#ifdef tvm_THREADED_CODE
  void * pMethodCode;
#endif
} tvm_CacheEntry;

typedef struct tvm_SuperCacheEntry {
  struct tvm_CacheEntry cache;
  tyc_ClassId  superKey;
} tvm_SuperCacheEntry;

/* 1st level read only cache */
static tvm_CacheEntry cache[CACHE_SIZE];
static tvm_SuperCacheEntry superCache[SUPERCACHE_SIZE];

/* 2nd level read/write cache */
static tvm_CacheEntry rwCache[CACHE_SIZE];
static tvm_SuperCacheEntry rwSuperCache[SUPERCACHE_SIZE];

static Int cacheMisses = CACHE_UPDATE;

void tvm_clearCache(Bool fInit)
{
  /* complete sync procedure to keep consistency on multiprocessor systems.
     don't sync if fInit indicates system startup initialisation */
  if(!fInit) {
    tmthread_criticalLock();
    tmthread_checkSyncRequest();
    tmthread_syncRequest();
  }
  memset(cache, 0, sizeof(tvm_CacheEntry) * CACHE_SIZE);
  memset(superCache, 0, sizeof(tvm_SuperCacheEntry) * SUPERCACHE_SIZE);
  memset(rwCache, 0, sizeof(tvm_CacheEntry) * CACHE_SIZE);
  memset(rwSuperCache, 0, sizeof(tvm_SuperCacheEntry) * SUPERCACHE_SIZE);
  /* release waiting threads */ 
  if(!fInit) {
    tmthread_syncRelease();
    tmthread_criticalUnlock();
  }    
}

static void updateCache(void)
{
  register int i;
  /* suspend all running threads for update */
  tmthread_criticalLock();
  tmthread_checkSyncRequest();
  tmthread_syncRequest();
  /* update cache */
  {
  register tvm_CacheEntry * firstLevel = cache;
  register tvm_CacheEntry * secondLevel = rwCache;
  for (i = CACHE_SIZE; i > 0; i--, firstLevel++, secondLevel++) {
    if(secondLevel->pMethod != NULL) {
      *firstLevel = *secondLevel;
      secondLevel->key = 0;
      secondLevel->pMethod = NULL;
    }
  }
  }
  /* update superCache */
  {
  register tvm_SuperCacheEntry * firstLevel = superCache;
  register tvm_SuperCacheEntry * secondLevel = rwSuperCache;
  for (i = SUPERCACHE_SIZE; i > 0; i--, firstLevel++, secondLevel++) {
    if(secondLevel->cache.pMethod != NULL) {
      *firstLevel = *secondLevel;
      secondLevel->cache.key = secondLevel->superKey = 0;
      secondLevel->cache.pMethod = NULL;
    }
  }
  }
  /* reset update counter */
  cacheMisses = CACHE_UPDATE;
  /* continue all threads */
  tmthread_syncRelease();
  tmthread_criticalUnlock();
}


/* method lookup: */

static tyc_Method * lookup(tyc_MethodDictionary * pDictionary,
			   tyc_Symbol pSelector)
{
  tyc_Symbol * pKeys = pDictionary->apszKeys;
  Word wHash = tsp_hash(pSelector);
  Word nSize = (tsp_size(pKeys) / sizeof(tyc_Symbol)) - 1;

  while(TRUE) {
    wHash &= nSize;
    if(pKeys[wHash] == NULL)
      return NULL;
    if(pKeys[wHash] == pSelector)
      return pDictionary->apElements[wHash];
    wHash++;
  }
}

tyc_Method * tvm_methodLookup(tyc_SelectorId idSelector, tyc_ClassId idClass)
{
  tyc_Class * pClass = tyc_CLASS(idClass);
  tyc_Symbol pSelector = tyc_SELECTOR(idSelector);
  tyc_List * pList = pClass->pMethodDictionaries;
  while(tsp_classId(pList) == tyc_ClassId_List) {
    tyc_MethodDictionary * pDictionary = pList->pHead;
    tyc_Method * pMethod = lookup(pDictionary, pSelector);
    if(pMethod) {
      return pMethod;
    }
    pList = pList->pTail;
  }
  /* method not found */
  return NULL;
}

tyc_Method * tvm_methodSuperLookup(tyc_SelectorId idSelector,
				   tyc_ClassId idClass,
				   tyc_Class * pSuperClass)
{
  Bool fStart = FALSE, fSelf = FALSE;
  tyc_Class * pClass = tyc_CLASS(idClass);
  tyc_Symbol pSelector = tyc_SELECTOR(idSelector);
  tyc_List * pList = pClass->pMethodDictionaries;
  while(tsp_classId(pList) == tyc_ClassId_List) {
    tyc_MethodDictionary * pDictionary = pList->pHead;
    if(!fStart) {
      /* skip dictionaries until superclass is found */
      if(fSelf) {
	if(pDictionary->pClass != pSuperClass)
	  fStart = TRUE;
      }
      else {
	if(pDictionary->pClass == pSuperClass)
	  fSelf = TRUE;
      }
    }
    if(fStart) {
      tyc_Method * pMethod = lookup(pDictionary, pSelector);
      if(pMethod) {
	return pMethod;
      }
    }
    pList = pList->pTail;
  }
  /* method not found */
  return NULL;
}


/* perform minicache: */

#define PERFORMCACHE_SIZE 256

#define PERFORMHASH(s, n, c, k) (((tsp_hash(s) >> 2) + n + c + tsp_size(k)) & (PERFORMCACHE_SIZE - 1)) 

typedef struct tvm_performCacheLine {
  tyc_Symbol pSymbol;                        /* pSymbol == NULL : free entry */
  Word wArity;
  Word wSorts;
  tyc_Symbol *apKeywords;
  tyc_SelectorId idSelector;
} tvm_performCacheLine;

static tvm_performCacheLine performCache[PERFORMCACHE_SIZE];

static void tvm_clearPerformCache(void)
{
  tmthread_performLock();
  memset(performCache, 0, sizeof(tvm_performCacheLine) * PERFORMCACHE_SIZE);
  tmthread_performUnlock();
}


/* doesNotUnderstand initialization: */

static tyc_SelectorId idDoesNotUnderstand;

static void tvm_initDoesNotUnderstand(void)
{
  Word i, n = tyc_pRoot->wSelectorTableSize;
  tyc_Selector ** ppSelector = tyc_pRoot->apSelectorTable;
  /* search for id */
  for(i = 0; i < n; i++) {
    tyc_Selector *pSelector = *ppSelector++;
    if(pSelector->wArity == 2 && strcmp(pSelector->pSymbol, "__doesNotUnderstand") == 0) {
      idDoesNotUnderstand = i;
      return;
    }
  }
  /* selector not found */
  tosLog_error("tvm",
               "initDoesNotUnderstand",
               "Fatal TVM error: cannot find catchAll selector.");
  tosError_ABORT();
}


/* tvm initialization and gc support: */

void tvm_init(void)
{
  tvm_clearCache(TRUE);
  tvm_clearPerformCache();
  /* search for catchAll selectorid */
  tvm_initDoesNotUnderstand();
}

void tvm_enumRootPtr(tsp_VisitPtr visitRootPtr)
{
  int i;
  tmthread_enumRootPtr(visitRootPtr);
  /* enumerate method cache */
  for (i = 0; i < CACHE_SIZE; i++) {
    if(cache[i].pMethod != NULL)
      visitRootPtr((tsp_OID*)&cache[i].pMethod);
  }
  for (i = 0; i < SUPERCACHE_SIZE; i++) {
    if(superCache[i].cache.pMethod != NULL)
      visitRootPtr((tsp_OID*)&superCache[i].cache.pMethod);
  }
  for (i = 0; i < CACHE_SIZE; i++) {
    if(rwCache[i].pMethod != NULL)
      visitRootPtr((tsp_OID*)&rwCache[i].pMethod);
  }
  for (i = 0; i < SUPERCACHE_SIZE; i++) {
    if(rwSuperCache[i].cache.pMethod != NULL)
      visitRootPtr((tsp_OID*)&rwSuperCache[i].cache.pMethod);
  }
  /* enumerate perform cache */
  for (i = 0; i < PERFORMCACHE_SIZE; i++) {
    if(performCache[i].pSymbol != NULL) {
      visitRootPtr((tsp_OID*)&performCache[i].pSymbol);
      visitRootPtr((tsp_OID*)&performCache[i].apKeywords);
    }
  }
}

void tvm_enumAmbiguousRootPtr(tsp_VisitPtr visitAmbiguousPtr)
{
  tmthread_enumAmbiguousRootPtr(visitAmbiguousPtr);
}


/* lookup builtin numbers: */

typedef struct tvm_BuiltinMap {
  char * pszClass;
  char * pszSelector;
  Int    iBuiltin;
} tvm_BuiltinMap;

#define TVM_BUILTIN(class,sel,op,args) \
                   { #class, sel, tvm_Builtin_ ## class ## _ ## op},
static tvm_BuiltinMap builtinMap[] = {

  #include "builtins.h"

  {NULL, NULL, tvm_Builtin_nBuiltins}
};
#undef TVM_BUILTIN

/* lookupBuiltin returns tvm_Builtin_nBuiltins to indicate that the
   desired method is not a builtin and the attached bytecode should be
   executed */
static Int lookupBuiltin(tyc_BuiltinMethod * pMethod)
{
  Int n = pMethod->iNumber;
  char * pszName = pMethod->compiledMethod.pClass->pszName;
  char * pszSelector = pMethod->compiledMethod.method.pSelector;
  tvm_BuiltinMap * p;
  if((n >= 0) && (n < tvm_Builtin_nBuiltins) &&
     strcmp(pszName, builtinMap[n].pszClass) == 0 &&
     strcmp(pszSelector, builtinMap[n].pszSelector) == 0) {
    return n;
  }
  for(p = builtinMap, n = 0; n < tvm_Builtin_nBuiltins; n++, p++) {
    if(strcmp(pszName, p->pszClass) == 0 &&
       strcmp(pszSelector, p->pszSelector) == 0) {
      /* update builtin number in method struct */
      return pMethod->iNumber = p->iBuiltin;
    }
  }
  return pMethod->iNumber = tvm_Builtin_nBuiltins;
}


/* blocking ccalls: */

static char * aBlockingCalls[] = {
  "tosFile_open", "tosFile_copy", "tosFile_write1", "tosFile_writeChar",
  "tosFile_read1", "tosFile_readChar",

  "tosSocket_write", "tosSocket_writeChar", "tosSocket_read", "tosSocket_readChar",
  "tosSocket_accept", "tosSocket_connect",

  "fwrite", "fread",
  NULL
};

static void checkBlocking(tyc_ExternalMethod * pMethod)
{
  char ** ppName = aBlockingCalls;
  while(*ppName) {
    if(strcmp(pMethod->pszLabel, *ppName) == 0) {
      pMethod->fBlocking = TRUE;
      break;
    }
    ppName++;
  }
}


/* interpreter macros: */

#define FETCH_THREAD() pThread = tmthread_currentThread();

#define SAVE_REGS() \
  pThread->sp = sp; \
  pThread->fp = fp; \
  pThread->ip = ip;

#define LOAD_REGS() \
  sp = pThread->sp; \
  fp = pThread->fp; \
  ip = pThread->ip;

#define SYNC() \
  { \
  SAVE_REGS(); \
  tmthread_criticalLock(); \
  tmthread_checkSyncRequest(); \
  tmthread_criticalUnlock();\
  FETCH_THREAD(); \
  LOAD_REGS(); \
  }


/* Better performance, if we have the ideal byte order */
#if (tosSystem_BYTEORDER == tosSystem_LITTLE_ENDIAN)
  #define FETCH_SELECTORID(v) \
    v = ((Short*)(ip + 1))[0];
  #define FETCH_SHORT(v) \
    Short v = ((Short*)(ip + 1))[0];
#else
  #define FETCH_SELECTORID(v) \
    v  = ip[1]; \
    v |= ip[2] << 8;
  #define FETCH_SHORT(v) \
    Short v = ip[1]; \
    v |= ip[2] << 8;
#endif


#define CHECK_STACK_OVERFLOW() \
  if(sp < pThread->pStackLimit) { \
    if(pThread->pStackLimit != tmthread_ILL_STACKLIMIT) { \
      SAVE_REGS(); \
      tsp_resizeStack(pThread, ((tyc_CompiledMethod*)pMethod)->cwStackPeak); \
      FETCH_THREAD(); \
      LOAD_REGS(); \
    } \
    else { \
      SYNC(); \
    } \
  }


/* profiling support */

#ifdef tvm_PROFILE
#define SAMPLE() \
  if(tmprofile_fTakeSample) { \
    SAVE_REGS(); \
    tmprofile_takeSample(pThread); \
  }
#else
#define SAMPLE()
#endif

#ifdef tvm_TRACE
/* tracing and debugging */

static Bool isFunClassId(Word wClassId)
{
  /* cf. Op_closure */
  const Word funN_GE = tyc_ClassId_Fun_FUNC_MAXARGS;
  const Word funN_LE = tyc_ClassId_Fun_FUNC_MAXARGS-FUNC_MAXARGS + FUN_MAXARGS;
  const Word funC_GE = tyc_ClassId_Fun0;
  const Word funC_LT = tyc_ClassId_Fun0 - 2 + (2<<FUNC_MAXARGS) - 1;
  return (wClassId >= funN_GE && wClassId <= funN_LE)
    || (wClassId >= funC_GE && wClassId < funC_LT);
}

static tsp_OID getTracedComponent(tsp_OID o)
{
  Word wClassId = tyc_CLASSID(o);
  if(isFunClassId(wClassId)) {
    if(tsp_isTracedComponent(o))
      return o;
    o = ((tyc_Fun *)o)->awGlobals[0];  /* global self */
  }
  while(!tyc_IS_NIL(o)) {
    if(tsp_IS_IMMEDIATE(o))
      return tyc_NIL;
    if(tsp_isTracedComponent(o))
      return o;
    o = tsp_superComponent(o);
  }
  return o;
}

static Bool debug_interesting(tyc_Thread *pThread,
			      Word wTraceMask,
			      tsp_OID pSender,
			      tsp_OID pReceiver)
{
  if((TRACE_FLAGS(pThread) & wTraceMask) != 0) {
    if (!tyc_IS_NIL(tyc_pRoot->pDebugger)) {
      tsp_OID pSendingComponent, pReceivingComponent;
    
      if(IS_TRACE_NONCOMPONENT(pThread)) {
	return TRUE;
      } else {
	pSendingComponent = getTracedComponent(pSender);
	pReceivingComponent = getTracedComponent(pReceiver);
	return pSendingComponent != pReceivingComponent
	  || (IS_TRACE_INTRACOMPONENT(pThread)
	      && pSendingComponent != tyc_NIL);
      }
    }
  }
  return FALSE;
}

#endif  /* tvm_TRACE */

/* exception handling: */

void tvm_raise(void * pException)
{
  Byte * ip;
  tyc_StackFrame * fp;
  tyc_Stack sp;
  tyc_Thread * pThread;
 
  FETCH_THREAD();  
  LOAD_REGS();
  CLEAR_PAST_EVENT(pThread);

  if(tmdebug_fShowBackTrace) {
    /* check for exceptions raised by the VM */
    tmdebug_showBackTrace(pThread, tsp_classId(pException));
  }
  assert(fp);
  /* loop through stack frames, search for handlers: */
  while(fp->parent.fp != NULL) {
    tyc_CatchFrame * pCatchFrame = fp->pCode->asHandlerTable;
    if (pCatchFrame) {
      Word nFrames = tsp_size(pCatchFrame) / sizeof(tyc_CatchFrame);
      for (; nFrames; nFrames--, pCatchFrame++)
	if (pCatchFrame->ipFrom <= (ip - fp->pByteCode) &&
	    pCatchFrame->ipTo > (ip - fp->pByteCode)) {
	  sp = ((tsp_OID*) fp) - pCatchFrame->cwLocals;
	  ip = fp->pByteCode;
	  ip += pCatchFrame->ip;		
	  /* drop exception package */
	  *--sp = pException;        
	  /* continue with handler code */
	  SAVE_REGS();
	  longjmp(tmthread_currentJmpBuf(), 1);
	}
    }
#ifdef tvm_TRACE
    /* no exception handler found in this stackframe,
       exception is passed up one frame */
    if(debug_interesting(pThread, TRACE_EXCEPTION_BIT,
			 fp->pReceiver,
			 fp->parent.fp->pReceiver)) {
	*--sp = pException;  /* ### check stack overflow? */
	SET_EXCEPTION(pThread);
	SAVE_REGS();
	tmthread_debugSuspend(SUSP_EXCEPTION_BIT);
	FETCH_THREAD();
	LOAD_REGS();
	CLEAR_EXCEPTION(pThread);
	pException = *sp++;
    }
    sp = ((tsp_OID*) (fp + 1)) + fp->pCode->method.nArgs;
#endif
    ip = fp->parent.ip;
    fp = fp->parent.fp;
  }
  sp = ((tsp_OID*) fp);
  /* no exception handler found */
  fprintf(stderr,"\nTVM error: uncaught exception %s.\n",
	         tyc_CLASS(tsp_classId(pException))->pszName);
  tsp_setPrintDepth(2, 5);
  tsp_printObject(pException);
  tmdebug_backTrace(pThread, stdout);
  /* kill thread */
  SAVE_REGS();
  tmthread_exit(FALSE);
  tosError_ABORT();
}


#define RAISE_MustBeBoolean(p) \
  raise1(tyc_ClassId_MustBeBoolean, p)
#define RAISE_DLLOpenError(p) \
  raise1(tyc_ClassId_DLLOpenError, p)
#define RAISE_DivisionByZero(p) \
  raise1(tyc_ClassId_DivisionByZero, p) 
#define RAISE_FetchBoundComponent(p) \
  raise1(tyc_ClassId_FetchBoundComponent, p)
#define RAISE_WriteToImmutable(p) \
  raise1(tyc_ClassId_WriteToImmutable, p)

#define RAISE_TypeError(p, a) \
  raise2(tyc_ClassId_TypeError, p, tyc_CLASS(a))
#define RAISE_DoesNotUnderstand(p, a) \
  raise2(tyc_ClassId_DoesNotUnderstand, p, a)
#define RAISE_DLLCallError(p, a) \
  raise2(tyc_ClassId_DLLCallError, p, a)
#define RAISE_IndexOutOfBounds(p, a) \
  raise2(tyc_ClassId_IndexOutOfBounds, p, a)
#define RAISE_PerformArityMismatch(p, a) \
  raise2(tyc_ClassId_PerformArityMismatch, p, a)
#define RAISE_CyclicComponent(a,b) \
  raise2(tyc_ClassId_CyclicComponent, a, b)

#define RAISE_WrongSignature(p, a, b, c) \
  raise4(tyc_ClassId_WrongSignature, p, a, b, c)


static void raise1(tsp_ClassId wClassId, tsp_OID p1)
/* raise generic exception with one argument */
{
  tsp_OID * pError;
  /* save argument on local OID stack */
  tmthread_pushStack(p1);
  /* create new exception object */
  pError = tsp_newArray(wClassId, 1);
  /* initialize object */
  pError[0] = tmthread_popStack();
  /* call generic handler */
  tvm_raise(pError);
}

static void raise2(tsp_ClassId wClassId, tsp_OID p1, tsp_OID p2)
/* raise generic exception with two arguments */
{ 
  tsp_OID * pError;
  /* save arguments on local OID stack */
  tmthread_pushStack(p1);
  tmthread_pushStack(p2);
  /* create new exception object */
  pError = tsp_newArray(wClassId, 2);
  /* initialize object */
  pError[1] = tmthread_popStack();
  pError[0] = tmthread_popStack();
  /* call generic handler */
  tvm_raise(pError);
}

static void raise4(tsp_ClassId wClassId, tsp_OID p1, tsp_OID p2, tsp_OID p3, tsp_OID p4)
/* raise generic exception with four arguments */
{ 
  tsp_OID * pError;
  /* save arguments on local OID stack */
  tmthread_pushStack(p1);
  tmthread_pushStack(p2);
  tmthread_pushStack(p3);
  tmthread_pushStack(p4);
  /* create new exception object */
  pError = tsp_newArray(wClassId, 4);
  /* initialize object */
  pError[3] = tmthread_popStack();
  pError[2] = tmthread_popStack();
  pError[1] = tmthread_popStack();
  pError[0] = tmthread_popStack();
  /* call generic handler */
  tvm_raise(pError);
}


void  tvm_raiseTypeError(tsp_OID p, tsp_ClassId id)
{ 
  RAISE_TypeError(p, id);
}


/* component handling */

/* Is a direct or indirect supercomponent of or identical to b ? */
static Bool isSuperComponentOf(tsp_OID a, tsp_OID b)
{
  if(tyc_IS_NIL(a) || tsp_IS_IMMEDIATE(a))
    return a == b;
  while(!tyc_IS_NIL(b) && !tsp_IS_IMMEDIATE(b)) {
    if(a == b)
      return TRUE;
    b = tsp_superComponent(b);
  }
  return FALSE;
}

static void assert_superComponent(tsp_OID expectedSuperComponent, tsp_OID obj)
{
  tsp_OID actualSuper; 
  assert(!tyc_IS_NIL(obj));
  assert(!tsp_IS_IMMEDIATE(obj));
  actualSuper = tsp_superComponent(obj);
  if(actualSuper != expectedSuperComponent) {
    tosLog_error("tvm", "assert_superComponent", "object %s@%p has super component %s@%p, expected super component %s@%p",
		 tyc_CLASS(tyc_CLASSID(obj))->pszName, obj,
		 tyc_CLASS(tyc_CLASSID(actualSuper))->pszName, actualSuper,
		 tyc_CLASS(tyc_CLASSID(expectedSuperComponent))->pszName, expectedSuperComponent);
  }
}

/* location pp is invalid after the call */
static void zapComponent(tsp_OID superComponent, tsp_OID *pp)
{
  tsp_OID obj = *pp;
  if (obj != NULL) {
    assert_superComponent(superComponent, obj);
    /* not necessary: *pp = NULL; */
    tsp_setSuperComponent(obj, NULL);
  }
}

/* source (stack) location is invalid after the call  */
/* require !isSuperComponentOf(*sp, destSuperComp) */
static void moveToComponent(tsp_OID destSuperComp, tsp_OID *dest, tyc_Thread *pThread, tsp_OID *sp)
{
  tsp_OID obj = *sp;
  zapComponent(destSuperComp, dest);
  if(!tyc_IS_NIL(obj)) {
    assert_superComponent(pThread, obj);
    /* not necessary: *sp = NULL; */
    tsp_setSuperComponent(obj, destSuperComp);
  }
  *dest = obj;
}

/* destination (stack) location is invalid before the call */
static void takeFromComponent(tyc_Thread *pThread, tsp_OID *sp, tsp_OID srcSuperComp, tsp_OID *src)
{
  tsp_OID obj = *src;
  /* NOT: zapComponent(pThread, sp) */
  if(!tyc_IS_NIL(obj)) {
    assert_superComponent(srcSuperComp, obj);
    tsp_setSuperComponent(obj, pThread);
    *src = NULL;
  }
  *sp = obj;
}

#define DEBUG_KW_REORDER 0

/**
 * compute the permutation required to assign keyword arguments
 * delivered according to ks to keyword parameters expected according
 * to km.  The permutation is written into permutation, using at most
 * MAX_KW/2*3+2 bytes.
 *
 * return 0 if successful, != 0 if sorts do not match or ks contains 
 * superfluous or duplicate keywords.
 * 
 * precondition: km does not contain duplicate keywords.
 * precondition: kmSize <= MAX_KEYWORD_ARGS
 */
static int computePermutation(unsigned char *permutation, 
			      tyc_Array pFillWords,
			      tyc_Array pKeywordDefaults,
			      int kmSize,
			      tyc_Symbol *km,
			      Word wSortsM,
			      int ksSize,
			      tyc_Symbol *ks,
			      Word wSortsS)
{
  int nAbsent;
  int posInMsg[MAX_KEYWORD_ARGS];
  int im,is;
  int undefSearch;
  unsigned char *cycleSizePatch;

  assert(kmSize <= MAX_KEYWORD_ARGS);
  assert(kmSize > 0);
  assert(tsp_size(pKeywordDefaults)==kmSize*sizeof(tsp_OID));
  assert(kmSize-ksSize == 0 || tsp_size(pFillWords)==(kmSize-ksSize)*sizeof(tsp_OID));
  assert(wSortsM < (1<<kmSize));
  assert(wSortsS < (1<<ksSize));
#if DEBUG_KW_REORDER
  tosLog_debug("tvm", "computePermutation", "enter, kmSize=%d, ksSize=%d", kmSize, ksSize);
  for(im=0;im<kmSize;++im) {
    tosLog_debug("tvm","computePermutation", "  km[%d]=%s", im, km[im]);
  }
  for(is=0;is<ksSize;++is) {
    tosLog_debug("tvm","computePermutation", "  ks[%d]=%s", is, ks[is]);
  }
#endif

  /*
  // build an array that indicates where which keyword value
  // can be obtained from:
  // parameter keyword number im corresponds to message keyword 
  // number is == posInMsg[im]
  // if a keyword is not given, distinct indices above ksSize are
  // assigned (at runtime, the corresponding element of pFillWords is 
  // pushed there before permutation).
  // try not to unnecessarily permute fill words 
  */
  nAbsent = 0;
  for(im=0; im<kmSize; ++im) {
    tyc_Symbol k = km[im];
    for(is=0; is<ksSize; ++is) {
      if(ks[is] == k) {
	/* check component sort */
	if ((wSortsS & (1<<is)) != 0) {
	  if ((wSortsM & (1<<im)) == 0)
	    /* component mismatch */
	    return 1;
	} else if ((wSortsM & (1<<im)) != 0) {
	  /* component mismatch */
	  return 1;
	}
#if DEBUG_KW_REORDER
	tosLog_debug("tvm","computePermutation", "  km[%d]==ks[%d]", im, is);
#endif
	break;
      }
    }
    if(is == ksSize) {
      ++nAbsent;
      /*
      // undefined keywords coinciding with pushed undefined values
      // keep their position (no push).  For others, the position
      // is determined later
      */
      if (im >= ksSize) {
	is = im;
	pFillWords[kmSize-1-is] = pKeywordDefaults[im];
#if DEBUG_KW_REORDER
	tosLog_debug("tvm","computePermutation", "  km[%d]==ks[%d]==%s@%p", im,is,
		     tyc_CLASS(tyc_CLASSID(pKeywordDefaults[im]))->pszName, pKeywordDefaults[im]);
#endif
      } else {
	is = -1;
      }
    }
    posInMsg[im] = is;
  }
  if (nAbsent != kmSize - ksSize) {
    /* unexpected keyword(s) */
    return 2;
  }
  /* find positions for absent keywords */
  undefSearch = ksSize;
  for(im=0; im<ksSize; ++im) {
    if(posInMsg[im] == -1) {
      /* skip undefined keywords */
      while(posInMsg[undefSearch] == undefSearch)
	++undefSearch;
      /*
      // undefSearch is the index of a defined keyword,
      // at an index where a default value will been pushed
      // take the default value from this index to avoid
      // fruitless permutation of default values
      */
      assert(undefSearch <= kmSize);
#if DEBUG_KW_REORDER
	tosLog_debug("tvm","computePermutation", "  km[%d]==ks[%d]==%s@%p", im,undefSearch,
		     tyc_CLASS(tyc_CLASSID(pKeywordDefaults[im]))->pszName, pKeywordDefaults[im]);
#endif
      pFillWords[kmSize-1-undefSearch] = pKeywordDefaults[im];
      posInMsg[im] = undefSearch;
      ++undefSearch;
    }
  }
  /*
  // now represent this permutation as a product of disjoint cycles,
  // ready for efficient interpretation
  // each cycle starts with the length of the cycle-1, followed by the
  // element's indices. A length 0 marks the end of the sequence.  E.g.
  // "2 1 4 7 1 2 3 0" represents "(1 4 7) o (2 3)".
  */
  for(im=0; im<kmSize; ++im) {
    /* ignore cycles of length 1 */
    if(posInMsg[im] != im) {
      int current;

      cycleSizePatch = permutation;
      *permutation++ = 0; /* placeholder for cycle length (patched later) */
      *permutation++ = kmSize-1-im;  /* count from end */
      current = posInMsg[im];
      posInMsg[im] = im;  /* mark seen */
      do {
	int next;
	*permutation++ = kmSize-1-current; /* count from end */
#if DEBUG_KW_REORDER
	tosLog_debug("tvm","computePermutation", "   permute %d - %d", permutation[-2], permutation[-1]);
#endif
	next = posInMsg[current];
	posInMsg[current] = current; /* mark seen */
	current = next;
      } while(current != im);
#if DEBUG_KW_REORDER
      tosLog_debug("tvm","computePermutation", "  cycle length was %d", (int)(permutation-cycleSizePatch-1));
#endif
      *cycleSizePatch = permutation-cycleSizePatch-2;
    }
  }
#if DEBUG_KW_REORDER
  tosLog_debug("tvm","computePermutation", "  no more cycles");
#endif
  *permutation = 0;
  return 0;
}


/* interpreter main loop: */

static tsp_OID run(void)
{
  tyc_Thread * pThread;
  register Byte * ip;
  register tyc_Stack sp;
  register tyc_StackFrame * fp;

  FETCH_THREAD();  
  LOAD_REGS();

  for(;;)
    {
#ifdef tvm_THREADED_CODE
    #define LABEL(l) label_ ## l
    #define NEXT()   goto *jmpTable[ip[0]]

    #define TVM_OP(op, n, sp) \
                  && label_tvm_Op_ ## op,
    static void * jmpTable[] = {
      #include "opcodes.h"
      NULL
    };
    #undef TVM_OP
    
    #define METHODLABEL(l) l

    #define BLABEL(l) label_ ## l
    #define BNEXT()   goto methodEnd
    
    #define TVM_BUILTIN(class,sel,op,args) \
                       && label_tvm_Builtin_ ## class ## _ ## op,
    static void * builtinTable[] = {
      #include "builtins.h"
      NULL
    };
    #undef TVM_BUILTIN

    NEXT();
#else
    #define LABEL(l) case l
    #define NEXT()   continue

    #define METHODLABEL(l) case tyc_ClassId_ ## l

    #define BLABEL(l) case l
    #define BNEXT()   break
    
    switch(ip[0])
#endif
      {

      /* load operands */
      LABEL(tvm_Op_global):
	*--sp = fp->pReceiver[ip[1]];
	ip += 2;
	NEXT();
      LABEL(tvm_Op_literal8):
	*--sp = fp->pCode->pLiterals[ip[1]];
	ip += 2;
	NEXT();
      LABEL(tvm_Op_literal16):
	{
	FETCH_SHORT(value);
	*--sp = fp->pCode->pLiterals[value];
	}
	ip += 3;
	NEXT();
      LABEL(tvm_Op_arg):
      LABEL(tvm_Op_referenceArgument):
	*--sp = ((tsp_OID*)(fp + 1))[ip[1]];
	ip += 2;
	NEXT();
      LABEL(tvm_Op_takeFromArgument):
        {
	int argidx = ip[1];
	*--sp = ((tsp_OID*)(fp + 1))[argidx];
	((tsp_OID*)(fp + 1))[argidx] = tyc_NIL;
	/* superComponent is not modified */
	if(!tyc_IS_NIL(*sp))
	  assert_superComponent(pThread, *sp);
	}
	ip += 2;
	NEXT();
      LABEL(tvm_Op_local):
      LABEL(tvm_Op_referenceLocal):
	*--sp = ((tsp_OID*)fp)[-(ip[1])];
	ip += 2;
	NEXT();
      LABEL(tvm_Op_takeFromLocal):
	{
	int idx = ip[1];
	*--sp = ((tsp_OID*)fp)[-idx];
	((tsp_OID*)fp)[-idx] = tyc_NIL;
	/* superComponent is not modified */
	if(!tyc_IS_NIL(*sp))
	  assert_superComponent(pThread, *sp);
	}
	ip += 2;
	NEXT();
      LABEL(tvm_Op_char):
	*--sp = (tsp_OID)tyc_boxChar(ip[1]);
	ip += 2;
	NEXT();
      LABEL(tvm_Op_byte):
	*--sp = tyc_TAG_INT((signed char)ip[1]);
	ip += 2;
	NEXT();
      LABEL(tvm_Op_short):
	{
	FETCH_SHORT(value);
	*--sp = tyc_TAG_INT(value);
	}
	ip += 3;
	NEXT();
      LABEL(tvm_Op_true):
	*--sp = tyc_TRUE;
	ip++;
	NEXT();
      LABEL(tvm_Op_false):
	*--sp = tyc_FALSE;
	ip++;
	NEXT();
      LABEL(tvm_Op_nil):
	*--sp = tyc_NIL;
	ip++;
	NEXT();
      LABEL(tvm_Op_minusOne):
	*--sp = tyc_TAG_INT(-1);
	ip++;
	NEXT();
      LABEL(tvm_Op_zero):
	*--sp = tyc_TAG_INT(0);
	ip++;
	NEXT();
      LABEL(tvm_Op_one):
	*--sp = tyc_TAG_INT(1);
	ip++;
	NEXT();
      LABEL(tvm_Op_two):
	*--sp = tyc_TAG_INT(2);
	ip++;
	NEXT();

      /* store operands */
      LABEL(tvm_Op_storeLocal):
	((tsp_OID*)fp)[-(ip[1])] = *sp;
	ip += 2;
	NEXT();
      LABEL(tvm_Op_storeArg):
	((tsp_OID*)(fp + 1))[ip[1]] = *sp;
	ip += 2;
	NEXT();
      LABEL(tvm_Op_moveToLocal):
	/* superComponent is not modified */
	if(!tyc_IS_NIL(*sp))
	  assert_superComponent(pThread, *sp);
	((tsp_OID*)fp)[-(ip[1])] = *sp;
	*sp = tyc_NIL;
	ip += 2;
	NEXT();
      LABEL(tvm_Op_moveToArgument):
	/* superComponent is not modified */
	if(!tyc_IS_NIL(*sp))
	  assert_superComponent(pThread, *sp);
	((tsp_OID*)(fp + 1))[ip[1]] = *sp;
	*sp = tyc_NIL;
	ip += 2;
	NEXT();

      /* access captured mutable bindings */
      LABEL(tvm_Op_cellNew):
	{
	tsp_OID * pCell;
	SAVE_REGS();
	pCell = tsp_newArray(tyc_ClassId_MutableArray, 1);
	FETCH_THREAD();
	LOAD_REGS();
	*pCell = *sp;
	*sp = pCell;
	}
	ip++;
	NEXT();
      LABEL(tvm_Op_componentCellNew):
	{
	tsp_OID * pCell;
	SAVE_REGS();
	pCell = tsp_newArray(tyc_ClassId_AtArray, 1);
	FETCH_THREAD();
	LOAD_REGS();
	/* no cycle check necessary because tyc_IS_NIL(*pCell) */
	moveToComponent(pCell, pCell, pThread, sp);
	*sp = pCell;
	}
	ip++;
	NEXT();
      LABEL(tvm_Op_cellLoad):
      LABEL(tvm_Op_cellReference):
	*sp = *((tsp_OID*)*sp);
	ip++;
	NEXT();
      LABEL(tvm_Op_takeFromCell):
	{
	tsp_OID * cell = (tsp_OID*)*sp;
	takeFromComponent(pThread, sp, cell, cell);
	}
	ip++;
	NEXT();
      LABEL(tvm_Op_cellStore):
	{
	tsp_OID * pCell = *sp++;
	*pCell = *sp;
	}
	ip++;
	NEXT();
      LABEL(tvm_Op_moveToCell):
	{
	tsp_OID * pCell = *sp++;
	assert_superComponent(tyc_NIL, pCell);
	assert(*sp != pCell);
	moveToComponent(pCell, pCell, pThread, sp);
	*sp = tyc_NIL;
	}
	ip++;
	NEXT();
	
      /* control */
      LABEL(tvm_Op_return):
	{
	tsp_OID v;
#ifdef tvm_TRACE
	if(debug_interesting(pThread, TRACE_RETURN_BIT,
			     fp->pReceiver,
			     fp->parent.fp->pReceiver)) {
	  /* important: keep old value of IS_PAST_EVENT */
	  SAVE_REGS();
	  tmthread_debugSuspend(SUSP_RETURN_BIT);
	  FETCH_THREAD();
	  LOAD_REGS();
	}
#endif
	v = *sp;
	sp = ((tsp_OID*)(fp + 1)) + fp->pCode->method.nArgs;
	ip = fp->parent.ip;
	fp = fp->parent.fp;
	*sp = v;
	}
	ip += 3;
	NEXT();
      {
      register tyc_Bool * bool;
     
      LABEL(tvm_Op_ifTrue8):
	bool = (tyc_Bool*)*sp++;
	if(tyc_IS_TRUE(bool)) {
	  ip += (signed char)ip[1];
	  NEXT();
	}
	if(tyc_IS_FALSE(bool)) {
	  ip += 2;
	  NEXT();
	}
	/* no boolean on stack: raise exception */
noBool:
	{
	SAVE_REGS();
	RAISE_MustBeBoolean(bool);
	}
	break; /* not reachable */
      LABEL(tvm_Op_ifTrue16):
	bool = (tyc_Bool*)*sp++;
	if(tyc_IS_TRUE(bool)) {
	  FETCH_SHORT(offset);
	  ip += offset;
	  NEXT();
	}
	if(tyc_IS_FALSE(bool)) {
	  ip += 3;
	  NEXT();
	}
	goto noBool;
      LABEL(tvm_Op_ifFalse8):
	bool = (tyc_Bool*)*sp++;
	if(tyc_IS_FALSE(bool)) {
	  ip += (signed char)ip[1];
	  NEXT();
	}
	if(tyc_IS_TRUE(bool)) {
	  ip += 2;
	  NEXT();
	}
	goto noBool;
      LABEL(tvm_Op_ifFalse16):
	bool = (tyc_Bool*)*sp++;
	if(tyc_IS_FALSE(bool)) {
	  FETCH_SHORT(offset);
	  ip += offset;
	  NEXT();
	}
	if(tyc_IS_TRUE(bool)) {
	  ip += 3;
	  NEXT();
	}
	goto noBool;
      }
      LABEL(tvm_Op_jump8):
	ip += (signed char)ip[1];
	NEXT();
      LABEL(tvm_Op_jump16):
	{
	FETCH_SHORT(offset);
	ip += offset;
	}
	NEXT();

      /* stack manipulation */
      LABEL(tvm_Op_drop):
	sp += ip[1];
	ip += 2;
	NEXT();
      LABEL(tvm_Op_adjust):
	{
	tsp_OID v = *sp;
	sp += ip[1];
	*sp = v;
	}
	ip += 2;
	NEXT();
      LABEL(tvm_Op_componentAdjust):
	zapComponent(pThread, sp+1);
	sp[1] = sp[0];
	sp++;
	ip++;
	NEXT();
      LABEL(tvm_Op_pop):
	sp++;
	ip++;
	NEXT();
      LABEL(tvm_Op_componentPop):
	zapComponent(pThread, sp);
	sp++;
	ip++;
	NEXT();
	
      /* create closure */
      LABEL(tvm_Op_closure):
	{
	Word n, nSlots, nArgs, wSorts;
	tyc_ClassId wClassId;
	Bool fReturnsComponent;
	tsp_OID * pClosure;
	nSlots = ip[1];
	nArgs = ((tyc_CompiledMethod*)*sp)->method.nArgs;
	wSorts = ((tyc_CompiledMethod*)*sp)->method.wSorts;
	fReturnsComponent = ((tyc_CompiledMethod*)*sp)->method.pSelector[2] == '@';
	SAVE_REGS();
	if (nArgs >= FUNC_MAXARGS) {
	  assert(nArgs <= FUN_MAXARGS && wSorts == 0 && !fReturnsComponent);
	  wClassId = tyc_ClassId_Fun_FUNC_MAXARGS - FUNC_MAXARGS + nArgs;
	} else {
	  assert(wSorts < (1 << nArgs));
	  wClassId = tyc_ClassId_Fun0 - 2 + ((fReturnsComponent?3:2)<<nArgs) + wSorts;
	}
	pClosure = tsp_newArray(wClassId, nSlots + 1);
	FETCH_THREAD();
	LOAD_REGS();
	for(n = 0; n <= nSlots; n++) {
	  pClosure[n] = *sp++;
	}
	*--sp = pClosure;
	}
	ip += 2;
	NEXT();

      /* syncronize threads */
      LABEL(tvm_Op_sync):
	{
        ip++;
        if (pThread->pStackLimit == tmthread_ILL_STACKLIMIT) {
	  SYNC();
	}
	}
	NEXT();

      LABEL(tvm_Op_makeArray):
	{
	Word nSlots;
	tsp_OID * pArray;
	nSlots = ip[1];
	SAVE_REGS();
	pArray = tsp_newArray(tyc_ClassId_MutableArray, nSlots);
	FETCH_THREAD();
	LOAD_REGS();
	while(nSlots) {
	  pArray[--nSlots] = *sp++;
	}
	tsp_setImmutable(pArray);
	*--sp = pArray;
	}
	ip += 2;
	NEXT();

      /* send message */
      {
      register tvm_CacheEntry * pEntry;
      register tsp_OID pReceiver;
      register tyc_Method * pMethod;
#ifdef tvm_THREADED_CODE
      register void * pMethodCode;
#endif
      tyc_ClassId idClass;
      tyc_SelectorId idSelector;

/* special sends */
/* ------------- */
      /* tagged integer add */
      LABEL(tvm_Op_sendAdd):
	{
	if(tsp_IS_IMMEDIATE(sp[0]) && tsp_IS_IMMEDIATE(sp[1])) {
	  Int wResult = tyc_UNTAG_INT(sp[1]) + tyc_UNTAG_INT(sp[0]);
	  if(!tyc_MUST_BOX_INT(wResult)) {
	    *++sp = tyc_TAG_INT(wResult);
	    ip += 3;
	    NEXT();
	  }
	  else {
	    tsp_OID pResult;
	    SAVE_REGS();
	    pResult = (tsp_OID)tyc_boxInt(wResult);
	    FETCH_THREAD();
	    LOAD_REGS();
	    *++sp = pResult;
	    ip += 3;
	    NEXT();
	  }
	}
	}
	goto label_send1;
      /* tagged integer sub */
      LABEL(tvm_Op_sendSub):
	{
	if(tsp_IS_IMMEDIATE(sp[0]) && tsp_IS_IMMEDIATE(sp[1])) {
	  Int wResult = tyc_UNTAG_INT(sp[1]) - tyc_UNTAG_INT(sp[0]);
	  if(!tyc_MUST_BOX_INT(wResult)) {
	    *++sp = tyc_TAG_INT(wResult);
	    ip += 3;
	    NEXT();
	  }
	  else {
	    tsp_OID pResult;
	    SAVE_REGS();
	    pResult = (tsp_OID)tyc_boxInt(wResult);
	    FETCH_THREAD();
	    LOAD_REGS();
	    *++sp = pResult;
	    ip += 3;
	    NEXT();
	  }
	}
	}
	goto label_send1;
      /* tagged integer lessOrEqual */	
      LABEL(tvm_Op_sendLessOrEqual):
	{
	if(tsp_IS_IMMEDIATE(sp[0]) && tsp_IS_IMMEDIATE(sp[1])) {
	  tyc_Bool * pResult =
	    tyc_boxBool(tyc_UNTAG_INT(sp[1]) <= tyc_UNTAG_INT(sp[0]));
	  *++sp = pResult;
	  ip += 3;
	  NEXT();
	}
	}
	goto label_send1;
      /* object equal */
      LABEL(tvm_Op_sendEqual):
	{
	tyc_Bool * pResult = tyc_boxBool(sp[1] == sp[0]);
	*++sp = pResult;
	}
	ip += 3;
	NEXT();
      /* object notEqual */ 
      LABEL(tvm_Op_sendNotEqual):
	{
	tyc_Bool * pResult = tyc_boxBool(sp[1] != sp[0]);
	*++sp = pResult;
	}
	ip += 3;
	NEXT();
      /* fun0 apply */
      LABEL(tvm_Op_sendFun0Apply):
#ifndef tvm_TRACE
	{
	if(tyc_CLASSID(sp[0]) == tyc_ClassId_Fun0) {
	  pReceiver = sp[0];
	  pMethod = (tyc_Method*)((tyc_Fun*)pReceiver)->pCompiledFun;
	  goto nonrec_methodcall;
	}
	}
#endif
	goto label_send0;
      /* fun1 apply */
      LABEL(tvm_Op_sendFun1Apply):
#ifndef tvm_TRACE
	{
	if(tyc_CLASSID(sp[1]) == tyc_ClassId_Fun1) {
	  pReceiver = sp[1];
	  pMethod = (tyc_Method*)((tyc_Fun*)pReceiver)->pCompiledFun;
	  goto nonrec_methodcall;
	}
	}
#endif
	goto label_send1;
      /* object isNil */
      LABEL(tvm_Op_sendIsNil):
	{
	*sp = tyc_boxBool(!(*sp));
	}
	ip += 3;
	NEXT();
      /* object isNotNil */
      LABEL(tvm_Op_sendIsNotNil):
	{
	*sp = tyc_boxBool(*sp);
	}
	ip += 3;
	NEXT();
/* ------------- */
/* special sends */
      
      LABEL(tvm_Op_sendSuper):
	{
	register tvm_SuperCacheEntry * pSuperEntry;
	tyc_ClassId idSuperClass;
	FETCH_SELECTORID(idSelector);
        pReceiver = sp[tyc_ARGUMENTS(idSelector)];
	if(tsp_IS_IMMEDIATE(pReceiver))
	  goto taggedInt1;
	if(tyc_IS_NIL(pReceiver))
	  goto nil1;
	idClass = tsp_classId(pReceiver);
super0:
	idSuperClass = tyc_UNTAG_INT(fp->pCode->pClass->wInstanceId);
#ifdef tvm_DEBUG
	tvm_debugSuperSend(idClass, idSuperClass, idSelector);
#endif
#ifdef tvm_TRACE
	/* ### signal super send event */
	SET_PAST_EVENT(pThread);
#endif
	pSuperEntry =
	  &superCache[SUPERHASH(idClass, idSuperClass, idSelector)];
	if(pSuperEntry->superKey == idSuperClass &&
	   pSuperEntry->cache.key == tvm_KEY(idSelector, idClass)) {
	  /* 1st level cache hit */
	  pEntry = &pSuperEntry->cache;
	  goto cacheHit;
	}
        /* 1st level cache miss */
	{
	/* aquire 2nd level r/w cache lock */
	tmthread_cacheLock();
	/* decrease update counter and update 1st level cache if necessary */
	if(--cacheMisses == 0) {
	  tmthread_cacheUnlock();
	  tmthread_pushStack(pReceiver);
	  SAVE_REGS();
	  updateCache();
	  FETCH_THREAD();
	  LOAD_REGS();
	  pReceiver = tmthread_popStack();
	  tmthread_cacheLock();
	}
	pSuperEntry =
	  &rwSuperCache[SUPERHASH(idClass, idSuperClass, idSelector)];
	if(pSuperEntry->superKey == idSuperClass &&
	   pSuperEntry->cache.key == tvm_KEY(idSelector, idClass)) {
	  /* 2nd level cache hit */
	  pMethod = pSuperEntry->cache.pMethod;
#ifdef tvm_THREADED_CODE
	  pMethodCode = pSuperEntry->cache.pMethodCode;
#endif
	  /* release 2nd level cache lock */
	  tmthread_cacheUnlock();
#ifdef tvm_THREADED_CODE
	  goto *pMethodCode;
#else
	  goto lookupHit;
#endif
	}
	/* cache miss */ 
	{
	tyc_Class * pSuperClass = fp->pCode->pClass;
	if((pMethod = tvm_methodSuperLookup(idSelector, idClass,
					    pSuperClass))) {
	  pSuperEntry->superKey = idSuperClass;
	  pEntry = &pSuperEntry->cache;
	  /* invalidate, to avoid half-written entry 
	     in case the signature is wrong */
	  pEntry->key = tvm_INVALID_KEY;
	  goto methodCandidateFound;
	}
	/* method not implemented */
	goto dontUnderstand;
	}
	}
taggedInt1:
	idClass = tyc_ClassId_Int;
	goto super0;	
nil1:
	idClass = tyc_ClassId_Nil;
	goto super0;
	}
      LABEL(tvm_Op_sendTail):
      LABEL(tvm_Op_send):
	FETCH_SELECTORID(idSelector);
        pReceiver = sp[tyc_ARGUMENTS(idSelector)];
	goto send1;
      LABEL(tvm_Op_send5):
        pReceiver = sp[5];
	goto send0;
      LABEL(tvm_Op_send4):
        pReceiver = sp[4];
	goto send0;
      LABEL(tvm_Op_send3):
        pReceiver = sp[3];
	goto send0;
      LABEL(tvm_Op_send2):
        pReceiver = sp[2];
	goto send0;
      LABEL(tvm_Op_send1):
label_send1:
        pReceiver = sp[1];
	goto send0;
      LABEL(tvm_Op_send0):
label_send0:
        pReceiver = sp[0];
send0:
	FETCH_SELECTORID(idSelector);
send1:
	if(tsp_IS_IMMEDIATE(pReceiver))
	  goto taggedInt0;
	if(tyc_IS_NIL(pReceiver))
	  goto nil0;
	idClass = tsp_classId(pReceiver);
send2:
#ifdef tvm_DEBUG
	tvm_debugSend(idClass, idSelector);
#endif
#ifdef tvm_TRACE
	/* for builtin/slot return event: */
	pThread->pReceiver = pReceiver;
	if(debug_interesting(pThread, TRACE_SEND_BIT,
			     fp->pReceiver, pReceiver)) {
	  /* important: keep old value of IS_PAST_EVENT */
	  SAVE_REGS();
	  tmthread_debugSuspend(SUSP_SEND_BIT);
	  FETCH_THREAD();
	  LOAD_REGS();

	  pReceiver = sp[tyc_ARGUMENTS(idSelector)];
	}
	SET_PAST_EVENT(pThread);
#endif
	pEntry = &cache[HASH(idClass, idSelector)];
	if(pEntry->key == tvm_KEY(idSelector, idClass))
cacheHit:
	  {
	  /* 1st level cache hit */
	  /* get method from read only cache */
	  pMethod = pEntry->pMethod;
#ifdef tvm_THREADED_CODE
	  goto *pEntry->pMethodCode;
#else
lookupHit:
	  switch(tsp_classId(pMethod))
	  {
#endif  
	  METHODLABEL(CompiledMethod):
methodcall:
	  {
	    /* valid variables: pReceiver, pMethod, idSelector */
	    if(*ip == tvm_Op_sendTail) {
	      /* tail recursive call */
	      Word i, nArgs;
	      tsp_OID * p;
	      nArgs = tyc_ARGUMENTS(idSelector);
	      /* remove frame */
	      p = sp + nArgs;
	      sp = ((tsp_OID*)(fp + 1)) + fp->pCode->method.nArgs;
	      ip = fp->parent.ip;
	      fp = fp->parent.fp;
	      /* copy receiver & arguments */
	      *sp = *p;
	      for(i = nArgs; i > 0; i--) {
		*--sp = *--p;
	      }
	    }
#ifndef tvm_TRACE
nonrec_methodcall:
#endif
	    /* ordinary method call */
	    /* valid variables: pReceiver, pMethod */
	    sp -= sizeof(tyc_StackFrame) / sizeof(tsp_OID);
	    ((tyc_StackFrame*)sp)->parent.fp = fp;
	    fp = (tyc_StackFrame*)sp;
	    fp->parent.ip = ip;
	    fp->pReceiver = pReceiver;
	    fp->pCode = (tyc_CompiledMethod*)pMethod;
	    ip = fp->pByteCode = ((tyc_CompiledMethod*)pMethod)->pbCode;
	    CHECK_STACK_OVERFLOW();
	    CLEAR_PAST_EVENT(pThread);
	    SAMPLE();
	    NEXT();            /* continue interpreter loop */
	  }

	  METHODLABEL(ReorderMethod):
	  {
	    unsigned char *p;
	    int cycleLen;
	    tyc_Array pFillWords;
	    /* copy defaulted values to the stack */
	    /* ### stack limit check? */
	    pFillWords = ((tyc_ReorderMethod *)pMethod)->pFillWords;
	    if(pFillWords) {
	      int i;
	      i=tsp_size(pFillWords)/sizeof(tsp_OID);
	      while(i>0)
		*--sp = pFillWords[--i];
	    }
	    /* now stir... */
	    p = ((tyc_ReorderMethod *)pMethod)->aPermutation;
	    while( (cycleLen = *p++) > 0) {
	      int prevIdx = *p++;
	      tsp_OID first = sp[prevIdx];
	      do {
		int idx = *p++;
		sp[prevIdx] = sp[idx];
		prevIdx = idx;
	      } while(--cycleLen > 0);
	      sp[prevIdx] = first;
	    }
	    /* ...and jump to the full method */
	    idSelector = ((tyc_ReorderMethod *)pMethod)->idDelegateSelector;
#ifdef tvm_THREADED_CODE
	    pMethodCode = ((tyc_ReorderMethod *)pMethod)->pDelegateMethodCode;
#endif
	    pMethod = ((tyc_ReorderMethod *)pMethod)->pDelegateMethod;
#ifdef tvm_THREADED_CODE
	    goto *pMethodCode;
#else
	    goto lookupHit;
#endif
	  }

	  METHODLABEL(SlotAccessMethod):
	  METHODLABEL(SlotReferenceMethod):
	  {
	    /* slot access */
	    Int i = ((tyc_SlotMethod*)pMethod)->iOffset;
	    assert(!tsp_IS_IMMEDIATE(pReceiver) && !tyc_IS_NIL(pReceiver));
	    assert(i >= 0 && i < (tsp_size(pReceiver) / sizeof(tsp_OID)));
	    *sp = ((tsp_OID*)pReceiver)[i];
	    BNEXT();	       /* continue interpreter loop */
	  }

	  METHODLABEL(SlotTakeFromMethod):
	  {
	    /* slot access */
	    Int i = ((tyc_SlotMethod*)pMethod)->iOffset;
	    assert(!tsp_IS_IMMEDIATE(pReceiver) && !tyc_IS_NIL(pReceiver));
	    assert(i >= 0 && i < (tsp_size(pReceiver) / sizeof(tsp_OID)));
	    takeFromComponent(pThread, sp, pReceiver, &((tsp_OID*)pReceiver)[i]);
	    BNEXT();
	  }

	  METHODLABEL(SlotUpdateMethod):
	  {
	    /* slot update */
	    tsp_OID pArgument = *sp;
	    Int i = ((tyc_SlotMethod*)pMethod)->iOffset;
	    assert(!tsp_IS_IMMEDIATE(pReceiver) && !tyc_IS_NIL(pReceiver));
	    assert(i >= 0 && i < (tsp_size(pReceiver) / sizeof(tsp_OID)));
	    ((tsp_OID*)pReceiver)[i] = pArgument;
	    *++sp = pArgument;
	    BNEXT();
	  }

	  METHODLABEL(SlotMoveToMethod):
	  {
	    /* slot update */
	    Int i = ((tyc_SlotMethod*)pMethod)->iOffset;
	    assert(!tsp_IS_IMMEDIATE(pReceiver) && !tyc_IS_NIL(pReceiver));
	    assert(i >= 0 && i < (tsp_size(pReceiver) / sizeof(tsp_OID)));
	    if(isSuperComponentOf(*sp, pReceiver)) {
	      SAVE_REGS();
	      RAISE_CyclicComponent(*sp, pReceiver);
	    }
	    moveToComponent(pReceiver, &((tsp_OID*)pReceiver)[i], pThread, sp);
	    *++sp = tyc_NIL;
	    BNEXT();
	  }

#ifndef tvm_THREADED_CODE 
	  METHODLABEL(BuiltinMethod):
	  {
	    /* builtin method call */
	    switch(((tyc_BuiltinMethod*)pMethod)->iNumber)
	      {
#endif
	      /* generic integer arithmetic operation */
              #define INTOP(op) \
		{ \
		tsp_OID pArgument = *sp; \
		assert(tyc_IS_TAGGED_INT(pReceiver)); \
		if(tyc_IS_TAGGED_INT(pArgument)) { \
		  tsp_OID pResult; \
		  Int wResult = tyc_TAGGED_INT_VALUE(pReceiver) op \
				tyc_TAGGED_INT_VALUE(pArgument); \
		  if(tyc_MUST_BOX_INT(wResult)) { \
		    SAVE_REGS(); \
		    pResult = (tsp_OID)tyc_boxInt(wResult); \
		    FETCH_THREAD(); \
		    LOAD_REGS(); \
		  } \
		  else { \
		    pResult = tyc_TAG_INT(wResult); \
		  } \
		  *++sp = pResult; \
		  BNEXT(); \
		} \
		SAVE_REGS(); \
		RAISE_TypeError(pArgument, tyc_ClassId_Int); \
		}
	      BLABEL(tvm_Builtin_Int_add):  INTOP(+);
	      BLABEL(tvm_Builtin_Int_sub):  INTOP(-);
	      BLABEL(tvm_Builtin_Int_mult): INTOP(*);
	      BLABEL(tvm_Builtin_Int_or):   INTOP(|);
	      BLABEL(tvm_Builtin_Int_and):  INTOP(&);
	      BLABEL(tvm_Builtin_Int_xor):  INTOP(^);
	      BLABEL(tvm_Builtin_Int_shiftRight): INTOP(>>);
	      BLABEL(tvm_Builtin_Int_shiftLeft):  INTOP(<<);
	      /* catch division by zero */
	      BLABEL(tvm_Builtin_Int_div):
		{
		tsp_OID pArgument = *sp;
		assert(tyc_IS_TAGGED_INT(pReceiver));
		if(tyc_IS_TAGGED_INT(pArgument)) {
		  tsp_OID pResult;
		  Int wResult = tyc_TAGGED_INT_VALUE(pArgument);
		  if(wResult != 0) {
		    wResult = div(tyc_TAGGED_INT_VALUE(pReceiver),
				  wResult).quot;
		    if(tyc_MUST_BOX_INT(wResult)) {
		      SAVE_REGS();
		      pResult = (tsp_OID)tyc_boxInt(wResult);
		      FETCH_THREAD();
		      LOAD_REGS();
		    }
		    else {
		      pResult = tyc_TAG_INT(wResult);
		    }
		    *++sp = pResult;
		    BNEXT();
		  }
		  /* division by zero */
divisionbyzero:
		  {
		  SAVE_REGS();
		  RAISE_DivisionByZero(*sp);
		  }
		}
		SAVE_REGS();
		RAISE_TypeError(pArgument, tyc_ClassId_Int);
		}
	      BLABEL(tvm_Builtin_Int_mod):
		{
		tsp_OID pArgument = *sp;
		assert(tyc_IS_TAGGED_INT(pReceiver));
		if(tyc_IS_TAGGED_INT(pArgument)) {
		  tsp_OID pResult;
		  Int wResult = tyc_TAGGED_INT_VALUE(pArgument);
		  if(wResult != 0) {
		    wResult = div(tyc_TAGGED_INT_VALUE(pReceiver),
				  wResult).rem;
		    if(tyc_MUST_BOX_INT(wResult)) {
		      SAVE_REGS();
		      pResult = (tsp_OID)tyc_boxInt(wResult);
		      FETCH_THREAD();
		      LOAD_REGS();
		    }
		    else {
		      pResult = tyc_TAG_INT(wResult);
		    }
		    *++sp = pResult;
		    BNEXT();
		  }
		  /* division by zero */
		  goto divisionbyzero;
		}
		SAVE_REGS();
		RAISE_TypeError(pArgument, tyc_ClassId_Int);
		}
		/* generic int compare operation */
              #define INTCMP(op) \
		{ \
		tsp_OID pArgument = *sp; \
		assert(tyc_IS_TAGGED_INT(pReceiver)); \
		if(tyc_IS_TAGGED_INT(pArgument)) { \
		  *++sp = tyc_boxBool(tyc_TAGGED_INT_VALUE(pReceiver) op \
				      tyc_TAGGED_INT_VALUE(pArgument)); \
		  BNEXT(); \
		} \
		SAVE_REGS(); \
		RAISE_TypeError(pArgument, tyc_ClassId_Int); \
		}	      
	      BLABEL(tvm_Builtin_Int_less):           INTCMP(<);
	      BLABEL(tvm_Builtin_Int_lessOrEqual):    INTCMP(<=);
	      BLABEL(tvm_Builtin_Int_greater):        INTCMP(>);
	      BLABEL(tvm_Builtin_Int_greaterOrEqual): INTCMP(>=);
	      BLABEL(tvm_Builtin_Int_equal):
		{
		tsp_OID pArgument = *sp;
		assert(tyc_IS_TAGGED_INT(pReceiver));
		if(tyc_IS_TAGGED_INT(pArgument)) {
		  *++sp = tyc_boxBool(tyc_TAGGED_INT_VALUE(pReceiver) ==
				      tyc_TAGGED_INT_VALUE(pArgument));
		}
		else {
		  *++sp = tyc_FALSE;
		}
		BNEXT();
		}	      
	      BLABEL(tvm_Builtin_Int_not):
	        {
		Int wResult;
		tsp_OID pResult;
		assert(tyc_IS_TAGGED_INT(pReceiver));
		wResult = ~(tyc_TAGGED_INT_VALUE(pReceiver));
		if(tyc_MUST_BOX_INT(wResult)) {
		  SAVE_REGS();
		  pResult = (tsp_OID)tyc_boxInt(wResult);
		  FETCH_THREAD();
		  LOAD_REGS();
		}
		else {
		  pResult = tyc_TAG_INT(wResult);
		}
		*sp = pResult;
		}
		BNEXT();
	      BLABEL(tvm_Builtin_Int_asChar):
		assert(tyc_IS_TAGGED_INT(pReceiver));
		*sp = tyc_boxChar(tyc_TAGGED_INT_VALUE(pReceiver));
		BNEXT();
	      BLABEL(tvm_Builtin_Object_equal):
		{
		tsp_OID pArgument = *sp;
		*++sp = tyc_boxBool(pReceiver == pArgument);
		}
		BNEXT();
	      BLABEL(tvm_Builtin_Object_class):
		*sp = tyc_CLASS(tyc_CLASSID(pReceiver));
		BNEXT();
	      BLABEL(tvm_Builtin_Object_true):
		*sp = tyc_TRUE;
		BNEXT();
	      BLABEL(tvm_Builtin_Object_false):
		*sp = tyc_FALSE;
		BNEXT();
	      BLABEL(tvm_Builtin_Object_nil):
	      BLABEL(tvm_Builtin_Object_nilAt):
		*sp = tyc_NIL;
		BNEXT();
	      BLABEL(tvm_Builtin_Object_shallowCopy):
		{
		tyc_Class *pClass;
		tsp_OID pNewObject;
		assert(!tyc_IS_NIL(pReceiver) &&
		       !tyc_IS_TAGGED_INT(pReceiver));
		SAVE_REGS();
		pNewObject = tsp_newEmptyCopy(pReceiver);
		FETCH_THREAD();
		LOAD_REGS();
		pReceiver = *sp;
		pClass = tyc_CLASS(tyc_CLASSID(pReceiver));
		memcpy(pNewObject, pReceiver, tsp_size(pReceiver));
		/* ### transfer immutability? */
		*sp = pNewObject;
		}
		BNEXT();
	      BLABEL(tvm_Builtin_Object_hollowCopy):
		{
		tsp_OID pNewObject;
		assert(!tyc_IS_NIL(pReceiver));
		SAVE_REGS();
		if (tyc_IS_TAGGED_INT(pReceiver))
		  pNewObject = tyc_boxInt(tyc_UNTAG_INT(pReceiver));
		else
		  pNewObject = tsp_newEmptyCopy(pReceiver);
		FETCH_THREAD();
		LOAD_REGS();
		*sp = pNewObject;
		}
		BNEXT();
	      BLABEL(tvm_Builtin_Object__typeCast):
		{
		tsp_OID v = *sp;
		*++sp = v;
		}
		BNEXT();
	      BLABEL(tvm_Builtin_Object_fetchAt):
		{
		tsp_OID v = *sp;
		if (!tyc_IS_NIL(v)) {
		  if (tsp_IS_IMMEDIATE(v) || tsp_superComponent(v) != tyc_NIL) {
		    SAVE_REGS();
		    RAISE_FetchBoundComponent(v);
		  }
		  tsp_setSuperComponent(v, pThread);
		}
		*++sp = v;
		}
		BNEXT();
	      BLABEL(tvm_Builtin_Object__hash):
		assert(!tyc_IS_NIL(pReceiver) &&
		       !tyc_IS_TAGGED_INT(pReceiver));
		*sp = tyc_TAG_INT(tsp_hash(pReceiver));
		BNEXT();
	      BLABEL(tvm_Builtin_Object__setHash):
		{
		tsp_OID pArgument = *sp;
		assert(!tyc_IS_NIL(pReceiver) &&
		       !tyc_IS_TAGGED_INT(pReceiver));
		if(tyc_IS_TAGGED_INT(pArgument)) {
		  tsp_setHash(pReceiver, tyc_TAGGED_INT_VALUE(pArgument));
		  *++sp = tyc_TAG_INT(tsp_hash(pReceiver));
		  BNEXT();
		}
		SAVE_REGS();
		RAISE_TypeError(pArgument, tyc_ClassId_Int);
		}
	      BLABEL(tvm_Builtin_Object_superComponent):
		if(tyc_IS_NIL(pReceiver)) {
		  SAVE_REGS();
		  RAISE_TypeError(pReceiver, tyc_ClassId_Object);
		}
		if (tyc_IS_TAGGED_INT(pReceiver))
		  *sp = tyc_pRoot;
		else
		  *sp = tsp_superComponent(pReceiver);
		BNEXT();
	      BLABEL(tvm_Builtin_Object_isImmutable):
		if(tyc_IS_NIL(pReceiver) || tyc_IS_TAGGED_INT(pReceiver))
		  *sp = tyc_TRUE;
		else
		  *sp = tyc_boxBool(tsp_immutable(pReceiver));
		BNEXT();
	      BLABEL(tvm_Builtin_Object_setImmutable):
		if(!tyc_IS_NIL(pReceiver) && !tyc_IS_TAGGED_INT(pReceiver))
		  tsp_setImmutable(pReceiver);
		*sp = tyc_TRUE;
		BNEXT();
	      BLABEL(tvm_Builtin_Object_isTracedComponent):
		if(tyc_IS_NIL(pReceiver) || tyc_IS_TAGGED_INT(pReceiver))
		  *sp = tyc_FALSE;
		else
		  *sp = tyc_boxBool(tsp_isTracedComponent(pReceiver));
		BNEXT();
	      BLABEL(tvm_Builtin_Object_setIsTracedComponent):
	        {
		  tyc_Bool * pArgument = *sp;
		  if(!tyc_IS_NIL(pReceiver) && !tyc_IS_TAGGED_INT(pReceiver)) {
		    if(tyc_IS_TRUE(pArgument)) {
		      tsp_setIsTracedComponent(pReceiver, TRUE);
		    } else if(tyc_IS_FALSE(pArgument)) {
		      tsp_setIsTracedComponent(pReceiver, FALSE);
		    } else {
		      SAVE_REGS();
		      RAISE_TypeError(pArgument, tyc_ClassId_Bool);
		    }
		    *++sp = pArgument;
		    BNEXT();
		  }
		  SAVE_REGS();
		  RAISE_TypeError(pReceiver, tyc_ClassId_Object);
		}
	      BLABEL(tvm_Builtin_Object__basicSize):
		if(tyc_IS_NIL(pReceiver)) {
		  *sp = tyc_TAG_INT(0);
		}
		else if (tyc_IS_TAGGED_INT(pReceiver)) {
		  *sp = tyc_TAG_INT(1);
		}
		else if(!tsp_isCStruct(pReceiver)) {
		  *sp = tyc_TAG_INT(tsp_size(pReceiver) / sizeof(tsp_OID));
		}
		else {
		  *sp = tyc_TAG_INT(tsp_getCSize(pReceiver));
		}
		BNEXT();
	      BLABEL(tvm_Builtin_Object__basicAt):
		{
	      	tsp_OID pArgument = *sp;
		if(tyc_IS_TAGGED_INT(pArgument)) {
		  Int i = tyc_TAGGED_INT_VALUE(pArgument);
		  if(tyc_IS_NIL(pReceiver)) {
		    /* index out of bounds */
		    SAVE_REGS();
		    RAISE_IndexOutOfBounds(pReceiver, pArgument);
		  }
		  else if(tsp_IS_IMMEDIATE(pReceiver)) {
		    if(i == 0) {
		      *++sp = pReceiver;
		      BNEXT();
		    }
		    /* index out of bounds */
		    SAVE_REGS();
		    RAISE_IndexOutOfBounds(pReceiver, pArgument);
		  } else if(!tsp_isCStruct(pReceiver)) {
		    if(i >= 0 && i < (tsp_size(pReceiver) / sizeof(tsp_OID))) {
		      *++sp = ((tsp_OID*)pReceiver)[i];
		      BNEXT();
		    }
		    /* index out of bounds */
		    SAVE_REGS();
		    RAISE_IndexOutOfBounds(pReceiver, pArgument);
		  }
		  else {
		    if(i >= 0 && i < tsp_getCSize(pReceiver)) {
		      tsp_OID pSlot;
		      SAVE_REGS();
		      pSlot = tsp_getCSlot(pReceiver, i);
		      FETCH_THREAD();
		      LOAD_REGS();
		      *++sp = pSlot;
		      BNEXT();
		    }
		    /* index out of bounds */
		    SAVE_REGS();
		    RAISE_IndexOutOfBounds(pReceiver, pArgument);
		  }
		}
		SAVE_REGS();
		RAISE_TypeError(pArgument, tyc_ClassId_Int);
		}
	      BLABEL(tvm_Builtin_Object__basicAtPut):
		{
		tsp_OID pElement =  sp[0];
		tsp_OID pPosition = sp[1];
		if(tyc_IS_NIL(pReceiver)
		   || tsp_IS_IMMEDIATE(pReceiver)
		   || tsp_immutable(pReceiver)) {
		  SAVE_REGS();
		  RAISE_WriteToImmutable(pReceiver);
		}
		if(tyc_IS_TAGGED_INT(pPosition)) {
		  Int i = tyc_TAGGED_INT_VALUE(pPosition);
		  if(!tsp_isCStruct(pReceiver)) {
		    if(i >= 0 && i < (tsp_size(pReceiver) / sizeof(tsp_OID))) {
		      tyc_Class *pClass = tyc_CLASS(tyc_CLASSID(pReceiver));
		      if((i < tyc_UNTAG_INT(pClass->wInstanceSize)
			  && pClass->slotMap[i]->isComponent)
			 || tyc_CLASSID(pReceiver) == tyc_ClassId_AtArray) {
			  /* put to component slot => element is magically fetched */
			  if (!tyc_IS_NIL(pElement)) {
			      if (tsp_IS_IMMEDIATE(pElement)
				  || tsp_superComponent(pElement) != tyc_NIL) {
				  SAVE_REGS();
				  RAISE_FetchBoundComponent(pElement);
			      }
			      if(isSuperComponentOf(pElement, pReceiver)) {
				SAVE_REGS();
				RAISE_CyclicComponent(pElement, pReceiver);
			      }
			      tsp_setSuperComponent(pElement, pReceiver);
			  }
		      }
		      ((tsp_OID*)pReceiver)[i] = pElement;
		      sp += 2;
		      *sp = pElement;
		      BNEXT();
		    }
		    /* index out of bounds */
		    SAVE_REGS();
		    RAISE_IndexOutOfBounds(pReceiver, pPosition);
		  }
		  else {
		    if(i >= 0 && i < tsp_getCSize(pReceiver)) {
		      tyc_Class *pClass = tyc_CLASS(tyc_CLASSID(pReceiver));
		      int fMoved = FALSE;
		      if(pClass->slotMap[i]->isComponent) {
			  /* put to component slot => element is magically fetched */
			  if (!tyc_IS_NIL(pElement)) {
			      if (tsp_IS_IMMEDIATE(pElement)
				  || tsp_superComponent(pElement) != tyc_NIL) {
				  SAVE_REGS();
				  RAISE_FetchBoundComponent(pElement);
			      }
			      if(isSuperComponentOf(pElement, pReceiver)) {
				SAVE_REGS();
				RAISE_CyclicComponent(pElement, pReceiver);
			      }
			      tsp_setSuperComponent(pElement, pReceiver);
			      fMoved = TRUE;
			  }
		      }
		      if(tsp_setCSlot(pReceiver, pElement, i)) {
			sp += 2;
			*sp = pElement;
			BNEXT();
		      }
		      if (fMoved) {
			  assert_superComponent(pReceiver, pElement);
			  tsp_setSuperComponent(pElement, tyc_NIL);
		      }
		      /* slot type mismatch */
		      SAVE_REGS();
		      RAISE_TypeError(pElement, tyc_ClassId_Object);
		    }
		    /* index out of bounds */
		    SAVE_REGS();
		    RAISE_IndexOutOfBounds(pReceiver, pPosition);
		  }
		}
		SAVE_REGS();
		RAISE_TypeError(pPosition, tyc_ClassId_Int);
		}
	      /* generic perform */
	      BLABEL(tvm_Builtin_Object__perform):
	      BLABEL(tvm_Builtin_Object__performAt):
		{
		tyc_Array pArray = sp[0];
		tyc_Selector *pSelector = sp[1];
		if(tyc_CLASSID(pSelector) == tyc_ClassId_Selector) {
		  tyc_Symbol pSymbol = pSelector->pSymbol;
		  Word wArity = pSelector->wArity;
		  Word wSorts = pSelector->wSorts;
		  tyc_Symbol *apKeywords = pSelector->apKeywords;
		  int nKeywords4;
		  assert(apKeywords);
		  nKeywords4 = tsp_size(apKeywords);
		  if(((tyc_BuiltinMethod*)pMethod)->iNumber ==
		     tvm_Builtin_Object__performAt)
		    assert(pSymbol[tsp_size(pSymbol)-2] == '@');
		  else
		    assert(pSymbol[tsp_size(pSymbol)-2] != '@');
		  if(tyc_CLASSID(pArray) == tyc_ClassId_Array ||
		     tyc_CLASSID(pArray) == tyc_ClassId_MutableArray) {
		    if(wArity == tsp_size(pArray) / sizeof(tsp_OID)) {
		      Word i, n;
		      tvm_performCacheLine * pCacheLine =
			&performCache[PERFORMHASH(pSymbol, wArity, wSorts, apKeywords)];
		      /* consult mini cache */
		      tmthread_performLock();
		      assert(!pCacheLine->pSymbol || pCacheLine->apKeywords);
		      if(pCacheLine->pSymbol == pSymbol &&
			 pCacheLine->wArity == wArity &&
			 pCacheLine->wSorts == wSorts &&
			 (pCacheLine->apKeywords == apKeywords
			 || (tsp_size(pCacheLine->apKeywords) == nKeywords4 &&
			     memcmp(pCacheLine->apKeywords, apKeywords, nKeywords4)==0))) {
			idSelector = pCacheLine->idSelector;
		      }
		      else {
			/* brute force lookup */  
			tyc_Selector ** ppSelector = tyc_pRoot->apSelectorTable;
			n = tyc_pRoot->wSelectorTableSize; 
			/* search for idSelector */
			for(i = 0; i < n; i++, ppSelector++) {
			  tyc_Selector * pSelector = *ppSelector;
			  assert(pSelector->apKeywords);
			  if(pSelector->pSymbol == pSymbol &&
			     pSelector->wArity == wArity &&
			     pSelector->wSorts == wSorts &&
			     (pSelector->apKeywords == apKeywords
			      || (tsp_size(pSelector->apKeywords) == nKeywords4 &&
			          memcmp(pSelector->apKeywords, apKeywords, nKeywords4)==0))) {
			    break;
			  }
			}
			if(i == n) {
			  tmthread_performUnlock();
			  SAVE_REGS();
			  /* tell TL2 code to register the selector */
			  tvm_raise(tyc_pUnknownSelectorError);
			}
			/* update cache */
			pCacheLine->pSymbol = pSymbol;
			pCacheLine->wArity = wArity;
			pCacheLine->wSorts = wSorts;
			pCacheLine->apKeywords = apKeywords;
			pCacheLine->idSelector = idSelector = i;
		      }
		      tmthread_performUnlock();

		      /* fetch component arguments */
		      for(i = 0; i < wArity; i++) {
			tsp_OID v = pArray[i];
			if(v != tyc_NIL && (wSorts & (1<<i)) != 0) {
			  if (tsp_IS_IMMEDIATE(v) || tsp_superComponent(v) != tyc_NIL) {
			    /* exception: set the components free again */
			    while(i-- > 0) {
			      tsp_OID u = pArray[i];
			      if(u != tyc_NIL && (wSorts & (1<<i)) != 0) {
				tsp_setSuperComponent(u, tyc_NIL);
			      }
			    }
			    SAVE_REGS();
			    RAISE_FetchBoundComponent(v);
			  }
			  tsp_setSuperComponent(v, pThread);
			}
		      }

		      CLEAR_PAST_EVENT(pThread);
		      /* build return continuation for generated send */
		      if(*ip == tvm_Op_sendTail) {
			/* tail recursive perform */
			/* remove frame */
			sp = ((tsp_OID*)(fp + 1)) + fp->pCode->method.nArgs;
			ip = fp->parent.ip;
			fp = fp->parent.fp;
			/* copy receiver & arguments */
			*sp = pReceiver;
			*--sp = pSelector;
			*--sp = pArray;
		      }
		      sp -= sizeof(tyc_StackFrame) / sizeof(tsp_OID);
		      ((tyc_StackFrame*)sp)->parent.fp = fp;
		      fp = (tyc_StackFrame*)sp;
		      fp->parent.ip = ip;
		      fp->pReceiver = pReceiver;
		      fp->pCode = (tyc_CompiledMethod*)pMethod;
		      ip = fp->pByteCode = pThread->pPerformCodeBuffer;
		      ip[0] = tvm_Op_sendTail;
		      ip[1] = idSelector & 0xff;
		      ip[2] = idSelector >> 8;
		      ip[3] = tvm_Op_return;

		      /* ### check for stack overflow? */

		      *--sp = pReceiver;
		      /* copy new arguments */
		      for(i = 0; i < wArity; i++) {
			*--sp = pArray[i];
		      }
		      /* execute send */
		      goto send1;
		    }
		    SAVE_REGS();
		    RAISE_PerformArityMismatch(pSelector, pArray);
		  }
		  SAVE_REGS();
		  RAISE_TypeError(pArray, tyc_ClassId_Array);
		}
		SAVE_REGS();
		RAISE_TypeError(pSelector, tyc_ClassId_Selector);
		}
	      /* catch all methods */
	      BLABEL(tvm_Builtin_Object___doesNotUnderstand):
		{
		Word nArgs = tyc_ARGUMENTS(idSelector);
		Word wSorts;
		tsp_OID * pArray;
		tyc_Selector *pSelector;
		SAVE_REGS();
		pArray = tsp_newArray(tyc_ClassId_MutableArray, nArgs);
		FETCH_THREAD();
		LOAD_REGS();
		pSelector = tyc_pRoot->apSelectorTable[idSelector];
		wSorts = pSelector->wSorts;
		while(nArgs-- > 0) {
		  tsp_OID obj = *sp++;
		  if(obj != tyc_NIL && (wSorts & (1 << nArgs)) != 0) {
		    assert_superComponent(pThread, obj);
		    tsp_setSuperComponent(obj, tyc_NIL);
		  }
		  pArray[nArgs] = obj;
		}
		tsp_setImmutable(pArray);
		*--sp = pSelector;
		*--sp = pArray;
		/* call bytecode */
		pReceiver = sp[2];
		idSelector =
		   ((tyc_BuiltinMethod*)pMethod)->compiledMethod.idSelector;
		pMethod = (tyc_Method*)
		  &(((tyc_BuiltinMethod*)pMethod)->compiledMethod);
		goto methodcall;
		}
	      /* atomic operations for compiler */
	      BLABEL(tvm_Builtin_Class__set):
		{
		tsp_OID pInstanceSize = sp[0];
		tsp_OID pSlotMap = sp[1];
		tsp_OID pDictionaries = sp[2];
		SAVE_REGS();
		tmthread_criticalLock();
		tmthread_checkSyncRequest();
		tmthread_syncRequest();
		{
		/* atomic modification of class object */
		((tyc_Class*)pReceiver)->pMethodDictionaries = pDictionaries;
		((tyc_Class*)pReceiver)->slotMap = pSlotMap;
		((tyc_Class*)pReceiver)->wInstanceSize = pInstanceSize;
		/* clear cache afterwards */
		tvm_clearCache(TRUE);
		}
		tmthread_syncRelease();
		tmthread_criticalUnlock();
		FETCH_THREAD();
		LOAD_REGS();
		sp += 3;
		*sp = tyc_NIL;
		}
		BNEXT();
	      BLABEL(tvm_Builtin_String_at):
		{
		tsp_OID pArgument = *sp;
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_String ||
		       tyc_CLASSID(pReceiver) == tyc_ClassId_MutableString ||
		       tyc_CLASSID(pReceiver) == tyc_ClassId_Symbol);
		if(tyc_IS_TAGGED_INT(pArgument)) {
		  Int i = tyc_TAGGED_INT_VALUE(pArgument);
		  if(i >= 0 && i < (tsp_size(pReceiver) - 1)) {
		    *++sp = tyc_boxChar(((Char*)pReceiver)[i]);
		    BNEXT();
		  }
		  /* index out of bounds */
		  SAVE_REGS();
		  RAISE_IndexOutOfBounds(pReceiver, pArgument);
		}
		SAVE_REGS();
		RAISE_TypeError(pArgument, tyc_ClassId_Int);
		}
	      BLABEL(tvm_Builtin_String_size):
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_String ||
		       tyc_CLASSID(pReceiver) == tyc_ClassId_MutableString ||
		       tyc_CLASSID(pReceiver) == tyc_ClassId_Symbol);
		*sp = tyc_TAG_INT(tsp_size(pReceiver) - 1);
		BNEXT();
	      BLABEL(tvm_Builtin_MutableString_atPut):
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_MutableString);
		{
		tsp_OID pChar =	     sp[0];
		tsp_OID pPosition  = sp[1];
		if(tyc_IS_TAGGED_INT(pPosition) &&
		   tyc_CLASSID(pChar) == tyc_ClassId_Char) {
		    if(!tsp_immutable(pReceiver)) {
			Int i = tyc_TAGGED_INT_VALUE(pPosition);
			if( i >= 0 && i < (tsp_size(pReceiver) - 1)) {
			    ((Char*)pReceiver)[i] = tyc_CHAR_VALUE(pChar);
			    sp += 2;
			    *sp = pChar;
			    BNEXT();
			}
			/* index out of bounds */
			SAVE_REGS();
			RAISE_IndexOutOfBounds(pReceiver, pPosition);
		    }
		    SAVE_REGS();
		    RAISE_WriteToImmutable(pReceiver);
		}
		SAVE_REGS();
		if(!tyc_IS_TAGGED_INT(pPosition)) {
		  RAISE_TypeError(pPosition, tyc_ClassId_Int);
		}
		else {
		  RAISE_TypeError(pChar, tyc_ClassId_Char);
		}
		}
	      BLABEL(tvm_Builtin_SymbolClass__new):
		{
		tsp_OID pArgument = *sp;
		tsp_OID pNew;
		tsp_ClassId id = tyc_CLASSID(pArgument);

		if (id == tyc_ClassId_String ||
		    id == tyc_ClassId_MutableString) {
		  Int n = tsp_size(pArgument) - 1;
		  SAVE_REGS();
		  pNew = tsp_newByteArray(tyc_ClassId_Symbol, n+1);
		  FETCH_THREAD();
		  LOAD_REGS();
		  pArgument = *sp;
		  memmove((Char*)pNew, (Char*)pArgument, n * sizeof(Char));
		  tsp_setImmutable(pNew);
		  *++sp = pNew;
		  BNEXT();
		}
		SAVE_REGS();
		RAISE_TypeError(pArgument, tyc_ClassId_String);
		}
	      BLABEL(tvm_Builtin_MutableString_replace):
		{
		tsp_OID pStartAt = sp[0];
		tsp_OID pWith =	   sp[1];
		tsp_OID pAt =	   sp[2];
		tsp_OID pN =	   sp[3];
		tsp_ClassId id = tyc_CLASSID(pWith);
		if(tyc_CLASSID(pReceiver) == tyc_ClassId_MutableString
		  && !tsp_immutable(pReceiver)) {
		  if(tyc_IS_TAGGED_INT(pAt) && tyc_IS_TAGGED_INT(pN) &&
		     tyc_IS_TAGGED_INT(pStartAt) &&
		     (id == tyc_ClassId_String ||
		      id == tyc_ClassId_MutableString ||
		      id == tyc_ClassId_Symbol)) {
		    Int at = tyc_TAGGED_INT_VALUE(pAt);
		    Int n = tyc_TAGGED_INT_VALUE(pN);
		    Int startAt = tyc_TAGGED_INT_VALUE(pStartAt);
		    if(n >= 0 && at >= 0 && (at + n) < tsp_size(pReceiver) &&
		       startAt >= 0 && (startAt + n) < tsp_size(pWith)) {
		      memmove((Char*)pReceiver + at, (Char*)pWith + startAt,
			      n * sizeof(Char));
		      sp += 4;
		      *sp = tyc_NIL;
		      BNEXT();
		    }
		    /* index out of bounds */
		    SAVE_REGS();
		    if(startAt < 0 || (startAt + n) >= tsp_size(pWith)) {
		      if(startAt < 0)
			RAISE_IndexOutOfBounds(pWith, pStartAt);
		      else
			RAISE_IndexOutOfBounds(pWith,
					       tyc_TAG_INT(startAt + n - 1));
		    }
		    else {
		      if(at < 0)
			RAISE_IndexOutOfBounds(pReceiver, pAt);
		      else
			RAISE_IndexOutOfBounds(pReceiver,
					       tyc_TAG_INT(at + n - 1));
		    }
		  }
		}
		/* call bytecode */
		pMethod = (tyc_Method*)
		  &(((tyc_BuiltinMethod*)pMethod)->compiledMethod);
		goto methodcall;
		}
	      BLABEL(tvm_Builtin_Array_at):
		{
		tsp_OID pArgument = *sp;
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_Array ||
		       tyc_CLASSID(pReceiver) == tyc_ClassId_MutableArray ||
		       tyc_CLASSID(pReceiver) == tyc_ClassId_AtArray);
		if(tyc_IS_TAGGED_INT(pArgument)) {
		  Int i = tyc_TAGGED_INT_VALUE(pArgument);
		  if(i >= 0 && i < (tsp_size(pReceiver) / sizeof(tsp_OID))) {
		    *++sp = ((tsp_OID*)pReceiver)[i];
		    BNEXT();
		  }
		  /* index out of bounds */
		  SAVE_REGS();
		  RAISE_IndexOutOfBounds(pReceiver, pArgument);
		}
		SAVE_REGS();
		RAISE_TypeError(pArgument, tyc_ClassId_Int);
		}
	      BLABEL(tvm_Builtin_Array_size):
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_Array ||
		       tyc_CLASSID(pReceiver) == tyc_ClassId_MutableArray ||
		       tyc_CLASSID(pReceiver) == tyc_ClassId_AtArray);
		*sp = tyc_TAG_INT(tsp_size(pReceiver) / sizeof(tsp_OID));
		BNEXT();
	      BLABEL(tvm_Builtin_AtArray_takeFrom):
		{
		tsp_OID pArgument = *sp;
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_AtArray);
		if(tyc_IS_TAGGED_INT(pArgument)) {
		  Int i = tyc_TAGGED_INT_VALUE(pArgument);
		  if(i >= 0 && i < (tsp_size(pReceiver) / sizeof(tsp_OID))) {
		    ++sp;
		    takeFromComponent(pThread, sp, pReceiver, &((tsp_OID*)pReceiver)[i]);
		    BNEXT();
		  }
		  /* index out of bounds */
		  SAVE_REGS();
		  RAISE_IndexOutOfBounds(pReceiver, pArgument);
		}
		SAVE_REGS();
		RAISE_TypeError(pArgument, tyc_ClassId_Int);
		}
	      BLABEL(tvm_Builtin_MutableArray_replace):
		{
		tsp_OID pStartAt = sp[0];
		tsp_OID pWith =	   sp[1];
		tsp_OID pAt =	   sp[2];
		tsp_OID pN =	   sp[3];
		tsp_ClassId id = tyc_CLASSID(pWith);
		if(tyc_CLASSID(pReceiver) == tyc_ClassId_MutableArray
		   && !tsp_immutable(pReceiver)) {
		  if(tyc_IS_TAGGED_INT(pAt) && tyc_IS_TAGGED_INT(pN) &&
		     tyc_IS_TAGGED_INT(pStartAt) &&
		     (id == tyc_ClassId_Array ||
		      id == tyc_ClassId_MutableArray)) {
		    Int at = tyc_TAGGED_INT_VALUE(pAt);
		    Int n = tyc_TAGGED_INT_VALUE(pN);
		    Int startAt = tyc_TAGGED_INT_VALUE(pStartAt);
		    if(n >= 0 && at >= 0 && startAt >= 0 &&
		       (at + n) <= (tsp_size(pReceiver) / sizeof(tsp_OID)) &&
		       (startAt + n) <= (tsp_size(pWith) / sizeof(tsp_OID))) {
		      memmove((tsp_OID*)pReceiver + at,
			      (tsp_OID*)pWith + startAt, n * sizeof(tsp_OID));
		      sp += 4;
		      *sp = tyc_NIL;
		      BNEXT();
		    }
		    /* index out of bounds */
		    SAVE_REGS();
		    if(startAt < 0 || (startAt + n) > tsp_size(pWith) / sizeof(tsp_OID)) {
		      if(startAt < 0)
			RAISE_IndexOutOfBounds(pWith, pStartAt);
		      else
			RAISE_IndexOutOfBounds(pWith,
					       tyc_TAG_INT(startAt + n - 1));
		    }
		    else {
		      if(at < 0)
			RAISE_IndexOutOfBounds(pReceiver, pAt);
		      else
			RAISE_IndexOutOfBounds(pReceiver,
					       tyc_TAG_INT(at + n - 1));
		    }
		  }
		}
		/* call bytecode */
		pMethod = (tyc_Method*)
		  &(((tyc_BuiltinMethod*)pMethod)->compiledMethod);
		goto methodcall;
		}
	      BLABEL(tvm_Builtin_AtArray_moveTo):
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_AtArray);
		{
		  /* tsp_OID pElement =	 sp[0]; */
		tsp_OID pPosition = sp[1];
		if(tyc_IS_TAGGED_INT(pPosition)) {
		  Int i = tyc_TAGGED_INT_VALUE(pPosition);
		  if(i >= 0 && i < (tsp_size(pReceiver) / sizeof(tsp_OID))) {
		    if(!isSuperComponentOf(*sp, pReceiver)) {
		      moveToComponent(pReceiver, &((tsp_OID*)pReceiver)[i], pThread, sp);
		      sp += 2;
		      *sp = tyc_NIL;
		      BNEXT();
		    }
		    SAVE_REGS();
		    RAISE_CyclicComponent(*sp, pReceiver);
		  }
		  /* index out of bounds */
		  SAVE_REGS();
		  RAISE_IndexOutOfBounds(pReceiver, pPosition);	    
		}
		SAVE_REGS();
		RAISE_TypeError(pPosition, tyc_ClassId_Int);
		}
	      BLABEL(tvm_Builtin_MutableArray_atPut):
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_MutableArray);
		{
		tsp_OID pElement =  sp[0];
		tsp_OID pPosition = sp[1];
		if(!tsp_immutable(pReceiver)) {
		  if(tyc_IS_TAGGED_INT(pPosition)) {
		    Int i = tyc_TAGGED_INT_VALUE(pPosition);
		    if(i >= 0 && i < (tsp_size(pReceiver) / sizeof(tsp_OID))) {
		      ((tsp_OID*)pReceiver)[i] = pElement;
		      sp += 2;
		      *sp = pElement;
		      BNEXT();
		    }
		    /* index out of bounds */
		    SAVE_REGS();
		    RAISE_IndexOutOfBounds(pReceiver, pPosition);	    
		  }
		  SAVE_REGS();
		  RAISE_TypeError(pPosition, tyc_ClassId_Int);
		}
		SAVE_REGS();
		RAISE_WriteToImmutable(pReceiver);
		}

	      /* arrays with integral values */
	      #define SHORTATOP(t) \
		{ \
		tsp_OID pArgument = *sp; \
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_ ## t ## Array); \
		if(tyc_IS_TAGGED_INT(pArgument)) { \
		  Int i = tyc_TAGGED_INT_VALUE(pArgument); \
		  if(i >= 0 && i < (tsp_size(pReceiver) / sizeof(t))) { \
		    *++sp = tyc_TAG_INT(((t*)pReceiver)[i]); \
		    BNEXT(); \
		  } \
		  /* index out of bounds */ \
		  SAVE_REGS(); \
		  RAISE_IndexOutOfBounds(pReceiver, pArgument); \
		} \
		SAVE_REGS(); \
		RAISE_TypeError(pArgument, tyc_ClassId_Int); \
		}
	      BLABEL(tvm_Builtin_ByteArray_at):	 SHORTATOP(Byte);
	      BLABEL(tvm_Builtin_ShortArray_at): SHORTATOP(Short);

	      #define ATPUTOP(t) \
		{ \
		tsp_OID pInt =	     sp[0]; \
		tsp_OID pPosition  = sp[1]; \
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_ ## t ## Array); \
		if(tyc_IS_TAGGED_INT(pPosition) && \
		   tyc_CLASSID(pInt) == tyc_ClassId_Int) { \
		  Int i = tyc_TAGGED_INT_VALUE(pPosition); \
		  if( i >= 0 && i < (tsp_size(pReceiver) / sizeof(t))) { \
		    ((t*)pReceiver)[i] = tyc_TAGGED_INT_VALUE(pInt); \
		    sp += 2; \
		    *sp = pInt; \
		    BNEXT(); \
		  } \
		  /* index out of bounds */ \
		  SAVE_REGS(); \
		  RAISE_IndexOutOfBounds(pReceiver, pPosition); \
		} \
		SAVE_REGS(); \
		if(!tyc_IS_TAGGED_INT(pPosition)) { \
		  RAISE_TypeError(pPosition, tyc_ClassId_Int); \
		} \
		else { \
		  RAISE_TypeError(pInt, tyc_ClassId_Int); \
		} \
		}
	      BLABEL(tvm_Builtin_ByteArray_atPut):  ATPUTOP(Byte);
	      BLABEL(tvm_Builtin_ShortArray_atPut): ATPUTOP(Short);
	      BLABEL(tvm_Builtin_IntArray_atPut):   ATPUTOP(Int);

	      BLABEL(tvm_Builtin_IntArray_at):
		{
		tsp_OID pNew, pArgument = *sp;
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_IntArray);
		if(tyc_IS_TAGGED_INT(pArgument)) {
		  Int i = tyc_TAGGED_INT_VALUE(pArgument);
		  if(i >= 0 && i < (tsp_size(pReceiver) / sizeof(Int))) {
		    SAVE_REGS();
		    pNew = tyc_TAG_MAYBEBOXED(((Int*)pReceiver)[i]);
		    FETCH_THREAD();
		    LOAD_REGS();
		    *++sp = pNew;
		    BNEXT();
		  }
		  /* index out of bounds */
		  SAVE_REGS();
		  RAISE_IndexOutOfBounds(pReceiver, pArgument);
		}
		SAVE_REGS();
		RAISE_TypeError(pArgument, tyc_ClassId_Int);
		}
	      BLABEL(tvm_Builtin_LongArray_at):
		{
		tsp_OID pNew, pArgument = *sp;
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_LongArray);
		if(tyc_IS_TAGGED_INT(pArgument)) {
		  Int i = tyc_TAGGED_INT_VALUE(pArgument);
		  if(i >= 0 && i < (tsp_size(pReceiver) / sizeof(Long))) {
		    SAVE_REGS();
		    pNew = tyc_boxLong(((Long*)pReceiver)[i]);
		    FETCH_THREAD();
		    LOAD_REGS();
		    *++sp = pNew;
		    BNEXT();
		  }
		  /* index out of bounds */
		  SAVE_REGS();
		  RAISE_IndexOutOfBounds(pReceiver, pArgument);
		}
		SAVE_REGS();
		RAISE_TypeError(pArgument, tyc_ClassId_Int);
		}
	      BLABEL(tvm_Builtin_LongArray_atPut):
		{
		tsp_OID pLong =	     sp[0];
		tsp_OID pPosition  = sp[1];
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_LongArray);
		if(tyc_IS_TAGGED_INT(pPosition) &&
		   tyc_CLASSID(pLong) == tyc_ClassId_Long) {
		  Int i = tyc_TAGGED_INT_VALUE(pPosition);
		  if( i >= 0 && i < (tsp_size(pReceiver) / sizeof(Long))) {
		    ((Long*)pReceiver)[i] = tyc_LONG_VALUE(pLong);
		    sp += 2;
		    *sp = pLong;
		    BNEXT();
		  } \
		  /* index out of bounds */
		  SAVE_REGS();
		  RAISE_IndexOutOfBounds(pReceiver, pPosition);
		}
		SAVE_REGS();
		if(!tyc_IS_TAGGED_INT(pPosition)) {
		  RAISE_TypeError(pPosition, tyc_ClassId_Int);
		}
		else {
		  RAISE_TypeError(pLong, tyc_ClassId_Long);
		}
		}

	      #define SIZEOP(t) \
		{ \
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_ ## t ## Array); \
		*sp = tyc_TAG_INT(tsp_size(pReceiver) / sizeof(t)); \
		}
	      BLABEL(tvm_Builtin_ByteArray_size):  SIZEOP(Byte); BNEXT();
	      BLABEL(tvm_Builtin_ShortArray_size): SIZEOP(Short); BNEXT();
	      BLABEL(tvm_Builtin_IntArray_size):   SIZEOP(Int); BNEXT();
	      BLABEL(tvm_Builtin_LongArray_size):  SIZEOP(Long); BNEXT();

	      #define NEWOP(t) \
		{ \
		tsp_OID pNew, pArgument = *sp; \
		tyc_Class * pClass = (tyc_Class*)pReceiver; \
		if(tyc_IS_TAGGED_INT(pArgument)) { \
		  Int i = tyc_TAGGED_INT_VALUE(pArgument); \
		  SAVE_REGS(); \
		  pNew = tsp_new ## t ## Array( \
			 tyc_UNTAG_INT(pClass->wInstanceId), i); \
		  assert(tsp_classId(pNew) == tyc_ClassId_ ## t ## Array); \
		  FETCH_THREAD(); \
		  LOAD_REGS(); \
		  *++sp = pNew; \
		  BNEXT(); \
		} \
		SAVE_REGS(); \
		RAISE_TypeError(pArgument, tyc_ClassId_Int); \
		}
	      BLABEL(tvm_Builtin_ByteArrayClass__new1):	 NEWOP(Byte);
	      BLABEL(tvm_Builtin_ShortArrayClass__new1): NEWOP(Short);
	      BLABEL(tvm_Builtin_IntArrayClass__new1):	 NEWOP(Int);
	      BLABEL(tvm_Builtin_LongArrayClass__new1):	 NEWOP(Long);
	      /* end of arrays */

	      BLABEL(tvm_Builtin_Char_asInt):
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_Char);
		*sp = tyc_TAG_INT(tyc_CHAR_VALUE(pReceiver));
		BNEXT();
	      BLABEL(tvm_Builtin_Char_equal):
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_Char);
		{
		tsp_OID pArgument = *sp;
		if(tyc_CLASSID(pArgument) == tyc_ClassId_Char)
		  *++sp = tyc_boxBool(tyc_CHAR_VALUE(pReceiver) ==
				      tyc_CHAR_VALUE(pArgument));
		else
		  *++sp = tyc_FALSE;
		}
		BNEXT();
	      BLABEL(tvm_Builtin_DLL___open):
		{
		void * handle;
		tsp_OID pNew, pPath = *sp;
		tsp_ClassId id = tyc_CLASSID(pPath);
		if( id == tyc_ClassId_String ||
		    id == tyc_ClassId_MutableString ||
		    id == tyc_ClassId_Symbol) {
		  SAVE_REGS();
		  if(!(handle = rtdll_open(pPath))) {
		    /* raise exception */
		    RAISE_DLLOpenError(pReceiver);
		  }
		  pNew = tyc_TAG_MAYBEBOXED(handle);
		  FETCH_THREAD();
		  LOAD_REGS();
		  *++sp = pNew;
		  BNEXT();
		}
		SAVE_REGS();
		RAISE_TypeError(pPath, tyc_ClassId_String);
		}
	      BLABEL(tvm_Builtin_DLL___close):
		{
		tsp_OID hDLL = *sp;
		if(tyc_IS_TAGGED_INT(hDLL)) {
		  *++sp =
		   tyc_TAG_INT(rtdll_close((void*)tyc_TAGGED_INT_VALUE(hDLL)));
		  BNEXT();
		}
		/* type error */
		SAVE_REGS();
		RAISE_TypeError(hDLL, tyc_ClassId_Int);
		}
	      BLABEL(tvm_Builtin_Exception__raise):
		SAVE_REGS();
		tvm_raise(pReceiver);

		/* generic fun apply */
		#define FUNAPPLY(nr) \
		  assert(tyc_CLASSID(pReceiver) == tyc_ClassId_Fun ## nr); \
		  pMethod = (tyc_Method*)((tyc_Fun*)pReceiver)->pCompiledFun; \
		  assert(pMethod->nArgs == nr); \
		  assert(pMethod->wSorts == 0); \
		  assert(strcmp(pMethod->pSelector, "[]") == 0); \
		  goto methodcall;
	      BLABEL(tvm_Builtin_Fun0_apply):  FUNAPPLY(0);
	      BLABEL(tvm_Builtin_Fun1_apply):  FUNAPPLY(1);
	      BLABEL(tvm_Builtin_Fun2_apply):  FUNAPPLY(2);
	      BLABEL(tvm_Builtin_Fun3_apply):  FUNAPPLY(3);
	      BLABEL(tvm_Builtin_Fun4_apply):  FUNAPPLY(4);
	      BLABEL(tvm_Builtin_Fun5_apply):  FUNAPPLY(5);
	      BLABEL(tvm_Builtin_Fun6_apply):  FUNAPPLY(6);
	      BLABEL(tvm_Builtin_Fun7_apply):  FUNAPPLY(7);
	      BLABEL(tvm_Builtin_Fun8_apply):  FUNAPPLY(8);
	      BLABEL(tvm_Builtin_Fun9_apply):  FUNAPPLY(9);
	      BLABEL(tvm_Builtin_Fun10_apply): FUNAPPLY(10);

		#define FUNAPPLY_R(args, sortsCR, sorts) \
		  assert(tyc_CLASSID(pReceiver) == tyc_ClassId_Fun ## sortsCR ## R); \
		  pMethod = (tyc_Method*)((tyc_Fun*)pReceiver)->pCompiledFun; \
		  assert(pMethod->nArgs == args); \
		  assert(pMethod->wSorts == sorts); \
		  assert(strcmp(pMethod->pSelector, "[]") == 0); \
		  goto methodcall;
	      BLABEL(tvm_Builtin_FunCR_apply):	FUNAPPLY_R(1,C,1);
	      BLABEL(tvm_Builtin_FunCRR_apply): FUNAPPLY_R(2,CR,1);
	      BLABEL(tvm_Builtin_FunRCR_apply): FUNAPPLY_R(2,RC,2);
	      BLABEL(tvm_Builtin_FunCCR_apply): FUNAPPLY_R(2,CC,3);

		#define FUNAPPLY_C(args, sortsCR, sorts) \
		  assert(tyc_CLASSID(pReceiver) == tyc_ClassId_Fun ## sortsCR ## C); \
		  pMethod = (tyc_Method*)((tyc_Fun*)pReceiver)->pCompiledFun; \
		  assert(pMethod->nArgs == args); \
		  assert(pMethod->wSorts == sorts); \
		  assert(strcmp(pMethod->pSelector, "[]@") == 0); \
		  goto methodcall;
		#define FUNAPPLY_C1(args, sorts) \
		  assert(tyc_CLASSID(pReceiver) == tyc_ClassId_Fun ## C); \
		  pMethod = (tyc_Method*)((tyc_Fun*)pReceiver)->pCompiledFun; \
		  assert(pMethod->nArgs == args); \
		  assert(pMethod->wSorts == sorts); \
		  assert(strcmp(pMethod->pSelector, "[]@") == 0); \
		  goto methodcall;
	      BLABEL(tvm_Builtin_FunC_apply):	FUNAPPLY_C1(0,0);
	      BLABEL(tvm_Builtin_FunRC_apply):	FUNAPPLY_C(1,R,0);
	      BLABEL(tvm_Builtin_FunCC_apply):	FUNAPPLY_C(1,C,1);
	      BLABEL(tvm_Builtin_FunRRC_apply): FUNAPPLY_C(2,RR,0);
	      BLABEL(tvm_Builtin_FunCRC_apply): FUNAPPLY_C(2,CR,1);
	      BLABEL(tvm_Builtin_FunRCC_apply): FUNAPPLY_C(2,RC,2);
	      BLABEL(tvm_Builtin_FunCCC_apply): FUNAPPLY_C(2,CC,3);
	      
	      BLABEL(tvm_Builtin_Tycoon_errno):
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_Tycoon);
		*sp = tyc_TAG_INT(tosError_getCode());
		BNEXT();
	      BLABEL(tvm_Builtin_Tycoon_backTrace):
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_Tycoon);
		SAVE_REGS();
		tmdebug_backTrace(pThread, stdout);
		*sp = tyc_NIL;
		BNEXT();
	      BLABEL(tvm_Builtin_Tycoon__rollback):
		/* brute force version */
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_Tycoon);
		/* restart the whole process */
		tosProcess_execute(tm_pszProg, tm_pArguments);
		/* should not proceed beyond this point */
		*sp = tyc_TAG_INT(1);
		BNEXT();
	      BLABEL(tvm_Builtin_Tycoon__commit):
		{
		tsp_ErrorCode wErrorCode;
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_Tycoon);
		SAVE_REGS();
		tmthread_criticalLock();
		tmthread_checkSyncRequest();
		FETCH_THREAD();
		LOAD_REGS();
		*sp = tyc_TAG_INT(2);
		ip += 3;
		CLEAR_PAST_EVENT(pThread);
		SAVE_REGS();
		/* shrink store (do a compacting gc) */
		tsp_gc(TRUE);
		tmthread_syncRequest();
		/* execute commit */
		wErrorCode = tsp_commit();
		/* leave critical section */
		tmthread_syncRelease();
		tmthread_criticalUnlock();
		/* check return code and raise exception if commit failed */
		if(wErrorCode) {
		  fprintf(stderr,"\nTSP error: commit failed: %s.\n",
			  tsp_errorCode(wErrorCode));
		  tvm_raise(tyc_pCommitError);
		}
		FETCH_THREAD();
		LOAD_REGS();
		ip -= 3;
		*sp = tyc_TAG_INT(0);
		}
		BNEXT();
	      BLABEL(tvm_Builtin_Tycoon_builtinArgv):
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_Tycoon);
		*sp = tyc_pArgv;
		BNEXT();
	      BLABEL(tvm_Builtin_ConcreteClass__new):
		{
		tsp_OID pNew;
		tyc_Class * pClass = (tyc_Class*)pReceiver;
		SAVE_REGS();
		pNew = tsp_newArray(tyc_UNTAG_INT(pClass->wInstanceId),
				    tyc_UNTAG_INT(pClass->wInstanceSize));
		FETCH_THREAD();
		LOAD_REGS();
		*sp = pNew;
		}
		BNEXT();
	      BLABEL(tvm_Builtin_MutableArrayClass__new1):
		{
		tsp_OID pNew, pArgument = *sp;
		tyc_Class * pClass = (tyc_Class*)pReceiver;
		if(tyc_IS_TAGGED_INT(pArgument)) {
		  Int i = tyc_TAGGED_INT_VALUE(pArgument);
		  SAVE_REGS();
		  pNew = tsp_newArray(tyc_UNTAG_INT(pClass->wInstanceId), i);
		  assert(tsp_classId(pNew) == tyc_ClassId_MutableArray);
		  FETCH_THREAD();
		  LOAD_REGS();
		  *++sp = pNew;
		  BNEXT();
		}
		SAVE_REGS();
		RAISE_TypeError(pArgument, tyc_ClassId_Int);
		}
	      BLABEL(tvm_Builtin_AtArrayClass__new1At):
		{
		tsp_OID pNew, pArgument = *sp;
		tyc_Class * pClass = (tyc_Class*)pReceiver;
		if(tyc_IS_TAGGED_INT(pArgument)) {
		  Int i = tyc_TAGGED_INT_VALUE(pArgument);
		  SAVE_REGS();
		  pNew = tsp_newArray(tyc_UNTAG_INT(pClass->wInstanceId), i);
		  assert(tsp_classId(pNew) == tyc_ClassId_AtArray);
		  FETCH_THREAD();
		  LOAD_REGS();
		  assert_superComponent(tyc_NIL, pNew);
		  tsp_setSuperComponent(pNew, pThread);
		  *++sp = pNew;
		  BNEXT();
		}
		SAVE_REGS();
		RAISE_TypeError(pArgument, tyc_ClassId_Int);
		}
	      BLABEL(tvm_Builtin_MutableStringClass__new1):
		{
		tsp_OID pNew, pArgument = *sp;
		tyc_Class * pClass = (tyc_Class*)pReceiver;
		if(tyc_IS_TAGGED_INT(pArgument)) {
		  Int i = tyc_TAGGED_INT_VALUE(pArgument) + 1;
		  SAVE_REGS();
		  pNew = tsp_newByteArray(tyc_UNTAG_INT(pClass->wInstanceId),
					  i);
		  assert(tsp_classId(pNew) == tyc_ClassId_MutableString);
		  FETCH_THREAD();
		  LOAD_REGS();
		  *++sp = pNew;
		  BNEXT();
		}
		SAVE_REGS();
		RAISE_TypeError(pArgument, tyc_ClassId_Int);
		}
	      BLABEL(tvm_Builtin_Builtin_fail):
		/* call bytecode */
		pMethod = (tyc_Method*)
		  &(((tyc_BuiltinMethod*)pMethod)->compiledMethod);
		goto methodcall;
	      BLABEL(tvm_Builtin_Root_flushAll):
		{
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_Root);
		SAVE_REGS();
		tvm_clearCache(FALSE);
		FETCH_THREAD();
		LOAD_REGS();
		*sp = pReceiver;
		}
		BNEXT();
	      BLABEL(tvm_Builtin_Root_flushClass):
		{
		  /* tsp_OID pClass = *sp; */
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_Root);
		SAVE_REGS();
		tvm_clearCache(FALSE); /* change later */
		FETCH_THREAD();
		LOAD_REGS();
		*++sp = pReceiver;
		}
		BNEXT();
	      BLABEL(tvm_Builtin_Root_flushSelector):
		{
		  /* tsp_OID pClass =	 sp[0]; */
		  /* tsp_OID pSelector = sp[1]; */
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_Root);
		SAVE_REGS();
		tvm_clearCache(FALSE); /* change later */
		FETCH_THREAD();
		LOAD_REGS();
		sp += 2;
		*sp = pReceiver;
		}
		BNEXT();
	      BLABEL(tvm_Builtin_EagerDFA_transition):
		/* this is a quick hack: arguments are not checked
		   and no exception is raised! */
		{
		Int ch = tyc_CHAR_VALUE(sp[0]);
		register Word wOffset = tyc_TAGGED_INT_VALUE(sp[1]);
		Short * pMoveTable = (Short*)(sp[2]);
		Int result;
		for(;; wOffset += 2) {
		  register Int ch2 = pMoveTable[wOffset];
		  if (ch2 > ch) {
		    result = pMoveTable[wOffset+1];
		    break;
		  }
		  if (ch2 == -1) {
		    result = -1;
		    break;
		  }
		}
		sp += 3;
		*sp = tyc_TAG_INT(result);
		}
		BNEXT();
	      BLABEL(tvm_Builtin_Root_newClassId):
		{
		tsp_OID pDescriptor = sp[0];
		tsp_OID pLayout =     sp[1];
		tsp_ClassId newId;
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_Root);
		if(tyc_IS_TAGGED_INT(pLayout)) {
		  tsp_ClassId id = tyc_CLASSID(pDescriptor);
		  if(id==tyc_ClassId_Nil || id==tyc_ClassId_MutableString ||
		     id==tyc_ClassId_String || id==tyc_ClassId_Symbol) {
		    newId = tsp_newClassId(tyc_TAGGED_INT_VALUE(pLayout),
					   pDescriptor);
		    sp += 2;
		    *sp = tyc_TAG_INT(newId);
		    BNEXT();
		  }
		  SAVE_REGS();
		  RAISE_TypeError(pDescriptor, tyc_ClassId_String);
		}
		SAVE_REGS();
		RAISE_TypeError(pLayout, tyc_ClassId_Int);
		}
	      BLABEL(tvm_Builtin_CStructClass__new):
		{
		tsp_OID pNew;
		tyc_Class * pClass = (tyc_Class*)pReceiver;
		SAVE_REGS();
		pNew = tsp_newStruct(tyc_UNTAG_INT(pClass->wInstanceId));
		FETCH_THREAD();
		LOAD_REGS();
		*sp = pNew;
		}
		BNEXT();
	      BLABEL(tvm_Builtin_Tycoon__platformCode):
		{
		int hostID;
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_Tycoon);
		hostID = tosSystem_getID();
		*sp = tyc_TAG_INT(hostID);
		}
		BNEXT();
	      /* thread & weakRef initilaization */
	      BLABEL(tvm_Builtin_ThreadClass__new):
		{
		tsp_OID pNew;
		SAVE_REGS();
		pNew = tsp_newThread(tyc_ClassId_Thread);
		FETCH_THREAD();
		LOAD_REGS();
		*sp = pNew;
		}
		BNEXT();
	      BLABEL(tvm_Builtin_ThreadClass_this):
		{
		*sp = pThread;
		}
		BNEXT();
	      BLABEL(tvm_Builtin_ThreadClass_sleep):
		{
		tyc_Long * pLong = *sp;
		if(tyc_CLASSID(pLong) == tyc_ClassId_Long) {
		  Long longTime = pLong->value;
		  SAVE_REGS();
		  tmthread_criticalLock();
		  tmthread_checkSyncRequest();
		  FETCH_THREAD();
		  SET_BLOCKED(pThread);
		  tmthread_criticalUnlock();
		  tosThread_sleep((Word)longTime);
		  tmthread_criticalLock();
		  tmthread_checkSyncRequest();
		  FETCH_THREAD();
		  LOAD_REGS();
		  CLEAR_BLOCKED(pThread);
		  tmthread_criticalUnlock();
		  sp++;
		  BNEXT();
		}
		SAVE_REGS();
		RAISE_TypeError(pLong, tyc_ClassId_Long);
		}
	      BLABEL(tvm_Builtin_ThreadClass_testCancel):
		{
		SAVE_REGS();
		tmthread_testCancel();
		}
		BNEXT();
	      BLABEL(tvm_Builtin_Thread___init):
		{
		tsp_OID pNew;
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_Thread);
#ifdef tvm_TRACE
		if(IS_TRACE_INHERIT(pThread)) {
		  SET_TRACE_FLAGS((tyc_Thread *)pReceiver,
				  TRACE_FLAGS(pThread));
		}
#endif
		SAVE_REGS();
		pNew = tmthread_new(pReceiver);
		FETCH_THREAD();
		LOAD_REGS();
		*sp = tyc_boxBool(pNew);
		}
		BNEXT();
	      BLABEL(tvm_Builtin_Thread_cancel):
		{
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_Thread);
		tmthread_setCancel((tyc_Thread*)pReceiver);
		}
		BNEXT();

	      BLABEL(tvm_Builtin_Thread_traceFlags):
		{
		Word wTraceFlags;
		tsp_OID pResult;
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_Thread);
		wTraceFlags = TRACE_FLAGS((tyc_Thread*)pReceiver);
		SAVE_REGS();
		pResult = (tsp_OID)tyc_TAG_MAYBEBOXED(wTraceFlags);
		FETCH_THREAD();
		LOAD_REGS();
		*sp = pResult;
		}
		BNEXT();
	      BLABEL(tvm_Builtin_Thread_setTraceFlags):
		{
		tsp_OID pFlags = sp[0];
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_Thread);
		if(tyc_CLASSID(pFlags) == tyc_ClassId_Int) {
		  Word wFlags = tyc_TAGGED_INT_VALUE(pFlags);
		  SAVE_REGS();
		  tmthread_criticalLock();
		  tmthread_checkSyncRequest();
		  /* global sync request because trace flags are local */
		  tmthread_syncRequest();
		  FETCH_THREAD();
		  LOAD_REGS();
		  pReceiver = sp[1];
		  SET_TRACE_FLAGS((tyc_Thread*)pReceiver, wFlags);
		  tmthread_syncRelease();
		  tmthread_criticalUnlock();
		  pFlags = sp[0];
		  *++sp = pFlags;
		  BNEXT();
		}
		SAVE_REGS();
		RAISE_TypeError(pFlags, tyc_ClassId_Int);
		}
	      BLABEL(tvm_Builtin_Thread_resume):
		{
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_Thread);
		SAVE_REGS();
		tmthread_debugResume((tyc_Thread*)pReceiver);
		FETCH_THREAD();
		LOAD_REGS();
		*sp = tyc_NIL;
		}
		BNEXT();
	      BLABEL(tvm_Builtin_Thread_debugEvent):
		{
		Word wSusp, wResult;
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_Thread);
		SAVE_REGS();
		tmthread_criticalLock();
		tmthread_checkSyncRequest();
		FETCH_THREAD();
		LOAD_REGS();
		wSusp = SUSPEND_FLAGS((tyc_Thread *)*sp);
		tmthread_criticalUnlock();
		if(wSusp & SUSP_SEND_BIT)
		  wResult = TRACE_SEND_BIT;
		else if(wSusp & SUSP_RETURN_BIT)
		  wResult = TRACE_RETURN_BIT;
		else if(wSusp & SUSP_EXCEPTION_BIT)
		  wResult = TRACE_EXCEPTION_BIT;
		else
		  wResult = 0;
		*sp = tyc_TAG_INT(wResult);
		}
		BNEXT();
	      BLABEL(tvm_Builtin_Thread_inspectSendSelector):
		{
		Word wSusp, wSuspSelectorId;
		tyc_Selector *pSelector;
		tyc_Thread *pSuspThread;
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_Thread);
		SAVE_REGS();
		tmthread_criticalLock();
		tmthread_checkSyncRequest();
		FETCH_THREAD();
		LOAD_REGS();
		pSuspThread = (tyc_Thread *)*sp;
		wSusp = SUSPEND_FLAGS(pSuspThread);
		if(wSusp & SUSP_SEND_BIT) {
		  Byte *pbSuspIp = pSuspThread->ip;
		  wSuspSelectorId = pbSuspIp[1] | (pbSuspIp[2] << 8);
		  pSelector = tyc_pRoot->apSelectorTable[wSuspSelectorId];
		} else {
		  pSelector = tyc_NIL;
		}
		tmthread_criticalUnlock();
		*sp = pSelector;
		}
		BNEXT();
	      BLABEL(tvm_Builtin_Thread_inspectActiveObject):
		{
		tyc_Thread *pSuspThread;
		tsp_OID pResult;
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_Thread);
		SAVE_REGS();
		tmthread_criticalLock();
		tmthread_checkSyncRequest();
		FETCH_THREAD();
		LOAD_REGS();
		pSuspThread = (tyc_Thread *)*sp;
		assert(tyc_CLASSID(pSuspThread) == tyc_ClassId_Thread);
		if(IS_SUSPENDED(pSuspThread)) {
		  if(IS_BUILTIN_RETURN(pSuspThread)) {
		    pResult = pSuspThread->pReceiver;
		  } else {
		    pResult = pSuspThread->fp->pReceiver;
		  }
		} else {
		  pResult = tyc_NIL;
		}
		tmthread_criticalUnlock();
		*sp = pResult;
		}
		BNEXT();
	      BLABEL(tvm_Builtin_Thread_inspectReturnReceiver):
		{
		tyc_Thread *pSuspThread;
		tsp_OID pResult;
		Word wSusp;
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_Thread);
		SAVE_REGS();
		tmthread_criticalLock();
		tmthread_checkSyncRequest();
		FETCH_THREAD();
		LOAD_REGS();
		pSuspThread = (tyc_Thread *)*sp;
		assert(tyc_CLASSID(pSuspThread) == tyc_ClassId_Thread);
		wSusp = SUSPEND_FLAGS(pSuspThread);
		if(wSusp & SUSP_RETURN_BIT) {
		  if(IS_BUILTIN_RETURN(pSuspThread)) {
		    pResult = pSuspThread->fp->pReceiver;
		  } else {
		    pResult = pSuspThread->fp->parent.fp->pReceiver;
		  }
		} else if(wSusp & SUSP_SEND_BIT) {
		  if(pSuspThread->ip[0] == tvm_Op_sendTail) {
		    pResult = pSuspThread->fp->parent.fp->pReceiver;
		  } else {
		    pResult = pSuspThread->fp->pReceiver;
		  }
		} else if(wSusp & SUSP_EXCEPTION_BIT) {
		  pResult = pSuspThread->fp->parent.fp->pReceiver;
		} else {
		  pResult = tyc_NIL;
		}
		tmthread_criticalUnlock();
		*sp = pResult;
		}
		BNEXT();
	      BLABEL(tvm_Builtin_Thread_inspectReturnComponent):
		{
		tyc_Thread *pSuspThread;
		tsp_OID pResult;
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_Thread);
		SAVE_REGS();
		tmthread_criticalLock();
		tmthread_checkSyncRequest();
		FETCH_THREAD();
		LOAD_REGS();
		pSuspThread = (tyc_Thread *)*sp;
		assert(tyc_CLASSID(pSuspThread) == tyc_ClassId_Thread);
		if(IS_SUSPENDED(pSuspThread)) {
		  Byte *pbSendIp;
		  tyc_Symbol pSymbol;
		  Word wSuspSelectorId;
		  Word wSize;
		  if(IS_BUILTIN_RETURN(pSuspThread)) {
		    pbSendIp = pSuspThread->ip - 3;
		  } else {
		    pbSendIp = pSuspThread->fp->parent.ip;
		  }
		  wSuspSelectorId = pbSendIp[1] | (pbSendIp[2] << 8);
		  pSymbol = tyc_SELECTOR(wSuspSelectorId);
		  wSize = tsp_size(pSymbol);
		  pResult = tyc_boxBool(wSize > 1 && pSymbol[wSize-2] == '@');
		} else {
		  pResult = tyc_NIL;
		}
		tmthread_criticalUnlock();
		*sp = pResult;
		}
		BNEXT();
	      BLABEL(tvm_Builtin_Thread_inspectStack):
		{
		tsp_OID pArgument = *sp;
		if(tyc_IS_TAGGED_INT(pArgument)) {
		  Word wArgument = tyc_UNTAG_INT(pArgument);
		  tsp_OID pResult;
		  tyc_Thread *pSuspThread;
		  SAVE_REGS();
		  tmthread_criticalLock();
		  tmthread_checkSyncRequest();
		  FETCH_THREAD();
		  LOAD_REGS();
		  pSuspThread = (tyc_Thread *)sp[1];
		  assert(tyc_CLASSID(pSuspThread) == tyc_ClassId_Thread);
		  if(IS_SUSPENDED(pSuspThread)) {
		    /* ### more checks on wArgument depending on event */
		    pResult = pSuspThread->sp[wArgument];
		  } else {
		    pResult = tyc_NIL;
		  }
		  tmthread_criticalUnlock();
		  *++sp = pResult;
		  BNEXT();
		}
		SAVE_REGS();
		RAISE_TypeError(pArgument, tyc_ClassId_Int);
		}
	      BLABEL(tvm_Builtin_TL2Debugger__fetchStoppedThread):
		{
		tyc_Thread *pResult;
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_Debugger);
		SAVE_REGS();
		pResult = tmthread_fetchStoppedThread((tyc_Debugger*)*sp);
		FETCH_THREAD();
		LOAD_REGS();
		*sp = pResult;
		}
		BNEXT();

	      BLABEL(tvm_Builtin_WeakRefClass__new):
		{
		tsp_OID pNew;
		SAVE_REGS();
		pNew = tsp_newWeakRef(tyc_ClassId_WeakRef);
		FETCH_THREAD();
		LOAD_REGS();
		*sp = pNew;
		}
		BNEXT();
	      BLABEL(tvm_Builtin_WeakRef___init):
		{
		tsp_OID pNew;
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_WeakRef);
		SAVE_REGS();
		pNew = tsp_initWeakRef(pReceiver);
		FETCH_THREAD();
		LOAD_REGS();
		*sp = pNew;
		}
		BNEXT();
	      /* mutex & condition variable initialization */
	      BLABEL(tvm_Builtin_Mutex___init):
		{
		Bool fSuccess;
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_Mutex);
		fSuccess = tmthread_initMutex(pReceiver);
		*sp = tyc_boxBool(fSuccess);
		}
		BNEXT();
	      BLABEL(tvm_Builtin_Mutex___finalize):
		{
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_Mutex);
		tmthread_finalizeMutex(pReceiver);
		}
		BNEXT();
	      BLABEL(tvm_Builtin_Condition___init):
		{
		Bool fSuccess;
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_Condition);
		fSuccess = tmthread_initSingleCondition(pReceiver);
		*sp = tyc_boxBool(fSuccess);
		}
		BNEXT();
	      BLABEL(tvm_Builtin_BroadcastingCondition___init):
		{
		Bool fSuccess;
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_BroadcastingCondition);
		fSuccess = tmthread_initBroadcastCondition(pReceiver);
		*sp = tyc_boxBool(fSuccess);
		}
		BNEXT();
	      BLABEL(tvm_Builtin_Condition___finalize):
		{
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_Condition ||
		       tyc_CLASSID(pReceiver) == tyc_ClassId_BroadcastingCondition);
		tmthread_finalizeCondition(pReceiver);
		}
		BNEXT();
	      /* mutex actions */
	      BLABEL(tvm_Builtin_Mutex_lock): /* blocking builtin */
		{
		int rValue;
		void * hOSMutex;
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_Mutex);
		hOSMutex = ((tyc_Mutex*)pReceiver)->hOSMutex;
		/* be optimistic and use a trylock first */
		if((rValue = tosMutex_trylock(hOSMutex)) == 0) {
		  ((tyc_Mutex*)pReceiver)->pOwner = pThread;
		}
		else {
		  SAVE_REGS();
		  tmthread_criticalLock();
		  tmthread_checkSyncRequest();
		  FETCH_THREAD();
		  SET_BLOCKED(pThread);
		  tmthread_criticalUnlock();

		  rValue = tosMutex_lock(hOSMutex);

		  tmthread_criticalLock();
		  tmthread_checkSyncRequest();
		  FETCH_THREAD();
		  LOAD_REGS();
		  CLEAR_BLOCKED(pThread);
		  ((tyc_Mutex*)*sp)->pOwner = pThread;
		  tmthread_criticalUnlock();
		}
		*sp = tyc_boxBool(rValue == 0);
		}
		BNEXT();
	      BLABEL(tvm_Builtin_Mutex_tryLock):
		{
		int rValue;
		void * hOSMutex;
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_Mutex);
		hOSMutex = ((tyc_Mutex*)pReceiver)->hOSMutex;
		if((rValue = tosMutex_trylock(hOSMutex)) == 0) {
		  ((tyc_Mutex*)pReceiver)->pOwner = pThread;
		}
		*sp = tyc_boxBool(rValue == 0);
		}
		BNEXT();
	      BLABEL(tvm_Builtin_Mutex_unlock):
		{
		int rValue;
		void * hOSMutex;
		tsp_OID pOwner;
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_Mutex);
		assert(((tyc_Mutex*)pReceiver)->pOwner == pThread);
		hOSMutex = ((tyc_Mutex*)pReceiver)->hOSMutex;
		pOwner = ((tyc_Mutex*)pReceiver)->pOwner;
		if(pOwner == pThread) {
		  ((tyc_Mutex*)pReceiver)->pOwner = NULL;
		  rValue = tosMutex_unlock(hOSMutex);
		}
		else {
		  fprintf(stderr,
			  "TVM error: trying to unlock foreign mutex.\n");
		  rValue = -1;
		}
		*sp = tyc_boxBool(rValue == 0);
		}
		BNEXT();
	      /*
	       * condition variable actions
	       *
	       * tycoonOS: We need different builtins for signal
	       * and broadcasting conditions...
	       */
	      BLABEL(tvm_Builtin_Condition_signal):
		{
		int rValue;
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_Condition);
		rValue = tosCondition_signal(((tyc_Condition*)pReceiver)->hOSCondition);
		*sp = tyc_boxBool(rValue == 0);
		}
		BNEXT();
	      BLABEL(tvm_Builtin_BroadcastingCondition_broadcast):
		{
		int rValue;
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_BroadcastingCondition);
		rValue = tosCondition_broadcast(((tyc_Condition*)pReceiver)->hOSCondition);
		*sp = tyc_boxBool(rValue == 0);
		}
		BNEXT();
	      BLABEL(tvm_Builtin_Condition_wait): /* blocking builtin */
		{
		int rValue;
		tyc_Mutex * pMutex = sp[0];
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_Condition ||
		       tyc_CLASSID(pReceiver) == tyc_ClassId_BroadcastingCondition);
		assert(pMutex->pOwner == pThread);
		if(tyc_CLASSID(pMutex) == tyc_ClassId_Mutex) {
		  void * hOSCondition = ((tyc_Condition*)pReceiver)->hOSCondition;
		  void * hOSMutex = pMutex->hOSMutex;
		  SAVE_REGS();
		  tmthread_criticalLock();
		  tmthread_checkSyncRequest();
		  FETCH_THREAD();
		  LOAD_REGS();
		  SET_BLOCKED(pThread);
		  SET_WAITING(pThread);
		  ((tyc_Mutex*)(sp[0]))->pOwner = NULL;
		  /* at startup time a cond_broadcast is simulated and all
		     waiting threads start competing for the associated mutex.
		   */
		  SAVE_REGS();
		  tmthread_criticalUnlock();

		  rValue = tosCondition_wait(hOSCondition, hOSMutex);

		  tmthread_criticalLock();
		  tmthread_checkSyncRequest();
		  FETCH_THREAD();
		  LOAD_REGS();
		  CLEAR_BLOCKED(pThread);
		  CLEAR_WAITING(pThread);
		  ((tyc_Mutex*)(sp[0]))->pOwner = pThread;
		  tmthread_criticalUnlock();
		  *++sp = tyc_boxBool(rValue == 0);
		  BNEXT();
		}
		SAVE_REGS();
		RAISE_TypeError(pMutex, tyc_ClassId_Mutex);
		}
	      BLABEL(tvm_Builtin_Condition_timedWait): /* blocking builtin */
		{
		int rValue;
		tyc_Mutex * pMutex = sp[1];
		tyc_Long * pLong = sp[0];
		assert(tyc_CLASSID(pReceiver) == tyc_ClassId_Condition ||
		       tyc_CLASSID(pReceiver) == tyc_ClassId_BroadcastingCondition);
		assert(pMutex->pOwner == pThread);
		if(tyc_CLASSID(pMutex) == tyc_ClassId_Mutex) {
		  if(tyc_CLASSID(pLong) == tyc_ClassId_Long) {
		    void * hOSCondition = ((tyc_Condition*)pReceiver)->hOSCondition;
		    void * hOSMutex = pMutex->hOSMutex;
		    Long longTime = (pLong->value) * 1000 ; /* sec -> msec */
		    SAVE_REGS();
		    tmthread_criticalLock();
		    tmthread_checkSyncRequest();
		    FETCH_THREAD();
		    LOAD_REGS();
		    SET_BLOCKED(pThread);
		    SET_WAITING(pThread);
		    ((tyc_Mutex*)(sp[1]))->pOwner = NULL;
		    /* at startup time a cond_broadcast is simulated and all
		       waiting threads start competing for the associated
		       mutex. */
		    SAVE_REGS();
		    tmthread_criticalUnlock();

		    rValue = tosCondition_timedwait(hOSCondition, hOSMutex, longTime);

		    tmthread_criticalLock();
		    tmthread_checkSyncRequest();
		    FETCH_THREAD();
		    LOAD_REGS();
		    CLEAR_BLOCKED(pThread);
		    CLEAR_WAITING(pThread);
		    ((tyc_Mutex*)(sp[1]))->pOwner = pThread;
		    tmthread_criticalUnlock();
		    sp += 2;
		    *sp = tyc_boxBool(rValue == 0);
		    BNEXT();
		  }
		  SAVE_REGS();
		  RAISE_TypeError(pLong, tyc_ClassId_Long);
		}
		SAVE_REGS();
		RAISE_TypeError(pMutex, tyc_ClassId_Mutex);
		}
	      BLABEL(tvm_Builtin_Finalizer___fetchWeaks):
		{
		*sp = tmthread_getWeakRefList();
		}
		BNEXT();
	      BLABEL(tvm_Builtin_BlockingExternalMethodCaller___suspend):
		{
		tyc_Thread * pCaller = *sp;
		*++sp = tyc_boxBool(tmthread_suspendBlockingCaller(pCaller));
		}
		BNEXT();
	      BLABEL(tvm_Builtin_BlockingExternalMethodCaller___resume):
		{
		tyc_Thread * pCaller = *sp;
		*++sp = tyc_boxBool(tmthread_resumeBlockingCaller(pCaller));
		}
		BNEXT();
#ifdef tvm_THREADED_CODE
	      BLABEL(tvm_Builtin_Undefined):
		/* (undefined builtin) */
		fprintf(stderr,"\nTVM warning: unknown builtin %s.%s.\n",
		       tyc_CLASS(tyc_CLASSID(pReceiver))->pszName,
		       tyc_SELECTOR(idSelector));
		/* call method body */
		goto methodcall;
#else
	      default:
		/* (undefined builtin) */
		fprintf(stderr,"\nTVM warning: unknown builtin %s.%s.\n",
		       tyc_CLASS(tyc_CLASSID(pReceiver))->pszName,
		       tyc_SELECTOR(idSelector));
		/* call method body */
		goto methodcall;
	      }
	    break;  /* method end */
	  }
#endif

	  METHODLABEL(CSlotAccessMethod):
	  METHODLABEL(CSlotReferenceMethod):
	  {
	    /* C slot access */
	    void * pAddress;
	    tsp_OID pResult = tyc_NIL;
	    Int iIndex = ((tyc_CSlotMethod*)pMethod)->slotMethod.iOffset;
	    Char cTycoonType = ((tyc_CSlotMethod*)pMethod)->cTycoonType;
	    Char cCType;
	    assert(!tsp_IS_IMMEDIATE(pReceiver) && !tyc_IS_NIL(pReceiver));
	    cCType = tsp_getCType(pReceiver, iIndex);
	    pAddress = tsp_getCAddress(pReceiver, iIndex);
	    /* convert C type -> Tycoon type */
	    if(cCType == tsp_Field_OID)
	      /* no conversion */
	      pResult = *(tsp_OID *)pAddress;
	    else switch(cTycoonType)
	      {
	      case tyc_Type_Bool:
		switch(cCType)
		  {
		  case tsp_Field_Char:
		  case tsp_Field_UChar:
		    pResult = tyc_boxBool(*((char*)pAddress)); break;
		  case tsp_Field_Short:
		  case tsp_Field_UShort:
		    pResult = tyc_boxBool(*((short*)pAddress)); break;
		  case tsp_Field_Int:
		  case tsp_Field_UInt:
		    pResult = tyc_boxBool(*((int*)pAddress)); break;
		  case tsp_Field_Long:
		  case tsp_Field_ULong:
		    pResult = tyc_boxBool(*((long*)pAddress)); break;
		  case tsp_Field_LongLong:
		    pResult = tyc_boxBool(*((Long*)pAddress)); break;
		  default:
		    tosError_ABORT();
		  }
		break;
	      case tyc_Type_Char:
		switch(cCType)
		  {
		  case tsp_Field_Char:
		  case tsp_Field_UChar:
		    pResult = tyc_boxChar(*((char*)pAddress)); break;
		  case tsp_Field_Short:
		  case tsp_Field_UShort:
		    pResult = tyc_boxChar(*((short*)pAddress)); break;
		  case tsp_Field_Int:
		  case tsp_Field_UInt:
		    pResult = tyc_boxChar(*((int*)pAddress)); break;
		  case tsp_Field_Long:
		  case tsp_Field_ULong:
		    pResult = tyc_boxChar(*((long*)pAddress)); break;
		  case tsp_Field_LongLong:
		    pResult = tyc_boxChar(*((Long*)pAddress)); break;
		  default:
		    tosError_ABORT();
		  }
		break;
	      case tyc_Type_Int:
		{
		Int i;
		switch(cCType)
		  {
		  case tsp_Field_Char:
		    i = *((signed char*)pAddress); break;
		  case tsp_Field_UChar:
		    i = *((unsigned char*)pAddress); break;
		  case tsp_Field_Short:
		    i = *((signed short*)pAddress); break;
		  case tsp_Field_UShort:
		    i = *((unsigned short*)pAddress); break;
		  case tsp_Field_Int:
		    i = *((signed int*)pAddress); break;
		  case tsp_Field_UInt:
		    i = *((unsigned int*)pAddress); break;
		  case tsp_Field_Long:
		    i = *((signed long*)pAddress); break;
		  case tsp_Field_ULong:
		    i = *((unsigned long*)pAddress); break;
		  case tsp_Field_LongLong:
		    i = *((Long*)pAddress); break;
		  default:
		    tosError_ABORT();
		  }
		SAVE_REGS();
		pResult = tyc_TAG_MAYBEBOXED(i);
		FETCH_THREAD();
		LOAD_REGS();
		}
		break;
	      case tyc_Type_Long:
		{
		Long l;
		switch(cCType)
		  {
		  case tsp_Field_Char:
		    l = *((signed char*)pAddress); break;
		  case tsp_Field_UChar:
		    l = *((unsigned char*)pAddress); break;
		  case tsp_Field_Short:
		    l = *((signed short*)pAddress); break;
		  case tsp_Field_UShort:
		    l = *((unsigned short*)pAddress); break;
		  case tsp_Field_Int:
		    l = *((signed int*)pAddress); break;
		  case tsp_Field_UInt:
		    l = *((unsigned int*)pAddress); break;
		  case tsp_Field_Long:
		    l = *((signed long*)pAddress); break;
		  case tsp_Field_ULong:
		    l = *((unsigned long*)pAddress); break;
		  case tsp_Field_LongLong:
		    l = *((Long*)pAddress); break;
		  default:
		    tosError_ABORT();
		  }
		SAVE_REGS();
		pResult = tyc_boxLong(l);
		FETCH_THREAD();
		LOAD_REGS();
		}
		break;
	      case tyc_Type_Real:
		{
		Real r;
		switch(cCType)
		  {
		  case tsp_Field_Float:
		    r = *((float*)pAddress); break;
		  case tsp_Field_Double:
		    r = *((double*)pAddress); break;
		  default:
		    tosError_ABORT();
		  }
		SAVE_REGS();
		pResult = tyc_boxReal(r);
		FETCH_THREAD();
		LOAD_REGS();
		}
		break;
	      case tyc_Type_Object:
		/* only permitted CType is OID */
		tosError_ABORT();
		break;
	      default:
		/* (illegal tycoon descriptor) */
	     tosError_ABORT();
	      }
	    *sp = pResult;
	    BNEXT();
	  }

	  METHODLABEL(CSlotTakeFromMethod):
	  {
	    /* C slot access */
	    void * pAddress;
	    Int iIndex = ((tyc_CSlotMethod*)pMethod)->slotMethod.iOffset;
#ifndef NDEBUG
	    Char cTycoonType = ((tyc_CSlotMethod*)pMethod)->cTycoonType;
	    Char cCType;
	    assert(!tsp_IS_IMMEDIATE(pReceiver) && !tyc_IS_NIL(pReceiver));
	    cCType = tsp_getCType(pReceiver, iIndex);
	    assert(cCType == tsp_Field_OID && cTycoonType == tyc_Type_Object);
#endif
	    pAddress = tsp_getCAddress(pReceiver, iIndex);
	    takeFromComponent(pThread, sp, pReceiver, ((tsp_OID*)pAddress));
	    BNEXT();
	  } 

	  METHODLABEL(CSlotUpdateMethod):
	  {
	    /* C slot update */
	    tsp_OID pArgument = *sp;
	    void * pAddress;
	    Int iIndex = ((tyc_CSlotMethod*)pMethod)->slotMethod.iOffset;
	    Char cTycoonType = ((tyc_CSlotMethod*)pMethod)->cTycoonType;
	    Char cCType;
	    assert(!tsp_IS_IMMEDIATE(pReceiver) && !tyc_IS_NIL(pReceiver));
	    cCType = tsp_getCType(pReceiver, iIndex);
	    pAddress = tsp_getCAddress(pReceiver, iIndex);
	    /* convert Tycoon type -> C type */
	    if(cCType == tsp_Field_OID)
	      /* no conversion */
	      *((tsp_OID *)pAddress) = pArgument;
	    else switch(cTycoonType)
	      {
	      case tyc_Type_Bool:
		{
		Bool f;
		if(tyc_IS_FALSE(pArgument))
		  f = FALSE;
		else if(tyc_IS_TRUE(pArgument))
		  f = TRUE;
		else {
		  SAVE_REGS();
		  RAISE_TypeError(pArgument, tyc_ClassId_Bool);
		  break;
		}
		switch(cCType)
		  {
		  case tsp_Field_Char:
		  case tsp_Field_UChar:
		    *((char*)pAddress) = f; break;
		  case tsp_Field_Short:
		  case tsp_Field_UShort:
		    *((short*)pAddress) = f; break;
		  case tsp_Field_Int:
		  case tsp_Field_UInt:
		    *((int*)pAddress) = f; break;
		  case tsp_Field_Long:
		  case tsp_Field_ULong:
		    *((long*)pAddress) = f; break;
		  case tsp_Field_LongLong:
		    *((Long*)pAddress) = f; break;
		  default:
		    tosError_ABORT();
		  }
		}
		break;
	      case tyc_Type_Char:
		if(tyc_CLASSID(pArgument) == tyc_ClassId_Char) {
		  Char c = tyc_CHAR_VALUE(pArgument);
		  switch(cCType)
		    {
		    case tsp_Field_Char:
		    case tsp_Field_UChar:
		      *((char*)pAddress) = c;	break;
		    case tsp_Field_Short:
		    case tsp_Field_UShort:
		      *((short*)pAddress) = c; break;
		    case tsp_Field_Int:
		    case tsp_Field_UInt:
		      *((int*)pAddress) = c; break;
		    case tsp_Field_Long:
		    case tsp_Field_ULong:
		      *((long*)pAddress) = c;	break;
		    case tsp_Field_LongLong:
		      *((Long*)pAddress) = c; break;
		    default:
		      tosError_ABORT();
		    }
		  break;
		}
		SAVE_REGS();
		RAISE_TypeError(pArgument, tyc_ClassId_Char);
		break;
	      case tyc_Type_Int:
		if(tyc_IS_TAGGED_INT(pArgument)) {
		  Int i = tyc_TAGGED_INT_VALUE(pArgument);
		  switch(cCType)
		    {
		    case tsp_Field_Char:
		    case tsp_Field_UChar:
		      *((char*)pAddress) = i;	break;
		    case tsp_Field_Short:
		    case tsp_Field_UShort:
		      *((short*)pAddress) = i; break;
		    case tsp_Field_Int:
		    case tsp_Field_UInt:
		      *((int*)pAddress) = i; break;
		    case tsp_Field_Long:
		    case tsp_Field_ULong:
		      *((long*)pAddress) = i; break;
		    case tsp_Field_LongLong:
		      *((Long*)pAddress) = i; break;
		    default:
		      tosError_ABORT();
		    }
		  break;
		}
		SAVE_REGS();
		RAISE_TypeError(pArgument, tyc_ClassId_Int);
		break;
	      case tyc_Type_Long:
		if(tyc_CLASSID(pArgument) == tyc_ClassId_Long) {
		  Long l = tyc_LONG_VALUE(pArgument);
		  switch(cCType)
		    {
		    case tsp_Field_Char:
		    case tsp_Field_UChar:
		      *((char*)pAddress) = l; break;
		    case tsp_Field_Short:
		    case tsp_Field_UShort:
		      *((short*)pAddress) = l; break;
		    case tsp_Field_Int:
		    case tsp_Field_UInt:
		      *((int*)pAddress) = l; break;
		    case tsp_Field_Long:
		    case tsp_Field_ULong:
		      *((long*)pAddress) = l;	break;
		    case tsp_Field_LongLong:
		      *((Long*)pAddress) = l; break;
		    default:
		      tosError_ABORT();
		    }
		  break;
		}
		SAVE_REGS();
		RAISE_TypeError(pArgument, tyc_ClassId_Long);
		break;
	      case tyc_Type_Real:
		if(tyc_CLASSID(pArgument) == tyc_ClassId_Real) {
		  Real r = tyc_REAL_VALUE(pArgument);
		  switch(cCType)
		    {
		    case tsp_Field_Float:
		      *((float*)pAddress) = r; break;
		    case tsp_Field_Double:
		      *((double*)pAddress) = r; break;
		    default:
		      tosError_ABORT();
		    }
		  break;
		}
		SAVE_REGS();
		RAISE_TypeError(pArgument, tyc_ClassId_Real);
		break;
	      case tyc_Type_Object:
		/* only permitted CType is OID */
		tosError_ABORT();
		break;
	      default:
		/* (illegal tycoon descriptor) */
		printf("### Illegal tycoon descriptor: %s, `%c', `%c'\n",
		       ((tyc_Method*)pMethod)->pSelector, cTycoonType, cCType);
		tosError_ABORT();
	      }
	    *++sp = pArgument;
	    BNEXT();
	  }

	  METHODLABEL(CSlotMoveToMethod):
	  {
	    /* component C slot update */
	    void * pAddress;
	    Int iIndex = ((tyc_CSlotMethod*)pMethod)->slotMethod.iOffset;
#ifndef NDEBUG
	    Char cTycoonType = ((tyc_CSlotMethod*)pMethod)->cTycoonType;
	    Char cCType;
	    assert(!tsp_IS_IMMEDIATE(pReceiver) && !tyc_IS_NIL(pReceiver));
	    cCType = tsp_getCType(pReceiver, iIndex);
	    assert(cCType == tsp_Field_OID && cTycoonType == tyc_Type_Object);
#endif
	    pAddress = tsp_getCAddress(pReceiver, iIndex);
	    /* ### CYCLE */
	    moveToComponent(pReceiver, (tsp_OID*)pAddress, pThread, sp);
	    *++sp = tyc_NIL;
	    BNEXT();
	  } 

	  METHODLABEL(PoolAccessMethod):
	  {
	    /* pool access */
	    *sp = *(((tyc_PoolAccessMethod*)pMethod)->poolMethod.pCell);
	    BNEXT();
	  }

	  METHODLABEL(PoolUpdateMethod):
	  {
	    /* pool update */
	    tsp_OID pArgument = *sp;
	    *(((tyc_PoolAccessMethod*)pMethod)->poolMethod.pCell) = pArgument;
	    *++sp = pArgument;
	    BNEXT();
	  }

	  METHODLABEL(ExternalMethod):
	  {
	    /* ccall */
	    tsp_OID pResult;
	    Word nArgs = pMethod->nArgs;
	    SAVE_REGS();

	    /* First call: Initialize error codes */
	    if (!((tyc_DLL*)pReceiver)->errCode) {
	       ((tyc_DLL*)pReceiver)->errCode = tyc_TAG_MAYBEBOXED(0);
	       ((tyc_DLL*)pReceiver)->errCodeDetail = tyc_TAG_MAYBEBOXED(0);
	    }
	    
	    /* Restore last error context from receiver object */
	    tosError_setContext(tyc_TAGGED_INT_VALUE(((tyc_DLL*)pReceiver)->errCode),
				tyc_TAGGED_INT_VALUE(((tyc_DLL*)pReceiver)->errCodeDetail));

	    pResult = tmccall_execute((tyc_ExternalMethod*)pMethod,
				      sp + nArgs);

	    /* Store current error context into receiver object */
	    ((tyc_DLL*)pReceiver)->errCode = tyc_TAG_MAYBEBOXED(tosError_getCode());
	    ((tyc_DLL*)pReceiver)->errCodeDetail = tyc_TAG_MAYBEBOXED(tosError_getCodeDetail());

	    FETCH_THREAD();
	    LOAD_REGS();
	    sp += nArgs;	/* remove arguments */
	    *sp = pResult;	/* push result onto stack */
	    BNEXT();
	  }

#ifndef tvm_THREADED_CODE
	  default:
	  {
	    /* no valid method */
	    fprintf(stderr,"\nTVM warning: unknown method type for %s.%s.\n",
		    tyc_CLASS(tyc_CLASSID(pReceiver))->pszName,
		    tyc_SELECTOR(idSelector));
	    goto dontUnderstand;
	  }
	}
#else
methodEnd:
#endif

	/* builtin method exit code */
	CLEAR_PAST_EVENT(pThread);
#ifdef tvm_TRACE
	if(ip[0] == tvm_Op_sendTail) {
	  /* in case of sendTail, do not trace the following return opcode */
	  tsp_OID v = *sp;
	  sp = ((tsp_OID*)(fp + 1)) + fp->pCode->method.nArgs;
	  ip = fp->parent.ip;
	  fp = fp->parent.fp;
	  *sp = v;
	}
#endif
	ip += 3;
#ifdef tvm_TRACE
	if(debug_interesting(pThread, TRACE_RETURN_BIT,
			     pThread->pReceiver,
			     fp->pReceiver)) {
	  SET_BUILTIN_RETURN(pThread);
	  SAVE_REGS();
	  tmthread_debugSuspend(SUSP_RETURN_BIT);
	  FETCH_THREAD();
	  LOAD_REGS();
	  CLEAR_BUILTIN_RETURN(pThread);
	}
#endif
	NEXT();
	}
	/* 1st level cache miss */
	{
	/* aquire 2nd level r/w cache lock */
	tmthread_cacheLock();
	/* decrease update counter and update 1st level cache if necessary */
	if(--cacheMisses == 0) {
	  tmthread_cacheUnlock();
	  tmthread_pushStack(pReceiver);
	  SAVE_REGS();
	  updateCache();
	  FETCH_THREAD();
	  LOAD_REGS();
	  pReceiver = tmthread_popStack();
	  tmthread_cacheLock();
	}
	pEntry = &rwCache[HASH(idClass, idSelector)];
	if(pEntry->key == tvm_KEY(idSelector, idClass)) {
	  /* 2nd level cache hit */
	  pMethod = pEntry->pMethod;
#ifdef tvm_THREADED_CODE
	  pMethodCode = pEntry->pMethodCode;
#endif
	  /* release 2nd level cache lock */
	  tmthread_cacheUnlock();
#ifdef tvm_THREADED_CODE
	  goto *pMethodCode;
#else
	  goto lookupHit;
#endif
	}
	/* 2nd level cache miss */
	{
	if((pMethod = tvm_methodLookup(idSelector, idClass))) {
	  /* check signatures (no. of positional arguments, keywords, sorts) */
	  tyc_Selector *pSelector;
	  tyc_ReorderMethod *pReorderMethod;
methodCandidateFound:
	  pSelector = tyc_pRoot->apSelectorTable[idSelector];
	  pReorderMethod = NULL;
	  assert(pMethod->apKeywords);
	  assert(pSelector->apKeywords);
	  if( pMethod->apKeywords == pSelector->apKeywords) {
	    if( pMethod->nArgs != pSelector->wArity || pMethod->wSorts != pSelector->wSorts)
	      goto wrongSignature;
	  } else {
	    int kmSize = tsp_size(pMethod->apKeywords)/sizeof(tsp_OID);
	    int ksSize = tsp_size(pSelector->apKeywords)/sizeof(tsp_OID);
	    tyc_Array pFillWords;
	    if(ksSize != kmSize
	       || memcmp(pMethod->apKeywords, pSelector->apKeywords, ksSize*sizeof(tsp_OID)) != 0) {
	      /* keyword lists are not identical */
	      /* need to generate a reorder method (or bail out) */
	      tyc_Selector *pDelegateSelector;
	      int idDelegateSelector;
	      Bool fIsPrivate;
	      int nPositional = pMethod->nArgs-kmSize;
	      if( nPositional != pSelector->wArity-ksSize
		  || (pMethod->wSorts & ((1<<nPositional)-1))
		     != (pSelector->wSorts & ((1<<nPositional)-1))
		  || kmSize < ksSize ) {
		/*
		// wrong number of positional arguments
		// or wrong component sort of positional arguments
		// or more keywords than expected
		*/
		goto wrongSignature;
	      }
	      /* needs reordering / default values */
	      /* determine selector id of actual method */
	      switch(tsp_classId(pMethod)) {
	      case tyc_ClassId_BuiltinMethod:
	      case tyc_ClassId_CompiledMethod:
		idDelegateSelector = ((tyc_CompiledMethod *)pMethod)->idSelector;
		break;
	      default:
		/* ### to do: ExternalMethod (see also pKeywordDefaults below) */
		/* other method types cannot have keyword parameters */
		tosLog_error("tvm", "keyword reordering", "unimplemented method class: id=%d", tsp_classId(pMethod));
		tosError_ABORT();  /* unimplemented */
	      }
	      fIsPrivate = pMethod->fIsPrivate;
	      tmthread_cacheUnlock();
	      tmthread_pushStack(pMethod);
	      SAVE_REGS();
	      pReorderMethod = tsp_newStruct(tyc_ClassId_ReorderMethod);
	      if (kmSize > ksSize) {
	        tmthread_pushStack(pReorderMethod);
	        pFillWords = tsp_newArray(tyc_ClassId_Array, kmSize - ksSize);
	        pReorderMethod = tmthread_popStack();
	      } else {
		pFillWords = NULL;
	      }
	      FETCH_THREAD();
	      LOAD_REGS();
	      pMethod = tmthread_popStack();
	      tmthread_cacheLock();
	      pSelector = tyc_pRoot->apSelectorTable[idSelector];
	      pReceiver = sp[pSelector->wArity];
	      pDelegateSelector = tyc_pRoot->apSelectorTable[idDelegateSelector];
	      pReorderMethod->method.pSelector = pSelector->pSymbol;
	      pReorderMethod->method.fIsPrivate = fIsPrivate;
	      pReorderMethod->method.nArgs = pSelector->wArity;
	      pReorderMethod->method.wSorts = pSelector->wSorts;
	      pReorderMethod->method.apKeywords = pSelector->apKeywords;
	      pReorderMethod->idDelegateSelector = idDelegateSelector;
	      pReorderMethod->pFillWords = pFillWords;
	      if (computePermutation(pReorderMethod->aPermutation,
				     pFillWords,
				     ((tyc_CompiledMethod *)pMethod)->pKeywordDefaults,
				     kmSize,
				     pDelegateSelector->apKeywords, 
				     pDelegateSelector->wSorts >> nPositional,
				     ksSize, 
				     pSelector->apKeywords, 
				     pSelector->wSorts >> nPositional)) {
		/* unexpected keywords or component mismatch */
		goto wrongSignature;
	      }
	    }
	  }
	  /* check for special method types */
	  switch(tsp_classId(pMethod)) {
	  case tyc_ClassId_ExternalMethod:
	    {
	    void * pFunction;
	    /* check if external dll is open */
	    if(!((tyc_DLL*)pReceiver)->hDLL) {
	      tmthread_cacheUnlock();
	      SAVE_REGS();
	      RAISE_DLLOpenError(pReceiver);
	    }
	    /* function lookup */
	    pFunction = rtdll_lookup(
	      (rtdll_T)tyc_TAGGED_INT_VALUE(((tyc_DLL*)pReceiver)->hDLL),
	      ((tyc_ExternalMethod*)pMethod)->pszLabel);
	    if(!pFunction) {
	      /* function does not exist in DLL: raise exception */
	      tmthread_cacheUnlock();
	      SAVE_REGS();
	      RAISE_DLLCallError(pReceiver,
				 ((tyc_ExternalMethod*)pMethod)->pszLabel);
	    }
	    checkBlocking((tyc_ExternalMethod*)pMethod);
	    ((tyc_ExternalMethod*)pMethod)->pFunction = pFunction;
#ifdef tvm_THREADED_CODE
	    pMethodCode = &&ExternalMethod;
#endif
	    }
	    break;
	  case tyc_ClassId_BuiltinMethod:
	    {
	    lookupBuiltin((tyc_BuiltinMethod*)pMethod);
#ifdef tvm_THREADED_CODE
	    {
	    Word n = ((tyc_BuiltinMethod*)pMethod)->iNumber;
	    if(n >= 0 && n < tvm_Builtin_nBuiltins) {
	      pMethodCode = builtinTable[n];
	    }
	    else {
	      pMethodCode = &&label_tvm_Builtin_Undefined;
	    }
	    }
#endif
	    }
	    break;
#ifdef tvm_THREADED_CODE
	  case tyc_ClassId_CompiledMethod:
	    pMethodCode = &&CompiledMethod;
	    break;
	  case tyc_ClassId_SlotAccessMethod:
	    pMethodCode = &&SlotAccessMethod;
	    break;
	  case tyc_ClassId_CSlotAccessMethod:
	    pMethodCode = &&CSlotAccessMethod;
	    break;
	  case tyc_ClassId_SlotUpdateMethod:
	    pMethodCode = &&SlotUpdateMethod;
	    break;
	  case tyc_ClassId_CSlotUpdateMethod:
	    pMethodCode = &&CSlotUpdateMethod;
	    break;
	  case tyc_ClassId_SlotReferenceMethod:
	    pMethodCode = &&SlotReferenceMethod;
	    break;
	  case tyc_ClassId_SlotMoveToMethod:
	    pMethodCode = &&SlotMoveToMethod;
	    break;
	  case tyc_ClassId_SlotTakeFromMethod:
	    pMethodCode = &&SlotTakeFromMethod;
	    break;
	  case tyc_ClassId_CSlotReferenceMethod:
	    pMethodCode = &&CSlotReferenceMethod;
	    break;
	  case tyc_ClassId_CSlotMoveToMethod:
	    pMethodCode = &&CSlotMoveToMethod;
	    break;
	  case tyc_ClassId_CSlotTakeFromMethod:
	    pMethodCode = &&CSlotTakeFromMethod;
	    break;
	  case tyc_ClassId_PoolAccessMethod:
	    pMethodCode = &&PoolAccessMethod;
	    break;
	  case tyc_ClassId_PoolUpdateMethod:
	    pMethodCode = &&PoolUpdateMethod;
	    break;
	  default:
	  {
	    /* no valid method */
	    fprintf(stderr,"\nTVM warning: unknown method type for %s.%s.\n",
		    tyc_CLASS(tyc_CLASSID(pReceiver))->pszName,
		    tyc_SELECTOR(idSelector));
	    goto dontUnderstand;
	  }
#endif
	  }
	  if(pReorderMethod != NULL) {
	    pReorderMethod->pDelegateMethod = pMethod;
	    pMethod = &pReorderMethod->method;
#ifdef tvm_THREADED_CODE
	    pReorderMethod->pDelegateMethodCode = pMethodCode;
	    pMethodCode = &&ReorderMethod;
#endif
	  }
	  pEntry->pMethod = pMethod;
#ifdef tvm_THREADED_CODE
	  pEntry->pMethodCode = pMethodCode;
#endif
	  pEntry->key = tvm_KEY(idSelector, idClass);
	  /* call method */
	  tmthread_cacheUnlock();
#ifdef tvm_THREADED_CODE
	  goto *pMethodCode;
#else
	  goto lookupHit;
#endif
	}
	}
dontUnderstand:
	/* find builtin __doesNotUnderstand in Object */
	if((pMethod = tvm_methodLookup(idDoesNotUnderstand, idClass))) {
	  /* check no. of arguments */
	  tyc_Selector *pSelector = tyc_pRoot->apSelectorTable[idDoesNotUnderstand];
	  if(pMethod->nArgs != pSelector->wArity || pMethod->wSorts != pSelector->wSorts) {
	    goto wrongSignature;
	  }
	  if(tsp_classId(pMethod) == tyc_ClassId_BuiltinMethod) {
	    /* map all methods to the same builtin code */
	    lookupBuiltin((tyc_BuiltinMethod*)pMethod);
#ifdef tvm_THREADED_CODE
	    {
	    Word n = ((tyc_BuiltinMethod*)pMethod)->iNumber;
	    if(n >= 0 && n < tvm_Builtin_nBuiltins) {
	      pMethodCode = builtinTable[n];
	    }
	    else {
	      pMethodCode = &&label_tvm_Builtin_Undefined;
	    }
	    }
#endif
	  }
	  else {
	    /* must be a builtin! */
	    tosLog_error("tvm",
			 "initDoesNotUnderstand",
			 "Fatal TVM error: __doesNotUnderstand override.");
	    tosError_ABORT();
	  }
	  pEntry->pMethod = pMethod;
#ifdef tvm_THREADED_CODE
	  pEntry->pMethodCode = pMethodCode;
#endif
	  pEntry->key = tvm_KEY(idSelector, idClass);
	  /* call method */
	  tmthread_cacheUnlock();
#ifdef tvm_THREADED_CODE
	  goto *pMethodCode;
#else
	  goto lookupHit;
#endif
	}
	tmthread_cacheUnlock();
	/* method not implemented */
	{
	SAVE_REGS();
	tosLog_error("tvm",
		     "run",
		     "Method not implemented");
	RAISE_DoesNotUnderstand(pReceiver, tyc_SELECTOR(idSelector));
	}
wrongSignature:
	tmthread_cacheUnlock();
	/* no. of arguments or argument sorts do not match */
	{
	int i, nArgs = tyc_ARGUMENTS(idSelector);
	tsp_OID * pArray;
	tyc_Selector * pSelector;
	Word wSorts;
	SAVE_REGS();
	tmthread_pushStack(pMethod);
	tmthread_pushStack(pReceiver);
	pArray = tsp_newArray(tyc_ClassId_MutableArray, nArgs);
	pReceiver = tmthread_popStack();
	pMethod = tmthread_popStack();
	FETCH_THREAD();
	LOAD_REGS();
	/* get selector object */
	pSelector = tyc_pRoot->apSelectorTable[idSelector];
	wSorts = pSelector->wSorts;
	/* copy arguments to array */
	for(i = 0, nArgs--; nArgs >= 0; i++, nArgs--) {
	  pArray[i] = sp[nArgs];
	  if((wSorts & (1<<i)) != 0) {
	    zapComponent(pThread, sp+nArgs);
	  }
	}
	tsp_setImmutable(pArray);
	/* raise exception */
	RAISE_WrongSignature(pMethod, pReceiver, pSelector, pArray);
	}
	}
taggedInt0:
	idClass = tyc_ClassId_Int;
	goto send2;
nil0:
	idClass = tyc_ClassId_Nil;
	goto send2;
      }

      /* special opcode terminating thread */
      LABEL(tvm_Op_terminateThread):
	{
	SAVE_REGS();
	tmthread_exit(TRUE);
	tosError_ABORT();
	}
	NEXT();
#ifndef tvm_THREADED_CODE
      default:
	tosError_ABORT();
#endif
      }
    }
  return NULL;
}


tsp_OID tvm_run(void)
{
  if(setjmp(tmthread_currentJmpBuf())) {
    return run();
  }
  else {
    return run();
  }
}


#ifdef tvm_DEBUG

static Bool fTraceSends = TRUE, fBreak = FALSE;
static char * pClassName = NULL , * pSelectorName = NULL;

void tvm_setTrace(Bool f)
{
  fTraceSends = f;
}

void tvm_setBreak(Bool f)
{
  fBreak = f;
}

void tvm_setBP(char * cn, char * sn)
{
  pClassName = cn ? tosString_strdup(cn) : NULL;
  pSelectorName = sn ? tosString_strdup(sn) : NULL;
}

void tvm_breakPoint(void)
{
  return;
}

void tvm_debugSend(tyc_ClassId idClass, tyc_SelectorId idSelector)
{
  tyc_Symbol pClassSymbol = tyc_CLASS(idClass)->pszName;
  tyc_Symbol pSelectorSymbol = tyc_SELECTOR(idSelector);
  if(fTraceSends) {
    tosLog_debug("TVM_DEBUG", "send",
		 "TID=%d: %s.%s",
		 *((int*)tmthread_currentThread()->hOSThread),
		 pClassSymbol,
		 pSelectorSymbol);
  }
  if(fBreak) {
    if((pClassName == NULL || strcmp(pClassSymbol, pClassName) == 0) &&
      (pSelectorName == NULL || strcmp(pSelectorSymbol, pSelectorName) == 0)) {
      tvm_breakPoint();
    }
  }
}

void tvm_debugSuperSend(tyc_ClassId idClass, tyc_ClassId idSuperClass,
			tyc_SelectorId idSelector)
{
  tyc_Symbol pClassSymbol = tyc_CLASS(idClass)->pszName;
  tyc_Symbol pSuperClassSymbol = tyc_CLASS(idSuperClass)->pszName;
  tyc_Symbol pSelectorSymbol = tyc_SELECTOR(idSelector);
  if(fTraceSends) {
    tosLog_debug("TVM_DEBUG", "sendSuper",
		 "TID=%d: %s.%s lookup starts at %s",
		 *((int*)tmthread_currentThread()->hOSThread),
		 pClassSymbol,
		 pSelectorSymbol,
		 pSuperClassSymbol);
  }
  if(fBreak) {
    if((pClassName == NULL || strcmp(pClassSymbol, pClassName) == 0) &&
      (pSelectorName == NULL || strcmp(pSelectorSymbol, pSelectorName) == 0)) {
      tvm_breakPoint();
    }
  }
}

#else /* tvm_DEBUG */

/* debugging support was not configured at compile time.
   create dummy functions to print an error message. */

#define DEBUG_ERROR(fun) \
  void fun(void) \
  { \
    tosLog_error("tvm",\
		 "DEBUG",\
		 "TMV Error: Machine not configured for debugging");\
  }

DEBUG_ERROR(tvm_setTrace)
DEBUG_ERROR(tvm_setBreak)
DEBUG_ERROR(tvm_setBP)
DEBUG_ERROR(tvm_breakPoint)
DEBUG_ERROR(tvm_debugSend)
DEBUG_ERROR(tvm_debugSuperSend)

#endif /* tvm_DEBUG */
