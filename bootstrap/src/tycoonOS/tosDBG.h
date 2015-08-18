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
  tosDBG.h 1.0 final  28-JUL-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  DEBUG Flags
*/

#ifndef TOSDBG_H
#define TOSDBG_H


#ifdef __cplusplus
extern "C" {
#endif


/*== Debug flags ==========================================================*/

/*
 * Debug flags for all function groups of the TOS layer. Please comment
 * out the flags for the groups, that should be traced ...
 *
 * Only have effects, if jam buildmode = debug
 *
 */

#ifdef BUILDMODE_DEBUG
 
           #define tosFilename_DEBUG	  
           #define tosDll_DEBUG
/*           #define tosTLS_DEBUG             */
/*           #define tosMutex_DEBUG           */
/*           #define tosCondition_DEBUG       */
           #define tosThread_DEBUG        
           #define tosProcess_DEBUG       
           #define tosStdio_DEBUG         
           #define tosDirectory_DEBUG	  
           #define tosFile_DEBUG          
           #define tosFilemode_DEBUG	  
           #define tosFileinfo_DEBUG	  
           #define tosFilepos_DEBUG         
           #define tosNet_DEBUG           
           #define tosSecurity_DEBUG	  
           #define tosSocket_DEBUG        

#endif


/*
 * Automatic flags, please don't set manually ...
 */

#if defined(tosFilename_DEBUG)
  #define tos_DEBUG
#endif
#if defined(tosDll_DEBUG)
  #define tos_DEBUG
#endif
#if defined(tosTLS_DEBUG)
  #define tos_DEBUG
#endif
#if defined(tosMutex_DEBUG)
  #define tos_DEBUG
#endif
#if defined(tosCondition_DEBUG)
  #define tos_DEBUG
#endif
#if defined(tosThread_DEBUG)
  #define tos_DEBUG
#endif
#if defined(tosProcess_DEBUG)
  #define tos_DEBUG
#endif
#if defined(tosStdio_DEBUG)
  #define tos_DEBUG
#endif
#if defined(tosDirectory_DEBUG)
  #define tos_DEBUG
#endif
#if defined(tosFile_DEBUG)
  #define tos_DEBUG
#endif
#if defined(tosFilemode_DEBUG)
  #define tos_DEBUG
#endif
#if defined(tosFileinfo_DEBUG)
  #define tos_DEBUG
#endif
#if defined(tosFilepos_DEBUG)
  #define tos_DEBUG
#endif
#if defined(tosNet_DEBUG)
  #define tos_DEBUG
#endif
#if defined(tosSecurity_DEBUG)
  #define tos_DEBUG
#endif
#if defined(tosSocket_DEBUG)
  #define tos_DEBUG
#endif

#ifdef __cplusplus
}
#endif

#endif /* TOSDBG_H */

