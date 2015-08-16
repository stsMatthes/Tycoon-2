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
  tosLog.h 1.0 final  29-JUL-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  System message logging
*/

#include "tosLog.h"

#include <stdio.h>
#include <stdarg.h>

#if defined(__BORLANDC__)
#include "tosFile.h"
#endif

/*== Output files =========================================================*/

FILE * errout = NULL;    /* Global for all threads ... */
FILE * dbgout = NULL;


void tosLog_redirectError(FILE *newFile) { errout = newFile; }
void tosLog_redirectDebug(FILE *newFile) { dbgout = newFile; }


/*== Log error message ====================================================*/

void tosLog_error(char *module, char *fun, char *msg, ...)
{
   va_list VaList;
   va_start(VaList, msg);

   if (errout == NULL) {
#if defined(__BORLANDC__)
     errout = fdopen(tosFile_stderr(), "w");
     setvbuf(errout, NULL, _IONBF, 0);
#else
     errout = stderr;
#endif
   }

   fprintf (errout, "ERR: %s.%s - ", module, fun);
   vfprintf(errout, msg, VaList);
   fprintf (errout, "\n");

   /* Will never be closed ... */
   fflush(errout);
}

void tosLog_debug(char *module, char *fun, char *msg, ...)
{
   va_list VaList;
   va_start(VaList, msg);

   if (dbgout == NULL) {
      dbgout = fopen("tycoon2.dbg", "wb");
      if (dbgout == NULL) {
         tosLog_error("tosLog", "debug", "Can´t open ""tycoon2.dbg""");
      }
   }

   if (dbgout != NULL) {
      fprintf (dbgout, "DBG: %s.%s - ", module, fun);
      vfprintf(dbgout, msg, VaList);
      fprintf (dbgout, "\n");

      /* Will never be closed ... */
      fflush(dbgout);
   }
}

