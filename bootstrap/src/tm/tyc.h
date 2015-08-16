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

  $File: //depot/tycoon2/stsmain/bootstrap/src/tm/tyc.h $ $Revision: #3 $ $Date: 2003/10/02 $ Andreas Gawecki, Marc Weikard

  Tycoon Objects

*/

#ifndef TYC_H
#define TYC_H

#include "tos.h"
#include "rtdll.h"
#include "tsp.h"


#ifdef __cplusplus
extern "C" {
#endif


/* Boxed values: */

typedef struct tyc_Bool {
  Bool value;
} tyc_Bool;

typedef struct tyc_Char {
  Char value;
} tyc_Char;

typedef struct tyc_Int {
  Int value;
} tyc_Int;

typedef struct tyc_Long {
  Long value;
} tyc_Long;

typedef struct tyc_Real {
  Real value;
} tyc_Real;


/* Tagged Integers: avoid bit loss
   00!0xxxxxxxxxxxxxxxxxxxxxxxxxxxxx 
   11!1xxxxxxxxxxxxxxxxxxxxxxxxxxxxx 
*/

#define tyc_MUST_BOX_INT(v) ( ((Int) (v)) != (( ((Int) (v)) << 2) >> 2) )

#define tyc_IS_TAGGED_INT(x) \
  ( (x) ? (tsp_IS_IMMEDIATE(x) ? TRUE : tsp_classId(x) == tyc_ClassId_Int) \
      : FALSE )
  
#define tyc_TAGGED_INT_VALUE(x) \
  ( tsp_IS_IMMEDIATE(x) ? ( ((Int) (x)) >> 2) : ( ((tyc_Int *) (x))->value ))
  
#define tyc_TAG_MAYBEBOXED(v) \
  ((tsp_OID) (tyc_MUST_BOX_INT(v) ? \
              (Int) tyc_boxInt((Int) (v)) : ( (((Int) (v)) << 2) | 1)) )

#define tyc_TAG_INT(v) \
  ((tsp_OID) ( (((Int) (v)) << 2) | 1) )

#define tyc_UNTAG_INT(v) \
  (((Int) (v)) >> 2)

#define tyc_CHAR_VALUE(x) (((tyc_Char *) (x))->value)
#define tyc_LONG_VALUE(x) (((tyc_Long *) (x))->value)
#define tyc_REAL_VALUE(x) (((tyc_Real *) (x))->value)

extern tsp_OID * tyc_boxInt(Int value);
extern tsp_OID * tyc_boxLong(Long value);
extern tsp_OID * tyc_boxReal(Real value);

#define tyc_boxChar(value) \
  ((tsp_OID)(tyc_pRoot->apCharTable[(value) & 0xff]))
#define tyc_boxBool(value) \
  ((tsp_OID)((value) ? tyc_pRoot->pTrue : tyc_pRoot->pFalse))

#define tyc_IS_TRUE(x)  ((x) == tyc_pRoot->pTrue)
#define tyc_IS_FALSE(x) ((x) == tyc_pRoot->pFalse)
#define tyc_IS_NIL(x)   ((x) == NULL)

#define tyc_TRUE  (tyc_pRoot->pTrue)
#define tyc_FALSE (tyc_pRoot->pFalse)
#define tyc_NIL   NULL

#define tyc_CLASSID(x) ((x) == NULL ? tyc_ClassId_Nil : tsp_IS_IMMEDIATE(x) ? \
			tyc_ClassId_Int : tsp_classId(x))

#define tyc_CLASS(id)     (tyc_pRoot->apClassTable[(id)])
#define tyc_SELECTOR(id)  (tyc_pRoot->apSelectorTable[(id)]->pSymbol)
#define tyc_ARGUMENTS(id) (tyc_pRoot->apSelectorTable[(id)]->wArity)
#define tyc_SORTS(id)     (tyc_pRoot->apSelectorTable[(id)]->wSorts)


/* ClassId codes used by the virtual machine: */

#define TYC_CLASSID(p, id, l, s) tyc_ClassId_ ## id,
typedef enum {

  #include "classids.h"
  
  tyc_ClassId_nPredefined
} tyc_PredefinedClassIds;
#undef TYC_CLASSID


/* Tycoon object format characters */
typedef enum {
  tyc_Type_Bool =          'B',
  tyc_Type_Char =          'C',
  tyc_Type_Int =           'I',
  tyc_Type_Long =          'L',
  tyc_Type_Real =          'R',
  tyc_Type_Object =        'O',
  tyc_Type_String =        'S',    /* Ccall argument or return type only */
  tyc_Type_MutableString = 'M',    /* Ccall argument or return type only */
  tyc_Type_Void =          'V'     /* Ccall return type only */
} tyc_Type;


/* Tycoon objects: */

typedef Word tyc_SelectorId;
typedef Word tyc_ClassId;

typedef char *    tyc_Symbol;
typedef tsp_OID * tyc_Array;
typedef Byte *    tyc_ByteArray;
typedef Short *   tyc_ShortArray;
typedef Int *	  tyc_IntArray;
typedef void      tyc_Nil;


typedef struct tyc_List {
  tsp_OID pHead;
  struct tyc_List * pTail;
} tyc_List;

typedef struct tyc_Slot {
  tsp_OID pos;
  tyc_Symbol pszName;
  tsp_OID type;
  char *pszDocumentation;
  int isPrivate;
  int isComponent;
  tsp_OID cache;
} tyc_Slot;

typedef struct tyc_Class { 
  tsp_OID pos;
  tyc_Long * sourceTime;
  tyc_Long * fingerPrint;
  char * pszName;                   /* class name */
  tsp_OID domain;
  tsp_OID supers;
  char * pszDocumentation;
  tsp_OID selfTypeSig;
  tsp_OID metaClassDeclaration;
  tsp_OID slots;
  tsp_OID _methodDictionary;
  tsp_OID wInstanceId;              /* instance id (tagged tyc_ClassId) */
  tsp_OID wInstanceSize;            /* instance size in slots (tagged Int) */
  tsp_OID cpl;
  tyc_List * pMethodDictionaries;   /* list of method dictionaries */
  tsp_OID typeIde;
  tsp_OID classManager;
  tyc_Slot **slotMap;
} tyc_Class;

typedef struct tyc_MethodDictionary { 
  tsp_OID _elementCount;	    /* tagged Int */
  tyc_Symbol * apszKeys;
  struct tyc_Method ** apElements;
  tyc_Class * pClass;
} tyc_MethodDictionary;

typedef struct tyc_DLL {
  tsp_WeakRef * weakRef;            /* from Resource */
  tsp_OID hDLL;                     /* must be an Int */
  char * pszPath;
  tsp_OID errCode;                  /* must be an Int, set by TVM */
  tsp_OID errCodeDetail;            /* must be an Int, set by TVM */
} tyc_DLL;

typedef struct tyc_CatchFrame {     /* ip relative to beginning of bytecode */
  short ipFrom;                     /* start of exception frame */
  short ipTo;                       /* end of exception frame */
  short ip;                         /* start of handler */
  short cwLocals;                   /* locals (words) on stack */
  void * pNativeCode;               /* handler entry points for jit compiler */
} tyc_CatchFrame;                   /* internal representation as ShortArray */

typedef struct tyc_Selector {
  tyc_Symbol pSymbol;
  Word wArity;
  Word wSorts;
} tyc_Selector;

typedef struct tyc_Method {
  tsp_OID sourcePos;
  tyc_Symbol pSelector;             /* selector */
  tsp_OID methodType;
  char * pszDocumentation;
  tsp_OID pPrecondition;
  tsp_OID pPostcondition;
  Bool fIsPrivate;
  Word nArgs;                       /* number of arguments */
  Word wSorts;                      /* bitvector: is param i a component? */
  void * pNativeCode;		    /* for jit compiler */
} tyc_Method;

typedef struct tyc_CompiledMethod {
  tyc_Method method;

  tsp_OID body;
  Word wDebugMode;
  tyc_Class * pClass;
  Byte * pbCode;
  tyc_Array * pLiterals;
  Word idSelector; 
  Word cwStackPeak;	      	    /* stack peak */
  tyc_CatchFrame * asHandlerTable;  /* may be null (no handlers) */
} tyc_CompiledMethod;

typedef struct tyc_BuiltinMethod {
  tyc_CompiledMethod compiledMethod;
  
  Word iNumber;
  void * pNativeBody;
} tyc_BuiltinMethod;

typedef struct tyc_CompiledFun {
  tyc_CompiledMethod compiledMethod;

  tsp_OID freeValueIdes;
  void * pNativeBody;
} tyc_CompiledFun;  

typedef struct tyc_ExternalMethod {
  tyc_Method method;
  
  char * pszLanguage;
  char * pszLabel;
  char * pszTycoonArgs;
  char * pszCArgs;
  Bool   fBlocking;                 /* mark blocking CCalls */
  void * pFunction;                 /* fun address (Int) */
} tyc_ExternalMethod;

typedef struct tyc_SlotMethod {
  tyc_Method method;
  
  tyc_Slot *slot;
  Word iOffset;
} tyc_SlotMethod;

typedef struct tyc_CSlotMethod {
  tyc_SlotMethod slotMethod;

  char cTycoonType;
} tyc_CSlotMethod;

typedef struct tyc_PoolMethod {
  tyc_Method method;

  tsp_OID * pCell;
} tyc_PoolMethod;

typedef struct tyc_PoolAccessMethod {
  tyc_PoolMethod poolMethod;

} tyc_PoolAccessMethod;

typedef struct tyc_PoolUpdateMethod {
  tyc_PoolMethod poolMethod;

} tyc_PoolUpdateMethod;

typedef struct tyc_Fun {            /* only one struct for all FunNs */  
  tyc_CompiledFun * pCompiledFun;   
  tsp_OID awGlobals[1];	            /* actually 0..n */
} tyc_Fun;


typedef struct tyc_Mutex {
  void * hOSMutex;
  tsp_OID pOwner;
} tyc_Mutex;

typedef struct tyc_Condition {
  void * hOSCondition;
} tyc_Condition;


typedef tsp_OID * tyc_Stack;

typedef struct tyc_StackFrame {
  tyc_ByteArray pByteCode;
  tyc_CompiledMethod * pCode;
  tyc_Array pReceiver;
  struct {
    struct tyc_StackFrame * fp;
    Byte * ip;
    void * nativeIp;
  } parent;
} tyc_StackFrame;

typedef struct tyc_Thread {
  void * hOSThread;
  tyc_Stack pStack;
  tyc_Stack sp;
  tyc_StackFrame * fp;
  Byte * ip;
  tyc_Stack pStackLimit;          /* set to pStack + tmthread_MAX_STACKPEAK */
  Word wFlags;			      /* flags used by the virtual machine */
  Word wLocalFlags;		      /* flags local to this thread */
  struct tyc_Thread * pNext, * pPrev; /* link fields */
  tyc_ByteArray pPerformCodeBuffer;   /* used by generic perform */
  tyc_Condition *pSuspendResume;      /* used by suspend/resume */
  tyc_Fun * pFun;		      /* startup closure */
  tsp_OID pReceiver;		      /* for tracing only */

  /* tycoon2 fields */

  tsp_OID fun;                    /* startup function */
  tsp_OID state;		  /* abstract state */
  tsp_OID value;                  /* return value */
  tyc_Mutex * mutex;              /* mutex synchronizing state */
  tyc_Condition * terminated;     /* condition variable */
  String name;                    /* thread name */
} tyc_Thread;

typedef struct tyc_Debugger {
  tyc_Thread *pStoppedThread;
  tyc_Condition *pThreadStopped;
  tyc_Condition *pThreadFetched;
} tyc_Debugger;

/* TVM Exceptions */

typedef struct tyc_TypeError {
  tsp_OID pObject;
  tyc_Class * pClass;
} tyc_TypeError;

typedef struct tyc_DoesNotUnderstand {
  tsp_OID pReceiver;
  tyc_Symbol pSelector;
} tyc_DoesNotUnderstand;

typedef struct tyc_MustBeBoolean {
  tsp_OID pObject;
} tyc_MustBeBoolean;

typedef struct tyc_DLLOpenError {
  tsp_OID pDLL;
} tyc_DLLOpenError;

typedef struct tyc_DLLCallError {
  tsp_OID pDLL;
  char * pszEntry; 
} tyc_DLLCallError;

typedef struct tyc_DivisionByZero {
  tsp_OID pObject;
} tyc_DivisionByZero;

typedef struct tyc_IndexOutOfBounds {
  tsp_OID pObject;
  tsp_OID pIndex;
} tyc_IndexOutOfBounds;

typedef struct tyc_WrongSignature {
  tsp_OID pMethod;                /* method object found */
  tsp_OID pReceiver;              /* message receiver */
  tsp_OID pArgs;                  /* argument list received */
} tyc_WrongSignature;


typedef struct tyc_Finalizer {
  tyc_Mutex * pMutex;
  tyc_Condition * pCondition;
  tsp_WeakRef * pWeaks;
  tyc_Thread * pThread;
  tsp_OID running;
  tsp_OID idle;
} tyc_Finalizer;  

typedef struct tyc_Root {
  tyc_Thread * pThread;         /* first thread in linked list */
  tyc_Class ** apClassTable;	/* mapping class id <-> tyc_Class object */
  tyc_Selector ** apSelectorTable; /* mapping selector id <-> tyc_Selector object */
  tyc_Bool * pTrue;
  tyc_Bool * pFalse;
  tyc_Nil  * pNil;              /* NULL */
  tyc_Char ** apCharTable;	/* mapping ascii <-> tyc_Char object */
  tsp_OID symbolTable;
  Word wSelectorTableSize;
  Word wClassTableSize;
  tyc_Finalizer * pFinalizer;   /* weak reference finalizer object */
  tsp_OID storeDescriptors;
  Word wActive;
  tyc_Debugger *pDebugger;
} tyc_Root;


extern tyc_Root * tyc_pRoot;
  /* the root of the persistent store */

extern tsp_OID * tyc_pArgv;
  /* the arguments supplied by the shell */

extern tsp_OID tyc_pBlockingCCallException;
extern tsp_OID tyc_pThreadCancelledException;
extern tsp_OID tyc_pCommitError;
  /* single exception objects */

extern void tyc_init(void);
extern void tyc_enumRootPtr(tsp_VisitPtr visitRootPtr);
extern void tyc_enumAmbiguousRootPtr(tsp_VisitPtr visitAmbiguousPtr);


#ifdef __cplusplus
}
#endif

#endif
