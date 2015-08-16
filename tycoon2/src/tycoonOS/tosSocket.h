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
  tosSocket.h 1.0 final  18-NOV-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  We support IP (Internet Protocol) only.

  All functions manipulating socket descriptors are wrapped to keep them
  together in this place (may help porting), esp. those which may block
  in system calls (may help porting threads).

  If not stated otherwise, all functions must implement the semantics
  of the 'corresponding'  POSIX functions, if any.

  Error handling: Unix style, i.e. all calls MUST set 'errno' in case of errors.
  Assumption: error numbers are non-zero.
*/

#ifndef TOSSOCKET_H
#define TOSSOCKET_H


#include "tos.h"


#ifdef __cplusplus
extern "C" {
#endif


/*== Basic types ==========================================================*/

typedef Int tosSocket_T;

extern Int tosSocket_nil(void);


/*== Socket creation ======================================================*/

extern tosSocket_T tosSocket_newStreamSocket(void);
extern tosSocket_T tosSocket_newDatagramSocket(void);
extern tosSocket_T tosSocket_newServerSocket(void);

extern int tosSocket_close(tosSocket_T s);


/*== Socket connection ====================================================*/

extern int tosSocket_connect(tosSocket_T  s,
                             Byte        *pbIPAddress,
                             int          cbIPAddress,
                             int          port);

extern int tosSocket_listen(tosSocket_T s, int queuelen);

extern int tosSocket_bind(tosSocket_T  s,
                          Byte        *pbIPAddress,
                          int          cbIPAddress,
                          int          port);

extern tosSocket_T tosSocket_accept(tosSocket_T s);


/*== Stream Socket I/O ====================================================*/

extern int tosSocket_read(tosSocket_T  s,
                          void        *pBuffer,
                          int          start,
                          int          n);

extern int tosSocket_write(tosSocket_T  s,
                           const void  *pBuffer,
                           int          start,
                           int          n);


extern int tosSocket_readChar(tosSocket_T s);
extern int tosSocket_writeChar(tosSocket_T s, char ch);


/*== Socket Informations ==================================================*/

extern int tosSocket_address(tosSocket_T  s,
                             Byte        *pbIPAddress,
                             int          cbIPAddress);

extern int tosSocket_remoteAddress(tosSocket_T  s,
                                   Byte        *pbIPAddress,
                                   int          cbIPAddress);

extern int tosSocket_port(tosSocket_T s);
extern int tosSocket_remotePort(tosSocket_T s);


#ifdef __cplusplus
}
#endif

#endif /* TOSSOCKET_H */

