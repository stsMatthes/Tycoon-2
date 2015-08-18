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
   $File: //depot/tycoon2/stsmain/bootstrap/src/tycoonOS/tosSocket.c $ $Revision: #4 $ $Date: 2003/11/03 $ Andreas Gawecki, Andre Willomat

   Interface to Portable Tycoon-2 operating system (TycoonOS)

   We support IP (Internet Protocol) only.

   All functions manipulating socket descriptors
   are wrapped to keep them together in this place (may help porting), esp.
   those which may block in system calls (may help porting threads).

   If not stated otherwise, all functions must implement the semantics
   of the 'corresponding'  POSIX functions, if any.

   Error handling: Unix style, i.e. all calls MUST set 'errno' in case of errors.
   Assumption: error numbers are non-zero.

*/


#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/types.h>


#ifdef rt_LIB_Win32_i386
  /* No direct import should be needed at newer SDK's
     #ifdef rt_LIB_Win95_i386
       #include <winsock.h>
     #else
       #include <winsock2.h>
     #endif
  */
  #include <windows.h>
#else
  #include <pthread.h>
  #include <unistd.h>
  #include <netdb.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
#endif

#if defined(rt_LIB_HPUX_PARISC)
  #include <sys/time.h>
  #define _XOPEN_SOURCE_EXTENDED 1
#endif


#include "tos.h"
#include "tosSocket.h"
#include "tosLog.h"
#include "tosNet.h"
#include "tosError.h"


/*== Basic types ==========================================================*/

/*
 * This types maps the Socket_T, seen from tycoon-2 to the
 * real machine socket type.
 */

#ifdef rt_LIB_Win32_i386
   #define tosSocket_system_T SOCKET
   #define tosSocket_NIL      INVALID_SOCKET
#else
   #define tosSocket_system_T int
   #define tosSocket_NIL      -1
#endif

Int tosSocket_nil(void)
{
  return (tosSocket_T) tosSocket_NIL;
}


/*== Socket creation ======================================================*/

tosSocket_T tosSocket_newStreamSocket(void)
{
   tosSocket_system_T s;

   s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

#ifdef rt_LIB_Win32_i386
   if (s == tosSocket_NIL) errno = EWINSOCKET;
#endif

  #ifdef tosSocket_DEBUG
     tosLog_debug("tosSocket",
                  "newStreamSocket",
                  "Open new stream socket, s=%d, res=(%d:%d)",
                  s,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return (tosSocket_T)s;
}


tosSocket_T tosSocket_newDatagramSocket(void)
{
   tosSocket_system_T s;

   s = socket(AF_INET, SOCK_DGRAM, IPPROTO_TCP);

#ifdef rt_LIB_Win32_i386
   if (s == tosSocket_NIL) errno = EWINSOCKET;
#endif

  #ifdef tosSocket_DEBUG
     tosLog_debug("tosSocket",
                  "newDatagramSocket",
                  "Open new datagram socket, s=%d, res=(%d:%d)",
                  s,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return (tosSocket_T)s;
}


tosSocket_T tosSocket_newServerSocket(void)
{
   tosSocket_system_T s;
   const int one = 1;

   s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

#ifdef rt_LIB_Win32_i386
   if (s == tosSocket_NIL) errno = EWINSOCKET;
#endif

   if (s != tosSocket_NIL &&
       setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(one)))
   {
      #ifdef rt_LIB_Win32_i386
         errno = EWINSOCKET;
         closesocket(s);
      #else
         close(s);
      #endif
      s = tosSocket_NIL;
   }

   #ifdef tosSocket_DEBUG
      tosLog_debug("tosSocket",
                   "newServerSocket",
                   "Open new server socket, s=%d, res=(%d:%d)",
                   s,
                   tosError_getCode(),
                   tosError_getCodeDetail());
   #endif
   return (tosSocket_T)s;
}


int tosSocket_close(tosSocket_T s)
{
   int res;
   
   #ifdef rt_LIB_Win32_i386
      if (closesocket((tosSocket_system_T)s) != 0) {
         errno = EWINSOCKET;
         res   = -1;
      }
   #else
      res = close((tosSocket_system_T)s);
   #endif

   #ifdef tosSocket_DEBUG
      tosLog_debug("tosSocket",
                   "close",
                   "Close socket, s=%d, res=(%d:%d:%d)",
                   s,
                   res,
                   tosError_getCode(),
                   tosError_getCodeDetail());
   #endif
   return res;
}


/*== Socket connection ====================================================*/

int tosSocket_connect(tosSocket_T  s,
                      Byte        *pbIPAddress,
                      int          cbIPAddress,
                      int          port)
{
   int res;
   struct sockaddr_in sa;

   /* avoid initialization problems (coming up on HP-UX!) */
   memset(&sa, 0, sizeof(sa));

   sa.sin_family = AF_INET;
   sa.sin_port = htons((u_short)port);

   tosError_reset();
   if (cbIPAddress != tosNet_IPADDR_LENGTH) {
      errno = EINVAL;
      return -1;
   }

   assert(cbIPAddress == tosNet_IPADDR_LENGTH);
   memcpy(&sa.sin_addr, pbIPAddress, tosNet_IPADDR_LENGTH);

   #ifdef tosSocket_DEBUG
      tosLog_debug("tosSocket",
                   "connect",
                   "Connecting socket, s=%d, host=%s, port=%d, fa=%d ...",
                   s,
                   inet_ntoa(sa.sin_addr),
                   port,
                   sa.sin_family);
   #endif
   res = connect((tosSocket_system_T)s, (struct sockaddr *)&sa, sizeof(sa));

#ifdef rt_LIB_Win32_i386
   if (res != 0) errno = EWINSOCKET;
#endif

   #ifdef tosSocket_DEBUG
      tosLog_debug("tosSocket",
                   "connect",
                   "Connected socket, s=%d, host=%s, port=%d, res=(%d:%d:%d)",
                   s,
                   inet_ntoa(sa.sin_addr),
                   port,
                   res,
                   tosError_getCode(),
                   tosError_getCodeDetail());
   #endif
   return res;
}


int tosSocket_listen(tosSocket_T s, int queuelen)
{
   int res = listen((tosSocket_system_T)s, queuelen);

#ifdef rt_LIB_Win32_i386
   if (res != 0) errno = EWINSOCKET;
#endif

   #ifdef tosSocket_DEBUG
      tosLog_debug("tosSocket",
                   "listen",
                   "Alter queuelen at socket, s=%d, len=%d, res=(%d:%d:%d)",
                   s,
                   queuelen,
                   res,
                   tosError_getCode(),
                   tosError_getCodeDetail());
   #endif
   return res;
}


int tosSocket_bind(tosSocket_T  s,
                   Byte        *pbIPAddress,
                   int          cbIPAddress,
                   int          port)
  /*
   * Bind the given socket to the given IP address and port.
   * A nil pbIPAddress value leads to use of IPADDR_ANY as IP
   * address. A zero port value chooses any available port
   */
{
   int res;
   struct sockaddr_in sa;
   assert(sizeof(sizeof(sa.sin_addr)) == tosNet_IPADDR_LENGTH);

   /* avoid initialization problems (coming up on HP-UX!) */
   memset(&sa, 0, sizeof(sa));

   sa.sin_family = AF_INET;
   sa.sin_port = htons((u_short)port);

   tosError_reset();
   if (pbIPAddress == NULL)
      sa.sin_addr.s_addr = htonl(INADDR_ANY);
   else {
      if (cbIPAddress != tosNet_IPADDR_LENGTH) {
         errno = EINVAL;
         return -1;
      }
      memcpy(&sa.sin_addr, pbIPAddress, tosNet_IPADDR_LENGTH);
   }

   tosError_reset();
   res = bind((tosSocket_system_T)s, (struct sockaddr *)&sa, sizeof(sa));

#ifdef rt_LIB_Win32_i386
   if (res != 0) errno = EWINSOCKET;
#endif

   #ifdef tosSocket_DEBUG
      tosLog_debug("tosSocket",
                   "bind",
                   "Binding socket to host, s=%d, host=%s, port=%d, res=(%d:%d:%d)",
                   s,
                   inet_ntoa(sa.sin_addr),
                   port,
                   res,
                   tosError_getCode(),
                   tosError_getCodeDetail());
   #endif
   return res;
}


tosSocket_T tosSocket_accept(tosSocket_T s)
{
  struct sockaddr addr;
  int addrlen = sizeof(addr);
  tosSocket_system_T res;

#ifdef tosSocket_DEBUG
  struct sockaddr_in *sa = (struct sockaddr_in *) &addr;
#endif

#if defined(rt_LIB_HPUX_PARISC) && defined(tmthread_DRAFT_INTERFACE)
  /* definitions needed for select */
  int ttymask;
  int readmask = 0;
  int readfds;
  int nfound;
  /*  struct timeval timeout; */
#endif

  /* avoid initialization problems (coming up on HP-UX!) */
  memset(&addr, 0, sizeof(addr));

#ifdef tosSocket_DEBUG
  tosLog_debug("tosSocket",
               "accept",
               "Waiting for connection on socket (s=%d) ...",
               s);
#endif

#if defined(rt_LIB_HPUX_PARISC) && defined(tmthread_DRAFT_INTERFACE)
  /* under HPUX accept seems not to be thread safe; so
     select is used instead */
  ttymask = 1 << (tosSocket_system_T)s;
  readmask |= ttymask;
  readfds = readmask;
  do {
    nfound = select(1, &readfds, 0, 0, NULL);
    if (ttymask & readfds)
      res = accept((tosSocket_system_T)s, &addr, &addrlen);
    else
      res = nfound;
  } while( nfound != -1 && (!ttymask | !readfds) );

#else
  res = accept((tosSocket_system_T)s, &addr, &addrlen);
#endif

#ifdef rt_LIB_Win32_i386
  if (res == tosSocket_NIL) errno = EWINSOCKET;
#endif

  if ((res != tosSocket_NIL) && (addr.sa_family != AF_INET)) {
     tosLog_error("tosSocket", "accept",
                  "Only IP protcol supported, but is %d",
                  addr.sa_family);
     return -1;
  }

  #ifdef tosSocket_DEBUG
    tosLog_debug("tosSocket",
                 "accept",
                 "Connected socket, s=%d, toHost=%s, toPort=%d, res=(%d:%d)",
                 s,
                 inet_ntoa(sa->sin_addr),
                 ntohs(sa->sin_port),
                 tosError_getCode(),
                 tosError_getCodeDetail());
  #endif
  return (tosSocket_T)res;
}


/*== Stream Socket I/O ====================================================*/

int tosSocket_read(tosSocket_T s, void *pBuffer, int start, int n)
{
   int res;

#ifdef rt_LIB_Win32_i386
   res = recv((tosSocket_system_T)s, ((char *)pBuffer)+start, n, 0);
   if (res == SOCKET_ERROR) errno = EWINSOCKET;
#else
   res = read((tosSocket_system_T)s, ((char *)pBuffer)+start, n);
#endif

   return res;
}


int tosSocket_write(tosSocket_T s, const void * pBuffer, int start, int n)
  /*
   * clear errno in case not all bytes can be written and
   * errno is not set by the systems 'write' function
   */
{
   int res;

   tosError_reset();

#ifdef rt_LIB_Win32_i386
   res = send((tosSocket_system_T)s, ((const char *)pBuffer)+start, n, 0);
   if (res == SOCKET_ERROR) errno = EWINSOCKET;
#else
   res = write((tosSocket_system_T)s, ((const char *)pBuffer)+start, n);
#endif

   if (res != n && res >= 0) {
      return -1;
   }
   return res;
}


int tosSocket_readChar(tosSocket_T s)
  /*
   * to distinguish error and eof conditions:
   * return -1 on error or eof. set errno to 0 if eof.
   */
{
  char buf[1];
  int result = tosSocket_read(s, buf, 0, 1);
  if (result <= 0) {
     if (result == 0) tosError_reset();
     return -1;
  }
  return (int) (unsigned char) buf[0];
}


int tosSocket_writeChar(tosSocket_T s, char ch)
{
  char buf[1];
  buf[0] = ch;
  return tosSocket_write(s, buf, 0, 1);
}


/*== Socket Informations ==================================================*/

int tosSocket_address(tosSocket_T  s,
                      Byte        *pbIPAddress,
                      int          cbIPAddress)
{
   struct sockaddr_in sa;
   int len = sizeof(sa);
   int res;

   tosError_reset();

   if (cbIPAddress != tosNet_IPADDR_LENGTH) {
      errno = EINVAL;
      return -1;
   }

   res = getsockname((tosSocket_system_T)s, (void *)&sa, &len);
#ifdef rt_LIB_Win32_i386
   if (res != 0) errno = EWINSOCKET;
#endif

   /* correct protocol ??? */
   if ((res == 0) && (sa.sin_family != AF_INET)) {
      tosLog_error("tosSocket", "address",
                   "Only IP protcol supported, but is %d",
                   sa.sin_family);
      return -1;
   }

   /* Debug message and return */
   if (res == 0) {
      #ifdef tosSocket_DEBUG
         tosLog_debug("tosSocket",
                      "address",
                      "Get socket address, s=%d, adr=%s, res=(0:%d:%d)",
                      s,
                      inet_ntoa(sa.sin_addr),
                      tosError_getCode(),
                      tosError_getCodeDetail());
      #endif
      memcpy(pbIPAddress, &sa.sin_addr, tosNet_IPADDR_LENGTH);
      return 0;
   } else {
      #ifdef tosSocket_DEBUG
         tosLog_debug("tosSocket",
                      "address",
                      "Get socket address, s=%d, res=(%d:%d:%d)",
                      s,
                      res,
                      tosError_getCode(),
                      tosError_getCodeDetail());
      #endif
      return -1;
   }
}


int tosSocket_remoteAddress(tosSocket_T  s,
                            Byte        *pbIPAddress,
                            int          cbIPAddress)
{
   struct sockaddr_in sa;
   int len = sizeof(sa);
   int res;

   tosError_reset();

   if (cbIPAddress != tosNet_IPADDR_LENGTH) {
      errno = EINVAL;
      return -1;
   }

   res = getpeername((tosSocket_system_T)s, (void *)&sa, &len);
#ifdef rt_LIB_Win32_i386
   if (res != 0) errno = EWINSOCKET;
#endif

   /* correct protocol ??? */
   if ((res == 0) && (sa.sin_family != AF_INET)) {
      tosLog_error("tosSocket", "remoteAddress",
                   "Only IP protcol supported, but is %d",
                   sa.sin_family);
      return -1;
   }

   /* Debug message and return */
   if (res == 0) {
      #ifdef tosSocket_DEBUG
         tosLog_debug("tosSocket",
                      "remoteAddress",
                      "Get socket peer address, s=%d, adr=%s, res=(0:%d:%d)",
                      s,
                      inet_ntoa(sa.sin_addr),
                      tosError_getCode(),
                      tosError_getCodeDetail());
      #endif
      memcpy(pbIPAddress, &sa.sin_addr, tosNet_IPADDR_LENGTH);
      return 0;
   } else {
      #ifdef tosSocket_DEBUG
         tosLog_debug("tosSocket",
                      "remoteAddress",
                      "Get socket peer address, s=%d, res=(%d:%d:%d)",
                      s,
                      res,
                      tosError_getCode(),
                      tosError_getCodeDetail());
      #endif
      return -1;
   }
}


int tosSocket_port(tosSocket_T s)
{
   struct sockaddr_in sa;
   int len = sizeof(sa);
   int res;

   res = getsockname((tosSocket_system_T)s, (void *) &sa, &len);
#ifdef rt_LIB_Win32_i386
   if (res != 0) errno = EWINSOCKET;
#endif

   /* correct protocol ??? */
   if ((res == 0) && (sa.sin_family != AF_INET)) {
      tosLog_error("tosSocket", "port",
                   "Only IP protcol supported, but is %d",
                   sa.sin_family);
      return -1;
   }

   /* Debug message and return */
   if (res == 0) {
      #ifdef tosSocket_DEBUG
         tosLog_debug("tosSocket",
                      "port",
                      "Get socket port, s=%d, port=%d, res=(0:%d:%d)",
                      s,
                      ntohs(sa.sin_port),
                      tosError_getCode(),
                      tosError_getCodeDetail());
      #endif
      return ntohs(sa.sin_port);
   } else {
      #ifdef tosSocket_DEBUG
         tosLog_debug("tosSocket",
                      "port",
                      "Get socket port, s=%d, res=(%d:%d:%d)",
                      s,
                      res,
                      tosError_getCode(),
                      tosError_getCodeDetail());
      #endif
      return -1;
   }
}


int tosSocket_remotePort(tosSocket_T s)
{
   struct sockaddr_in sa;
   int len = sizeof(sa);
   int res;

   tosError_reset();
   res = getpeername((tosSocket_system_T)s, (void *) &sa, &len);
#ifdef rt_LIB_Win32_i386
   if (res != 0) errno = EWINSOCKET;
#endif

   /* correct protocol ??? */
   if ((res == 0) && (sa.sin_family != AF_INET)) {
      tosLog_error("tosSocket", "reomtePort",
                   "Only IP protcol supported, but is %d",
                   sa.sin_family);
      return -1;
   }

   /* Debug message and return */
   if (res == 0) {
      #ifdef tosSocket_DEBUG
         tosLog_debug("tosSocket",
                      "remotePort",
                      "Get socket peer port, s=%d, port=%d, res=(0:%d:%d)",
                      s,
                      ntohs(sa.sin_port),
                      tosError_getCode(),
                      tosError_getCodeDetail());
      #endif
      return ntohs(sa.sin_port);
   } else {
      #ifdef tosSocket_DEBUG
         tosLog_debug("tosSocket",
                      "remotePort",
                      "Get socket peer port, s=%d, res=(%d:%d:%d)",
                      s,
                      res,
                      tosError_getCode(),
                      tosError_getCodeDetail());
      #endif
      return -1;
   }
}

