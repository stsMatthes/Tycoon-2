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

  $File: //depot/tycoon2/stsmain/tycoon2/src/tm/rtdll.c $ $Revision: #3 $ $Date: 2003/10/01 $ Andreas Gawecki, Marc Weikard, Andre Willomat

  Dynamic Link Library Support
  
*/

#include <string.h>

#include "tos.h"
#include "tosLog.h"
#include "tosDll.h"
#include "tosString.h"

#include "rtdll.h"


/* avoid duplicated libraries: */

static char * apszDLL[1000];		/* should grow dynamically */
static rtdll_T ahDLL[1000];
static Int nDLL = 0;

static rtstatic_Lib * pStaticLibs = NULL;
static Int nStaticLibs = 0;

void rtdll_begin(rtstatic_Lib * pStaticLibs_)
{
  pStaticLibs = pStaticLibs_;
  while(pStaticLibs[nStaticLibs].pszName) {
    nStaticLibs++;
  }
}

rtdll_T rtdll_open(char * pszPath)
{
  Int i;  

  for(i = 0; i < nStaticLibs; i++) {
    if (strcmp(pszPath, pStaticLibs[i].pszName) == 0)
      return &pStaticLibs[i];
  }

  for(i = 0; i < nDLL; i++)
    if (strcmp(pszPath, apszDLL[i]) == 0)
      break;

  if (i == nDLL) {
     void *h = tosDll_open(pszPath);
     if (!h) return h;

     apszDLL[i] = tosString_strdup(pszPath);
     ahDLL[i] = h;
     nDLL++;
  }
  return ahDLL[i];
}

void * rtdll_lookup(rtdll_T hDLL, char * pszSym)
{
  if ((Word) hDLL >= (Word) &pStaticLibs[0] &&
      (Word) hDLL <= (Word) &pStaticLibs[nStaticLibs]) {
    rtstatic_Sym * pSym = &(((rtstatic_Lib *)hDLL)->aSyms[0]);
    for(; pSym->pszSym; pSym++)
      if (strcmp(pszSym, pSym->pszSym) == 0)
        return pSym->pValue;
    tosLog_debug("rtdll", "lookup", "Static lookup failed for %s", pszSym);
    return NULL;
  }
  return tosDll_lookup(hDLL, pszSym);
}
  
  
Int rtdll_close(rtdll_T hDLL)
{
  return tosDll_close(hDLL);
}

void rtdll_end()
{
  nDLL = 0;
  nStaticLibs = 0;
}
