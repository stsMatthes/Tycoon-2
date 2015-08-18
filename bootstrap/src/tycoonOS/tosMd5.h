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
  tosMd5.h 1.0 final  16-OCT-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  MD5 algorithm taken from rfc1321.
  This code is in the public domain.
  All pasted into one file, without mddriver.c (Tycoon is the driver)
  MD5Foo entry points made static, new entry points:

  tosMd5_init,
     allocate and call MD5Init

  tosMd5_update,
     tycoon interface: buffer + offset

  tosMd5_final
     finalize and free context


  ===========================================================================
   Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991.
   All rights reserved.

   License to copy and use this software is granted provided that it
   is identified as the "RSA Data Security, Inc. MD5 Message-Digest
   Algorithm" in all material mentioning or referencing this software
   or this function.

   License is also granted to make and use derivative works provided
   that such works are identified as "derived from the RSA Data
   Security, Inc. MD5 Message-Digest Algorithm" in all material
   mentioning or referencing the derived work.

   RSA Data Security, Inc. makes no representations concerning either
   the merchantability of this software or the suitability of this
   software for any particular purpose. It is provided "as is"
   without express or implied warranty of any kind.

   These notices must be retained in any copies of any part of this
   documentation and/or software.
  ===========================================================================

*/

#define PROTOTYPES 1

#ifndef TOSMD5_H
#define TOSMD5_H

#ifdef __cplusplus
extern "C" {
#endif


/*== GLOBAL.H - RSAREF types and constants ================================*/

/*
 * PROTOTYPES should be set to one if and only if the compiler supports
 * function argument prototyping. The following makes PROTOTYPES default
 * to 0 if it has not already been defined with C compiler flags.
 */
#ifndef PROTOTYPES
#define PROTOTYPES 0
#endif


/*
 * POINTER defines a generic pointer type
 */
typedef unsigned char *POINTER;


/*
 * UINT2 defines a two byte word
 */
typedef unsigned short int UINT2;


/*
 * UINT4 defines a four byte word
 */
typedef unsigned long int UINT4;


/*
 * PROTO_LIST is defined depending on how PROTOTYPES is defined above.
 * If using PROTOTYPES, then PROTO_LIST returns the list, otherwise it
 * returns an empty list.
 */
#if PROTOTYPES
#define PROTO_LIST(list) list
#else
#define PROTO_LIST(list) ()
#endif


/*== MD5.H - Header file for MD5C.C =======================================*/

/*== MD5 context ==*/

typedef struct {
  UINT4 state[4];                                   /* state (ABCD) */
  UINT4 count[2];        /* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64];                         /* input buffer */
} MD5_CTX;


/*== MD5 Tycoon interface =================================================*/

/*== Size of MD5_CTX ==*/

extern unsigned int tosMd5_contextSize(void);


/*== Allocate and call MD5Init ==*/

extern void tosMd5_init(MD5_CTX *context);


/*== Tycoon interface: buffer + offset ==*/

extern void tosMd5_update(MD5_CTX       *context,
                          unsigned char *buffer,
                          unsigned int   offset,
                          unsigned int   n);

/*== Finalize and free context ==*/

extern void tosMd5_final(MD5_CTX *context, unsigned char signature[16]);


#ifdef __cplusplus
}
#endif

#endif /* TOSMD5_H */

