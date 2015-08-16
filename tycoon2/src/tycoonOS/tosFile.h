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
  tosFile.h 1.0 final  07-OCT-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Runtime support for low-level file operations.
*/

#ifndef TOSFILE_H
#define TOSFILE_H


#include "tos.h"


#ifdef __cplusplus
extern "C" {
#endif

/*== Standard files =======================================================*/

extern int tosFile_stdin(void);
extern int tosFile_stdout(void);
extern int tosFile_stderr(void);


/*== Open / Close =========================================================*/

/*
 * File open modes. Don't use SYNC at Win32.
 */

#define TYC_O_RDONLY 0
#define TYC_O_WRONLY 1
#define TYC_O_RDWR   2
#define TYC_O_APPEND 4
#define TYC_O_SYNC   8
#define TYC_O_CREAT  16
#define TYC_O_EXCL   32
#define TYC_O_TRUNC  64


extern int tosFile_open(char *pszPath, int flags);
extern int tosFile_sync(int fd);
extern int tosFile_close(int fd);


/*== File I/O =============================================================*/

extern int tosFile_read1 (int fd, void * pBuffer, int start, int n);
extern int tosFile_write1(int fd, const void * pBuffer, int start, int n);

extern int tosFile_readChar (int fd);
extern int tosFile_writeChar(int fd, char ch);


/*== File information =====================================================*/

extern Bool tosFile_exists(char *pszPath);

extern Bool tosFile_isCharacterDevice(int fd);


/*== File operations ======================================================*/

extern int tosFile_copy  (char *from, char *to);
extern int tosFile_rename(char *from, char *to);
extern int tosFile_remove(char *path);


/*== Symbolic links =======================================================*/

extern int tosFile_mkSymLink(char *name1, char *name2);


#ifdef __cplusplus
}
#endif

#endif /* TOSFILE_H */

