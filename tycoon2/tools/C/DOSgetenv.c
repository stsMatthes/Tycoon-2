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
 *  DOSgetenv.c   V1.0   18.2.99  A.Willomat
 *  ========================================
 *  
 *  Gets the current working directory and Copies it to stdout
 *  in .bat notation. The output is generated in DOS and Unix
 *  filename convention (last without drive letter)
 *
 *  Then, if we have already has set HOX_HOME and HOX_HOST, we
 *  extract a clean PATH, without old HOX_BIN.
 *
 *  At the last extract Visual C++ location from path.
 *
 *  Output:
 *
 *  SET HOX_CURR_DIR = <currentDir>
 *  SET HOX_CURR_DIR_UNIX = <currentDir>
 *  SET HOX_CLEAN_PATH = <pathWithoutHOX_BIN>
 *  SET HOX_MSVC = <VC++location>
 *
 *  We need this program because of stupid DOS Batch-Processing.
 *
 *  Simple compile this program with DOSgetenv.bat
 *
 */

#include <direct.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void main() {

  int  i, j, pos, len;
  char currentDir[_MAX_PATH];
  char currentDirUnix[_MAX_PATH];

  char *hoxHome;
  char *hoxHost;
  char *path;
  char *help;
  char *MSDevDir;
  
  char oldHoxBinPath[_MAX_PATH];
  char MSVCDir[_MAX_PATH];
  char cleanPath[4096];


  /*
   * Get the current working directory
   * =================================
   */
  if (_getcwd(currentDir, _MAX_PATH) == NULL) {
     fprintf(stderr, "_getcwd error: %d = %s\n", errno, strerror(errno));
     abort();
  }

  /*
   * Convert to Unix format without drive letter
   * ===========================================
   */
  i = 0;
  j = 0;
  while (currentDir[i]) {
     if (i == _MAX_PATH) break;
     currentDirUnix[j] = currentDir[i];
     if (currentDirUnix[j] == '\\') currentDirUnix[j] = '/';
     if (currentDirUnix[j] == ':')  j = -1;

     i++; j++;
  }
  currentDirUnix[j] = 0;

  /* Generate output */
  printf("@SET HOX_CURR_DIR=%s\n", currentDir);
  printf("@SET HOX_CURR_DIR_UNIX=%s\n", currentDirUnix);


  /*
   * Check if environment already was set, if so, clean up PATH
   * ==========================================================
   */
  hoxHome = getenv("HOX_HOME");
  hoxHost = getenv("HOX_HOST");
  path    = getenv("PATH");

  strcpy(cleanPath, path);

  if ((hoxHome != NULL) && (hoxHost != NULL)) {
     /*
      * clean up path
      */
     strcpy(oldHoxBinPath, ";");
     strcat(oldHoxBinPath, hoxHome);
     strcat(oldHoxBinPath, "\\bin\\");
     strcat(oldHoxBinPath, hoxHost);
     help = strstr(cleanPath, oldHoxBinPath);
     if (help) {
        /*
         * found in old PATH: delete!
         */
        pos = help-cleanPath;
        len = strlen(cleanPath);
        i = pos + strlen(oldHoxBinPath);
        while (i < len) {
          cleanPath[pos++] = cleanPath[i++];
        }
        /*
         * Eliminate terminating ';'
         */
        while (cleanPath[pos-1] == ';') pos --;
        cleanPath[pos] = 0;
     }
  }
  printf("@SET HOX_CLEAN_PATH=%s\n", cleanPath);


  /*
   * Get Visual Studio loaction
   * ==========================
   */
  MSDevDir = getenv("MSDevDir");
  if (MSDevDir == NULL) {
     MSDevDir = getenv("MSVCDir");
  }

  if (MSDevDir == NULL) {
     fprintf(stderr, "\n *** WARNING: Can't find MS Visual Studio, please call VCVARS32.BAT...\n");
     fprintf(stderr, " *** WARNING: Compiling with Jam is disabled\n");
  }
  else {
     /*
      * Cut the last \\xxx
      */
     len = strlen(MSDevDir);
     while (MSDevDir[len-1] != '\\') len--;
     MSDevDir[len-1]=0;

     
     /*
      * Search Visual C++ at PATH
      */
     strcpy(MSVCDir, MSDevDir);
     strcat(MSVCDir, "\\VC");
     help = strstr(_strupr(_strdup(path)),
                   _strupr(_strdup(MSVCDir)));

     if (help == NULL) {
        /*
         * Cut the next \\xxx and search again
         */
        len = strlen(MSDevDir);
        while (MSDevDir[len-1] != '\\') len--;
        MSDevDir[len-1]=0;
        strcpy(MSVCDir, MSDevDir);
        strcat(MSVCDir, "\\VC");
        help = strstr(_strupr(_strdup(path)),
                      _strupr(_strdup(MSVCDir)));
     }

     if (help) {
        /*
         * Copy the full MSVC-Path from PATH
         */
        i = 0;
        while ((help[i] != ';') && (help[i] != 0)) {
          MSVCDir[i] = help[i++];
        }
        /*
         * Cut the \\bin suffix
         */
        MSVCDir[i-4]=0;
        printf("@SET HOX_MSVC=%s\n", MSVCDir);
     }
     else {
        fprintf(stderr, "\n *** WARNING: Can't find MS Visual C++, please call VCVARS32.BAT...\n");
        fprintf(stderr, " *** WARNING: Compiling with Jam is disabled\n");
     }
  }
}

