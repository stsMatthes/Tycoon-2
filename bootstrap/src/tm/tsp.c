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

  $File: //depot/tycoon2/stsmain/bootstrap/src/tm/tsp.c $ $Revision: #3 $ $Date: 2003/10/02 $ Andreas Gawecki, Marc Weikard

  Tycoon Store Protocol

  Extended 'Mostly-Copying Garbage Collection' algorithm by Joel F. Bartlett:
  
    "Compacting Garbage Collection with Ambiguous Roots",
    DEC WRL Research Report, 1988.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef HAS_MMAP
#include <sys/mman.h>
#include <fcntl.h>
#endif

#include "tos.h"
#include "tosLog.h"
#include "tosError.h"
#include "tosStdio.h"
#include "tosString.h"
#include "tosSystem.h"
#include "tosFilename.h"
#include "tosFile.h"
#include "tosFilemode.h"
#include "tosDirectory.h"


#include "tyc.h"
#include "tmthread.h"
#include "tsp.h"
#include "tmdebug.h"


#ifndef max
  #define max(a, b)  ((a) < (b) ? (b) : (a))
#endif
#ifndef min
  #define min(a, b)  ((a) < (b) ? (a) : (b))
#endif

static tsp_OID tsp_new(tsp_ClassId classId, Word wSize);


#define PAGE_SIZE ((Word)512)		/* the size of a logical page */
#define LOG_PAGE_SIZE 9


/* Page (number) <-> pointer conversion: */
#define NPAGE_TO_PTR(n) ((Byte *) ((n) << LOG_PAGE_SIZE))
#define PTR_TO_NPAGE(p) (((Word) (p)) >> LOG_PAGE_SIZE)

#define PPAGE(n)         (store.aPages + ((n) - store.nFirstPage))
#define PTR_TO_PPAGE(p)  (PPAGE(PTR_TO_NPAGE(p)))

typedef enum {
  PageType_Object = 0,		/* Ordinary page containing objects */
  PageType_Continued,		/* Page holding some part of a large object */
  PageType_Foreign		/* This is not our page */
} PageType;
  
typedef struct Page {
  PageType type;
  Word nNext;		        /* link to next page # in gc queue */
  Byte * pb;	       	        /* the virtual memory address of the page */
  Byte * pbFree, * pbEnd;       /* address of unallocated space */
  Int age;
    /* the 'age' of the page. pages with age = store.age are in
    'fromSpace', pages with age = store.newAge are in 'toSpace'.
    pages with other ages are free (if not FOREIGN!). */
} Page;

static Page invalidPage = {PageType_Foreign, 0, NULL, NULL, NULL, 0};

  
typedef struct {
  /* a descriptor defines the layout of classIds. wSize, pzString
     and pzOffsets are only needed for classIds which represent a C structure;
     the entries are NULL in other cases */
  Word wSize;                   /* size of structure in bytes (worst case) */
  tsp_Object wLayout;           /* layout of store objects */ 
  tsp_CType * pzString;         /* descriptor string */
  tsp_Offset * pzOffsets;       /* offset table for fast access */
} Descriptor;

#define DESCRIPTOR(n)  (store.aDescriptors + (n))


#define ALIGN_OBJECT  (8)       /* alignment of created objects */
#define ALIGN_WORD    (4)       /* alignment of Words */

#define ALIGN_UP_OBJECT(x) \
              (((Word)(x) + ALIGN_OBJECT - 1) & ~(Word)(ALIGN_OBJECT-1))

#define MAGIC    0x4d617263UL
#define SVERSION 0x30303930UL
              
#define FILEBUFFER_SIZE (8 * 1024)

#define INITIAL_PAGES (16384) /* initial amount of pages */
#define MIN_ALLOC     (1024)  /* expand store by at least # pages */ 
#define MEMRESERVE    (8192)

#define STOREHEADER_SIZE (16)
#define PAGEHEADER_SIZE  (4)

static tosStdio_T * pFile = NULL;       /* filehandle */
static void       * pbIOBuffer = NULL;  /* IO buffer for tsp_open(), tsp_commit() */

static Bool fSwap,         /* perform conversion from big to little endian */
            fRealign;      /* realign C Structures and adjust offset tables */


/* multiple architecture support: */

/* byteorder: bigEndian (SPARC, PA-RISC), littleEndian (Intel 386) */ 
typedef enum {
  bigEndian = 0,
  littleEndian
} ByteOrder;

/* alignment of C types */
typedef struct {
  tsp_CType cType;              /* tsp_Field identifier */
  Word nBytes, wAlign;          /* size in bytes and alignment boundary */
} AlignTable;

/* architekture specific settings: , byteorder, alignment */
typedef struct {
  String pszName;
  ByteOrder wByteOrder;
  AlignTable * pAlignTable;
} ArchTable;

/* alignment definition tables */

static AlignTable alignTable_SPARC[FIELDS_DEFINED] =
/* SPARC & PowerPC & PA-RISC */
{ {tsp_Field_Char,     1, 1}, {tsp_Field_UChar,    1, 1},
  {tsp_Field_Short,    2, 2}, {tsp_Field_UShort,   2, 2},
  {tsp_Field_Int,      4, 4}, {tsp_Field_UInt,     4, 4},
  {tsp_Field_Long,     4, 4}, {tsp_Field_ULong,    4, 4},
  {tsp_Field_LongLong, 8, 8}, {tsp_Field_Float,    4, 4},
  {tsp_Field_Double,   8, 8}, {tsp_Field_OID,      4, 4}};

static AlignTable alignTable_i386[FIELDS_DEFINED] =
/* Intel 386 */
{ {tsp_Field_Char,     1, 1}, {tsp_Field_UChar,    1, 1},
  {tsp_Field_Short,    2, 2}, {tsp_Field_UShort,   2, 2},
  {tsp_Field_Int,      4, 4}, {tsp_Field_UInt,     4, 4},
  {tsp_Field_Long,     4, 4}, {tsp_Field_ULong,    4, 4},
  {tsp_Field_LongLong, 8, 4}, {tsp_Field_Float,    4, 4},
  {tsp_Field_Double,   8, 4}, {tsp_Field_OID,      4, 4}};

static ArchTable archTable[] = {
  {"Solaris2_SPARC", bigEndian,    alignTable_SPARC},
  {"Linux_i386",     littleEndian, alignTable_i386},
  {"HPUX_PARISC",    bigEndian,    alignTable_SPARC},
  {"Nextstep_i386",  littleEndian, alignTable_i386},
  {"WinNT_i386",     littleEndian, alignTable_i386},  
  {"Win95_i386",     littleEndian, alignTable_i386},
  {"Win98_i386",     littleEndian, alignTable_i386},
  {NULL, 0, NULL}
};
  
static AlignTable * pMaxAlignment = alignTable_SPARC;
/* set to worst case in order to reserve enough memory on all architectures */

static ArchTable * pHostArch;   /* alignment on current architecture */
static ArchTable * pStoreArch;  /* alignment of store file */

static AlignTable * lookupAlign(tsp_CType cType, AlignTable * pTable)
/* lookup alignment for specified C type */
{
  Word n;
  for(n = 0; n < FIELDS_DEFINED; n++) {
    if(cType == pTable->cType)
      return pTable;
    pTable++;
  }
  assert(n < FIELDS_DEFINED);
  return NULL;
}

static ArchTable * lookupArch(String pszName)
/* lookup architecture specific settings */
{
  ArchTable * pTable = archTable;
  while(pTable->pszName != NULL) {
    if(strncmp(pTable->pszName, pszName, tosSystem_NAME_SIZE) == 0) {
      return pTable;
    }
    pTable++;
  }
  return NULL; 
}

static ArchTable * initArch()
{
  ArchTable * pTable = lookupArch(tosSystem_getName());
  if(pTable != NULL) {
    return pHostArch = pTable;
  }
  tosLog_error("tsp",
               "initArch",
               "Cannot run on this architecture: %s", tosSystem_getName());
  tosError_ABORT();
}


typedef struct MemBlock{
/* allocated memory blocks are remembered in this linked list */
  void * p;                        /* start of allocated space */
  Word nPages;                     /* # of pages */
  struct MemBlock * pNext;         /* link to next descriptor */
} MemBlock;

	    
/* This structure is wrapped around the root seen by the user: */

typedef struct {

  String szName;
  
  tsp_OID pRoot;

  Word nFirstPage;                 /* # of first page */
  Word nLastPage;                  /* # of last page */
  Word nAllocatedPages;            /* # of allocated pages */
  Word nPages;                     /* Total # of pages */
  Word nRealPages;                 /* Total # of usable pages */
  Word age;                        /* current age for pages (from space) */
  Word newAge;                     /* next age for pages (to space) */
  Page * aPages;  
  Descriptor * aDescriptors;       /* The Descriptors defined so far */
  Word nDescriptors;

  tsp_WeakRef * aWeakRefs;
  
  struct {
    Int nPage;
    Page * pPage;
    } current;                     /* Page in use */
  
  struct {
    Word nHead;
    Word nTail;
    } queue;                       /* Queued pages during garbage collection */

  MemBlock * pBlock;               /* List of allocated memory blocks */
  
  tsp_GcHandler gcHandler;
  tsp_EnumRootPtr enumRootPtr;
  tsp_EnumRootPtr enumAmbiguousRootPtr;
  tsp_Finalizer finalizer;
} Store;

static Store store;


static Word fGc = 0;               /* Flag if Garbage Collection is running */
static Bool fCompacting = FALSE;   /* Flag if GC should copy large objects */


typedef struct Header {
  tsp_OID superComponent;
  union {
    Word wSize;		     /* Object size in bytes */
    tsp_OID pForward;	     /* Forwarding pointer to relocated object */
    } u;
  Word bitfield;
} Header;

/* the start of the object data is supposed to be aligned by ALIGN_OBJECT,
   but Header is not a multiple of ALIGN_OBJECT.  So there remains a gap
   at the beginning of each page. */
#define PAGE_START_GAP \
	      (ALIGN_UP_OBJECT(sizeof(Header)) - sizeof(Header))

/* cannot use bitfields in header 'cause we must know where the bits are: */  
  
#define PHEADER(p)    ((Header *) ((Byte *)(p) - sizeof(Header)))
#define PTR(pHeader)  ((tsp_OID) ((Byte *)(pHeader) + sizeof(Header)))

/* fast classId access: no shifting */
#define CLASSID_MASK		((Word) 0x00000fff)
#define HASH_MASK		((Word) 0x03fff000)
#define TRACED_BIT		((Word) 0x08000000)
#define FORWARDED_BIT		((Word) 0x10000000)
#define WEAKREF_BIT		((Word) 0x20000000)
#define IMMUTABLE_BIT		((Word) 0x40000000)

#define CLASSID(p)        (PHEADER(p)->bitfield & CLASSID_MASK)
#define SET_CLASSID(p,f)  (PHEADER(p)->bitfield = \
			    (PHEADER(p)->bitfield & ~CLASSID_MASK) | (f))

#define HASH(p)        ((PHEADER(p)->bitfield & HASH_MASK) >> 12)
#define SET_HASH(p,h)  (PHEADER(p)->bitfield = ((PHEADER(p)->bitfield & ~HASH_MASK) \
		         | (((h) << 12) & HASH_MASK)))

#define PFORWARD(p)        (PHEADER(p)->u.pForward)
#define SET_PFORWARD(p,pf) (PHEADER(p)->u.pForward = (pf))

#define IS_FORWARDED(p)	   (PHEADER(p)->bitfield &  FORWARDED_BIT)
#define FLAG_FORWARDED(p)  (PHEADER(p)->bitfield |= FORWARDED_BIT)

#define IS_WEAKREFED(p)	   (PHEADER(p)->bitfield &  WEAKREF_BIT)
#define FLAG_WEAKREFED(p)  (PHEADER(p)->bitfield |= WEAKREF_BIT)
#define CLEAR_WEAKREFED(p) (PHEADER(p)->bitfield &= ~WEAKREF_BIT)

#define SIZE(p)	      (PHEADER(p)->u.wSize)
#define SET_SIZE(p,s) (PHEADER(p)->u.wSize = (s))

#define SUPERCOMPONENT(p)	(PHEADER(p)->superComponent)
#define SET_SUPERCOMPONENT(p,s) (PHEADER(p)->superComponent = (s))

#define IS_IMMUTABLE(p)	   (PHEADER(p)->bitfield &  IMMUTABLE_BIT)
#define FLAG_IMMUTABLE(p)  (PHEADER(p)->bitfield |= IMMUTABLE_BIT)

#define IS_TRACED(p)	(PHEADER(p)->bitfield &  TRACED_BIT)
#define FLAG_TRACED(p)  (PHEADER(p)->bitfield |= TRACED_BIT)
#define CLEAR_TRACED(p) (PHEADER(p)->bitfield &= ~TRACED_BIT)


#ifdef HAS_MMAP
static size_t mmap_pagesize = 0;
static int    mmap_fd = 0;

#define round_down(a, b) (((a) / (b)) * (b))
#define round_up(a, b)   round_down((a) + (b), b)

static void unmap(unsigned long linear_start, unsigned long linear_end)
{
  int prot, flags, fd, retval;

  /* unmap memory-pages between linear_start and linear_end */
  linear_start = round_up (linear_start, mmap_pagesize);
  linear_end = round_down (linear_end, mmap_pagesize);
  if(linear_start < linear_end) {
    prot = PROT_READ | PROT_WRITE | PROT_EXEC;
    flags = MAP_FIXED | MAP_PRIVATE;
    fd = mmap_fd;
    retval = (int)mmap((caddr_t)linear_start, linear_end - linear_start,
		       prot, flags, fd, 0);
    if(retval == -1) {
      tosLog_error("tsp", "unmap", "TSP error: mmap failed!");
      tosError_ABORT();
    }
  }
}

static void unmap_old_pages(void)
{
  Word nPage;
  unsigned long linear_start;
  unsigned long linear_end;

  if(mmap_fd != 0) {
    /* We unmap all pages that aren't foreign or currently alive.
       We do not only consider pages that have just passed out,
       because the physical page may contain logical pages of
       different ages */
    nPage = store.nFirstPage;
    while(nPage <= store.nLastPage) {
      while(PPAGE(nPage)->type == PageType_Foreign
	    || PPAGE(nPage)->age == store.age) {
	/* skip living and foreign pages */
	nPage++;
	if(nPage > store.nLastPage) {
	  return;
	}
      }
      /* this one's dead for sure */
      linear_start = (unsigned long) NPAGE_TO_PTR(nPage);
      nPage++;
      while(nPage <= store.nLastPage
	    && PPAGE(nPage)->type != PageType_Foreign
	    && PPAGE(nPage)->age != store.newAge) {
	/* all old pages... */
	nPage++;
      }
      linear_end = (unsigned long) NPAGE_TO_PTR(nPage);
      /* unmap area */
      unmap(linear_start, linear_end);
    }
  }
  return;
}

static void init_mmap(void)
{
#ifdef rt_LIB_Linux_i386
  mmap_pagesize = getpagesize();
#else
  mmap_pagesize = sysconf(_SC_PAGESIZE);
#endif
  if(mmap_fd == 0) {
    /* zero device must be opened */
    if((mmap_fd = open("/dev/zero", O_RDONLY)) == 0) {
      fprintf(stderr, "TSP error: opening zero device for mmap failed.\n");
    }
  }
}
#endif /* HAS_MMAP */


void tsp_init()
{
  assert(sizeof(Header) == 3 * sizeof(Word));  /*axe:where is this exploited?*/
  assert(sizeof(tsp_OID) == sizeof(Word));
  assert(TRUE == 1);
  
  memset(&store,0,sizeof(Store));
  initArch();
#ifdef HAS_MMAP
  init_mmap();
#endif
}

tsp_OID tsp_root(void)
{
  return store.pRoot;
}

void tsp_setRoot(tsp_OID pRoot)
{
  store.pRoot = pRoot;
}

void tsp_setGcHandler(tsp_GcHandler gcHandler)
{
  store.gcHandler = gcHandler;
}
  
void tsp_setEnumRootPtr(tsp_EnumRootPtr enumRootPtr)
{
  store.enumRootPtr = enumRootPtr;
}
  
void tsp_setEnumAmbiguousRootPtr(tsp_EnumRootPtr enumAmbiguousRootPtr)
{
  store.enumAmbiguousRootPtr = enumAmbiguousRootPtr;
}
  
void tsp_setFinalizer(tsp_Finalizer finalizer)
{
  store.finalizer = finalizer;
}

  
static Word nextPage(Word nPage)
{
  for(;;) {
    nPage++;
    if (nPage > store.nLastPage)
      nPage = store.nFirstPage;
    if (PPAGE(nPage)->type != PageType_Foreign)
      return nPage;
  }
}

static void enqueue(Word nPage)
{
  if (store.queue.nHead != 0)
    PPAGE(store.queue.nTail)->nNext = nPage;
  else
    store.queue.nHead = nPage;
  PPAGE(nPage)->nNext = 0;
  store.queue.nTail = nPage;
}
        
static void visitPtr(tsp_OID * pp)
{
  if(*pp) assert(PTR_TO_NPAGE(*pp) >= store.nFirstPage &&
		 PTR_TO_NPAGE(*pp) <= store.nLastPage);
  /* nothing to do if referring to nil */
  if (!*pp) {
    return;
  }
  /* clear weakref bit in promoted pages */
  if(PTR_TO_PPAGE(*pp)->age == store.newAge) {
    CLEAR_WEAKREFED(*pp);
    return;
  }

  if (IS_FORWARDED(*pp)) {
    *pp = PFORWARD(*pp);
    /* found a path to this object, unmark it */
    CLEAR_WEAKREFED(*pp);
    return;
    }

  if(SIZE(*pp) >= (PAGE_SIZE - ALIGN_UP_OBJECT(sizeof(Header)))
       && !fCompacting) {
    /* promote large objects to new space: */
    Word nPage = PTR_TO_NPAGE(*pp);
    /* stop further allocation in current page */
    store.current.pPage = &invalidPage;
    /* promote first page and enqueue for scanning*/
    PPAGE(nPage)->age = store.newAge;
    store.nAllocatedPages++;
    enqueue(nPage++);
    /* scan for continued pages */
    while(nPage <= store.nLastPage) {
      if(PPAGE(nPage)->type != PageType_Continued ||
	 PPAGE(nPage)->age != store.age) {
	break;
      }
      store.nAllocatedPages++;
      PPAGE(nPage)->age = store.newAge;
      nPage++;
    }
  }
  else {
    /* forward object, leave forwarding pointer in header: */
    Word wSize = SIZE(*pp);
    tsp_OID pNew = tsp_new(CLASSID(*pp), wSize);
    SET_HASH(pNew, HASH(*pp));    /* copy identity hash */
    SET_SUPERCOMPONENT(pNew, SUPERCOMPONENT(*pp));
    if( IS_IMMUTABLE(*pp) )
      FLAG_IMMUTABLE(pNew);
    if( IS_TRACED(*pp) )
      FLAG_TRACED(pNew);
    memcpy(pNew, *pp, wSize);
    PFORWARD(*pp) = pNew;
    FLAG_FORWARDED(*pp);    
    *pp = pNew;
  }
}
  
static void visitWeakRefPtr(tsp_OID * pp)
{
  if(*pp) assert(PTR_TO_NPAGE(*pp) >= store.nFirstPage &&
		 PTR_TO_NPAGE(*pp) <= store.nLastPage);

  if (!*pp || PTR_TO_PPAGE(*pp)->age == store.newAge)
    return;

  if (IS_FORWARDED(*pp)) {
    *pp = PFORWARD(*pp);
    return;
    }

  if((SIZE(*pp) >= (PAGE_SIZE - sizeof(Header))) && !fCompacting) {
    /* promote large objects to new space: */
    Word nPage = PTR_TO_NPAGE(*pp);
    /* stop further allocation in current page */
    store.current.pPage = &invalidPage;
    /* promote first page and enqueue for scanning*/
    PPAGE(nPage)->age = store.newAge;
    store.nAllocatedPages++;
    enqueue(nPage++);
    /* scan for continued pages */
    while(nPage <= store.nLastPage) {
      if(PPAGE(nPage)->type != PageType_Continued ||
	 PPAGE(nPage)->age != store.age) {
	break;
      }
      store.nAllocatedPages++;
      PPAGE(nPage)->age = store.newAge;
      nPage++;
    }
    /* mark new object */
    FLAG_WEAKREFED(*pp);
  }
  else {
    /* forward object, leave forwarding pointer in header: */
    Word wSize = SIZE(*pp);
    tsp_OID pNew = tsp_new(CLASSID(*pp), wSize);
    SET_HASH(pNew, HASH(*pp));    /* copy identity hash */
    SET_SUPERCOMPONENT(pNew, SUPERCOMPONENT(*pp));
    if(IS_IMMUTABLE(*pp))
      FLAG_IMMUTABLE(pNew);
    if(IS_TRACED(*pp))
      FLAG_TRACED(pNew);
    memcpy(pNew, *pp, wSize);
    PFORWARD(*pp) = pNew;
    FLAG_FORWARDED(*pp);    
    *pp = pNew;
    /* mark new object */
    FLAG_WEAKREFED(*pp);
  }
}

  
static void promotePage(Word nPage)
/* pages which have ambiguous references are 'promoted' to newAge
   (including all continued pages). a list of promoted pages is
   formed through the link cells for each page.
*/
{
  if (nPage >= store.nFirstPage &&
      nPage <= store.nLastPage &&
      PPAGE(nPage)->type != PageType_Foreign &&
      PPAGE(nPage)->age == store.age) {
    Word nUpPage = nPage + 1;
    /* scan down continued pages */
    while(PPAGE(nPage)->type == PageType_Continued) {
      store.nAllocatedPages++;
      PPAGE(nPage)->age = store.newAge;
      nPage--;
    }
    /* set object page */
    PPAGE(nPage)->age = store.newAge;
    store.nAllocatedPages++;
    enqueue(nPage);
    /* scan up continued pages */
    while(nUpPage <= store.nLastPage) { 
      if(PPAGE(nUpPage)->type != PageType_Continued ||
	 PPAGE(nUpPage)->age != store.age) {
	break;
      }
      store.nAllocatedPages++;
      PPAGE(nUpPage)->age = store.newAge;
      nUpPage++;
    }      
  }
}


static void visitAmbiguousPtr(tsp_OID * pp)
{
  promotePage(PTR_TO_NPAGE(*pp));
}
  
static void visitArrayFields(register tsp_OID * pArray, Word nSlots)
{
  tsp_OID * pp = pArray;
  pp += nSlots;
  while (pp != pArray) {
    pp--;
    if(!tsp_IS_IMMEDIATE(*pp))
      visitPtr(pp);
  }
}


static void visitPtrFields(tsp_OID p)
{
  Word wClassId = CLASSID(p); 
  switch(DESCRIPTOR(wClassId)->wLayout)
    {
    case tsp_Object_Struct:
      {
      register tsp_Offset * pOffset = DESCRIPTOR(wClassId)->pzOffsets;
      register tsp_CType * pszType  = DESCRIPTOR(wClassId)->pzString;
      for(;;) 
	switch(*pszType++)
	  {
	  case 0:
	    return;
	  case tsp_Field_OID:
	    {
	    tsp_OID * pp = (tsp_OID*)((Byte*)p + (*pOffset++));
	    if(!tsp_IS_IMMEDIATE(*pp))
	      visitPtr(pp);
	    }
	    break;
	  default:
	    pOffset++;
	    break;
	  }
      }
    case tsp_Object_WeakRef:
      /* weak reference node */
      {
      tsp_WeakRef * pWeakRef = p;
      if(!tsp_IS_IMMEDIATE(pWeakRef->p)) {
	visitWeakRefPtr(&pWeakRef->p);
	visitPtr((tsp_OID*)&pWeakRef->pPrev);
	visitPtr((tsp_OID*)&pWeakRef->pNext);
	visitPtr(&pWeakRef->pData);
      }
      else {
	visitArrayFields(p, SIZE(p) / sizeof(Word));
      }
      }
      break;
    case tsp_Object_Array:
      /* array with tagged layout */
      visitArrayFields(p, SIZE(p) / sizeof(Word));
      break;
    case tsp_Object_Thread:
      /* promote stack. thread and stack object are adjusted in one pass.
         uninitialized thread objects are not linked to a runtime stack */
      {
      tyc_Thread * pThread = (tyc_Thread*)p;
      /* visit stackbase */
      visitPtr((tsp_OID*)&(pThread->pStack));
      /* set new limit for check stack */
      if(pThread->pStackLimit != tmthread_ILL_STACKLIMIT) {
	pThread->pStackLimit = pThread->pStack + tmthread_MAX_STACKPEAK
	  + sizeof(tyc_StackFrame) / sizeof(tsp_OID);
      }
      /* visit OIDs */
      visitArrayFields((tsp_OID*)&(pThread->pNext), 12);
      }
      break;
    case tsp_Object_Stack:
      /* collect stack object and associated thread object.
	 a stack always contains 2 fixed elements:
	 a pointer to the thread object and its (former) address */
      {
      Int stackOffset;
      tsp_OID * sp;      
      tsp_OID * pStackEnd = (tsp_OID*)((Byte*)p + SIZE(p) - sizeof(tsp_OID));
      tyc_Thread * pThread;
      tyc_StackFrame * fp, ** oldfp;
      tyc_ByteArray * oldip, oldbp;
      /* visit thread object */
      visitPtr(pStackEnd);
      pThread = *pStackEnd--;
      /* calc offset new <-> old stack object */
      stackOffset = (Byte*)p - (Byte*)*pStackEnd;
      *pStackEnd = p;
      /* set address of outdated references */
      oldfp = &pThread->fp;
      oldip = &pThread->ip;
      /* adjust stack pointer */
      sp = (tsp_OID*)((Byte*)(pThread->sp) + stackOffset);
      pThread->sp = sp;
      fp = pThread->fp;
      /* loop through stack frames */
      while(fp) {
	/* adjust frame pointer and old reference */
	fp = (tyc_StackFrame*)((Byte*)fp + stackOffset);
	*oldfp = fp;
	/* adjust gap between sp and fp */
	while((tsp_OID)sp < (tsp_OID)fp) {
	  if(!tsp_IS_IMMEDIATE(*sp))
	    visitPtr(sp);
	  sp++;
	}
	if(!tsp_IS_IMMEDIATE(fp->pReceiver))         /* tagged receiver */
	  visitPtr((tsp_OID*)&fp->pReceiver);
	visitPtr((tsp_OID*)&fp->pCode);
	/* adjust bytecode array and ip */
	oldbp = fp->pByteCode;
	visitPtr((tsp_OID*)&fp->pByteCode);
	*oldip = *oldip + (fp->pByteCode - oldbp);
	/* advance to next outdated reference */ 
	oldfp = &fp->parent.fp;
	oldip = &fp->parent.ip;
	/* skip one frame and load new fp */
	sp = (tsp_OID *)(fp + 1);
	fp = *oldfp;
      }
      /* adjust gap between sp and fixed elements */
      while((tsp_OID)sp < (tsp_OID)pStackEnd) {
	if(!tsp_IS_IMMEDIATE(*sp))
	  visitPtr(sp);
	sp++;
      }
      }
      break;
    default:
      break;
    }
  return;
}


static void scanPages()
/* Sweep across promoted pages and copy their objects */
{
  while (store.queue.nHead != 0) {
    Word nPage = store.queue.nHead;
    Byte * pb = PPAGE(nPage)->pb;
    /* point to first object (behind header): */
    pb += ALIGN_UP_OBJECT(sizeof(Header));
    /* copy all objects within page: */
    while(pb < PPAGE(nPage)->pbFree) {
      /* don't visit objects with size 0 at end of page (<) */
      visitPtrFields(pb);

      /* round up to next alignment boundary after next header: */
      pb += ALIGN_UP_OBJECT(SIZE(pb)+sizeof(Header));
    }
    /* remove page from queue: */
    store.queue.nHead = PPAGE(nPage)->nNext;    
  }
}

static void updateSuperComponent(tsp_OID p)
{
  tsp_OID superComponent = SUPERCOMPONENT(p);
  if (superComponent == NULL)
    return;
  if (PTR_TO_PPAGE(superComponent)->age == store.newAge)
    return;  /* nothing to do, superComponent was promoted */
  if (IS_FORWARDED(superComponent)) {
    SET_SUPERCOMPONENT(p, PFORWARD(superComponent));
    return;
  }
  fprintf(stderr,"TSP debug info: component becomes orphan\n");
  SET_SUPERCOMPONENT(p, NULL);
}

void tsp_gc(Bool fCompact)
{
  /*
   * gc not available on Windoof 95, because of thread sync problems ...
   */
  if ((tosSystem_getID() == tosSystem_ID_WIN95) ||
      (tosSystem_getID() == tosSystem_ID_WIN98))
  {
     tosLog_error("tsp", "gc", "Warning: Garbage collection not available on Windows 95/98");
     return;
  }

  /* check out-of-space during collection:
     these are equal if a gc is already running: */
  assert(store.age == store.newAge);

  /* syncronizing all running threads */
  tmthread_syncRequest();

  /* garbage collection is running */
  fGc++;
  fCompacting = fCompact;
  
  /* advance space (see Bartlett paper for this peculiar constant): */
  store.newAge = (store.age+1) % 0xffff;
  /* prevent clash with age of pages allocated by moreCore(): */
  if (store.newAge == 0)
    store.newAge = 1;

  store.nAllocatedPages = 0;
  store.queue.nHead = store.queue.nTail = 0;

  /* no page available */
  store.current.pPage = &invalidPage;
  
  /* promote pages pointed to by ambiguous roots: */
  if (store.enumAmbiguousRootPtr) 
    (*store.enumAmbiguousRootPtr)(visitAmbiguousPtr);
  /* copy objects pointed to by non-ambiguos roots: */
  visitPtr(&store.pRoot);
  if (store.enumRootPtr) 
    (*store.enumRootPtr)(visitPtr);
  /* copy first weakref node: */
  visitPtr((tsp_OID*)&store.aWeakRefs);
  
  scanPages();
  
  /* update/zap weak superComponent pointers */
  tsp_scanObjects(updateSuperComponent);

  /* sweep and copy phase is complete at this point, switch to new space
     (and schedule WeakRefs) */
  store.age = store.newAge;
  /* start allocation at first free page */
  store.current.nPage = store.nLastPage;
  store.current.pPage = &invalidPage;

#ifdef HAS_MMAP
  unmap_old_pages();  /* experimental */
#endif

  /* schedule WeakRefs for finalzation: */
  {
    tsp_WeakRef * pWeakRef = store.aWeakRefs;
    tsp_WeakRef ** ppScheduleRefs = &(tyc_pRoot->pFinalizer->pWeaks);
    while(pWeakRef) {
      if(IS_WEAKREFED(pWeakRef->p)) {
	tsp_WeakRef * pFirstRef = *ppScheduleRefs;
	tsp_WeakRef * pOldNext = pWeakRef->pNext;
	tsp_WeakRef * pOldPred = pWeakRef->pPrev;
	/* remove from weakref list */
	if(pOldNext) {
	  pOldNext->pPrev = pOldPred;
	}
	if(pOldPred) {
	  pOldPred->pNext = pOldNext;
	}
	else {
	  store.aWeakRefs = pOldNext;
	}
	/* insert in scheduling list */
	pWeakRef->pNext = pFirstRef;
	pWeakRef->pPrev = NULL;
	if(pFirstRef) {
	  pFirstRef->pPrev = pWeakRef;
	}
	*ppScheduleRefs = pWeakRef;	
	/* move to next weakref */
	pWeakRef = pOldNext;
      }
      else {
	pWeakRef = pWeakRef->pNext;
      }
    }
    /* finalize */
    if(store.finalizer && *ppScheduleRefs) {
      (*store.finalizer)(*ppScheduleRefs);
    }
  }
  /* garbage collection is finished */
  fGc--;

  /* count objects and compare with last gc */
  if(tmdebug_fShowObjectCount && tmdebug_fCountOnGc) {
    tmdebug_showObjectCount();
  }
  
  /* release suspended threads */ 
  tmthread_syncRelease();
}


static void * moreCore(Word cb)
/* Return a pointer to cb bytes of more memory.
   Return NULL if out of memory,
   else initialize with 0. */
{
  void * p = malloc((int) cb);
  if (p) {
    /* enqueue address in memory list */
    MemBlock * pBlock = (MemBlock*) malloc(sizeof(MemBlock));
    if(pBlock) {
      pBlock->p = p;
      pBlock->nPages = cb / PAGE_SIZE;
      pBlock->pNext = store.pBlock;
      store.pBlock = pBlock;
    }
    /* initialize memory */
    memset(p, 0, (int) cb);
  }
  else {
    tosLog_error("tsp",
                 "moreCore",
                 "TSP error: can't expand store: malloc failed!");
    tosError_ABORT();
  }
  return p;
}

static void expandHeap(const Word n)
/* get n more pages from operating system */
{
  Page * aPages;
  Word cb = (n + 1) * PAGE_SIZE - 1;
  Byte * p = moreCore(cb);
  Word nPage, nNewPage;
  assert(p);
  
#ifdef tsp_DEBUG
  printf("\n## Expand by %d Pages.  ",n);
#endif
  
  /* align on page boundary */
  p += PAGE_SIZE-1;
  p = (Byte *) ((Word)p & ~(PAGE_SIZE-1));
  nNewPage = PTR_TO_NPAGE(p);

  /* allocated space is growing upward */
  if(store.nLastPage < nNewPage) {
    /* set range for initialization */
    nPage = store.nLastPage + 1;
    store.nLastPage = PTR_TO_NPAGE(p+(n*PAGE_SIZE)-1);
    /* expand page array */
    aPages = realloc(store.aPages,
		    (store.nLastPage-store.nFirstPage+1) * sizeof(Page));
    if(aPages == NULL) {
      tosLog_error("tsp",
                   "expandHeap",
                   "TSP error: can't reallocate page descriptors!");
      tosLog_error("tsp",
                   "expandHeap",
                   "TSP error: %d bytes of memory needed.",
                   (store.nLastPage-store.nFirstPage+1) * sizeof(Page));
      tosError_ABORT();
    }
    store.aPages = aPages;
    /* initialize 'foreign gap' between current heap pages and new pages */
    for(; nPage < nNewPage; nPage++) {
      memset(PPAGE(nPage), 0, sizeof(Page));
      PPAGE(nPage)->type = PageType_Foreign;
    }
    /* initialize new pages */
    assert(PageType_Object == 0);
    for(; nPage <= store.nLastPage; nPage++) {
      memset(PPAGE(nPage), 0, sizeof(Page));
      PPAGE(nPage)->pb = NPAGE_TO_PTR(nPage);
      PPAGE(nPage)->pbEnd = NPAGE_TO_PTR(nPage) + PAGE_SIZE;
    }
    store.nPages = store.nLastPage - store.nFirstPage + 1;
    store.nRealPages += n;
    
#ifdef tsp_DEBUG
    printf("grow up ##\n\n");
#endif

    if(tmdebug_fShowStoreGrow) {
      tmdebug_showStoreGrow(n, 1);
    }
  } 
  /* allocated space is growing downward */
  else if(store.nFirstPage > nNewPage) {
    /* set range for initialization */
    nPage = store.nFirstPage - 1;
    store.nFirstPage = nNewPage;
    nNewPage = PTR_TO_NPAGE(p+(n*PAGE_SIZE)-1);
    /* expand page array */
    aPages = realloc(store.aPages,
		    (store.nLastPage-store.nFirstPage+1) * sizeof(Page));
    if(aPages == NULL) {
      tosLog_error("tsp",
                   "expandHeap",
                   "TSP error: can't reallocate page descriptors!");
      tosLog_error("tsp",
                   "expandHeap",
                   "TSP error: %d bytes of memory needed.",
                   (store.nLastPage-store.nFirstPage+1) * sizeof(Page));
      tosError_ABORT();
    }
    store.aPages = aPages;
    /* initialize 'foreign gap' between new pages and current heap pages*/
    for(; nPage > nNewPage; nPage--) {
      memset(PPAGE(nPage), 0, sizeof(Page));
      PPAGE(nPage)->type = PageType_Foreign;
    }
    /* initialize new pages */
    assert(PageType_Object == 0);
    for(; nPage >= store.nFirstPage; nPage--) {
      memset(PPAGE(nPage), 0, sizeof(Page));
      PPAGE(nPage)->pb = NPAGE_TO_PTR(nPage);
      PPAGE(nPage)->pbEnd = NPAGE_TO_PTR(nPage) + PAGE_SIZE;
    }      
    store.nPages = store.nLastPage - store.nFirstPage + 1;
    store.nRealPages += n;
    
#ifdef tsp_DEBUG
    printf("grow down ##\n\n");
#endif
    if(tmdebug_fShowStoreGrow) {
      tmdebug_showStoreGrow(n, 0);
    }
  }
  /* reusing foreign memory */
  else if(store.nFirstPage < nNewPage && store.nLastPage > nNewPage) {
    /* set range for initialization */
    nPage = PTR_TO_NPAGE(p+(n*PAGE_SIZE)-1);
    /* initialize new pages */
    assert(PageType_Object == 0);
    for(; nPage >= nNewPage; nPage--) {
      memset(PPAGE(nPage), 0, sizeof(Page));
      PPAGE(nPage)->pb = NPAGE_TO_PTR(nPage);
      PPAGE(nPage)->pbEnd = NPAGE_TO_PTR(nPage) + PAGE_SIZE;
    }      
    store.nPages = store.nLastPage - store.nFirstPage + 1;
    store.nRealPages += n;
    
#ifdef tsp_DEBUG
    printf("grow between ##\n\n");
#endif
    if(tmdebug_fShowStoreGrow) {
      tmdebug_showStoreGrow(n, 2);
    }
  }
  else {
    tosLog_error("tsp",
                 "expandHeap",
                 "TSP error: store expanded in unknown region!");
    tosError_ABORT();
  }
  
  if(tmdebug_fShowObjectCount && !tmdebug_fCountOnGc) {
    tmdebug_showObjectCount();
  }
  return;
}

static void allocatePages(Int n)
/* allocate n consecutive pages. set up store.current appropriately. */
{
  Bool fLoop = FALSE; /* allocation loop after gc */
  
  /* start gc if half or more pages are allocated */
  if(!fGc) {
    if (store.nAllocatedPages >= store.nRealPages / 2) {

#ifdef tsp_DEBUG
      fflush(stdout);
      tsp_storeStatus();
      printf("\n-- Garbage Collection 1 --\n");
#endif

      tsp_gc(FALSE);
      
#ifdef tsp_DEBUG
      tsp_storeStatus();
      fflush(stdout);
#endif

      /* expand memory if less than MEMRESERVE pages are free */
      {
      Int wSpace = (store.nRealPages / 2) - store.nAllocatedPages;
      if(wSpace < MEMRESERVE) {
	Word nReserve = 2 * (MEMRESERVE - wSpace);
	expandHeap(max(nReserve, max(n, MIN_ALLOC)));
	/* start allocation at first free page */
	store.current.nPage = store.nLastPage;
	store.current.pPage = &invalidPage;	
      }
      }
    }
  }
  
  for(;;) {
    /* try to find n consecutive pages among the currently free ones: */
    Word nPages = store.nPages;
    Word nPage = nextPage(store.current.nPage);  /* skip current page */
    Word nFreeConsecutive = 0;    

    while (nPages--) {
      if (PPAGE(nPage)->age != store.age &&
          PPAGE(nPage)->age != store.newAge &&
          PPAGE(nPage)->type != PageType_Foreign)
	{
	if (nFreeConsecutive++ == 0)
	  store.current.nPage = nPage;
	if (nFreeConsecutive == n) {
	  /* so we found them. move back to first free page: */
	  nPage = store.current.nPage ;
	  if (store.age != store.newAge)
	    /* gc is going on, remember page to scan later: */
	    enqueue(nPage);
	  {
	  Page * pPage = PPAGE(nPage);
	  store.current.pPage = pPage;
	  store.nAllocatedPages += n;
	  /* init first page */
	  pPage->type = PageType_Object;
	  pPage->age = store.newAge;
	  pPage->pbFree = NPAGE_TO_PTR(nPage) + PAGE_START_GAP;
	  /* init continued pages */
	  while(--n) {
	    pPage++;                             /* consecutive pages! */
	    pPage->type = PageType_Continued;
	    pPage->age = store.newAge;
	    }
	  }
	  return;
	  }
	}
      else
        nFreeConsecutive = 0;

      nPage++;
      if(nPage > store.nLastPage) {
	nPage = store.nFirstPage;
	nFreeConsecutive = 0;
      }
    }
    
    /* so there are not n consecutive pages. if we have not already,
       perform gc and try again. otherwise, expand the heap. */
    if (fGc || fLoop)
      expandHeap(max(n, MIN_ALLOC)); /* there MUST be n consecutive pages! */
    else {
    
#ifdef tsp_DEBUG
      fflush(stdout);
      tsp_storeStatus();
      printf("\n-- Garbage Collection 2 --\n");
#endif

      tsp_gc(FALSE);	        /* now there MAY be n consecutive pages... */
      
#ifdef tsp_DEBUG
      tsp_storeStatus();
      fflush(stdout);
#endif
      /* expand memory if less than MEMRESERVE pages are free */
      {
      Int wSpace = (store.nRealPages / 2) - store.nAllocatedPages;
      if(wSpace < MEMRESERVE) {
	Word nReserve = 2 * (MEMRESERVE - wSpace);
	expandHeap(max(nReserve, max(n, MIN_ALLOC)));
	/* start allocation at first free page */
	store.current.nPage = store.nLastPage;
	store.current.pPage = &invalidPage;	
      }
      }
      fLoop = TRUE;     /* avoid endless gc if large objects are allocated */
    }
  }
}

static tsp_OID tsp_new(tsp_ClassId classId, Word wSize)
/* create a new object */
{
  register Word wAllocSize = ALIGN_UP_OBJECT( wSize + sizeof(Header) );
  register Page * pPage = store.current.pPage;
  register Byte * pb = pPage->pbFree;
  if((pPage->pbFree = pb + wAllocSize) >= pPage->pbEnd) {
    Word nPages = (PAGE_START_GAP + wAllocSize + PAGE_SIZE - 1) / PAGE_SIZE;
    /* restore free pointer of current page */
    pPage->pbFree = pb;
    /* allocate space for new object */
    allocatePages(nPages);
    /* reload and update current page */
    pPage = store.current.pPage;
    pb = pPage->pb + PAGE_START_GAP;
    pPage->pbFree = pb + min(wAllocSize, PAGE_SIZE-PAGE_START_GAP);
  }
  assert(((Word)(pb+sizeof(Header)) & (Word)(ALIGN_OBJECT - 1)) == 0);
  /* initialize object */
  memset(pb, 0, wAllocSize);
  /* set object header */
  pb = PTR(pb);
  SET_SIZE(pb, wSize);
  SET_CLASSID(pb, classId);
  SET_HASH(pb, (Word)pb >> 4);    /* initialize identity hash */
  assert(SUPERCOMPONENT(pb) == NULL);
  return pb;
}


tsp_OID tsp_newArray(tsp_ClassId classId, Word nElements)
{
  tsp_OID pNew;
  assert((store.nDescriptors > classId) &&
	 (DESCRIPTOR(classId)->wLayout == tsp_Object_Array));
  tmthread_criticalLock();
  if(!fGc)
    tmthread_checkSyncRequest();
  pNew = tsp_new(classId, sizeof(tsp_OID) * nElements);
  tmthread_criticalUnlock();
  return pNew;
}

tsp_OID tsp_newByteArray(tsp_ClassId classId, Word nElements)
{
  tsp_OID pNew;
  assert((store.nDescriptors > classId) &&
	 (DESCRIPTOR(classId)->wLayout == tsp_Object_ByteArray));
  tmthread_criticalLock();
  if(!fGc)
    tmthread_checkSyncRequest();
  pNew = tsp_new(classId, sizeof(Byte) * nElements);
  tmthread_criticalUnlock();
  return pNew;
}

tsp_OID tsp_newShortArray(tsp_ClassId classId, Word nElements)
{
  tsp_OID pNew;
  assert((store.nDescriptors > classId) &&
	 (DESCRIPTOR(classId)->wLayout == tsp_Object_ShortArray));
  tmthread_criticalLock();
  if(!fGc)
    tmthread_checkSyncRequest();
  pNew = tsp_new(classId, sizeof(Short) * nElements);
  tmthread_criticalUnlock();
  return pNew;
}

tsp_OID tsp_newIntArray(tsp_ClassId classId, Word nElements)
{
  tsp_OID pNew;
  assert((store.nDescriptors > classId) &&
	 (DESCRIPTOR(classId)->wLayout == tsp_Object_IntArray));
  tmthread_criticalLock();
  if(!fGc)
    tmthread_checkSyncRequest();
  pNew = tsp_new(classId, sizeof(Int) * nElements);
  tmthread_criticalUnlock();
  return pNew;
}

tsp_OID tsp_newLongArray(tsp_ClassId classId, Word nElements)
{
  tsp_OID pNew;
  assert((store.nDescriptors > classId) &&
	 (DESCRIPTOR(classId)->wLayout == tsp_Object_LongArray));
  tmthread_criticalLock();
  if(!fGc)
    tmthread_checkSyncRequest();
  pNew = tsp_new(classId, sizeof(Long) * nElements);
  tmthread_criticalUnlock();
  return pNew;
}

tsp_OID tsp_newStack(tsp_ClassId classId, Word nElements)
{
  tsp_OID pNew;
  assert((store.nDescriptors > classId) &&
	 (DESCRIPTOR(classId)->wLayout == tsp_Object_Stack));
  tmthread_criticalLock();
  if(!fGc)
    tmthread_checkSyncRequest();
  pNew = tsp_new(classId, sizeof(tsp_OID) * nElements);
  tmthread_criticalUnlock();
  return pNew;
}

tsp_OID tsp_newThread(tsp_ClassId classId)
{
  tsp_OID pNew;
  assert((store.nDescriptors > classId) &&
	 (DESCRIPTOR(classId)->wLayout == tsp_Object_Thread));
  tmthread_criticalLock();
  if(!fGc)
    tmthread_checkSyncRequest();
  pNew = tsp_new(classId, sizeof(tyc_Thread));
  tmthread_criticalUnlock();
  return pNew;
}

tsp_OID tsp_newStruct(tsp_ClassId classId)
{
  tsp_OID pNew;
  assert((store.nDescriptors > classId) &&
	 (DESCRIPTOR(classId)->wLayout == tsp_Object_Struct));
  tmthread_criticalLock();
  if(!fGc)
    tmthread_checkSyncRequest();
  pNew = tsp_new(classId, DESCRIPTOR(classId)->wSize);
  tmthread_criticalUnlock();
  return pNew;
}

tsp_OID tsp_newWeakRef(tsp_ClassId classId)
{
  tsp_OID pNew;
  assert((store.nDescriptors > classId) &&
	 (DESCRIPTOR(classId)->wLayout == tsp_Object_WeakRef));
  tmthread_criticalLock();
  if(!fGc)
    tmthread_checkSyncRequest();
  pNew = tsp_new(classId, sizeof(tsp_WeakRef));
  tmthread_criticalUnlock();
  return pNew;
}

tsp_OID tsp_newEmptyCopy(tsp_OID p)
{
  Word wSize = SIZE(p), wClassId = CLASSID(p);
  tsp_OID pNew;
  assert(!tsp_IS_IMMEDIATE(p) && p != NULL);
  tmthread_criticalLock();
  if(!fGc)
    tmthread_checkSyncRequest();
  pNew = tsp_new(wClassId, wSize);
  SET_HASH(pNew, HASH(p));  /* for deep copy of hash tables */
  tmthread_criticalUnlock();
  return pNew;
}


void tsp_resizeStack(void * p, Word minElements)
{
  tyc_Thread * pThread = (tyc_Thread*)p;
  Word wOffset, wAdd, wSize = SIZE(pThread->pStack) / sizeof(tsp_OID);
  tyc_StackFrame * fp;
  tsp_OID * pNew, * pOld;
  /* allocate space for larger stack */
  wAdd = max(wSize, minElements * 100); 
  tmthread_pushStack(pThread);
  tmthread_criticalLock();
  if(!fGc)
    tmthread_checkSyncRequest();
  pNew = tsp_new(tyc_ClassId_TVMStack, sizeof(tsp_OID) * (wSize + wAdd));
  pThread = tmthread_popStack();
  pOld = pThread->pStack;
  /* copy stack content */
  memcpy(pNew + wAdd, pOld, wSize * sizeof(tsp_OID));
  SET_HASH(pNew, HASH(pOld));     /* copy identity hash */
  /* calculate offset between old stack and copy */
  wOffset = (Word)(pNew + wAdd) - (Word)pOld;
  /* adjust thread object */
  pThread->pStack = pNew;
  /* set new limit for check stack */
  if(pThread->pStackLimit != tmthread_ILL_STACKLIMIT) {
    pThread->pStackLimit = pNew + tmthread_MAX_STACKPEAK;
  }
  pThread->sp = (tsp_OID*)((Byte*)pThread->sp + wOffset);
  pThread->fp = fp = (tyc_StackFrame*)((Byte*)pThread->fp + wOffset);
  /* loop through stack and adjust frame pointer */
  while(fp) {
    if(fp->parent.fp == NULL) {
      break;
    }
    fp = fp->parent.fp = (tyc_StackFrame*)((Byte*)fp->parent.fp + wOffset);
  }
  /* adjust fixed reference to stack base */
  pNew[(SIZE(pNew) / sizeof(tsp_OID)) - 2] = pNew;
  tmthread_criticalUnlock();
  return;
}


#ifdef tsp_DEBUG    /* inline declaration in tsp.h if no debug option is set */

Word tsp_size(tsp_OID p)
{
  assert(p && !tsp_IS_IMMEDIATE(p));
  if (!IS_FORWARDED(p))
    return SIZE(p);
  return SIZE(PFORWARD(p));  
}

tsp_ClassId tsp_classId(tsp_OID p) 
{
  assert(p && !tsp_IS_IMMEDIATE(p));
  return CLASSID(p);
}

Word tsp_hash(tsp_OID p)
{
  assert(p && !tsp_IS_IMMEDIATE(p));
  return HASH(p);
}

void tsp_setHash(tsp_OID p, Word h)
{
  assert(p && !tsp_IS_IMMEDIATE(p));
  SET_HASH(p, h);
}

tsp_OID tsp_superComponent(tsp_OID p)
{
  assert(p && !tsp_IS_IMMEDIATE(p));
  return SUPERCOMPONENT(p);
}

void tsp_setSuperComponent(tsp_OID p, tsp_OID superComponent)
{
  assert(p && !tsp_IS_IMMEDIATE(p));
  SET_SUPERCOMPONENT(p, superComponent);
}

Bool tsp_immutable(tsp_OID p)
{
  assert(p && !tsp_IS_IMMEDIATE(p));
  return IS_IMMUTABLE(p);
}

void tsp_setImmutable(tsp_OID p)
{
  assert(p && !tsp_IS_IMMEDIATE(p));
  FLAG_IMMUTABLE(p);
}

Bool tsp_isTracedComponent(tsp_OID p)
{
  assert(p && !tsp_IS_IMMEDIATE(p));
  return IS_TRACED(p);
}

void tsp_setIsTracedComponent(tsp_OID p, Bool isTraced)
{
  assert(p && !tsp_IS_IMMEDIATE(p));
  if(isTraced)
    FLAG_TRACED(p);
  else
    CLEAR_TRACED(p);
}
#endif


tsp_CType tsp_getCType(tsp_OID p, Word i)
/* treat thread objects as C structs */
{
  Word wClassId = CLASSID(p);
  assert(DESCRIPTOR(wClassId)->wLayout == tsp_Object_Struct ||
	 DESCRIPTOR(wClassId)->wLayout == tsp_Object_Thread);
  assert(i < strlen(DESCRIPTOR(wClassId)->pzString));
  return DESCRIPTOR(wClassId)->pzString[i];
}

void * tsp_getCAddress(tsp_OID p, Word i)
/* treat thread objects as C structs */
{
  Word wClassId = CLASSID(p);
  assert(DESCRIPTOR(wClassId)->wLayout == tsp_Object_Struct ||
	 DESCRIPTOR(wClassId)->wLayout == tsp_Object_Thread);
  assert(i < strlen(DESCRIPTOR(wClassId)->pzString));
  return (((char*)p) + DESCRIPTOR(wClassId)->pzOffsets[i]);
}

Word tsp_getCSize(tsp_OID p)
/* return no of slots */
{
  Word wClassId = CLASSID(p);
  assert(DESCRIPTOR(wClassId)->wLayout == tsp_Object_Struct ||
	 DESCRIPTOR(wClassId)->wLayout == tsp_Object_Thread);
  return strlen(DESCRIPTOR(wClassId)->pzString);
}

tsp_OID tsp_getCSlot(tsp_OID p, Word i)
{
  void * pAddress;
  Word wClassId = CLASSID(p);
  assert(DESCRIPTOR(wClassId)->wLayout == tsp_Object_Struct ||
	 DESCRIPTOR(wClassId)->wLayout == tsp_Object_Thread);
  assert(i < strlen(DESCRIPTOR(wClassId)->pzString));
  pAddress = ((char*)p) + DESCRIPTOR(wClassId)->pzOffsets[i];
  /* get slot with default conversion */
  {
  switch(DESCRIPTOR(wClassId)->pzString[i])
    {
    case tsp_Field_Char:     return tyc_TAG_INT((*(char*)pAddress));
    case tsp_Field_UChar:    return tyc_TAG_INT((*(unsigned char*)pAddress));
    case tsp_Field_Short:    return tyc_TAG_INT((*(short*)pAddress));
    case tsp_Field_UShort:   return tyc_TAG_INT((*(unsigned short*)pAddress));
    case tsp_Field_Int:
    case tsp_Field_UInt:
    case tsp_Field_Long:
    case tsp_Field_ULong:  { int v = *(int*)pAddress;
                             return tyc_TAG_MAYBEBOXED(v); }
    case tsp_Field_LongLong: return tyc_boxLong((*(Long*)pAddress));
    case tsp_Field_Float:    return tyc_boxReal((*(float*)pAddress));
    case tsp_Field_Double:   return tyc_boxReal((*(double*)pAddress));
    case tsp_Field_OID:      return *(tsp_OID*)pAddress;
    default:
      tosLog_error("tsp",
                   "getCSlot",
                   "TSP error: illegal C-Type identifier");
      tosError_ABORT();
    }
  } 
  return NULL;
}

Bool tsp_setCSlot(tsp_OID p, tsp_OID v, Word i)
{
  void * pAddress;
  Word wClassId = CLASSID(p);
  Word wClassIdVal = tyc_CLASSID(v);
  tsp_Field field;
  assert(DESCRIPTOR(wClassId)->wLayout == tsp_Object_Struct ||
	 DESCRIPTOR(wClassId)->wLayout == tsp_Object_Thread);
  assert(i < strlen(DESCRIPTOR(wClassId)->pzString));
  pAddress = ((char*)p) + DESCRIPTOR(wClassId)->pzOffsets[i];
  /* try to set slot with ad-hoc conversion */
  field = DESCRIPTOR(wClassId)->pzString[i];
  switch (field) {
  case tsp_Field_OID:
    *(tsp_OID*)pAddress = v;
    break;
  case tsp_Field_Float:
    if(wClassIdVal != tyc_ClassId_Real) return FALSE;
    *(float *)pAddress = tyc_REAL_VALUE(v);
    break;
  case tsp_Field_Double:
    if(wClassIdVal != tyc_ClassId_Real) return FALSE;
    *(double *)pAddress = tyc_REAL_VALUE(v);
    break;
  default:
      {
      Long val;
      /* get scalar value */
      switch(wClassIdVal) {
      case tyc_ClassId_True:
      case tyc_ClassId_False:
	  val = tyc_IS_TRUE(v);
	  break;
      case tyc_ClassId_Int:
	  val = tyc_TAGGED_INT_VALUE(v);
	  break;
      case tyc_ClassId_Char:
	  val = tyc_CHAR_VALUE(v);
	  break;
      case tyc_ClassId_Long:
	  val = tyc_LONG_VALUE(v);
	  break;
      default:
	  return FALSE;
      }
      switch(field) {
      case tsp_Field_Char:
	  *(char *)pAddress = val;
	  break;
      case tsp_Field_UChar:
	  *(unsigned char *)pAddress = val;
	  break;
      case tsp_Field_Short:
	  *(short *)pAddress = val;
	  break;
      case tsp_Field_UShort:
	  *(unsigned short *)pAddress = val;
	  break;
      case tsp_Field_Int:
      case tsp_Field_UInt:
      case tsp_Field_Long:
      case tsp_Field_ULong:
	  *(int *)pAddress = val;
	  break;
      case tsp_Field_LongLong:
	  *(Long *)pAddress = val;
	  break;
      default:
          tosError_ABORT();
      }
      }
  }
  return TRUE;
}

Bool tsp_isCStruct(tsp_OID p)
{
  if(!tsp_IS_IMMEDIATE(p) && p != NULL) {
    Word wClassId = CLASSID(p);
    if(DESCRIPTOR(wClassId)->wLayout == tsp_Object_Struct ||
       DESCRIPTOR(wClassId)->wLayout == tsp_Object_Thread) {
      return TRUE;
    }
  }
  return FALSE;
}


static int calcOffsets(tsp_ClassId wClassId)
{
  Word iPosition = 0, nMaxBytes = 0;
  AlignTable * pAlign;
  tsp_CType bType, * pszType = DESCRIPTOR(wClassId)->pzString;
  tsp_Offset * pOffset = DESCRIPTOR(wClassId)->pzOffsets;
  assert(pOffset && pszType);
  while((bType = *pszType++)) {
    pAlign = lookupAlign(bType, pHostArch->pAlignTable);
    /* align to next boundary and save offset */ 
    iPosition = (iPosition + (pAlign->wAlign - 1)) & ~(pAlign->wAlign - 1);
    *pOffset++ = iPosition;
    /* advance to next position */ 
    iPosition += pAlign->nBytes;
    /* calculate size of structure (worst case) */
    pAlign = lookupAlign(bType, pMaxAlignment);
    nMaxBytes = (nMaxBytes + (pAlign->wAlign - 1)) & ~(pAlign->wAlign - 1);
    nMaxBytes += pAlign->nBytes;    
  }
  /* terminate with 0 */  
  *pOffset = 0;
  /* adjust size to object alignment */
  nMaxBytes = (nMaxBytes + (ALIGN_OBJECT-1)) & ~(Word)(ALIGN_OBJECT-1);
  return nMaxBytes;
}
  
tsp_ClassId tsp_newClassId(tsp_Object wLayout, tsp_CType * pszDescriptor)
{
  Word wClassId = store.nDescriptors++;
  assert(store.nDescriptors <= tsp_MAX_CLASSIDS);
  assert(wLayout >= 0 && wLayout < tsp_Object_nPredefined);
  /* set layout for new object */
  DESCRIPTOR(wClassId)->wLayout = wLayout;
  /* special handling for C structs */
  if(wLayout == tsp_Object_Struct) {
    assert(pszDescriptor);
    /* offset list is terminated by 0 so allocate one additional item */
    DESCRIPTOR(wClassId)->pzString = tosString_strdup(pszDescriptor);
    DESCRIPTOR(wClassId)->pzOffsets = malloc((strlen(pszDescriptor)+1)
					      * sizeof(tsp_Offset));
    DESCRIPTOR(wClassId)->wSize = calcOffsets(wClassId);
  }
  return wClassId;
}


Word tsp_referencesTo(tsp_OID p, tsp_OID * pArray)
{
  return 0;
}

Word tsp_instancesOf(tsp_ClassId f, tsp_OID * pArray)
{
  return 0;
}


tsp_WeakRef * tsp_initWeakRef(tsp_WeakRef * pWeakRef)
{
  tmthread_pushStack(pWeakRef);
  tmthread_criticalLock();
  tmthread_checkSyncRequest();
  pWeakRef = tmthread_popStack();
  /* insert weak reference into double linked list: */
  {
    tsp_WeakRef * pFirstRef = store.aWeakRefs;
    pWeakRef->pNext = pFirstRef;
    pWeakRef->pPrev = NULL;
    if(pFirstRef) {
      pFirstRef->pPrev = pWeakRef;
    }
    store.aWeakRefs = pWeakRef;
  }
  tmthread_criticalUnlock();
  return pWeakRef;
}

tsp_WeakRef * tsp_weakRefs(void)
{
  return store.aWeakRefs;
}

tsp_WeakRef * tsp_scheduleRefs(void)
{ tsp_WeakRef * scheduleList;
  tmthread_criticalLock();
  scheduleList = tyc_pRoot->pFinalizer->pWeaks;
  tyc_pRoot->pFinalizer->pWeaks = NULL;
  tmthread_criticalUnlock();
  return scheduleList;
}


void tsp_inhibitGc(void)
{
  tmthread_criticalLock();
  fGc++;
  tmthread_criticalUnlock();
}

void tsp_allowGc(void)
{
  tmthread_criticalLock();
  fGc--;
  tmthread_criticalUnlock();
}



/* Store IO */

/* storefile layout

   execheader (64 Bytes):       b0-b63: invoke executable on unix systems
   
   fileheader (16 Bytes):       b0-b15: hostname

   header (16 Words):               w0: magic number
                                    w1: store version
                                    w2: # first page saved
                                    w3: # last page saved
                                    w4: # used pages          nAllocatedPages
			            w5: # class descriptors   nDescriptors
				    w6: # saved pageblocks
		                    w7: * root object
				    w8: * weak references
				w9-w15: unused

   descriptors (1 Word + var):      w0: object layout
   (# class descriptors)                (w1-bZ only for C structs) 
				    w1: # elements in descriptor string and
				        offset table
                                    w2: calculated size of struct
		                 w3-wX: n (# elements) calculated offsets
		    	         bY-bZ: descriptor string of len n (# elements)
			               (0 appended up to next Word boundary)
				       
   pages (4 Words + n * 512 Bytes): w0: # page
   (# saved pageblocks)             w1: # pageblocks (1  = one object page
   (continued objects are saved                       >1 = obj + n*continued)
    in one chunk)	            w2: filled       (for 1st page)     
    		                    w3: _unused 
			       b0-b511: page     
*/


#define EXECSTRING_SIZE 64

static char * execString = "#!/bin/sh\nexec $HOX_HOME/bin/$HOX_HOST/tycoon2 $0 $@\n";

typedef struct {
  Word wMagic, wVersion, nFirstPage, nLastPage, nUsedPages;
  Word nDescriptors, nPageBlocks, pRoot, pWeakRefs;
  Word w0, w1, w2, w3, w4, w5, w6;
} StoreHeader;

static StoreHeader storeHeader;

typedef struct {
  Word nPage, nBlocks, nFilled;
  Word wUnused;
} PageHeader;


void swapShort(void * p)
/* swap 2 bytes from big to little endian and back */
{
  Short o = *(Short*)p;
  Byte * t = (Byte*)p, * f = (Byte*)&o;
  t[0] = f[1];
  t[1] = f[0];
  return;
}

void swapWord(void * p)
/* swap 4 bytes from big to little endian and back */
{
  Word o = *(Word*)p;
  Byte * t = (Byte*)p, * f = (Byte*)&o;
  t[0] = f[3];
  t[1] = f[2];
  t[2] = f[1];
  t[3] = f[0];
  return;
}

void swapDouble(void * p)
/* swap 8 bytes from big to little endian and back */
{
  Word * pp = (Word*)p;
  Word o1 = *pp, o2 = *(pp + 1);
  *pp = o2;
  *(pp + 1) = o1;
  swapWord(pp);
  swapWord(pp + 1);
  return;
}

/* Read/write sequence of bytes or words. Conversion from big to
   little endian is performed if necessary. */
static Bool readBytes(tosStdio_T * file, void * space, Word n)
{
  if(tosStdio_read(space, sizeof(Byte), n, file) != n) {
    return TRUE;
  }
  return FALSE;
}
static Bool readWords(tosStdio_T * file, void * space, Word n)
{
  if(tosStdio_read(space, sizeof(Word), n, file) != n) {
    return TRUE;
  }
  if(fSwap) {
    Word * p = space;
    for(; n > 0; n--) {
      swapWord(p++);
    }
  }
  return FALSE;
}
static Bool writeBytes(tosStdio_T * file, void * space, Word n)
{
  if(tosStdio_write(space, sizeof(Byte), n, file) != n) {
    return TRUE;
  }
  return FALSE;
}
static Bool writeWords(tosStdio_T * file, void * space, Word n)
{
  if(tosStdio_write(space, sizeof(Word), n, file) != n) {
    return TRUE;
  }
  return FALSE;
}


static void adjustDescriptors(void)
{
  Word wClassId, nMaxBytes;
  if(fRealign) {
    /* look for C struct descriptors and adjust offset table */ 
    for(wClassId = 0; wClassId < store.nDescriptors; wClassId++) {
      if(DESCRIPTOR(wClassId)->wLayout == tsp_Object_Struct) {
	nMaxBytes = calcOffsets(wClassId);
	assert(nMaxBytes == DESCRIPTOR(wClassId)->wSize);
      }
    }
  }
}

static Int restoreDescriptors(tosStdio_T * file)
{
  Bool fErr;
  Word n, nFields, wZero;
  Descriptor * pTable;
  /* reserve and clear memory for descriptor table */
  if((pTable = malloc(tsp_MAX_CLASSIDS * sizeof(Descriptor))) == NULL) {
    return tsp_ERROR_NOMEMORY;
  }
  memset(pTable, 0, tsp_MAX_CLASSIDS * sizeof(Descriptor));
  /* update store header */
  store.aDescriptors = pTable;
  n = store.nDescriptors = storeHeader.nDescriptors;
  /* read descriptors */
  for(; n > 0; n--, pTable++) {
    if(readWords(file, &pTable->wLayout, 1)) {
      return tsp_ERROR_READ;
    }
    /* check if descriptor is defining a C struct */
    if(pTable->wLayout == tsp_Object_Struct) {
      if(readWords(file, &nFields, 1)) {
	return tsp_ERROR_READ;
      }
      /* reserve memory for descriptor string and offset table */
      if((pTable->pzOffsets = malloc(nFields * sizeof(tsp_Offset))) == NULL) {
	return tsp_ERROR_NOMEMORY;
      }
      if((pTable->pzString = malloc(nFields * sizeof(tsp_CType))) == NULL) {
	return tsp_ERROR_NOMEMORY;
      }
      
      /* convince SUN debugger (rui) */
      memset(pTable->pzOffsets, 0, nFields * sizeof(tsp_Offset));
      memset(pTable->pzString, 0, nFields * sizeof(tsp_CType));
      /* convince SUN debugger (rui) */
      
      /* read size, descriptor string and offset table from file */ 
      fErr  = readWords(file, &pTable->wSize, 1);
      fErr |= readWords(file,  pTable->pzOffsets, nFields);
      fErr |= readBytes(file,  pTable->pzString, nFields);
      /* advance to next word boundary */
      nFields %= ALIGN_WORD;
      if(nFields != 0) {
	fErr |= readBytes(file, &wZero, ALIGN_WORD - nFields);
      }
      if(fErr) {
	return tsp_ERROR_READ;
      }
    }
  }
  /* adjust classId descriptors */
  adjustDescriptors();
  /* fake thread descriptor entry */
  {
  Int i;
  char *pzFormat = "ioiiiiiioooooooooooo";
  Int nSlots = strlen(pzFormat);
  DESCRIPTOR(tyc_ClassId_Thread)->pzString = pzFormat;
  DESCRIPTOR(tyc_ClassId_Thread)->pzOffsets =
    malloc(nSlots * sizeof(tsp_Offset));
  DESCRIPTOR(tyc_ClassId_Thread)->wSize = nSlots * sizeof(Word);
  for(i = 0; i < nSlots; i++)
    (DESCRIPTOR(tyc_ClassId_Thread)->pzOffsets)[i] = i * sizeof(Word);
  }
  return tsp_OK;
}


static void realignStruct(tsp_OID p)
{
  Word wClassId = CLASSID(p);
  tsp_CType * pszDescriptor = DESCRIPTOR(wClassId)->pzString;
  assert(pszDescriptor);
  /* remapp data if necessary */
  if(fRealign) {
    void * m = malloc(DESCRIPTOR(wClassId)->wSize);
    Word iRead = 0, iWrite = 0;
    tsp_CType bType, * pszType = pszDescriptor;
    assert(m);
    /* copy and clear struct */
    memcpy(m, p, DESCRIPTOR(wClassId)->wSize);
    memset(p, 0, DESCRIPTOR(wClassId)->wSize);
    while((bType = *pszType++)) {
      AlignTable * pFrom = lookupAlign(bType, pStoreArch->pAlignTable);
      AlignTable * pTo   = lookupAlign(bType, pHostArch->pAlignTable);
      Word nFrom = pFrom->nBytes, nTo = pTo->nBytes;
      /* align to next read/write position */
      iRead = (iRead + (pFrom->wAlign - 1)) & ~(pFrom->wAlign - 1);
      iWrite = (iWrite + (pTo->wAlign - 1)) & ~(pTo->wAlign - 1);
      if(nFrom == nTo) {
	memcpy((Byte*)p + iWrite, (Byte*)m + iRead, nTo);
      }
      else {
	if(nFrom > nTo) {
	  if(pStoreArch->wByteOrder == bigEndian) {
	    /* big endian: copy last n bytes */
	    memcpy((Byte*)p + iWrite, (Byte*)m + iRead + nFrom - nTo, nTo);
	  }
	  else {
	    /* little endian: copy first n bytes */
	    memcpy((Byte*)p + iWrite, (Byte*)m + iRead, nTo);
	  }
	}
	else {
	  if(pStoreArch->wByteOrder == bigEndian) {
	    /* big endian: copy to last n bytes */
	    memcpy((Byte*)p + iWrite + nTo - nFrom, (Byte*)m + iRead, nFrom);
	  }
	  else {
	    /* little endian: copy to first n bytes */
	    memcpy((Byte*)p + iWrite, (Byte*)m + iRead, nFrom);
	  }
	}
      }
      iRead += nFrom, iWrite += nTo;
    }
    free(m);
  }
  /* swap bytes is necessary */
  if(fSwap) {
    tsp_CType bType, * pszType = pszDescriptor;
    tsp_Offset * pOffset = DESCRIPTOR(wClassId)->pzOffsets;
    while((bType = *pszType++)) {
      AlignTable * pAlign = lookupAlign(bType, pHostArch->pAlignTable);
      switch(pAlign->nBytes) {
        case 1:
	  break;
        case 2:
	  swapShort((Byte*)p + *pOffset);
	  break;
        case 4:
	  swapWord((Byte*)p + *pOffset);
	  break;
        case 8:
	  swapDouble((Byte*)p + *pOffset);
	  break;
        default:
	  tosError_ABORT();
      }
      pOffset++;
    }
  }
}

static void adjustPtr(tsp_OID * pp, Int nOffset)
{
  if(*pp != NULL) {
    if(fSwap)
      swapWord(pp);
    *pp = (tsp_OID)((Word)*pp + nOffset);
  }
}

static void adjustOid(tsp_OID * pp, Int nOffset)
{
  if(*pp != NULL) {
    if(fSwap)
      swapWord(pp);
    if(!tsp_IS_IMMEDIATE(*pp) && *pp != NULL)
      *pp = (tsp_OID)((Word)*pp + nOffset);
  }
}

static void adjustArrayFields(tsp_OID * pArray, Word nSlots, Int nOffset)
{
  tsp_OID * pp = pArray;
  pp += nSlots;
  while (pp != pArray) {
    pp--;
    if(fSwap)
      swapWord(pp);
    if(!tsp_IS_IMMEDIATE(*pp) && *pp != NULL)
      *pp = (tsp_OID)((Word)*pp + nOffset);
  }
}

static void correctStack(tsp_OID p, Int nOffset)
{
  tsp_OID * sp;      
  tsp_OID * pStackEnd = (tsp_OID*)((Byte*)p + SIZE(p) - sizeof(tsp_OID));
  tyc_Thread * pThread;
  tyc_StackFrame * fp;
  /* adjust pointer to stackbase */
  pThread = *pStackEnd--;
  *pStackEnd = p;
  /* adjust thread object */
  adjustPtr((tsp_OID*)&pThread->fp, nOffset);
  adjustPtr((tsp_OID*)&pThread->sp, nOffset);
  adjustPtr((tsp_OID*)&pThread->ip, nOffset);
  fp = pThread->fp;
  sp = pThread->sp;
  /* loop through stack frames */
  while(fp) {
    /* adjust gap between sp and fp */
    while((tsp_OID)sp < (tsp_OID)fp) {
      adjustOid(sp, nOffset);
      sp++;
    }
    /* adjust frame */
    adjustOid((tsp_OID*)&fp->pReceiver, nOffset);
    adjustPtr((tsp_OID*)&fp->pCode, nOffset);
    adjustPtr((tsp_OID*)&fp->pByteCode, nOffset);
    if(fp->parent.fp == NULL) {
      /* last frame */
      sp = (tsp_OID*)(fp + 1);
      break;
    }
    adjustPtr((tsp_OID*)&fp->parent.fp, nOffset);
    adjustPtr((tsp_OID*)&fp->parent.ip, nOffset);
    /* skip to next frame */
    sp = (tsp_OID *)(fp + 1);
    fp = fp->parent.fp;
  }      
  /* adjust gap between sp and fixed elements */
  while((tsp_OID)sp < (tsp_OID)pStackEnd) {
    adjustOid(sp, nOffset);
    sp++;
  }
}

static void adjustPtrFields(tsp_OID p, Int nOffset)
{
  Word wClassId = CLASSID(p);
  switch(DESCRIPTOR(wClassId)->wLayout)
    {
    case tsp_Object_Struct:
      {
      register tsp_Offset * pOffset = DESCRIPTOR(wClassId)->pzOffsets;
      register tsp_CType * pszType = DESCRIPTOR(wClassId)->pzString;
      /* remap data if necessary */
      if(fSwap || fRealign) {
	realignStruct(p);
      }
      for(;;)
	switch(*pszType++)
	  {
	  case 0:
	    return;
	  case tsp_Field_OID:
	    {
	    tsp_OID * pp = (tsp_OID*)((Byte*)p + (*pOffset++));
	    if(!tsp_IS_IMMEDIATE(*pp) && *pp != NULL)
	      *pp = (tsp_OID)((Word)*pp + nOffset);
	    }
	    break;
	  default:
	    pOffset++;
	    break;
	  }
      }
    case tsp_Object_ShortArray:
      if(fSwap) {
	Word n = SIZE(p) / sizeof(Short);
	Short * pShort = (Short*)p;
	while(n-- > 0)
	  swapShort(pShort++);
      }
      break;
    case tsp_Object_IntArray:
      if(fSwap) {
	Word n = SIZE(p) / sizeof(Int);
	Int * pInt = (Int*)p;
	while(n-- > 0)
	  swapWord(pInt++);
      }
      break;
    case tsp_Object_LongArray:
      if(fSwap) {
	Word n = SIZE(p) / sizeof(Long);
	Long * pLong = (Long*)p;
	while(n-- > 0)
	  swapDouble(pLong++);
      }
      break;
    case tsp_Object_Thread:
      {
      tyc_Thread * pThread = (tyc_Thread*)p;
      /* adjust pointer to stackbase */
      adjustPtr((tsp_OID*)&(pThread->pStack), nOffset);
      /* set new limit for check stack. clear sync state */
      pThread->pStackLimit = pThread->pStack + tmthread_MAX_STACKPEAK;
      /* swap flags if byteorder changed */
      if(fSwap) {
	swapWord(&pThread->wFlags);
	swapWord(&pThread->wLocalFlags);
      }
      /* visit OIDs values */
      adjustArrayFields((tsp_OID*)&(pThread->pNext), 12, nOffset);
      if(p > (tsp_OID)pThread->pStack && pThread->pStack != NULL) {
	/* stack has been read already. ignore uninitialized thread objects */
	correctStack(pThread->pStack, nOffset);
      }
      }
      break;
    case tsp_Object_Stack:
      {
      tsp_OID * pStackEnd = (tsp_OID*)((Byte*)p + SIZE(p) - sizeof(tsp_OID));
      assert(*pStackEnd);
      /* adjust pointer to thread */
      adjustPtr((tsp_OID*)pStackEnd, nOffset);
      if(p > (tsp_OID)*pStackEnd) {
	/* thread has been read already */
	correctStack(p, nOffset);
      }
      }
      break;
    case tsp_Object_WeakRef:
    case tsp_Object_Array:
      adjustArrayFields(p, SIZE(p) / sizeof(Word), nOffset);
      break;
    default:
      /* tsp_Object_ByteArray */
      break;
    }
  return;
}


static void adjustPage(Word nPage, Int nOffset)
{  
  /* Sweep across page and correct pointers, swap bytes and
     realign C structs if needed */
  Byte * p = NPAGE_TO_PTR(nPage);
  Byte * pp = PPAGE(nPage)->pbFree;
  /* point to first object (aligned behind header): */
  p += ALIGN_UP_OBJECT(sizeof(Header));
  /* adjust all objects within page */
  while(p <= pp) {
    /* adjust header incl. objects with size 0 at end of page (<=) */
    Header *h = PHEADER(p);
    adjustOid(&h->superComponent, nOffset);
    if(fSwap) {
      swapWord(&h->u.wSize);
      swapWord(&h->bitfield);
    }
    /* adjust object fields */
    adjustPtrFields(p, nOffset);    
    /* round up to next alignment boundary after next header */
    p += ALIGN_UP_OBJECT(SIZE(p)+sizeof(Header));
  }
}

static Int restorePages(tosStdio_T * file)
{
  Word nPages, nSavedRange, nBytes, nPage,
       nLoadedPages = 0, n = storeHeader.nPageBlocks;
  Int iPageOffset, iAddressOffset;
  Byte * pPageBase;
  Page * pPageTable;

  /* calculate amount of memory to be reserved */
  nPages = (storeHeader.nUsedPages + MEMRESERVE) * 2;
  nSavedRange = storeHeader.nLastPage - storeHeader.nFirstPage + 1;
  /* print warning if saved range exceeds needed amount */
  if(nPages < nSavedRange) {
    fprintf(stderr,"TSP warning: reserved space exceeds needs by %d kb\n",
	    ((nSavedRange - nPages) * PAGE_SIZE) / 1024);
  }
  nPages = max(nPages, nSavedRange);
  nBytes = (nPages + 1) * PAGE_SIZE - 1;
  /* reserve memory for pages and pagetable */
  pPageBase = moreCore(nBytes);
  pPageTable = malloc(nPages * sizeof(Page));
  if(!pPageBase || !pPageTable) {
    return tsp_ERROR_NOMEMORY;
  }
  memset(pPageTable, 0, nPages * sizeof(Page));
  /* align baseaddress to page boundary */
  pPageBase += PAGE_SIZE-1;
  pPageBase = (Byte*) ((Word)pPageBase & ~(Word)(PAGE_SIZE-1));
  /* setup store data area */
  store.nPages = store.nRealPages = nPages;
  store.nFirstPage = PTR_TO_NPAGE(pPageBase);
  store.nLastPage = store.nFirstPage + nPages - 1;
  store.nAllocatedPages = storeHeader.nUsedPages;
  store.aPages = pPageTable;
  store.age = store.newAge = 1;
  /* setup virtual memory address */
  for(nPage = store.nFirstPage; nPage <= store.nLastPage; nPage++) {
    PPAGE(nPage)->pb = NPAGE_TO_PTR(nPage);
    PPAGE(nPage)->pbEnd = NPAGE_TO_PTR(nPage) + PAGE_SIZE;
  }
  /* load pages */
  iPageOffset = store.nFirstPage - storeHeader.nFirstPage;
  iAddressOffset = iPageOffset * PAGE_SIZE;
  for(; n > 0; n--) {
    Word nBlocks, nAdjust;
    PageHeader pageHeader = {0,0,0,0}; /* convince SUN debugger (rui) */
    /* read pageheader and n pages */
    if(readWords(file, &pageHeader, PAGEHEADER_SIZE)) {
      return tsp_ERROR_READ;
    }
    nAdjust = nPage = pageHeader.nPage + iPageOffset;
    nBlocks = pageHeader.nBlocks;
    if(readBytes(file, NPAGE_TO_PTR(nPage), nBlocks * PAGE_SIZE)) {
      return tsp_ERROR_READ;
    }
    nLoadedPages += nBlocks;
    /* update pagetable */
    {
    Page * pPage = PPAGE(nPage);
    pPage->type = PageType_Object;
    pPage->age = store.age;
    pPage->pbFree = pPage->pb + pageHeader.nFilled;
    while(--nBlocks) {
      pPage++;
      pPage->type = PageType_Continued;
      pPage->age = store.age;
    }
    }
    /* adjust pointer and data structures */
    adjustPage(nAdjust, iAddressOffset);    
  }
  assert(storeHeader.nUsedPages == nLoadedPages);
  /* restore root object */
  if(storeHeader.pRoot) {
    store.pRoot = (tsp_OID)(storeHeader.pRoot + iAddressOffset);
  }
  /* restore weak reference pointer */
  if(storeHeader.pWeakRefs) {
    store.aWeakRefs = (tsp_WeakRef*)(storeHeader.pWeakRefs + iAddressOffset);
  }
  /* no current page available */
  store.current.nPage = store.nLastPage;
  store.current.pPage = &invalidPage;
  
  return tsp_OK;
}

static Int loadStore(tosStdio_T * file)
{
  char storeFormat[tosSystem_NAME_SIZE];
  Word success = 0;
  assert(sizeof(Word) == ALIGN_WORD);
  /* skip exec header */
  tosStdio_seek(file, EXECSTRING_SIZE, tosStdio_SEEK_SET);
  /* check store format */
  if(readBytes(file, &storeFormat, tosSystem_NAME_SIZE)) {
    return tsp_ERROR_READ;
  }
  pStoreArch = lookupArch(storeFormat);
  if(pStoreArch == NULL) {
    return tsp_ERROR_ARCHITECTURE;
  }
  /* swap words and realign C structs if necessary */
  fSwap = fRealign = FALSE;
  if(pStoreArch->wByteOrder != pHostArch->wByteOrder) {
    fSwap = TRUE;
  }
  if(pStoreArch->pAlignTable != pHostArch->pAlignTable) {
    fRealign = TRUE;
  }
  /* read header from file */
  if(readWords(file, &storeHeader, STOREHEADER_SIZE)) {
    return tsp_ERROR_READ;
  }
  /* check for correct version */
  if(storeHeader.wMagic != MAGIC && storeHeader.wVersion != SVERSION) {
    return tsp_ERROR_VERSION;
  }
  /* restore descriptortable */
  if((success = restoreDescriptors(file)) != tsp_OK) {
    return success;
  }
  /* restore pagetable and load pages */
  if((success = restorePages(file)) != tsp_OK) {
    return success;
  }
  return tsp_OK;
}

Int tsp_open(String szName)
{
  Word success = 0;
  char realName[tosFilename_MAXLEN];

  /* get real filename */
  if(tosDirectory_realpath(szName, realName, tosFilename_MAXLEN + 1)) {
    return tsp_ERROR_IO;
  }

  /* open store file */
  if((pFile = tosStdio_open(realName, tosStdio_READ_MODE)) == NULL) {
    return tsp_ERROR_OPEN;
  }
  /* reassign a larger IO buffer */
  if((pbIOBuffer = malloc(FILEBUFFER_SIZE)) == NULL) {
    return tsp_ERROR_NOMEMORY;
  }
  if(setvbuf(pFile, pbIOBuffer, _IOFBF, FILEBUFFER_SIZE) != 0) {
    return tsp_ERROR_BUFFERALLOC;
  }
  tsp_init();
  store.szName = tosString_strdup(realName);
  /* load store */
  if((success = loadStore(pFile)) != tsp_OK) {
    tosStdio_close(pFile);
    free(pbIOBuffer);
    pFile = NULL, pbIOBuffer = NULL;
  }
  return success;
}


Int tsp_create(String szName)
{
  Word nPages, nBytes, nPage;
  Byte * pPageBase;
  Descriptor * pTable;
  Page * pPageTable;
  assert(pFile == NULL && pbIOBuffer == NULL);
  tsp_init();
  /* initialize descriptor table */
  if((pTable = malloc(tsp_MAX_CLASSIDS * sizeof(Descriptor))) == NULL) {
    return tsp_ERROR_NOMEMORY;
  }
  memset(pTable, 0, tsp_MAX_CLASSIDS * sizeof(Descriptor));
  /* reserve initial amount of memory */
  nPages = INITIAL_PAGES;
  nBytes = (nPages + 1) * PAGE_SIZE - 1;
  pPageBase = moreCore(nBytes);
  pPageTable = malloc(nPages * sizeof(Page));
  if(!pPageBase || !pPageTable) {
    return tsp_ERROR_NOMEMORY;
  }
  memset(pPageTable, 0, nPages * sizeof(Page));
  /* align baseaddress to page boundary */
  pPageBase += PAGE_SIZE-1;
  pPageBase = (Byte*) ((Word)pPageBase & ~(Word)(PAGE_SIZE-1));
  /* setup store data area */
  store.szName = tosString_strdup(szName);
  store.aDescriptors = pTable;
  store.nDescriptors = 0;
  store.nPages = store.nRealPages = nPages;
  store.nFirstPage = PTR_TO_NPAGE(pPageBase);
  store.nLastPage = store.nFirstPage + nPages - 1;
  store.nAllocatedPages = 0;
  store.aPages = pPageTable;
  store.age = store.newAge = 1;
  /* setup virtual memory address */
  for(nPage = store.nFirstPage; nPage <= store.nLastPage; nPage++) {
    PPAGE(nPage)->pb = NPAGE_TO_PTR(nPage);
    PPAGE(nPage)->pbEnd = NPAGE_TO_PTR(nPage) + PAGE_SIZE;
    }
  /* no current page available */
  store.current.nPage = store.nLastPage;
  store.current.pPage = &invalidPage;
  
  return tsp_OK;
}


void tsp_close(void)
{
  if(pFile) {
    tosStdio_flush(pFile);
    tosStdio_close(pFile);
  }
  if(pbIOBuffer) {
    free(pbIOBuffer);
  }
  pFile = NULL, pbIOBuffer = NULL, store.szName = NULL;
}


static Int dumpClassIds(tosStdio_T * file)
{
  Word wClassId, wSize, wZero = 0;
  Bool fErr;
  Descriptor * pDescriptor = store.aDescriptors;
  for(wClassId = 0; wClassId < store.nDescriptors; wClassId++, pDescriptor++) {
    /* write layout descriptor */
    fErr  = writeWords(file, &pDescriptor->wLayout, 1);
    /* check for C struct */
    if(pDescriptor->wLayout == tsp_Object_Struct) {
      /* save string & offsets including terminating zero */
      wSize = strlen(pDescriptor->pzString) + 1;
      fErr |= writeWords(file, &wSize, 1);
      fErr |= writeWords(file, &pDescriptor->wSize, 1);
      fErr |= writeWords(file, pDescriptor->pzOffsets, wSize);
      fErr |= writeBytes(file, pDescriptor->pzString, wSize);
      /* fill up to next word boundary */
      wSize %= ALIGN_WORD;
      if(wSize != 0) {
	fErr |= writeBytes(file, &wZero, ALIGN_WORD - wSize);
      }
    }
    if(fErr) {
      return tsp_ERROR_WRITE;
    }
  }
  storeHeader.nDescriptors = store.nDescriptors;
  return tsp_OK;
}

static Int dumpPages(tosStdio_T * file)
{
  Word nPage, nBlocks = 0, nSavedPages = 0, iFirst = 0, iLast = 0;
  Bool fErr, fFirst = TRUE;
  PageHeader pageHeader;
  /* scan through pages; only save allocated ones */
  nPage = store.nFirstPage;
  while(nPage <= store.nLastPage) {
    if(PPAGE(nPage)->age == store.age &&
       PPAGE(nPage)->type == PageType_Object)
      {
	memset(&pageHeader, 0, sizeof(PageHeader));
	/* remember # of first page saved */
	if(fFirst) {
	  iFirst = nPage, fFirst = FALSE;
	}
	pageHeader.nFilled = PPAGE(nPage)->pbFree - PPAGE(nPage)->pb;
	pageHeader.nBlocks = 1;
	pageHeader.nPage = nPage++;
	/* save continued pages in one chunk */
	while(PPAGE(nPage)->age == store.age &&
	      PPAGE(nPage)->type == PageType_Continued)
	  {
	    pageHeader.nBlocks++;
	    nPage++;
	  }
	fErr  = writeWords(file, &pageHeader, PAGEHEADER_SIZE);
	fErr |= writeBytes(file, NPAGE_TO_PTR(pageHeader.nPage),
			         pageHeader.nBlocks * PAGE_SIZE);
	if(fErr) {
	  return tsp_ERROR_WRITE;
	}
	/* remember # of last page saved */
	iLast = nPage - 1;
	nSavedPages += pageHeader.nBlocks;
	nBlocks++;
      }
    else {
      nPage++;
    }
  }
  assert(nSavedPages == store.nAllocatedPages);
  assert((iFirst <= iLast) && (nBlocks <= nSavedPages));
  storeHeader.nUsedPages = nSavedPages;
  storeHeader.nFirstPage = iFirst;
  storeHeader.nLastPage = iLast;
  storeHeader.nPageBlocks = nBlocks;
  return tsp_OK;
}

static Int saveStore(tosStdio_T * file)
{
  char thisOS[tosSystem_NAME_SIZE];
  char execHeader[EXECSTRING_SIZE];
  Word success = 0;
  /* write exec header */
  strncpy(execHeader, execString, EXECSTRING_SIZE);
  if(writeBytes(file, execHeader, EXECSTRING_SIZE)) {
    return tsp_ERROR_WRITE;
  }  
  /* write OS identifier */
  strncpy(thisOS, tosSystem_getName(), tosSystem_NAME_SIZE);
  if(writeBytes(file, thisOS, tosSystem_NAME_SIZE)) {
    return tsp_ERROR_WRITE;
  }
  /* write empty header */
  memset(&storeHeader, 0, sizeof(StoreHeader));
  if(writeWords(file, &storeHeader, STOREHEADER_SIZE)) {
    return tsp_ERROR_WRITE;
  }
  /* dump classIds */
  if((success = dumpClassIds(file)) != tsp_OK) {
    return success;
  }
  /* dump pages */
  if((success = dumpPages(file)) != tsp_OK) {
    return success;
  }
  /* write updated header */
  tosStdio_seek(file, tosSystem_NAME_SIZE + EXECSTRING_SIZE, tosStdio_SEEK_SET);
  storeHeader.pRoot = (Word)store.pRoot;
  storeHeader.pWeakRefs = (Word)store.aWeakRefs;
  storeHeader.wMagic = MAGIC;
  storeHeader.wVersion = SVERSION;
  if(writeWords(file, &storeHeader, STOREHEADER_SIZE)) {
    return tsp_ERROR_WRITE;
  }
  tosStdio_flush(pFile);
  return tsp_OK;
}

Int tsp_commit(void)
{
  Word success = 0;
  char * fname;
  tosFilemode_T fmode;

  /* close current store image */
  if(pFile) {
    tosStdio_close(pFile);
  }
  if(pbIOBuffer) {
    free(pbIOBuffer);
  }
  pFile = NULL, pbIOBuffer = NULL;
  /* get image permissions */
  if(tosFile_exists(store.szName)) {
    if(tosFilemode_get(store.szName, &fmode) != 0)
       return tsp_ERROR_OPEN;
  }
  else
    /* if no image present (bootstrap) set default mode */
    fmode = tosFilemode_DEFAULT;

  /* create temorary filename */
  if((fname = malloc(strlen(store.szName) + 8)) == NULL) {
    return tsp_ERROR_NOMEMORY;
  }
  strcpy(fname, store.szName);
  strcat(fname, ".commit");
  /* open new file */
  if((pFile = tosStdio_open(fname, tosStdio_CREATE_MODE)) == NULL) {
    return tsp_ERROR_OPEN;
  }
  if((pbIOBuffer = malloc(FILEBUFFER_SIZE)) == NULL) {
    return tsp_ERROR_NOMEMORY;
  }
  if(setvbuf(pFile, pbIOBuffer, _IOFBF, FILEBUFFER_SIZE) != 0) {
    return tsp_ERROR_BUFFERALLOC;
  }
  /* truncate if file already exists */
  if(tosStdio_truncate(pFile, 0) != 0) {
    return tsp_ERROR_IO;
  }
  /* save store */
  if((success = saveStore(pFile)) != tsp_OK) {
    tosStdio_close(pFile);
    free(pbIOBuffer);
    pFile = NULL, pbIOBuffer = NULL;
  }
  else {
    tosStdio_flush(pFile);
    tosStdio_close(pFile);

    /* rename saved image */
    if(tosFile_rename(fname, store.szName) != 0) {
      return tsp_ERROR_IO;
    }
    /* set image permissions */
    if(tosFilemode_set(store.szName, fmode) != 0) {
      return tsp_ERROR_IO;
    }

    /* Re-open new file (in respect to other operating systems) */
    if((pFile = tosStdio_open(store.szName, tosStdio_READ_MODE)) == NULL) {
      return tsp_ERROR_OPEN;
    }
    if(setvbuf(pFile, pbIOBuffer, _IOFBF, FILEBUFFER_SIZE) != 0) {
      return tsp_ERROR_BUFFERALLOC;
    }
  }
  free(fname);
  return success;
}


Int tsp_rollback(void)
{
  Word i;
  char * name = store.szName;
  MemBlock * pBlock = store.pBlock, * pOld;
  /* close file handle and free iobuffer */
  tsp_close();
  /* free descriptor table */
  for(i = 0; i < store.nDescriptors; i++) {
    if(DESCRIPTOR(i)->wLayout == tsp_Object_Struct) {
      free(DESCRIPTOR(i)->pzString);
      free(DESCRIPTOR(i)->pzOffsets);
    }
  }
  free(store.aDescriptors);
  /* free page table */
  free(store.aPages);
  /* free memory blocks */
  while(pBlock) {
    free(pBlock->p);
    pOld = pBlock;
    pBlock = pBlock->pNext;
    free(pOld);
  }
  /* reopen store */
  return tsp_open(name); 
}


/* get error message from error code */

static char * aErrorStr[] = {
  "OK",
  "Cannot open file",
  "Error reading from file",
  "Error writing to file",
  "IO error",
  "Not enough memory",
  "Cannot set IO buffer",
  "Unknown store version",
  "Unsupported architecture",
  NULL
};

char * tsp_errorCode(tsp_ErrorCode wErrorCode)
{
  assert(wErrorCode > tsp_OK && wErrorCode <= tsp_ERROR_ARCHITECTURE);
  return aErrorStr[wErrorCode];
}


/* print a store object */

static Int recDepth = 2, objWidth = 5;

static void printRec(tsp_OID p, Int depth)
{
  tsp_ClassId wClassId = CLASSID(p);

  printf("%s(0x%x){", tyc_CLASS(wClassId)->pszName, (Word)p);

  if(depth == 0) {
    printf("...");
    goto recEnd;
  }

  switch(DESCRIPTOR(wClassId)->wLayout)
    {
    case tsp_Object_Struct:
      {
      register tsp_Offset * pOffset = DESCRIPTOR(wClassId)->pzOffsets;
      register tsp_CType * pszType = DESCRIPTOR(wClassId)->pzString;
      for(;;) {
	tsp_OID * pp = (tsp_OID*)((Byte*)p + (*pOffset++));
	switch(*pszType++)
	  {
	  case 0:
	    printf("}");
	    fflush(stdout);
	    return;
	  case tsp_Field_Char:
	    printf("%c ", *(signed char*)pp);
	    break;
	  case tsp_Field_UChar:
	    printf("%c ", *(unsigned char*)pp);
	    break;
	  case tsp_Field_Short:
	    printf("%d ", *(signed short*)pp);
	    break;
	  case tsp_Field_UShort:
	    printf("%du ", *(unsigned short*)pp);
	    break;
	  case tsp_Field_Int:
	    printf("%d ", *(signed int*)pp);
	    break;
	  case tsp_Field_UInt:
	    printf("%du ", *(unsigned int*)pp);
	    break;
	  case tsp_Field_Long:
	    printf("%ld ", *(signed long*)pp);
	    break;
	  case tsp_Field_ULong: 
	    printf("%lu ", *(unsigned long*)pp);
	    break;
	  case tsp_Field_LongLong:
	    printf("%ld|%ld ", *(unsigned long*)pp, *(unsigned long*)(pp + 1));
	    break;
	  case tsp_Field_Float:
	    printf("%f ", *(float*)pp);
	    break;
	  case tsp_Field_Double:
	    printf("%f ", *(double*)pp);
	    break;
	  case tsp_Field_OID:
	    if(tsp_IS_IMMEDIATE(*pp)) {
	      printf("%d ", tyc_UNTAG_INT(*pp));
	    }
	    else if(tyc_IS_NIL(*pp)) {
	      printf("nil ");
	    }
	    else {
	      printRec(*pp, depth - 1);
	    }
	    break;
	  default:
            tosError_ABORT();
	    break;
	  }
      }
      }
    case tsp_Object_Array:
      {
      Int i;
      tsp_OID * pp = p;
      for(i = 0; i < (SIZE(p) / sizeof(tsp_OID)); i++, pp++) {
	if(i == objWidth) {
	  printf("...");
	  goto recEnd;
	}
	if(tsp_IS_IMMEDIATE(*pp)) {
	  printf("%d ", tyc_UNTAG_INT(*pp));
	}
	else if(tyc_IS_NIL(*pp)) {
	  printf("nil ");
	}
	else {
	  printRec(*pp, depth - 1);
	}
      }
      }
      break;
    case tsp_Object_ByteArray:
      {
      Int i;
      Byte * pp = p;
      if(wClassId == tyc_ClassId_String || wClassId == tyc_ClassId_Symbol ||
	 wClassId == tyc_ClassId_MutableString) {
	printf("\"%s\" ", pp);
      }
      else {
	for(i = 0; i < (SIZE(p) / sizeof(Byte)); i++, pp++) {
	  if(i == objWidth) {
	    printf("...");
	    goto recEnd;
	  }
	  printf("%d ", *pp);
	}
      }
      }
      break;
    case tsp_Object_ShortArray:
      {
      Int i;
      Short * pp = p;
      for(i = 0; i < (SIZE(p) / sizeof(Short)); i++, pp++) {
	if(i == objWidth) {
	  printf("...");
	  goto recEnd;
	}
	printf("%d ", *pp);
      }
      }
      break;
    case tsp_Object_Stack:
    case tsp_Object_Thread:
      break;
    default:
      printf("unknown object type");
      break;
    }
recEnd:

  printf("} ");
  fflush(stdout);
  return;
}

void tsp_printObject(tsp_OID p)
{
  printf("\n");
  printRec(p, recDepth);
  printf("\n");
}

void tsp_setPrintDepth(Int depth, Int width)
{
  recDepth = depth, objWidth = width;
}


/* debugging support */

void tsp_scanObjects(void (*pFun)(tsp_OID))
/* scan store for objects and apply generic function */
{
  Word n;
  /* scan all store objects */
  for(n = store.nFirstPage; n <= store.nLastPage; n++) {
    if(PPAGE(n)->type == PageType_Object && PPAGE(n)->age == store.newAge) {
      Byte * p = NPAGE_TO_PTR(n);
      Byte * pp = PPAGE(n)->pbFree;
      /* point to first object (behind header): */
      p += ALIGN_UP_OBJECT(sizeof(Header));
      /* scan all objects within page */
      while(p < pp) {
	/* don't test objects with size 0 at end of page (<) */
	pFun(p);
	/* round up to next alignment boundary after next header */
	p += ALIGN_UP_OBJECT(SIZE(p) + sizeof(Header));
      }
    }
  }
}


/* extended debugging support */

#ifdef tsp_DEBUG

#define GROUP_NPAGES 100

void showStoreStruct(void)
{
  printf("\n\nStore:   %s\n", store.szName);
  printf("---------------------\n");
  printf("nPages     : %d\n", store.nPages);
  printf("First Page : %d\n", store.nFirstPage);
  printf("Last Page  : %d\n", store.nLastPage);
  printf("Allocated  : %d\n", store.nAllocatedPages);
  printf("Current Age: %d\n", store.age);
  printf("StoreBase  : 0x%x\n",  (Word)NPAGE_TO_PTR(store.nFirstPage));
  printf("Descriptors: %d\n", store.nDescriptors);
  printf("\n");
  printf("Page in Use: %d\n", store.current.nPage);
  printf("Base:        0x%x\n",  (Word)NPAGE_TO_PTR(store.current.nPage));
  printf("\n");
}

void showWeakRef(tsp_WeakRef * p)
{
  printf("Object:: ClassId: %d  Size: %d\n",
	 CLASSID(p), SIZE(p));
}

void showObjects(Word nPage)
{
  Byte * p = NPAGE_TO_PTR(nPage) + ALIGN_UP_OBJECT(sizeof(Header));
  Byte * pp = PPAGE(nPage)->pbFree;
  while(p <= pp) {
    /* show objects with size 0 at end of page (<=) */
    printf("Object:: ClassId: %d  Size: %d  At: %d 0x%x\n",
	   CLASSID(p), SIZE(p), (Word)p - (Word)NPAGE_TO_PTR(nPage), (Word)p);
    p += ALIGN_UP_OBJECT(SIZE(p) + sizeof(Header));
  }
}

void showPages(void)
{
  Word nPage = store.nFirstPage;
  printf("\nStarting Pagedump");
  printf("\n-----------------\n");
  for(; nPage <= store.nLastPage; nPage++) {
    printf("\n");
    if(PPAGE(nPage)->type == PageType_Foreign) {
      printf("%d: 0x%x  Foreign\n", nPage, (Word)NPAGE_TO_PTR(nPage));
      continue;
    }
    if(PPAGE(nPage)->age != store.age) {
      printf("%d: 0x%x  Free\n", nPage, (Word)NPAGE_TO_PTR(nPage));
      continue;
    }
    if(PPAGE(nPage)->type == PageType_Continued) {
      printf("%d: 0x%x  Continued\n", nPage, (Word)NPAGE_TO_PTR(nPage));
      continue;
    }
    printf("%d: 0x%x  Page Content:\n", nPage, (Word)NPAGE_TO_PTR(nPage));
    printf("--------------------\n");
    showObjects(nPage);
  }
}

void showLayout(Word nPage)
{
  Int n = min(GROUP_NPAGES, store.nLastPage - nPage);
  Int nAlloc = 0, nCont = 0, nFree = 0, nForeign = 0;  
  for(;n > 0; n--, nPage++) {
    if(PPAGE(nPage)->type == PageType_Foreign) {
      nForeign++; continue;
    }
    if(PPAGE(nPage)->age != store.age) {
      nFree++; continue;
    }
    if(PPAGE(nPage)->type == PageType_Continued) {
      nCont++; continue;
    }
    nAlloc++;
  }
  if(nForeign > 50) {
    putchar('X');
  }
  else {
    nAlloc = max(0, (nAlloc + nCont) - 1) / (GROUP_NPAGES/10);
    putchar('0' + nAlloc);
  }
  return;
}

void storeLayout()
{
  Word n = store.nFirstPage;
  printf("\n---------------------------\n");
  for(; n <= store.nLastPage; n += GROUP_NPAGES) {
    showLayout(n);
  }
  printf("\n---------------------------\n");
}

void tsp_storeStatus()
{
  Word n, nFree = 0, nForeign = 0, nAlloc = 0, nCont = 0;
  printf("\nStore Status:\n");
  for(n = store.nFirstPage; n <= store.nLastPage; n++) {
    if(PPAGE(n)->type == PageType_Foreign) {
      nForeign++; continue;
    }
    if(PPAGE(n)->age != store.age) {
      nFree++; continue;
    }
    if(PPAGE(n)->type == PageType_Continued) {
      nCont++; continue;
    }
    nAlloc++;
  }
  printf("Total: %d  Real: %d  First: %d  Last: %d\n",
	 store.nPages, store.nRealPages, store.nFirstPage, store.nLastPage);
  printf("Allocated: %d  Continued: %d  Free: %d  Foreign: %d\n",
	 nAlloc, nCont, nFree, nForeign); 
  assert((nForeign + store.nRealPages) == store.nPages);
  assert((nCont + nAlloc) == store.nAllocatedPages);
  assert((nFree + nAlloc + nCont) == store.nRealPages);
  storeLayout();
}


/* object integrity test */

static Bool fHunt = FALSE;

static void integrityRec(tsp_OID p, Int depth)
{
  tsp_ClassId wClassId = CLASSID(p);

  switch(DESCRIPTOR(wClassId)->wLayout)
    {
    case tsp_Object_Struct:
      {
      register tsp_Offset * pOffset = DESCRIPTOR(wClassId)->pzOffsets;
      register tsp_CType * pszType = DESCRIPTOR(wClassId)->pzString;
      for(;;) {
	tsp_OID * pp = (tsp_OID*)((Byte*)p + (*pOffset++));
	switch(*pszType++)
	  {
	  case 0:
	    return;
	  case tsp_Field_Char:
	  case tsp_Field_UChar:
	  case tsp_Field_Short:
	  case tsp_Field_UShort:
	  case tsp_Field_Int:
	  case tsp_Field_UInt:
	  case tsp_Field_Long:
	  case tsp_Field_ULong: 
	  case tsp_Field_LongLong:
	  case tsp_Field_Float:
	  case tsp_Field_Double:
	    break;
	  case tsp_Field_OID:
	    if(!tsp_IS_IMMEDIATE(*pp) && depth > 0) {
	      if(*pp)
		integrityRec(*pp, depth - 1);
	    }
	    break;
	  default:
            tosError_ABORT();
	  }
      }
      }
    case tsp_Object_Array:
      {
      Int i;
      tsp_OID * pp = p;
      for(i = 0; i < (SIZE(p) / sizeof(tsp_OID)); i++, pp++) {
	if(!tsp_IS_IMMEDIATE(*pp) && depth > 0) {
	  if(*pp)
	    integrityRec(*pp, depth - 1);
	}
      }
      }
      break;
    case tsp_Object_WeakRef:
      {
      tsp_WeakRef* pWeakRef = p;
      integrityRec(pWeakRef->p, depth - 1);
      integrityRec(pWeakRef->pData, depth - 1);
      }
      break;
    case tsp_Object_ByteArray:
      break;
    case tsp_Object_ShortArray:
      break;
    case tsp_Object_Stack:
    case tsp_Object_Thread:
      break;
    default:
      tosError_ABORT();
    }
  return;
}

static void integrity(tsp_OID p)
{
  integrityRec(p, recDepth);
}


/* hunt for illegal OIDs */

void tsp_hunt(void)
{
  if(fHunt) {
    tsp_scanObjects(integrity);
  }
}

void tsp_huntOn(void)
{
  fHunt = TRUE;
}
void tsp_huntOff(void)
{
  fHunt = FALSE;
}

#else   /* tsp_DEBUG */

void tsp_storeStatus()
{
  tosLog_error("tsp",
               "createFromFile",
               "TSP error: Not configured for debugging");
}

#endif /* tsp_DEBUG */


/* Bootstrap */

#ifdef BOOTSTRAP

#include "tmthread.h"

static tosStdio_T * bootFile = NULL;
static tsp_OID    * translationTable = NULL;

#define TYC_CLASSID(p, id, l, s) boot_ClassId_ ## id,

typedef enum {

  #include "classids.h"
  
  boot_ClassId_nPredefined
} boot_ClassId;
#undef TYC_CLASSID


typedef struct boot_DefinedClass{
  boot_ClassId wClassId;
  tsp_Object   wObjectType;
  tsp_CType *  pDescriptor;
} boot_DefinedClass;

#define TYC_CLASSID(p, id, l, s) {boot_ClassId_ ## id, tsp_Object_ ## l, s },

static boot_DefinedClass definedClasses[boot_ClassId_nPredefined + 1] = {

  #include "classids.h"

  {boot_ClassId_nPredefined, tsp_Object_nPredefined, NULL}
};
#undef TYC_CLASSID


static void replaceNumbers(tsp_OID p)
{
  Word wClassId = CLASSID(p);
  assert(SUPERCOMPONENT(p) == NULL);
  switch(DESCRIPTOR(wClassId)->wLayout)
    {
    case tsp_Object_Struct:
      {
      register tsp_Offset * pOffset = DESCRIPTOR(wClassId)->pzOffsets;
      register tsp_CType * pszType = DESCRIPTOR(wClassId)->pzString;
      for(;;)
	switch(*pszType++)
	  {
	  case 0:
	    return;
	  case tsp_Field_OID:
	    {
	    tsp_OID * pp = (tsp_OID*)((Byte*)p + (*pOffset++));
	    if(!tsp_IS_IMMEDIATE(*pp) && *pp != NULL)
	      *pp = translationTable[((Word)(*pp)) >> 2];
	    }
	    break;
	  default:
	    pOffset++;
	    break;
	  }
      }
    case tsp_Object_Array:
      {
      tsp_OID * pp = p;
      pp += SIZE(p) / sizeof(Word);
      while (pp != p) {
	pp--;
	if(!tsp_IS_IMMEDIATE(*pp) && *pp != NULL)
	  *pp = translationTable[((Word)(*pp)) >> 2];
      }
      }
      break;
    case tsp_Object_ByteArray:
    case tsp_Object_ShortArray:
      break;
    default:
      tosError_ABORT();
    }
  return;
}

static void sweep()
{
  Word n;
  /* sweep store and replace object numbers with OIDs */
  for(n = store.nFirstPage; n <= store.nLastPage; n++) {
    if(PPAGE(n)->type == PageType_Object && PPAGE(n)->age == store.age) {
      Byte * p = NPAGE_TO_PTR(n);
      Byte * pp = PPAGE(n)->pbFree;
      /* point to first object (behind header): */
      p += ALIGN_UP_OBJECT(sizeof(Header));
      /* adjust all objects within page */
      while(p < pp) {
	/* don't visit objects with size 0 at end of page (<) */
	replaceNumbers(p);    
	/* round up to next alignment boundary after next header */
	p += ALIGN_UP_OBJECT(SIZE(p) + sizeof(Header));
      }
    }
  }
}

static tsp_OID createNew(Word wClassId, Word wSize)
{
  tsp_OID pNew;
  if(wClassId < boot_ClassId_nPredefined) {
    assert(definedClasses[wClassId].wClassId == wClassId);
    switch(definedClasses[wClassId].wObjectType)
      {
      case tsp_Object_Struct:
	pNew = tsp_newStruct(wClassId);
	assert(SIZE(pNew) >= wSize);         /* size is n * ALIGN_OBJECT */
	assert(CLASSID(pNew) == wClassId);
	return pNew;
      case tsp_Object_Array:
	pNew = tsp_newArray(wClassId, wSize / sizeof(tsp_OID));
	break;
      case tsp_Object_ByteArray:
	pNew = tsp_newByteArray(wClassId, wSize / sizeof(Byte));
	break;
      case tsp_Object_ShortArray:
	pNew = tsp_newShortArray(wClassId, wSize / sizeof(Short));
	break;
      /* none of these object types should be dumped */
      case tsp_Object_IntArray:
      case tsp_Object_LongArray:
      case tsp_Object_Thread:
      case tsp_Object_Stack:
      case tsp_Object_WeakRef:
      default:
	pNew = NULL;
	fprintf(stderr,"\nTSP error: unsupported object type %d.\n",
		definedClasses[wClassId].wObjectType);
        tosError_ABORT();
      }
  }
  else {
    pNew = tsp_newArray(wClassId, wSize / sizeof(tsp_OID));
  }
  assert(SIZE(pNew) == wSize);
  assert(CLASSID(pNew) == wClassId);
  return pNew;
}

typedef struct BootHeader {
  Word wSize;		     /* Object size in bytes */
  Word bitfield;
} BootHeader;

static void readObjects(Word nObjects, Word nClassIds, Word nBytes)
{
  Word i;
  Word wHash, wSize, wClassId;
  BootHeader objectHeader;
  tsp_OID pNew;
  /* read n objects */
  for(i = 1; i <= nObjects; i++) {
    /* read object header */
    fread(&objectHeader, sizeof(BootHeader), 1, bootFile);
    wSize = objectHeader.wSize;
    wClassId = objectHeader.bitfield & CLASSID_MASK;
    wHash = (objectHeader.bitfield & HASH_MASK) >> 12;
    assert(wClassId >= 0 && wClassId < nClassIds);
    pNew = createNew(wClassId, wSize);
    /* insert into translation table */
    translationTable[i] = pNew;
    /* set hash */
    SET_HASH(pNew, wHash);
    assert(HASH(pNew) == wHash);
    assert(SUPERCOMPONENT(pNew) == NULL);
    /* ### immutability... */
    /* read object (extended to word boundary) */
    tosStdio_read(pNew, sizeof(Byte),
          (wSize + (sizeof(Word) - 1)) & ~(sizeof(Word) - 1), bootFile);
    /* debugging output */
    if((i % 1000) == 0) {
      printf("."); fflush(stdout);
    }
  }
  assert(tosStdio_tell(bootFile) == nBytes);
}

static void registerClassIds(Word nIds)
{
  Word i, n;
  /* register predefined ids */
  for(i = 0; i < boot_ClassId_nPredefined; i++) {
    n = tsp_newClassId(definedClasses[i].wObjectType,
		       definedClasses[i].pDescriptor);
    assert(n == i);
  }
  /* register generic ids */
  for(i = boot_ClassId_nPredefined; i < nIds; i++) {
    n = tsp_newClassId(tsp_Object_Array, NULL);
    assert(n == i);
  }
}

Int tsp_createFromFile(String szFileName)
{
  Word wFileSize, nClassIds, nObjects;
  tyc_CompiledFun * pFun;
  tyc_Root * pRoot;
  /* open file */
  if((bootFile = tosStdio_open(szFileName, tosStdio_READ_MODE)) == NULL) {
    return tsp_ERROR_OPEN;
  }
  /* read status */
  tosStdio_seek(bootFile, -8, tosStdio_SEEK_END);
  wFileSize = tosStdio_tell(bootFile);
  tosStdio_read(&nClassIds, sizeof(Word), 1, bootFile);
  tosStdio_read(&nObjects, sizeof(Word), 1, bootFile);
  tosStdio_seek(bootFile, 0, tosStdio_SEEK_SET);
  /* output status */
  printf("\n --- Reading Bootstrap File ---\n\n");
  printf("Size: %u   Objects: %u   ClassIds: %u\n",
         wFileSize, nObjects, nClassIds);
  printf("\n ------------------------------\n\n");
  /* create empty store */
  tsp_create("Bootstrap.ts");
  /* allocate address translation table */
  if((translationTable = malloc((nObjects + 1) * sizeof(tsp_OID))) == NULL) {
    return tsp_ERROR_NOMEMORY;
  }
  memset(translationTable, 0, (nObjects + 1) * sizeof(tsp_OID));
  /* register classids (predefined and generic) */
  registerClassIds(nClassIds);
  /* read objects and set table entries */
  printf("reading objects...\n");
  tsp_inhibitGc();
  readObjects(nObjects, nClassIds, wFileSize);
  /* set root object */
  pRoot = (tyc_Root*)translationTable[1];
  tsp_setRoot(pRoot);
  /* sweep through store and set OIDs */
  printf("\nadjusting OIDs...\n");
  sweep();
  /* create single thread */
  pFun = (tyc_CompiledFun*)(pRoot->pThread);
  tmthread_createBootThread(pRoot, pFun);
  tsp_allowGc();
  /* free translation table */
  free(translationTable);
  /* save store */
  printf("\nwriting store...\n");
  return tsp_commit();
}

#else  /* BOOTSTRAP */

Int tsp_createFromFile(String szFileName)
{
  tosLog_error("tsp",
               "createFromFile",
               "TSP error: Not configured for bootstrap");
  return -1;
}

#endif /* BOOTSTRAP */

