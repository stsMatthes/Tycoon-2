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

  tmthread.c 1.34 98/11/13 Marc Weikard

  Tycoon Virtual Machine

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <assert.h>

#include "tos.h"
#include "tosLog.h"
#include "tosTLS.h"
#include "tosThread.h"
#include "tosMutex.h"
#include "tosCondition.h"
#include "tosError.h"

#include "tsp.h"
#include "tyc.h"
#include "tvm.h"
#include "tmprofile.h"
#include "tmthread.h"


static Word nThreads, nRunners, nResyncing;
static Bool fSyncRequest = FALSE;

static tosMutex_T mxAlloc;
static tosMutex_T mxCache;
static tosMutex_T mxPerform;

static tosCondition_Single    cvSync;
static tosCondition_Broadcast cvBroadcast;
static tosCondition_Broadcast cvResynced;
static tosCondition_Broadcast cvCommitted;

/* thread local data key */
static tosTLS_T descriptorKey;


/* used by profile timer handler */
#ifdef tvm_PROFILE

static tosMutex_T          mxSample;
static tosCondition_Single cvSample;

static Bool fRestartTimer = FALSE;

#endif


/* thread local save stack: */

#define PTR_PRESERVE 16

typedef struct SaveStack {
  tsp_OID aOID[PTR_PRESERVE];
  Word wFilled;
} SaveStack;

static void pushStack(SaveStack * pStack, tsp_OID * p)
{
  assert(pStack->wFilled < PTR_PRESERVE);
  pStack->aOID[pStack->wFilled++] = p;
  return;
}

static tsp_OID popStack(SaveStack * pStack)
{
  assert(pStack->wFilled > 0);
  return pStack->aOID[--pStack->wFilled];
}

static void visitStack(SaveStack * pStack, tsp_VisitPtr visitRootPtr)
/* visit objects on internal stack */
{
  Word i = 0;
  for (; i < pStack->wFilled; i++) {
    if(!tsp_IS_IMMEDIATE(pStack->aOID[i]))
      visitRootPtr(&pStack->aOID[i]);
  }
}


/* thread data structure: */

typedef struct ThreadDescriptor {
  void * pJumpBuffer;
  tyc_Thread * pThread;
  SaveStack * pSaveStack;
  /* link fields */
  struct ThreadDescriptor * pNext, * pPrev;
} ThreadDescriptor;

static ThreadDescriptor * pDescriptorList = NULL;


/* weaks reference finalizer: */

static void tmthread_finalizer(tsp_WeakRef * pWeakRef)
{
#ifdef rt_DEBUG
  Int i = 0;
  printf("WeakRef scheduling list:\n");
  while(pWeakRef) {
    printf("%x  ", (Word)pWeakRef->p);
    pWeakRef = pWeakRef->pNext;
    if(++i % 8 == 0)
      printf(" #%d#\n", i);
  }
  printf("\n");
#endif
  tmthread_notifyFinalizer();
}


/* init & enumeration functions: */

void tmthread_init(void)
{
  Bool fSuccess;

  /* create thread specific data */
  fSuccess = tosTLS_keyCreate(&descriptorKey);
  assert(fSuccess == 0);

  /* initialize mutexes */
  fSuccess = tosMutex_init(&mxAlloc);
  assert(fSuccess == 0);
  fSuccess = tosMutex_init(&mxCache);
  assert(fSuccess == 0);
  fSuccess = tosMutex_init(&mxPerform);
  assert(fSuccess == 0);

  /* initialize condition variables */
  fSuccess = tosCondition_initSingle(&cvSync);
  assert(fSuccess == 0);
  fSuccess = tosCondition_initBroadcast(&cvBroadcast);
  assert(fSuccess == 0);
  fSuccess = tosCondition_initBroadcast(&cvResynced);
  assert(fSuccess == 0);
  fSuccess = tosCondition_initBroadcast(&cvCommitted);
  assert(fSuccess == 0);

#ifdef tvm_PROFILE
  fSuccess = tosMutex_init(&mxSample);
  assert(fSuccess == 0);
  fSuccess = tosCondition_initSingle(&cvSample);
  assert(fSuccess == 0);
#endif

  /* threads are restarted after system initialization is finished */
  nThreads = 0;
  pDescriptorList = NULL;
  tsp_setFinalizer(tmthread_finalizer);
}


void tmthread_enumRootPtr(tsp_VisitPtr visitRootPtr)
/* enumerate OIDs for all running threads */
{
  ThreadDescriptor * pDescriptor = pDescriptorList;
  while(pDescriptor) {
    visitRootPtr((tsp_OID*)&pDescriptor->pThread);
    visitStack(pDescriptor->pSaveStack, visitRootPtr);
    pDescriptor = pDescriptor->pNext;
  }
}

void tmthread_enumAmbiguousRootPtr(tsp_VisitPtr visitAmbiguousPtr)
/* enumerate ambiguous roots for blocking ccalls */
{
  ThreadDescriptor * pDescriptor = pDescriptorList;
  while(pDescriptor) {
    tyc_Thread * pThread = pDescriptor->pThread;
    if(DOES_CCALL(pThread)) {
      tsp_OID * p = pThread->sp;
      while(p < (tsp_OID*)pThread->fp) {
        visitAmbiguousPtr(p++);
      }
    }
    pDescriptor = pDescriptor->pNext;
  }
}


/* access thread local data: */

void * tmthread_currentJmpBuf(void)
{
  ThreadDescriptor * pDescriptor = tosTLS_getValue(descriptorKey);
  return pDescriptor->pJumpBuffer;
}

tyc_Thread * tmthread_currentThread(void)
{
  ThreadDescriptor * pDescriptor = tosTLS_getValue(descriptorKey);
  return pDescriptor->pThread;
}


/* access thread local OID save stack: */

void tmthread_pushStack(tsp_OID p)
{
  ThreadDescriptor * pDescriptor = tosTLS_getValue(descriptorKey);
  pushStack(pDescriptor->pSaveStack, p);
}

tsp_OID tmthread_popStack(void)
{
  ThreadDescriptor * pDescriptor = tosTLS_getValue(descriptorKey);
  return popStack(pDescriptor->pSaveStack);
}


/* method cache access control: */

void tmthread_cacheLock(void)
{
  tosMutex_lock(&mxCache);
}

void tmthread_cacheUnlock(void)
{
  tosMutex_unlock(&mxCache);
}


/* perform cache access control: */

void tmthread_performLock(void)
{
  tosMutex_lock(&mxPerform);
}

void tmthread_performUnlock(void)
{
  tosMutex_unlock(&mxPerform);
}


/* critical region (store, blocking builtins etc.) access control: */

void tmthread_criticalLock(void)
{
  tosMutex_lock(&mxAlloc);
}

void tmthread_criticalUnlock(void)
{
  tosMutex_unlock(&mxAlloc);
}

void tmthread_checkSyncRequest(void)
{
  while(fSyncRequest) {
    tyc_Thread * pThread = tmthread_currentThread();
    if(MUST_SYNC(pThread)) {
      CLEAR_SYNC(pThread);
      assert(nRunners > 0);
      if(--nRunners == 0) {
        tosCondition_signal(&cvSync);
      }
    }
    tosCondition_wait(&cvBroadcast, &mxAlloc);
  }
  return;
}


/* thread syncronization: */

void tmthread_syncRequest(void)
{
  fSyncRequest = TRUE;
  nRunners = nThreads;
  {
    ThreadDescriptor * pDescriptor = pDescriptorList;
    while(pDescriptor) {
      tyc_Thread * pThread = pDescriptor->pThread;
      SET_SYNC(pThread);
      SET_PEAK(pThread);                  /* speed interpreter sync */
      if(IS_BLOCKED(pThread)) {
        CLEAR_SYNC(pThread);
        nRunners--;
      }
      pDescriptor = pDescriptor->pNext;
    }
    CLEAR_SYNC(tmthread_currentThread()); /* we don't need to sync */
  }
  assert(nRunners > 0);
  nRunners --;

  while(nRunners > 0) {
    tosCondition_wait(&cvSync, &mxAlloc);
  }
}

void tmthread_syncRelease(void)
{
  fSyncRequest = FALSE;
  {
    ThreadDescriptor * pDescriptor = pDescriptorList;
    while(pDescriptor) {
      tyc_Thread * pThread = pDescriptor->pThread;
      /* clear sync request indicator */
      CLEAR_PEAK(pThread);
      pDescriptor = pDescriptor->pNext;
    }
  }
  tosCondition_broadcast(&cvBroadcast);
}


/* init/finalize mutex & condition variables */

Bool tmthread_initMutex(tyc_Mutex * pMutex)
{
  tosMutex_T * pOSMutex;
  Bool fSuccess;
  /* create OS mutex */
  pOSMutex = (tosMutex_T*)malloc(sizeof(tosMutex_T));
  assert(pOSMutex);
  fSuccess = tosMutex_init(pOSMutex);
  assert(fSuccess == 0);
  /* init tycoon object */
  pMutex->hOSMutex = pOSMutex;
  /* don't modify owner since mutexes are reinitialized at system restart */
  return (fSuccess == 0);
}

void tmthread_finalizeMutex(tyc_Mutex * pMutex)
{
  Bool fSuccess;
  fSuccess = tosMutex_destroy(pMutex->hOSMutex);
  assert(fSuccess == 0);
  free(pMutex->hOSMutex);
}

Bool tmthread_initSingleCondition(tyc_Condition * pCondition)
{
  tosCondition_Single * pOSCondition;
  Bool fSuccess;
  /* create OS condition variable */
  pOSCondition = (tosCondition_Single*)malloc(sizeof(tosCondition_Single));
  assert(pOSCondition);
  fSuccess = tosCondition_initSingle(pOSCondition);
  assert(fSuccess == 0);
  /* init tycoon object */
  pCondition->hOSCondition = pOSCondition;
  return (fSuccess == 0);
}

Bool tmthread_initBroadcastCondition(tyc_Condition * pCondition)
{
  tosCondition_Broadcast * pOSCondition;
  Bool fSuccess;
  /* create OS condition variable */
  pOSCondition = (tosCondition_Broadcast*)malloc(sizeof(tosCondition_Broadcast));
  assert(pOSCondition);
  fSuccess = tosCondition_initBroadcast(pOSCondition);
  assert(fSuccess == 0);
  /* init tycoon object */
  pCondition->hOSCondition = pOSCondition;
  return (fSuccess == 0);
}

void tmthread_finalizeCondition(tyc_Condition * pCondition)
{
  Bool fSuccess;
  fSuccess = tosCondition_destroy(pCondition->hOSCondition);
  assert(fSuccess == 0);
  free(pCondition->hOSCondition);
}


/* weak refernce finalizer */

void tmthread_notifyFinalizer(void)
{
  tosCondition_signal(tyc_pRoot->pFinalizer->pCondition->hOSCondition);
}

tsp_OID tmthread_getWeakRefList(void)
{
  return tsp_scheduleRefs();
}


/* virtual machine profile timer */
/* ----------------------------- */
#ifdef tvm_PROFILE

static void virtualTimerOff(void)
{
  struct itimerval timing;
  /* stop timer */
  timing.it_interval.tv_sec = 0;
  timing.it_interval.tv_usec = 0;
  timing.it_value.tv_sec = 0;
  timing.it_value.tv_usec = 0;
  if(setitimer(ITIMER_PROF, &timing, 0) != 0) {
     tosLog_error("tmthread",
                  "virtualTimerOff",
                  "Error: Cannot stop virtual timer");
  }
}

static void virtualTimerOn(void)
{
  struct itimerval timing;
  /*start timer */
  timing.it_interval.tv_sec = 0;
  timing.it_interval.tv_usec = tmprofile_nMicroSeconds;
  timing.it_value.tv_sec = 0;
  timing.it_value.tv_usec = tmprofile_nMicroSeconds;
  if(setitimer(ITIMER_PROF, &timing, 0) != 0) {
     tosLog_error("tmthread",
                  "virtualTimerOn",
                  "Error: Cannot start virtual timer");
  }
}


static void profileHandler(int s)
{
  /* notify profiler */
  tmprofile_fTakeSample = TRUE;
  /* disable timer */
  virtualTimerOff();
}


static void * profileTimer(void * p)
{
  struct sigaction action;
  /* install SIGPROF handler */
  action.sa_handler = profileHandler;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  sigaction(SIGPROF, &action, 0);
  /* start timer */
  virtualTimerOn();
  /* wait until sample was taken and restore timer */
  while(TRUE) {
    Bool fRestartLoop = FALSE;
    while(!fRestartLoop) {
      tosMutex_lock(&mxSample);
      if(!tmprofile_fStarted) {
        tosCondition_wait(&cvSample, &mxSample);
      }
      fRestartLoop = fRestartTimer;
      tosMutex_unlock(&mxSample);
    }
    tosMutex_lock(&mxSample);
    fRestartTimer = FALSE;
    tosMutex_unlock(&mxSample);
    /* restart virtual timer */
    virtualTimerOn();
  }
  tosMutex_unlock(&mxSample);
  return NULL;
}

void tmthread_notifyTimerThread(void)
{
  tosMutex_lock(&mxSample);
  /* restart profiling timer */
  fRestartTimer = TRUE;
  tosMutex_unlock(&mxSample);
}


Bool tmthread_timerExpired(void)
{
  Bool fTakeSample;
  tosMutex_lock(&mxSample);
  /* unprotected access on tmprofile_fTakeSample in the VM is for speed only.
     the flag must be checked again when the profiler is entered using this
     function. */
  fTakeSample = tmprofile_fTakeSample && tmprofile_fStarted;
  /* only one thread should take a sample. */
  tmprofile_fTakeSample = FALSE;
  tosMutex_unlock(&mxSample);
  return fTakeSample;
}


void tmthread_startTimer(void)
{
  tosMutex_lock(&mxSample);
  /* start taking samples */
  tmprofile_fStarted = TRUE;
  /* restart profiling timer */
  fRestartTimer = TRUE;
  /* wake up timer thread */
  tosCondition_signal(&cvSample);
  tosMutex_unlock(&mxSample);
}

void tmthread_stopTimer(void)
{
  tosMutex_lock(&mxSample);
  /* stop taking samples */
  tmprofile_fStarted = FALSE;
  tosMutex_unlock(&mxSample);
}


static void installTimer(void)
{
  pthread_attr_t attr;
  pthread_t new;
  /* set attributes for a bound thread */
#if defined(rt_LIB_HPUX_PARISC) && defined(tmthread_DRAFT_INTERFACE)
  pthread_attr_create(&attr);
#else
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
#endif
  /* create bound thread for profiling */
  if(pthread_create(&new, &attr, profileTimer, NULL) != 0) {
     tosLog_error("tmthread",
                  "installTimer",
                  "Error: Cannot create bound thread");
  }
  return;
}

#else /* tvm_PROFILE */

/* profiling support was not configured at compile time.
   create dummy functions to print an error message. */

#define PROFILE_ERROR(fun) \
  void fun(void) \
  { \
    tosLog_error("tmthread",\
                 "PROFILE",\
                 "TMTHREAD error: Machine not configured for profiling");\
  }

PROFILE_ERROR(tmthread_notifyTimerThread)
PROFILE_ERROR(tmthread_timerExpired)
PROFILE_ERROR(tmthread_startTimer)
PROFILE_ERROR(tmthread_stopTimer)

#endif /* tvm_PROFILE */
/* ----------------------------- */


/* thread creation and restart ======================================== */

#define INITIAL_STACK 1024

static tyc_Thread * allocStoreData(tyc_Thread * pThread)
{
  tsp_OID * sp;
  tyc_StackFrame * pFrame, * pBarrierFrame;
  tyc_Fun * pFun;
  tyc_Stack pStack;
  tyc_ByteArray pBarrierCode;

  /* create store objects */
  tmthread_pushStack(pThread);
  pBarrierCode = tsp_newByteArray(tyc_ClassId_ByteArray, 4);
  tmthread_pushStack(pBarrierCode);
  pStack = tsp_newStack(tyc_ClassId_TVMStack, INITIAL_STACK);
  pBarrierCode = tmthread_popStack();
  pThread = tmthread_popStack();
  /* get initial closure */
  pFun = pThread->pFun;
  /* initialize barrier bytecode */
  memset(pBarrierCode, tvm_Op_terminateThread, 4);
  /* initialize fixed refernces */
  sp = pStack + INITIAL_STACK;
  *--sp = pThread;
  *--sp = pStack;
  /* initialize barrier frame */
  sp -= sizeof(tyc_StackFrame) / sizeof(tsp_OID);
  pBarrierFrame = (tyc_StackFrame*)sp;
  pBarrierFrame->pByteCode = pBarrierCode;
  /* push receiver */
  *--sp = pFun;
  /* initialize frame */
  sp -= sizeof(tyc_StackFrame) / sizeof(tsp_OID);
  pFrame = (tyc_StackFrame*)sp;
  pFrame->parent.ip = pBarrierCode;
  pFrame->parent.fp = pBarrierFrame;
  pFrame->pReceiver = (void**)pFun;
  pFrame->pCode = (tyc_CompiledMethod*)pFun->pCompiledFun;
  pFrame->pByteCode = pFun->pCompiledFun->compiledMethod.pbCode;
  /* initialize thread */
  pThread->pStack = pStack;
  pThread->pStackLimit = pStack + tmthread_MAX_STACKPEAK;
  pThread->sp = sp;
  pThread->fp = pFrame;
  pThread->ip = pFun->pCompiledFun->compiledMethod.pbCode;
  return pThread;
}

static ThreadDescriptor * allocMemData(tyc_Thread * pThread)
{
  void * pJumpBuffer;
  ThreadDescriptor * pDescriptor;
  SaveStack * pSaveStack;

  /* create new thread descriptor */
  pDescriptor = malloc(sizeof(ThreadDescriptor));   /* thread descriptor */
  pJumpBuffer = malloc(sizeof(jmp_buf));            /* jmp_buf for setjmp */
  pSaveStack = malloc(sizeof(SaveStack));           /* OID save stack */
  assert(pJumpBuffer && pSaveStack && pDescriptor);
  memset(pJumpBuffer, 0, sizeof(jmp_buf));
  memset(pSaveStack, 0, sizeof(SaveStack));
  memset(pDescriptor, 0, sizeof(ThreadDescriptor));
  /* initialize descriptor */
  pDescriptor->pJumpBuffer = pJumpBuffer;
  pDescriptor->pSaveStack = pSaveStack;
  pDescriptor->pThread = pThread;
  /* insert into list */
  {
    ThreadDescriptor * pFirst = pDescriptorList;
    pDescriptor->pNext = pFirst;
    pDescriptor->pPrev = NULL;
    if(pFirst) {
      pFirst->pPrev = pDescriptor;
    }
    pDescriptorList = pDescriptor;
  }
  return pDescriptor;
}


static tosThread_FUNC_DECL runThread(ThreadDescriptor * pDescriptor)
{
  Bool fSuccess;
  /* obtail global lock to ensure that we are properly initialized */
  tmthread_criticalLock();
  /* set thread specific data */
  fSuccess = tosTLS_setValue(descriptorKey, pDescriptor);
  assert(fSuccess == 0);
  tmthread_criticalUnlock();
  /* enter interpreter loop */
  return (tosThread_FUNC_RET) tvm_run();
}

static void debugSuspend(Word wEvent);  /* forward */

static tosThread_FUNC_DECL restartThread(ThreadDescriptor * pDescriptor)
{
  tyc_Thread * pThread;
  tsp_WeakRef * pWeakRef;
  Bool fSuccess;

  /* set thread specific data */
  fSuccess = tosTLS_setValue(descriptorKey, pDescriptor);
  assert(fSuccess == 0);
  /* relock system resources */
  pWeakRef = tsp_weakRefs();
  while(pWeakRef) {
    tsp_OID pObject = pWeakRef->p;
    if(!tsp_IS_IMMEDIATE(pObject) && pObject != NULL) {
      if(tsp_classId(pObject) == tyc_ClassId_Mutex) {
        if(((tyc_Mutex*)pObject)->pOwner == pDescriptor->pThread) {
          tosMutex_lock(((tyc_Mutex*)pObject)->hOSMutex);
        }
      }
    }
    pWeakRef = pWeakRef->pNext;
  }
  /* syncronize with other threads */
  tmthread_criticalLock();
  nResyncing--;
  if(nResyncing == 0) {
    tosCondition_broadcast(&cvResynced);
  }
  else {
    while(nResyncing > 0) {
      tosCondition_wait(&cvResynced, &mxAlloc);
    }
  }
  /* check for cond_waiting threads and simulate a broadcast */
  pThread = pDescriptor->pThread;
  if(IS_WAITING(pThread)) {
    void * hOSMutex;
    tyc_Mutex * pMutex = (tyc_Mutex*)(*(pThread->sp));
    Bool fTimedWait = FALSE;
    /* check for wait or timedWait */
    if(tyc_CLASSID(pMutex) != tyc_ClassId_Mutex) {
      assert(tyc_CLASSID(pMutex) == tyc_ClassId_Long);
      fTimedWait = TRUE;
      /* load mutex (timeOut is ignored) */
      pMutex = (tyc_Mutex*)(pThread->sp[1]);
    }
    assert(tyc_CLASSID(pMutex) == tyc_ClassId_Mutex);
    /* fetch the released mutex handle */
    hOSMutex = pMutex->hOSMutex;
    /* release allocation lock */
    tmthread_criticalUnlock();

    /* relock the condition associated mutex */
    tosMutex_lock(hOSMutex);

    /* we finally received the broadcast */
    tmthread_criticalLock();
    tmthread_checkSyncRequest();
    pThread = pDescriptor->pThread;
    /* clear status */
    CLEAR_BLOCKED(pThread);
    CLEAR_WAITING(pThread);
    /* we are the new owner */
    ((tyc_Mutex*)(*(pThread->sp)))->pOwner = pThread;
    /* end of critical section */
    /* remove arguments from stack */
    if(fTimedWait)
      pThread->sp += 2;
    else
      pThread->sp++;
    /* push result */
    *(pThread->sp) = tyc_boxBool(TRUE);
    /* continue with the next opcode */
    pThread->ip += 3;
  }
  /* raise an exception and continue execution if thread was caught
     in a ccall */
  else if(DOES_CCALL(pThread) && IS_BLOCKED(pThread)) {
    /* wait until commit thread called resumeBlockingCall */
    while(MUST_INTERRUPT(pThread)) {
      tosCondition_wait(&cvCommitted, &mxAlloc);
      pThread = pDescriptor->pThread;
    }
    LEAVE_CCALL(pThread);
    CLEAR_BLOCKED(pThread);
    tmthread_criticalUnlock();
    /* raise exception and continue execution */
    if(setjmp(pDescriptor->pJumpBuffer)) {
      return (tosThread_FUNC_RET) tvm_run();
    }
    else {
      tvm_raise(tyc_pBlockingCCallException);
    }
  }
  else if(IS_BUILTIN_RETURN(pThread)) {
    /* builtin return event: ip is already on the next opcode.
       may generate the event or may just wait for resume,
       depending on the SUSPEND_FLAGS
    */
    debugSuspend(SUSP_RETURN_BIT);
    pThread = pDescriptor->pThread;
    CLEAR_BUILTIN_RETURN(pThread);
  }
  else if(IS_EXCEPTION(pThread)) {
    /* exception event: ip is on an instruction that directly or
       indirectly caused the exception.  Exception object is on
       top of the stack.  Raise the exception again.  May or may
       not generate an event, depending on the SUSPEND_FLAGS.
    */
    void *pException = *pThread->sp++;
    CLEAR_BLOCKED(pThread);
    tmthread_criticalUnlock();
    CLEAR_EXCEPTION(pThread);
    if(setjmp(pDescriptor->pJumpBuffer)) {
      return (tosThread_FUNC_RET) tvm_run();
    }
    else {
      tvm_raise(pException);
    }
  }
  else if(IS_BLOCKED(pThread)) {
    /* other blocking operations are reexecuted, and will block again.
       This includes suspension due to primary debug events. */
    CLEAR_BLOCKED(pThread);
  }
  tmthread_criticalUnlock();
  /* enter interpreter loop */
  return (tosThread_FUNC_RET) tvm_run();
}


tyc_Thread * tmthread_new(tyc_Thread * pThread)
{
  tosThread_T * pOSThread;
  ThreadDescriptor * pDescriptor;
  Bool fSuccess;

  /* create store thread object */
  pThread = allocStoreData(pThread);
  /* create memory thread object */
  tmthread_pushStack(pThread);
  tmthread_criticalLock();
  tmthread_checkSyncRequest();
  pThread = tmthread_popStack();
  pDescriptor = allocMemData(pThread);
  /* insert store thread into list */
  {
    tyc_Thread * pFirst = tyc_pRoot->pThread;
    pThread->pNext = pFirst;
    pThread->pPrev = NULL;
    if(pFirst) {
      pFirst->pPrev = pThread;
    }
    tyc_pRoot->pThread = pThread;
  }
  /* alloc memory for thread handle */
  pOSThread = (tosThread_T*)malloc(sizeof(tosThread_T));
  assert(pOSThread);

  /* create a real thread */
  fSuccess = tosThread_create(pOSThread,
                              (tosThread_FUNC_T)runThread,
                              pDescriptor);
  assert(fSuccess == 0);
  /* if something went wrong destroy thread */
  if(fSuccess != 0) {
    /* unlink descriptor */
    {
      ThreadDescriptor * pNext = pDescriptor->pNext;
      if(pNext) {
        pNext->pPrev = NULL;
      }
      pDescriptorList = pNext;
    }
    free(pDescriptor->pSaveStack);
    free(pDescriptor->pJumpBuffer);
    free(pDescriptor);
    /* free reserved space for handle */
    free(pOSThread);
    /* unlink thread */
    {
      tyc_Thread * pNext = pThread->pNext;
      if(pNext) {
        pNext->pPrev = NULL;
      }
      tyc_pRoot->pThread = pNext;
    }
    /* clear tvm link fields. other tvm fields are needed until the thread is
       discarded by the garbage collection */
    pThread->pNext = pThread->pPrev = NULL;
    tmthread_criticalUnlock();
    return NULL;
  }
  /* complete initialization */
  pThread->hOSThread = pOSThread;
  nThreads++;
  /* we are ready */
  tmthread_criticalUnlock();
  return pThread;
}


static void tmthread_createWeakRefs(tsp_WeakRef * pWeakRef)
{
  while(pWeakRef) {
    tsp_OID pObject = pWeakRef->p;
    if(!tsp_IS_IMMEDIATE(pObject) && pObject != NULL) {
      if(tsp_classId(pObject) == tyc_ClassId_Mutex) {
        tmthread_initMutex(pObject);
      }
      else if(tsp_classId(pObject) == tyc_ClassId_Condition ||
              tsp_classId(pObject) == tyc_ClassId_BroadcastingCondition) {
        tmthread_initBroadcastCondition(pObject);
      }
    }
    pWeakRef = pWeakRef->pNext;
  }
}

void tmthread_restart(void)
{
  tosThread_T * pOSThread;
  tyc_Thread * pThread;
  ThreadDescriptor * pDescriptor;
  Bool fSuccess;

  tmthread_criticalLock();
  /* create mutexes and condition variables */
  tmthread_createWeakRefs(tsp_weakRefs());
  /* there is no finalizer when running a bootstrapped store! */
  if(tyc_pRoot->pFinalizer) {
    tmthread_createWeakRefs(tyc_pRoot->pFinalizer->pWeaks);
  }
  /* get thread list */
  pThread = tyc_pRoot->pThread;
  /* restart all threads */
  while(pThread) {
    pDescriptor = allocMemData(pThread);
    /* alloc memory for thread handle */
    pOSThread = (tosThread_T*)malloc(sizeof(tosThread_T));
    assert(pOSThread);
    /* create a real thread */
    fSuccess = tosThread_create(pOSThread,
                                (tosThread_FUNC_T)restartThread,
                                pDescriptor);
    assert(fSuccess == 0);
    /* complete initialization */
    pThread->hOSThread = pOSThread;
    nThreads++;
    pThread = pThread->pNext;
  }
  assert(nThreads > 0);
  /* syncronize threads (be sure all have relocked their resources) */
  nResyncing = nThreads;
  tmthread_criticalUnlock();

#ifdef tvm_PROFILE
  installTimer();
#endif

  /* That's it for the main thread */
  tosThread_mainWaitedExit();
  return;
}


void tmthread_exit(Bool fSuccess)
{
  tyc_Thread * pThread;
  ThreadDescriptor * pDescriptor;

  tmthread_criticalLock();
  tmthread_checkSyncRequest();

  pDescriptor = tosTLS_getValue(descriptorKey);
  pThread = pDescriptor->pThread;
  /* unlink descriptor */
  {
    ThreadDescriptor * pNext = pDescriptor->pNext;
    ThreadDescriptor * pPrev = pDescriptor->pPrev;
    if(pNext) {
      pNext->pPrev = pPrev;
    }
    if(pPrev) {
      pPrev->pNext = pNext;
    }
    else {
      pDescriptorList = pNext;
    }
  }
  free(pDescriptor->pSaveStack);
  free(pDescriptor->pJumpBuffer);
  free(pDescriptor);
  /* unlink thread */
  {
    tyc_Thread * pNext = pThread->pNext;
    tyc_Thread * pPrev = pThread->pPrev;
    if(pNext) {
      pNext->pPrev = pPrev;
    }
    if(pPrev) {
      pPrev->pNext = pNext;
    }
    else {
      tyc_pRoot->pThread = pNext;
    }
  }
  /* free thread handle */
  free(pThread->hOSThread);
  /* clear tvm link fields. other tvm fields are needed until the thread is
     discarded by the garbage collection */
  pThread->pNext = pThread->pPrev = NULL;
  /* if toplevel thread is terminated stop the whole process */
  if(pThread->fun == NULL) {
     tosLog_error("tmthread",
                  "exit",
                  "Fatal TMTHREAD error: Toplevel terminated");
     tosError_ABORT();
  }
  /* wake joined threads if thread was killed by an unhandled exception */
  /* ### this is actually a hack because waiting for a Tycoon Mutex is
         unsafe wrt. persistence */
  if(!fSuccess) {
    tosMutex_lock(pThread->mutex->hOSMutex);
    SET_TERMINATED(pThread);
    tosCondition_broadcast(pThread->terminated->hOSCondition);
    /* no return value */
    pThread->value = NULL;
    tosMutex_unlock(pThread->mutex->hOSMutex);
  }
  /* cleanup is complete */
  nThreads--;
  tmthread_criticalUnlock();
  tosThread_exit(NULL);
  return;
}


void tmthread_enterBlockingCCall(void)
{
  tyc_Thread * pThread = tmthread_currentThread();
  tmthread_criticalLock();
  /* raise an exception if a commit request is running */
  if(MUST_INTERRUPT(pThread)) {
    tmthread_criticalUnlock();
    tvm_raise(tyc_pBlockingCCallException);
  }
  /* thread is going to enter a blocking ccall */
  ENTER_CCALL(pThread);
  SET_BLOCKED(pThread);
  tmthread_checkSyncRequest();
  tmthread_criticalUnlock();
}

void tmthread_leaveBlockingCCall(void)
{
  tyc_Thread * pThread = tmthread_currentThread();
  tmthread_criticalLock();
  /* wait if a commit request is running */
  while(MUST_INTERRUPT(pThread)) {
    tosCondition_wait(&cvCommitted, &mxAlloc);
    pThread = tmthread_currentThread();
  }
  tmthread_checkSyncRequest();
  /* thread is leaving a blocking ccall */
  pThread = tmthread_currentThread();
  LEAVE_CCALL(pThread);
  CLEAR_BLOCKED(pThread);
  tmthread_criticalUnlock();
}


Int tmthread_suspendBlockingCaller(tyc_Thread * pThread)
{
  Bool fState;
  tmthread_criticalLock();
  SET_INTERRUPT(pThread);
  /* check if thread has entered a blocking ccall */
  fState = DOES_CCALL(pThread) && IS_BLOCKED(pThread);
  tmthread_criticalUnlock();
  return fState;
}

Int tmthread_resumeBlockingCaller(tyc_Thread * pThread)
{
  Bool fState;
  tmthread_criticalLock();
  CLEAR_INTERRUPT(pThread);
  /* wakeup all waiting threads */
  fState = tosCondition_broadcast(&cvCommitted);
  tmthread_criticalUnlock();
  return fState;
}


/* thread cancellation */

void tmthread_setCancel(tyc_Thread * pThread)
{
  tmthread_criticalLock();
  SET_CANCEL(pThread);
  /* throw thread out of a blocking system call */
  if(DOES_CCALL(pThread) && IS_BLOCKED(pThread)) {
    tosThread_kill(*((tosThread_T*)(pThread->hOSThread)));
  }
  tmthread_criticalUnlock();
}

void tmthread_testCancel(void)
{
  tmthread_criticalLock();
  tmthread_checkSyncRequest();
  /* test cancellation flags */
  {
  tyc_Thread * pThread = tmthread_currentThread();
  if(TEST_CANCEL(pThread)) {
    /* clear cancel flag */
    CLEAR_CANCEL(pThread);
    tmthread_criticalUnlock();
    /* raise exception */
    tvm_raise(tyc_pThreadCancelledException);
  }
  }
  tmthread_criticalUnlock();
}


#ifdef tvm_TRACE

void tmthread_debugSuspend(Word wEvent)
{
  tmthread_criticalLock();
  tmthread_checkSyncRequest();
  debugSuspend(wEvent);
  tmthread_criticalUnlock();
}

#endif /* tvm_TRACE */

static void debugSuspend(Word wEvent)
{
  tyc_Thread *pThread;

  assert(wEvent != 0 && (wEvent & SUSP_MASK) == wEvent);

  pThread = tmthread_currentThread();

  /* If the system was saved during actual execution of an
     instruction that generated an event (after resume), and
     the ip still points to that instruction, we must neither
     generate another event nor wait for another resume. */
  if(!IS_PAST_EVENT(pThread)) {
  
    SET_BLOCKED(pThread);

    if(IS_SUSPENDED(pThread) && SUSPEND_FLAGS(pThread) != wEvent) {
      tosLog_error("tmthread",
                   "debugSuspend",
                   "TMTHREAD warning: different debug event after restart, old=%08x, new=%08x",
                   IS_SUSPENDED(pThread),
                   wEvent);
      CLEAR_SUSPENDED(pThread);
    }

    if(!IS_SUSPENDED(pThread)) {
      tyc_Debugger *pDebugger = tyc_pRoot->pDebugger;
      assert(tyc_CLASSID(pDebugger) == tyc_ClassId_Debugger);

      /* first time through: generate event */
      while(!tyc_IS_NIL(pDebugger->pStoppedThread)) {
	assert(tyc_CLASSID(pDebugger->pThreadFetched) ==tyc_ClassId_Condition);
        tosCondition_wait(pDebugger->pThreadFetched->hOSCondition, &mxAlloc);
	/* no sync request - this thread is blocked */
	pDebugger = tyc_pRoot->pDebugger;
	assert(tyc_CLASSID(pDebugger) == tyc_ClassId_Debugger);
      }
      pThread = tmthread_currentThread();
      SET_SUSPENDED(pThread, wEvent);
      /* fprintf(stderr, "suspended: %s, %08x\n", pThread->name, wEvent); */
      pDebugger->pStoppedThread = pThread;
      assert(tyc_CLASSID(pDebugger->pThreadStopped) == tyc_ClassId_Condition);
      tosCondition_signal(pDebugger->pThreadStopped->hOSCondition);
    }

    /* now wait for resume */
    do {
      assert(tyc_CLASSID(pThread->pSuspendResume) == tyc_ClassId_Condition);
      tosCondition_wait(pThread->pSuspendResume->hOSCondition, &mxAlloc);
      /* no sync request - this thread is blocked */
      pThread = tmthread_currentThread();
    } while(IS_SUSPENDED(pThread));
    /* fprintf(stderr, "resumed: %s\n", pThread->name); */
    CLEAR_BLOCKED(pThread);
    /* logically, the thread is now in state "PAST_EVENT" (executing).
       It is the responsibility of the caller to set the thread's PAST_EVENT
       flag for every synchronization point until the ip is advanced. */
  } else {
    assert(!IS_SUSPENDED(pThread));
  }
}

void tmthread_debugResume(tyc_Thread *pThread)
{
  tmthread_criticalLock();
  tmthread_checkSyncRequest();
  CLEAR_SUSPENDED(pThread);
  tosCondition_signal(pThread->pSuspendResume->hOSCondition);
  tmthread_criticalUnlock();
}

tyc_Thread *tmthread_fetchStoppedThread(tyc_Debugger *pDebugger)
{
  tyc_Thread *pResult;
  tyc_Thread *pThread;

  tmthread_criticalLock();
  tmthread_checkSyncRequest();

  assert(tyc_CLASSID(pDebugger) == tyc_ClassId_Debugger);

  pThread = tmthread_currentThread();
  SET_BLOCKED(pThread);
  while(tyc_IS_NIL(pDebugger->pStoppedThread)) {
    tmthread_pushStack(pDebugger);
    assert(tyc_CLASSID(pDebugger->pThreadStopped) == tyc_ClassId_Condition);
    tosCondition_wait(pDebugger->pThreadStopped->hOSCondition, &mxAlloc);
    tmthread_checkSyncRequest();
    pDebugger = (tyc_Debugger *)tmthread_popStack();
  }
  pThread = tmthread_currentThread();
  CLEAR_BLOCKED(pThread);

  pResult = pDebugger->pStoppedThread;
  assert(tyc_CLASSID(pResult) == tyc_ClassId_Thread);
  /* fprintf(stderr, "fetched: %s\n", pResult->name); */
  pDebugger->pStoppedThread = tyc_NIL;
  assert(tyc_CLASSID(pDebugger->pThreadFetched) == tyc_ClassId_Condition);
  tosCondition_signal(pDebugger->pThreadFetched->hOSCondition);
  tmthread_criticalUnlock();
  return pResult;
}


void tmthread_createBootThread(tyc_Root * pRoot, tyc_CompiledFun * pFun)
{
#ifdef BOOTSTRAP

  tyc_Fun * pClosure;
  tsp_OID * sp;
  tyc_Stack pStack;
  tyc_StackFrame * pFrame, * pBarrierFrame;
  tyc_Thread * pThread;
  tyc_ByteArray pBarrierCode;

  /* create new thread & stack object */
  pStack = tsp_newStack(tyc_ClassId_TVMStack, INITIAL_STACK);
  pThread = tsp_newThread(tyc_ClassId_Thread);
  /* create barrier bytecode */
  pBarrierCode = tsp_newByteArray(tyc_ClassId_ByteArray, 4);
  memset(pBarrierCode, tvm_Op_terminateThread, 4);
  /* create empty closure */
  pClosure = tsp_newArray(tyc_ClassId_Fun0, 1);
  pClosure->pCompiledFun = pFun;
  /* init stack */
  sp = pStack + INITIAL_STACK;
  *--sp = pThread;
  *--sp = pStack;
  sp -= sizeof(tyc_StackFrame) / sizeof(tsp_OID);
  pBarrierFrame = (tyc_StackFrame*)sp;
  pBarrierFrame->pByteCode = pBarrierCode;
  /* push receiver */
  *--sp = pClosure;
  /* init frame */
  sp -= sizeof(tyc_StackFrame) / sizeof(tsp_OID);
  pFrame = (tyc_StackFrame*)sp;
  pFrame->parent.ip = pBarrierCode;
  pFrame->parent.fp = pBarrierFrame;
  pFrame->pReceiver = (void**)pClosure;
  pFrame->pCode = (tyc_CompiledMethod*)pFun;
  pFrame->pByteCode = pFun->compiledMethod.pbCode;
  /* init thread */
  pThread->pStack = pStack;
  pThread->pStackLimit = pStack + tmthread_MAX_STACKPEAK;
  pThread->sp = sp;
  pThread->fp = pFrame;
  pThread->ip = pFun->compiledMethod.pbCode;
  pThread->pPerformCodeBuffer = tsp_newByteArray(tyc_ClassId_ByteArray, 4);
  /* patch root object */
  pRoot->pThread = pThread;
  return;

#else
  tosLog_error("tmthread",
               "createBootThread",
               "TMTHREAD error: Machine not configured for bootstrap");
#endif
}

