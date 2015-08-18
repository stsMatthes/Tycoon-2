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
  tosStdio.h 1.0 final  06-OCT-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Standard file I/O for TSP / TVM.

  You can also use <stdio.h>, but than you can go into trouble
  whith future filename-conventions or default open-modes.
*/

#ifndef TOSSTDIO_H
#define TOSSTDIO_H

#include <stdio.h>


#ifdef __cplusplus
extern "C" {
#endif


/*== Types / open modes ===================================================*/

#define tosStdio_T FILE

#define tosStdio_READWRITE_MODE "rb+"
#define tosStdio_READ_MODE      "rb"
#define tosStdio_WRITE_MODE     "wb"
#define tosStdio_CREATE_MODE    "wb+"


/*== Open / Close =========================================================*/

extern tosStdio_T *tosStdio_open(const char *szFileName, const char *szMode);

#define tosStdio_close fclose


/*== I /O =================================================================*/

#define tosStdio_read  fread
#define tosStdio_write fwrite
#define tosStdio_flush fflush


/*== Positioning ==========================================================*/

#define tosStdio_SEEK_CUR  SEEK_CUR
#define tosStdio_SEEK_END  SEEK_END
#define tosStdio_SEEK_SET  SEEK_SET

#define tosStdio_seek  fseek
#define tosStdio_tell  ftell

extern int tosStdio_truncate(tosStdio_T *file, int truncPos);


#ifdef __cplusplus
}
#endif

#endif /* TOSSTDIO_H */

