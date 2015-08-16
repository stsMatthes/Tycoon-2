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

  $File: //depot/tycoon2/stsmain/tycoon2/src/tm/tmccall.c $ $Revision: #3 $ $Date: 2003/10/01 $ Marc Weikard

  CCall Interface
  
*/

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "tos.h"
#include "tosLog.h"
#include "tosError.h"

#include "tsp.h"
#include "tyc.h"
#include "tvm.h"
#include "tmthread.h"
#include "tmccall.h"


#ifdef rt_LIB_HPUX_PARISC
#define align64BitValue() \
  if((pCCArgs - pw) == 1 || (pCCArgs - pw) == 3) { \
    *pCCArgs++ = 0; \
  }
#endif


static void raiseConversionError(tsp_OID p, char t, char c)
{
  if(p) {
    tosLog_error("tmccall",
                 "execute",
                 "Cannot convert object from '%c' to '%c'",
                  t, c);
  }
  else {
    tosLog_error("tmccall",
                 "execute",
                 "Cannot convert result from '%c' to '%c'",
                  c, t);
  }
  tosError_ABORT();
}


tsp_OID tmccall_execute(tyc_ExternalMethod * pMethod, tsp_OID * pArguments)
{
  /* Execute a ccall. arguments are verified at runtime. The receiver
     must be a tyc_DLL object. */
  char * pszTycoonDescriptor, * pszCDescriptor;
  char cCType, cTycoonType, cCResult, cTycoonResult;
  Word pw[MAX_CCALL_ARGUMENTS];
  Word * pCCArgs = pw;
  long * pAddress;
  tsp_OID * pResult = NULL;
  void * pFunction = pMethod->pFunction;
  Bool fBlocking = pMethod->fBlocking;
  /* possible return values */
  long longValue;              /* 32 bit integer */
  Long longlongValue;          /* 64 bit integer */
  float floatValue;            /* 32 bit real */
  double doubleValue;          /* 64 bit real */

  memset(pw, 0, MAX_CCALL_ARGUMENTS * sizeof(Word));/* rui SUN Debugger only */

  pszCDescriptor = pMethod->pszCArgs;
  pszTycoonDescriptor = pMethod->pszTycoonArgs;

  assert(pszCDescriptor && pszTycoonDescriptor);
  assert(strlen(pszCDescriptor) == strlen(pszTycoonDescriptor));

  /* prefetch first argument type */
  cCType = *pszCDescriptor++, cTycoonType = *pszTycoonDescriptor++;

  /* convert arguments */
  while(*pszCDescriptor) {
    tsp_OID pArgument = *--pArguments;
    switch(cTycoonType)
      {
      case tyc_Type_Bool:
        {
        Bool f;
        if(tyc_IS_FALSE(pArgument)) {
          f = FALSE;
        }
        else if(tyc_IS_TRUE(pArgument)) {
          f = TRUE;
        }
        else {
          tvm_raiseTypeError(pArgument, tyc_ClassId_Bool);
          break;
        }
        switch(cCType)
          {
          case tsp_Field_Char:
          case tsp_Field_UChar:
          case tsp_Field_Short:
          case tsp_Field_UShort:
          case tsp_Field_Int:
          case tsp_Field_UInt:
          case tsp_Field_Long:
          case tsp_Field_ULong:
            *pCCArgs++ = f; break;
          case tsp_Field_LongLong:
            {
            Long l = f;
            Word * p = (Word*) &l;
#ifdef rt_LIB_HPUX_PARISC
            align64BitValue();
            *pCCArgs++ = p[1];
            *pCCArgs++ = p[0];
#else
            *pCCArgs++ = p[0];
            *pCCArgs++ = p[1];
#endif
            }
            break;
          default:
            raiseConversionError(pArgument, cTycoonType, cCType);
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
            case tsp_Field_Short:
            case tsp_Field_UShort:
            case tsp_Field_Int:
            case tsp_Field_UInt:
            case tsp_Field_Long:
            case tsp_Field_ULong:
              *pCCArgs++ = c; break;
            case tsp_Field_LongLong:
              {
              Long l = c;
              Word * p = (Word*) &l;
#ifdef rt_LIB_HPUX_PARISC
              align64BitValue();
              *pCCArgs++ = p[1];
              *pCCArgs++ = p[0];
#else
              *pCCArgs++ = p[0];
              *pCCArgs++ = p[1];
#endif
              }
              break;
            default:
              raiseConversionError(pArgument, cTycoonType, cCType);
            }
          break;
        }
        tvm_raiseTypeError(pArgument, tyc_ClassId_Char);
        break;
      case tyc_Type_Int:
        if(tyc_IS_TAGGED_INT(pArgument)) {
          Int i = tyc_TAGGED_INT_VALUE(pArgument);
          switch(cCType)
            {
            case tsp_Field_Char:
            case tsp_Field_UChar:
            case tsp_Field_Short:
            case tsp_Field_UShort:
            case tsp_Field_Int:
            case tsp_Field_UInt:
            case tsp_Field_Long:
            case tsp_Field_ULong:
              *pCCArgs++ = i; break;
            case tsp_Field_LongLong:
              {
              Long l = i;
              Word * p = (Word*) &l;
#ifdef rt_LIB_HPUX_PARISC
              align64BitValue();
              *pCCArgs++ = p[1];
              *pCCArgs++ = p[0];
#else
              *pCCArgs++ = p[0];
              *pCCArgs++ = p[1];
#endif
              }
              break;
            default:
              raiseConversionError(pArgument, cTycoonType, cCType);
            }
          break;
        }
        tvm_raiseTypeError(pArgument, tyc_ClassId_Int);
        break;
      case tyc_Type_Long:
        if(tyc_CLASSID(pArgument) == tyc_ClassId_Long) {
          Long l = tyc_LONG_VALUE(pArgument);
          switch(cCType)
            {
            case tsp_Field_Char:
            case tsp_Field_UChar:
            case tsp_Field_Short:
            case tsp_Field_UShort:
            case tsp_Field_Int:
            case tsp_Field_UInt:
            case tsp_Field_Long:
            case tsp_Field_ULong:
              *pCCArgs++ = l; break;
            case tsp_Field_LongLong:
              {
              Word * p = (Word*) &l;
#ifdef rt_LIB_HPUX_PARISC
              align64BitValue();
              *pCCArgs++ = p[1];
              *pCCArgs++ = p[0];
#else
              *pCCArgs++ = p[0];
              *pCCArgs++ = p[1];
#endif
              }
              break;
            default:
              raiseConversionError(pArgument, cTycoonType, cCType);
            }
          break;
        }
        tvm_raiseTypeError(pArgument, tyc_ClassId_Long);
        break;
      case tyc_Type_Real:
        if(tyc_CLASSID(pArgument) == tyc_ClassId_Real) {
          Real r = tyc_REAL_VALUE(pArgument);
          switch(cCType)
            {
            case tsp_Field_Float:
              *((float*)pCCArgs) = r;
              pCCArgs ++;
              break;
            case tsp_Field_Double:
              {
              Word * p = (Word*) &r;
#ifdef rt_LIB_HPUX_PARISC
              align64BitValue();
              *pCCArgs++ = p[1];
              *pCCArgs++ = p[0];
#else
              *pCCArgs++ = p[0];
              *pCCArgs++ = p[1];
#endif
              }
              break;
            default:
              raiseConversionError(pArgument, cTycoonType, cCType);
            }
          break;
        }
        tvm_raiseTypeError(pArgument, tyc_ClassId_Real);
        break;
      case tyc_Type_String:
        if(tyc_CLASSID(pArgument) == tyc_ClassId_String ||
           tyc_CLASSID(pArgument) == tyc_ClassId_MutableString ||
           tyc_CLASSID(pArgument) == tyc_ClassId_Symbol ||
           tyc_CLASSID(pArgument) == tyc_ClassId_Nil) {
          switch(cCType)
            {
            case tsp_Field_String:
              *pCCArgs++ = (Word)pArgument; break;
            default:
              raiseConversionError(pArgument, cTycoonType, cCType);
            }
          break;
        }
        tvm_raiseTypeError(pArgument, tyc_ClassId_String);
        break;
      case tyc_Type_MutableString:
        if(tyc_CLASSID(pArgument) == tyc_ClassId_MutableString ||
           tyc_CLASSID(pArgument) == tyc_ClassId_Nil) {
          switch(cCType)
            {
            case tsp_Field_String:
              *pCCArgs++ = (Word)pArgument; break;
            default:
              raiseConversionError(pArgument, cTycoonType, cCType);
            }
          break;
        }
        tvm_raiseTypeError(pArgument, tyc_ClassId_MutableString);
        break;
      case tyc_Type_Object:
        if(!tsp_IS_IMMEDIATE(pArgument)) {
          switch(cCType)
            {
            case tsp_Field_OID:
              *pCCArgs++ = (Word)pArgument; break;
            default:
              raiseConversionError(pArgument, cTycoonType, cCType);
            }
          break;
        }
        tvm_raiseTypeError(pArgument, tyc_ClassId_Object);
        break;
      default:
        /* (illegal tycoon descriptor) */
        tosLog_error("tmccall", "execute", "Illegal tycoon descriptor in argument");
        tosError_ABORT();
      }
    /* get next field descriptor */
    cCType = *pszCDescriptor++, cTycoonType = *pszTycoonDescriptor++;
  }
  /* get result type */
  cCResult = cCType, cTycoonResult = cTycoonType;

  /* check for blocking call */
  if(fBlocking) {
    tmthread_enterBlockingCCall();
  }

  /* execute ccall */
  if(cCResult == tsp_Field_Double)
    {
      doubleValue = (*(double (*)()) pFunction)(
                       pw[ 0], pw[ 1], pw[ 2], pw[ 3], pw[ 4],
                       pw[ 5], pw[ 6], pw[ 7], pw[ 8], pw[ 9],
                       pw[10], pw[11], pw[12], pw[13], pw[14],
                       pw[15], pw[16], pw[17], pw[18], pw[19]);
      pAddress = (long*)&doubleValue;
    }
  else if(cCResult == tsp_Field_Float) {
      floatValue = (*(float (*)()) pFunction)(
                      pw[ 0], pw[ 1], pw[ 2], pw[ 3], pw[ 4],
                      pw[ 5], pw[ 6], pw[ 7], pw[ 8], pw[ 9],
                      pw[10], pw[11], pw[12], pw[13], pw[14],
                      pw[15], pw[16], pw[17], pw[18], pw[19]);
      pAddress = (long*)&floatValue;
  }
  else if(cCResult == tsp_Field_LongLong) {
      longlongValue = (*(Long (*)()) pFunction)(
                         pw[ 0], pw[ 1], pw[ 2], pw[ 3], pw[ 4],
                         pw[ 5], pw[ 6], pw[ 7], pw[ 8], pw[ 9],
                         pw[10], pw[11], pw[12], pw[13], pw[14],
                         pw[15], pw[16], pw[17], pw[18], pw[19]);
      pAddress = (long*)&longlongValue;
  }
  else
    {
      longValue = (*(long (*)()) pFunction)(
                     pw[ 0], pw[ 1], pw[ 2], pw[ 3], pw[ 4],
                     pw[ 5], pw[ 6], pw[ 7], pw[ 8], pw[ 9],
                     pw[10], pw[11], pw[12], pw[13], pw[14],
                     pw[15], pw[16], pw[17], pw[18], pw[19]);
      pAddress = &longValue;
    }

  /* check for blocking call */
  if(fBlocking) {
    tmthread_leaveBlockingCCall();
  }

  /* convert result */
  switch(cTycoonResult)
    {
    case tyc_Type_Void:
      pResult = tyc_NIL; break;
    case tyc_Type_Bool:
      switch(cCResult)
        {
        case tsp_Field_Char:
        case tsp_Field_UChar:
        case tsp_Field_Short:
        case tsp_Field_UShort:
        case tsp_Field_Int:
        case tsp_Field_UInt:
        case tsp_Field_Long:
        case tsp_Field_ULong:
          pResult = tyc_boxBool(*pAddress); break;
        case tsp_Field_LongLong:
          pResult = tyc_boxBool(*((Long*)pAddress)); break;
        default:
          raiseConversionError(NULL, cTycoonResult, cCResult);
        }
      break;
    case tyc_Type_Char:
      switch(cCResult)
        {
        case tsp_Field_Char:
        case tsp_Field_UChar:
        case tsp_Field_Short:
        case tsp_Field_UShort:
        case tsp_Field_Int:
        case tsp_Field_UInt:
        case tsp_Field_Long:
        case tsp_Field_ULong:
          pResult = tyc_boxChar(*pAddress); break;
        case tsp_Field_LongLong:
          pResult = tyc_boxChar(*((Long*)pAddress)); break;
        default:
          raiseConversionError(NULL, cTycoonResult, cCResult);
        }
      break;
    case tyc_Type_Int:
      {
      Int i = 0;
      switch(cCResult)
        {
        case tsp_Field_Char:
          i = (signed char)*pAddress; break;
        case tsp_Field_UChar:
          i = (unsigned char)*pAddress; break;
        case tsp_Field_Short:
          i = (signed short)*pAddress; break;
        case tsp_Field_UShort:
          i = (unsigned short)*pAddress; break;
        case tsp_Field_Int:
          i = (signed int)*pAddress; break;
        case tsp_Field_UInt:
          i = (unsigned int)*pAddress; break;
        case tsp_Field_Long:
          i = (signed long)*pAddress; break;
        case tsp_Field_ULong:
          i = (unsigned long)*pAddress; break;
        case tsp_Field_LongLong:
          i = *((Long*)pAddress); break;
        default:
          raiseConversionError(NULL, cTycoonResult, cCResult);
        }
      if(!tyc_MUST_BOX_INT(i)) {
        pResult = tyc_TAG_INT(i);
      }
      else {
        tsp_inhibitGc();
        pResult = (tsp_OID)tyc_boxInt(i);
        tsp_allowGc();
      }
      }
      break;
    case tyc_Type_Long:
      {
      Long l = 0;
      switch(cCResult)
        {
        case tsp_Field_Char:
          l = (signed char)*pAddress; break;
        case tsp_Field_UChar:
          l = (unsigned char)*pAddress; break;
        case tsp_Field_Short:
          l = (signed short)*pAddress; break;
        case tsp_Field_UShort:
          l = (unsigned short)*pAddress; break;
        case tsp_Field_Int:
          l = (signed int)*pAddress; break;
        case tsp_Field_UInt:
          l = (unsigned int)*pAddress; break;
        case tsp_Field_Long:
          l = (signed long)*pAddress; break;
        case tsp_Field_ULong:
          l = (unsigned long)*pAddress; break;
        case tsp_Field_LongLong:
          l = *((Long*)pAddress); break;
        default:
          raiseConversionError(NULL, cTycoonResult, cCResult);
        }
      tsp_inhibitGc();
      pResult = tyc_boxLong(l);
      tsp_allowGc();
      }
      break;
    case tyc_Type_Real:
      {
      Real r = 0;
      switch(cCResult)
        {
        case tsp_Field_Float:
          r = *((float*)pAddress); break;
        case tsp_Field_Double:
          r = *((double*)pAddress); break;
        default:
          raiseConversionError(NULL, cTycoonResult, cCResult);
        }
      tsp_inhibitGc();
      pResult = tyc_boxReal(r);
      tsp_allowGc();
      }
      break;
    {
    tsp_ClassId id;
    case tyc_Type_MutableString:
      id = tyc_ClassId_MutableString;
      goto copyString;
    case tyc_Type_String:
      id = tyc_ClassId_String;
copyString:
      {
      switch(cCResult)
        {
        case tsp_Field_String:
          if(!(*((char**)pAddress))) {
            pResult = NULL;
          }
          else {
            tsp_inhibitGc();
            pResult = tsp_newByteArray(id, strlen(*((char**)pAddress)) + 1);
            strcpy((char*)pResult, *((char**)pAddress));
            tsp_allowGc();
          }
          break;
        default:
          raiseConversionError(NULL, cTycoonResult, cCResult);
        }
      }
      break;
    }
    default:
      /* (illegal tycoon descriptor) */
      tosLog_error("tmccall", "execute", "Illegal tycoon descriptor in result");
      tosError_ABORT();
    }
  return pResult;
}
