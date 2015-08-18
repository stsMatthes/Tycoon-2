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

  $File: //depot/tycoon2/stsmain/tycoon2/src/tm/tmthread.h $ $Revision: #3 $ $Date: 2003/10/01 $ Andreas Gawecki, Marc Weikard

  Tycoon Machine Threads
  
*/

#ifndef TMTHREAD_H
#define TMTHREAD_H

#include "tyc.h"
#include "tsp.h"


#ifdef __cplusplus
extern "C" {
#endif


/* thread state flags: */

/* tycoon state (###deprecated) */
#define STATE_TERMINATED tyc_TAG_INT(1)

#define IS_TERMINATED(p)  ((p)->state == STATE_TERMINATED)
#define SET_TERMINATED(p) ((p)->state = STATE_TERMINATED)


/* used by the virtual machine only */
#define BLOCKED_BIT		 ((Word) 0x00000001)
#define SYNC_BIT        	 ((Word) 0x00000002)
#define CCALL_BIT       	 ((Word) 0x00000004)
#define WAITING_BIT     	 ((Word) 0x00000008)
#define INTERRUPT_BIT   	 ((Word) 0x00000010)
#define CANCEL_BIT      	 ((Word) 0x00000020)
#define SUSPENDED_BIT      	 ((Word) 0x00000040)

#define SUSP_SEND_BIT		 ((Word) 0x01000000)
#define SUSP_RETURN_BIT		 ((Word) 0x02000000)
#define SUSP_EXCEPTION_BIT	 ((Word) 0x04000000)
#define SUSP_MASK		 ((Word) 0x07000000)

/* thread local flags (do not require criticalLock).
   Only accessible to other threads at synchronization points. */

#define TRACE_SEND_BIT	         ((Word) 0x00000001)
#define TRACE_RETURN_BIT	 ((Word) 0x00000002)
#define TRACE_EXCEPTION_BIT	 ((Word) 0x00000004)
#define TRACE_INTRACOMPONENT_BIT ((Word) 0x00000008)
#define TRACE_NONCOMPONENT_BIT   ((Word) 0x00000010)
#define TRACE_INHERIT_BIT	 ((Word) 0x00000020)
#define TRACE_MASK		 ((Word) 0x0000003f)

/* PAST_EVENT_BIT: Has to be set if the opcode at the current ip
   		   may already have generated an event, to avoid
		   duplicate events after restart. */
#define PAST_EVENT_BIT		 ((Word) 0x00000040)
/* BUILTIN_RETURN_BIT: The thread is waiting to deliver a builtin return
                   event for the value at top of stack (for the previous
		   opcode).  In combination with IS_SUSPENDED, thread is
		   waiting to be resumed after a builtin return event */
#define BUILTIN_RETURN_BIT	 ((Word) 0x00000080)
/* EXCEPTION_BIT:  The thread is waiting to deliver an exception
                   event for the exception at top of stack.
		   In combination with IS_SUSPENDED, thread is
		   waiting to be resumed after an exception event */
#define EXCEPTION_BIT	 	 ((Word) 0x00000100)


#define MUST_SYNC(p)       ((p)->wFlags &   SYNC_BIT)
#define SET_SYNC(p)        ((p)->wFlags |=  SYNC_BIT)
#define CLEAR_SYNC(p)      ((p)->wFlags &= ~SYNC_BIT)

#define IS_BLOCKED(p)      ((p)->wFlags &   BLOCKED_BIT)
#define SET_BLOCKED(p)     ((p)->wFlags |=  BLOCKED_BIT)
#define CLEAR_BLOCKED(p)   ((p)->wFlags &= ~BLOCKED_BIT)

#define DOES_CCALL(p)      ((p)->wFlags &   CCALL_BIT)
#define ENTER_CCALL(p)     ((p)->wFlags |=  CCALL_BIT)
#define LEAVE_CCALL(p)     ((p)->wFlags &= ~CCALL_BIT)

#define IS_WAITING(p)      ((p)->wFlags &   WAITING_BIT)
#define SET_WAITING(p)     ((p)->wFlags |=  WAITING_BIT)
#define CLEAR_WAITING(p)   ((p)->wFlags &= ~WAITING_BIT)

#define MUST_INTERRUPT(p)  ((p)->wFlags &   INTERRUPT_BIT)
#define SET_INTERRUPT(p)   ((p)->wFlags |=  INTERRUPT_BIT)
#define CLEAR_INTERRUPT(p) ((p)->wFlags &= ~INTERRUPT_BIT)

#define TEST_CANCEL(p)     ((p)->wFlags &   CANCEL_BIT)
#define SET_CANCEL(p)      ((p)->wFlags |=  CANCEL_BIT)
#define CLEAR_CANCEL(p)    ((p)->wFlags &= ~CANCEL_BIT)

#define SUSPEND_FLAGS(p)	   ((p)->wFlags &   SUSP_MASK)
#define IS_SUSPENDED(p)		   ((p)->wFlags &   SUSP_MASK)
#define CLEAR_SUSPENDED(p)	   ((p)->wFlags &= ~SUSP_MASK)
#define SET_SUSPENDED(p,f)         ((p)->wFlags |= ((f) & SUSP_MASK))

#define IS_TRACE_SEND(p)	   ((p)->wLocalFlags & TRACE_SEND_BIT)
#define IS_TRACE_RETURN(p)	   ((p)->wLocalFlags & TRACE_RETURN_BIT)
#define IS_TRACE_EXCEPTION(p)	   ((p)->wLocalFlags & TRACE_EXCEPTION_BIT)
#define IS_TRACE_INTRACOMPONENT(p) ((p)->wLocalFlags & TRACE_INTRACOMPONENT_BIT)
#define IS_TRACE_NONCOMPONENT(p)   ((p)->wLocalFlags & TRACE_NONCOMPONENT_BIT)
#define IS_TRACE_INHERIT(p)	   ((p)->wLocalFlags & TRACE_INHERIT_BIT)

#define TRACE_FLAGS(p)	     ((p)->wLocalFlags & TRACE_MASK)
#define SET_TRACE_FLAGS(p,f) \
 ((p)->wLocalFlags = \
   ((p)->wLocalFlags & ~TRACE_MASK) | ((f) & TRACE_MASK))

#define IS_PAST_EVENT(p)	   ((p)->wLocalFlags &   PAST_EVENT_BIT)
#define CLEAR_PAST_EVENT(p)	   ((p)->wLocalFlags &= ~PAST_EVENT_BIT)
#define SET_PAST_EVENT(p)          ((p)->wLocalFlags |=  PAST_EVENT_BIT)

#define IS_BUILTIN_RETURN(p)	   ((p)->wLocalFlags &   BUILTIN_RETURN_BIT)
#define CLEAR_BUILTIN_RETURN(p)	   ((p)->wLocalFlags &= ~BUILTIN_RETURN_BIT)
#define SET_BUILTIN_RETURN(p)      ((p)->wLocalFlags |=  BUILTIN_RETURN_BIT)

#define IS_EXCEPTION(p)		   ((p)->wLocalFlags &   EXCEPTION_BIT)
#define CLEAR_EXCEPTION(p)	   ((p)->wLocalFlags &= ~EXCEPTION_BIT)
#define SET_EXCEPTION(p)	   ((p)->wLocalFlags |=  EXCEPTION_BIT)


/* limits for stack overflow */
#define tmthread_MAX_STACKPEAK       ((Word)0x00000100)
#define tmthread_ILL_STACKLIMIT  ((tsp_OID*)0xffffffff)

#define SET_PEAK(p)   ((p)->pStackLimit = tmthread_ILL_STACKLIMIT)
#define CLEAR_PEAK(p) ((p)->pStackLimit = (p)->pStack + tmthread_MAX_STACKPEAK)


extern void tmthread_init(void);
extern void tmthread_enumRootPtr(tsp_VisitPtr visitRootPtr);
extern void tmthread_enumAmbiguousRootPtr(tsp_VisitPtr visitAmbiguousPtr);

extern void tmthread_restart(void);
extern tyc_Thread * tmthread_new(tyc_Thread * pThread);
extern void tmthread_exit(Bool fSuccess);

extern void * tmthread_currentJmpBuf(void);
extern tyc_Thread * tmthread_currentThread(void);

/* protect critical region and check for sync requests */
extern void tmthread_criticalLock(void);
extern void tmthread_criticalUnlock(void);
extern void tmthread_checkSyncRequest(void);

/* syncronize all threads for garbage collection and commit */
extern void tmthread_syncRequest(void);
extern void tmthread_syncRelease(void);

/* #ifdef tvm_TRACE */
/* debugging. All methods are synchronization points. */
/* wEvent is a SUSP_... constant */
extern void        tmthread_debugSuspend(Word wEvent);
extern void        tmthread_debugResume(tyc_Thread *pThread);
extern tyc_Thread *tmthread_fetchStoppedThread(tyc_Debugger *pDebugger);
/* #endif */

/* thread local OID save stack */
extern void tmthread_pushStack(tsp_OID);
extern tsp_OID tmthread_popStack(void);

/* cache locks */ 
extern void tmthread_cacheLock(void);
extern void tmthread_cacheUnlock(void);

extern void tmthread_performLock(void);
extern void tmthread_performUnlock(void);

/* mutex & condition variabled */
extern Bool tmthread_initMutex(tyc_Mutex * pMutex);
extern Bool tmthread_initSingleCondition(tyc_Condition * pCondition);
extern Bool tmthread_initBroadcastCondition(tyc_Condition * pCondition);
extern void tmthread_finalizeMutex(tyc_Mutex * pMutex);
extern void tmthread_finalizeCondition(tyc_Condition * pCondition);

/* blocking ccall protocol */
extern void tmthread_enterBlockingCCall(void);
extern void tmthread_leaveBlockingCCall(void);

extern Int tmthread_suspendBlockingCaller(tyc_Thread * pThread);
extern Int tmthread_resumeBlockingCaller(tyc_Thread * pThread);

/* register finalizer thread */
extern void tmthread_notifyFinalizer(void);
extern tsp_OID tmthread_getWeakRefList(void);

/* thread cancellation */
extern void tmthread_setCancel(tyc_Thread * pThread);
extern void tmthread_testCancel(void);


/* profile timer notificaton */
extern void tmthread_notifyTimerThread(void);
extern void tmthread_startTimer(void);
extern void tmthread_stopTimer(void);

#ifdef tvm_PROFILE
  extern Bool tmthread_timerExpired(void);
#else
  extern void tmthread_timerExpired(void);
#endif


/* bootstrap support */
extern void tmthread_createBootThread(tyc_Root *pRoot, tyc_CompiledFun *pFun);


#ifdef __cplusplus
}
#endif

#endif

