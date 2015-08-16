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
  $File: //depot/tycoon2/stsmain/tycoon2/src/tycoonOS/tosNet.c $ $Revision: #4 $ $Date: 2003/11/01 $ Andreas Gawecki, Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  We support IP (Internet Protocol) only.
*/


#include <errno.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
  #include <unistd.h>
  #include <pthread.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <sys/param.h>
  #include <netdb.h>
#endif


#include "tos.h"
#include "tosNet.h"
#include "tosLog.h"
#include "tosError.h"
#include "tosSystem.h"


/*== Constants ============================================================*/

int tosNet_getIPAddrLen(void) {
    return tosNet_IPADDR_LENGTH;
}

int tosNet_getMaxHostNameLen(void) {
#ifdef rt_LIB_Win32_i386
    return 256;
#else
    return MAXHOSTNAMELEN;
#endif
}


/*== Obtain host informations =============================================*/

int tosNet_getHostName(char * pszName, int cbName)
{
  int res;
  res = gethostname(pszName, cbName);

#ifdef rt_LIB_Win32_i386
  if (res != 0)
     errno = EWINSOCKET;
#endif

  #ifdef tosNet_DEBUG
     tosLog_debug("tosNet",
                  "getHostName",
                  "Host=%s, res=(%d:%d:%d)",
                  pszName,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}


int tosNet_getHostByName(const char *pszName,
                         Byte       *pbAddr,
                         int         cbAddr)
{
    /*
     * Do not modify following buffer, or use the thread-safe
     * function gethostbyname_r
     */
  struct hostent *hp;

  if (cbAddr != tosNet_IPADDR_LENGTH) {
     errno = EINVAL;
     return -1;
  }

  if ((hp = gethostbyname(pszName)) == NULL)  {
#ifdef rt_LIB_Win32_i386
     errno = EWINSOCKET;
#endif
     return -1;
  }

  if (hp->h_length != tosNet_IPADDR_LENGTH) {
     tosLog_error("tosNet", "getHostByName",
                  "Invalid IP address length returned: %d",
                  hp->h_length);
     return -1;
  }

  #ifdef tosNet_DEBUG
     tosLog_debug("tosNet",
                  "getHostByName",
                  "Host=%s, IP=%s, res=(%d:%d)",
                  pszName,
                  hp->h_addr,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  memcpy(pbAddr, hp->h_addr, hp->h_length);
  return 0;
}


/*== Init =================================================================*/

void tosNet_init(void)
{

#ifdef rt_LIB_Win32_i386

  WORD wVersionRequested;
  WSADATA wsaData;
  int err;

  if (tosSystem_getID() == tosSystem_ID_WIN95)
     wVersionRequested = MAKEWORD( 1, 1 );
  else
     wVersionRequested = MAKEWORD( 2, 0 );

  err = WSAStartup( wVersionRequested, &wsaData );
  if ( err != 0 ) {
     tosLog_error("tosNet",
                  "init",
                  "WinSocket DLL Version %d.%d not found",
                  LOBYTE(wVersionRequested),
                  HIBYTE(wVersionRequested));
     return;
  }

  /*
   * Confirm that the WinSock DLL supports the requested version.
   */
  if ( LOBYTE( wsaData.wVersion ) != LOBYTE(wVersionRequested) ||
       HIBYTE( wsaData.wVersion ) != HIBYTE(wVersionRequested)) {
     tosLog_error("tosNet",
                  "init",
                  "WinSocket DLL Version %d.%d not found",
                  LOBYTE(wVersionRequested),
                  HIBYTE(wVersionRequested));
     WSACleanup( );
     return;
  }

  #ifdef tos_DEBUG
     tosLog_debug("tosNet", 
		  "init",
		  "Binding WinSocket DLL Version %d.%d\n",
		  LOBYTE(wsaData.wVersion),
		  HIBYTE(wsaData.wVersion));
  #endif

#endif
}

