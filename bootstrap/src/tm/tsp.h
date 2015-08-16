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

  $File: //depot/tycoon2/stsmain/bootstrap/src/tm/tsp.h $ $Revision: #3 $ $Date: 2003/10/02 $ Andreas Gawecki, Marc Weikard

  Tycoon Store Protocol
  
*/

#ifndef TSP_H
#define TSP_H

#include "tos.h"


#ifdef __cplusplus
extern "C" {
#endif


/* tsp error codes */
typedef enum {
  tsp_OK = 0,
  tsp_ERROR_OPEN = 1,           /* cannot open file */
  tsp_ERROR_READ,               /* error reading from file */
  tsp_ERROR_WRITE,              /* error writing to file */
  tsp_ERROR_IO,                 /* general IO error */
  tsp_ERROR_NOMEMORY,           /* not enough memory left */
  tsp_ERROR_BUFFERALLOC,        /* cannot set IO buffer */
  tsp_ERROR_VERSION,            /* storefile of unknown version */
  tsp_ERROR_ARCHITECTURE        /* storefile from unknown architecture */
} tsp_ErrorCode;


#define tsp_IS_IMMEDIATE(x) (((Word)(x))&1)


  /* A string describing the layout of heap objects, one character per field.
     Fields are word-aligned. On architectures with double alignment, in
     general two single word fetches are necessary.

     Field type codes:

     c       8 bit signed char (byte)
     C       8 bit unsigned char
     s       16 bit signed int
     S       16 bit unsigned int
     i       32 bit signed int
     I       32 bit unsigned int
     l       32 bit signed int
     L       32 bit unsigned int
     w       64 bit signed int (long long)
     f	     32 bit float
     d	     64 bit float (double)
     o	     pointer to object

     Use the tsp_Field constants below, do not directly use character constants.
  */

typedef enum {
  tsp_Field_Char =     'c',
  tsp_Field_UChar =    'C',
  tsp_Field_Short =    's',
  tsp_Field_UShort =   'S',
  tsp_Field_Int =      'i',
  tsp_Field_UInt =     'I',
  tsp_Field_Long =     'l',
  tsp_Field_ULong =    'L',
  tsp_Field_LongLong = 'w',
  tsp_Field_Float =    'f',
  tsp_Field_Double =   'd',
  tsp_Field_OID =      'o',
  tsp_Field_String =   'z',     /* only valid for Ccall descriptor strings */
  tsp_Field_void =     'v'      /* only valid for Ccall descriptor strings */
  } tsp_Field;

#define FIELDS_DEFINED ((Word)12)

typedef char tsp_CType;         /* a descriptor character */ 
typedef Word tsp_Offset;        /* byteoffset accessing C structs */ 


/* A tsp_Object code describing the layout of heap objects indirectly. */     

typedef enum {
  tsp_Object_Struct = 0,                /* C structure */
  tsp_Object_WeakRef,	                /* "oooo" */
  tsp_Object_Array,			/* "o*" */
  tsp_Object_ByteArray,		        /* "b*" */
  tsp_Object_ShortArray,		/* "s*" */
  tsp_Object_IntArray,			/* "i*" */
  tsp_Object_LongArray,  		/* "w*" */
  
  tsp_Object_Thread,                    /* "ioiii" */
  tsp_Object_Stack,                     /* "*o" */
  
  tsp_Object_nPredefined
} tsp_Object;

typedef Word tsp_ClassId;

typedef void * tsp_OID;                 /* object identifier */


/* 12 bit limit of header field (may be changed): */
#define tsp_MAX_CLASSIDS (0x1000)


extern tsp_ClassId tsp_newClassId(tsp_Object wLayout,
				  tsp_CType * pszDescriptor);
  /* Define a new classId for heap objects.

     If wLayout is tsp_Object_Struct pszDescriptor must define the
     layout of the returned classId.
  */

typedef void (*tsp_VisitPtr)(tsp_OID * pp);

extern void tsp_init();

extern Int tsp_open(String szName);
  /* for the time beeing: 0 on success, != 0 on error */
  
extern void tsp_close(void); 

extern Int tsp_commit(void); 
extern Int tsp_rollback(void); 

extern tsp_OID tsp_newArray(tsp_ClassId classId, Word nElements);
extern tsp_OID tsp_newByteArray(tsp_ClassId classId, Word nElements);
extern tsp_OID tsp_newShortArray(tsp_ClassId classId, Word nElements);
extern tsp_OID tsp_newIntArray(tsp_ClassId classId, Word nElements);
extern tsp_OID tsp_newLongArray(tsp_ClassId classId, Word nElements);
extern tsp_OID tsp_newStack(tsp_ClassId classId, Word nElements);
extern tsp_OID tsp_newThread(tsp_ClassId classId);
  /* Allocate a new object with the given classId and size in elements.
     The objects contents is initialized with zero. */
extern tsp_OID tsp_newStruct(tsp_ClassId classId);
  /* Allocate a new 'C struct' object with the given classId. 
     Layout and size are defined by the registered descriptor.  
     The objects contents is initialized with zero. */
extern tsp_OID tsp_newWeakRef(tsp_ClassId classId);

extern void tsp_resizeStack(void * p, Word minElements);
  /* expand Stack by at least minElements */
  
extern tsp_OID tsp_newEmptyCopy(tsp_OID p);
  /* Create an empty Object that has the same class and size as object p. Will fail if p is Nil or
     a tagged integer. */
  
extern tsp_OID tsp_root(void);
extern void tsp_setRoot(tsp_OID Root);
  /* get/set the root of the persistent store */

#ifdef tsp_DEBUG
extern Word	   tsp_size(tsp_OID p); 
extern tsp_ClassId tsp_classId(tsp_OID p); 
extern Word	   tsp_hash(tsp_OID p);
extern void        tsp_setHash(tsp_OID p, Word h);

extern tsp_OID	   tsp_superComponent(tsp_OID p);
extern void        tsp_setSuperComponent(tsp_OID p, tsp_OID superComponent);
extern Bool	   tsp_immutable(tsp_OID p);
extern void        tsp_setImmutable(tsp_OID p);
extern Bool	   tsp_isTracedComponent(tsp_OID p);
extern void        tsp_setIsTracedComponent(tsp_OID p, Bool b);

#else

#define tsp_size(p)       (((Word*)(p))[-2])
#define tsp_classId(p)    ((tsp_ClassId)((((Word*)(p))[-1]) & 0x00000fff))
#define tsp_hash(p)       ((Word)(((((Word*)(p))[-1]) & 0x03fff000) >> 12))  
#define tsp_setHash(p, h) ((((Word*)(p))[-1]) = ((((Word*)(p))[-1]) \
			   & 0xfc000fff) | (((h) << 12) & 0x03fff000))

#define tsp_superComponent(p)       (((tsp_OID*)(p))[-3])
#define tsp_setSuperComponent(p, superComponent)  (((tsp_OID*)(p))[-3] = (superComponent))
#define tsp_immutable(p)  (((Word*)(p))[-1] & 0x40000000)
#define tsp_setImmutable(p)  (((Word*)(p))[-1] |= 0x40000000)
#define tsp_isTracedComponent(p)  (((Word*)(p))[-1] & 0x08000000)
#define tsp_setIsTracedComponent(p,b) ((b) ? (((Word*)(p))[-1] |= 0x08000000) \
					   : (((Word*)(p))[-1] &= 0xf7ffffff))
#endif

extern Bool      tsp_isCStruct(tsp_OID p);
extern tsp_CType tsp_getCType(tsp_OID p, Word i);
extern void *    tsp_getCAddress(tsp_OID p, Word i);
extern Word      tsp_getCSize(tsp_OID p);
extern tsp_OID   tsp_getCSlot(tsp_OID p, Word i);
extern Bool      tsp_setCSlot(tsp_OID p, tsp_OID v, Word i);
  /* access C struct objects and descriptors */

typedef void (*tsp_GcHandler)(void); 

extern void tsp_setGcHandler(tsp_GcHandler gcHandler); 

typedef void (*tsp_EnumRootPtr)(tsp_VisitPtr visitRootPtr);

extern void tsp_setEnumRootPtr(tsp_EnumRootPtr enumRootPtr);
  /* Set the enumerator function for non-ambigous roots.
     The  enumerator function should call back the given visitRootPtr()
     function with the address of each non-ambiguous root pointer, which
     will be modified to point to the (possibly) relocated object.
  */

extern void tsp_setEnumAmbiguousRootPtr(tsp_EnumRootPtr enumRootPtr);
  /* Set the enumerator function for ambigous roots.
     The  enumerator function should call back the given visitRootPtr()
     function with the address of each ambiguous root pointer;
     if its value is a valid heap pointer, the corresponding object
     is locked in memory (not moved).
     The root pointer given to tsp_setRoot() should not be enumerated.
  */

/* extern tsp_EnumRootPtr tsp_enumAmbiguousRootPtr(void); */
  /* return the enumerator function for ambigous roots,
     or NULL if none was set. */

/* extern tsp_EnumRootPtr tsp_enumRootPtr(void); */
  /* return the enumerator function for non-ambigous roots,
     or NULL if none was set. */

extern void tsp_gc(Bool fCompact);
  /* trigger an un-conditional garbage collection */

  
extern Word tsp_referencesTo(tsp_OID p, tsp_OID * pArray);
extern Word tsp_instancesOf(tsp_ClassId f, tsp_OID * pArray);
  /* Collect objects with fields pointing to p,
     or objects with the given classId code, respectively,
     and store their references into pArray.
     pArray must have been allocated with tsp_newArray and must have a
     classId with layout tsp_Object_Array (i.e., only pointer fields).
     At most tsp_size(pArray)/sizeof(tsp_OID) objects are collected.
     Remaining free slots of pArray are set to NULL.
  */

typedef struct tsp_WeakRef {
  tsp_OID p;
  tsp_OID pData;
  struct tsp_WeakRef * pNext;
  struct tsp_WeakRef * pPrev;
} tsp_WeakRef;

extern tsp_WeakRef * tsp_weakRefs(void);
extern tsp_WeakRef * tsp_scheduleRefs(void);

typedef void (*tsp_Finalizer)(tsp_WeakRef * pWeakRef);

extern void tsp_setFinalizer(tsp_Finalizer finalizer);

extern tsp_WeakRef * tsp_initWeakRef(tsp_WeakRef * pWeakRef);
  /* tsp_newWeakRef() creates a 'weak reference' to p and associates pData
     with it. tsp_newWeakRef(p, pData) is equivalent to
        WeakRef * pw = tsp_new(tyc_ClassId_WeakRef, sizeof(WeakRef));
	ps->p = p;
	ps->pData = pData;
     Both p and pData must point to valid heap objects allocated with
     tsp_new().
     
     If a finalizer has been registered with tsp_setFinalizer(),
     it will be called with the allocated weak reference (which, in the
     meantime, might have been relocated) if the heap object pointed to by
     p became unreachable. An object is 'reachable' if it can be reached
     by a path from the store root, from any root pointer enumerated, or
     from a WeakRef other than itself.

     The object is not collected immediately. The finalizer may install
     references to the object in other objects, i.e. re-establish reachability.
     Ohterwise, the object will be eventually collected by some of the next
     garbage collections (provided there are no ambiguous pointers emerging).

     For example, if A and B are objects which are weakly referenced by weak
     references, then if B is reachable from A, B is reachable. But if A is not
     reachable, then the garbage collector will eventually detect this and
     schedule the finalizer with A. If the finalizer returns without
     resecurrecting A, then A's storage will be reclaimed, at which point B
     will be unreachable, which will lead to a finalizer call with B.

     If A and B are weakly referenced objects which are connected by a cycle,
     then both A and B are reachable. This situation represents a storage leak
     and should be avoided.

     Note: A and B may denote the same object.

     Note: objects allocated directly with tsp_new(tyc_ClassId_WeakRef, ...)
     will not be finalized.
   */

extern void tsp_inhibitGc(void);
extern void tsp_allowGc(void);

extern void tsp_setPrintDepth(Int depth, Int width);
extern void tsp_printObject(tsp_OID p);

extern void tsp_scanObjects(void (*pFun)(tsp_OID));

extern char * tsp_errorCode(tsp_ErrorCode wError);

/* For BOOTSTRAP */
extern Int tsp_createFromFile(String szFileName);

/* For tsp_DEBUG */
extern void tsp_storeStatus(void);      /* print store layout */


#ifdef __cplusplus
}
#endif

#endif


