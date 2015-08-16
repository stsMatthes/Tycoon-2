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
  tosError.c 1.0 final  30-JUL-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Error handling of TOS layer
*/


#include "tos.h"
#include "tosError.h"
#include "tosLog.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef rt_LIB_Win32_i386
  #include <windows.h>
#endif


/*== TVM and tycoonOS error management ====================================*/

int tosError_getCode()
{
  return errno;
}

int tosError_getCodeDetail()
{
#ifdef rt_LIB_Win32_i386
  switch (errno) {
    case EWIN32API:
         return (int) GetLastError();
    case EWINSOCKET:
         return (int) WSAGetLastError();
    default:
         return 0;
  }
#else
  return 0;
#endif
}


void tosError_setContext(int errCode, int errCodeDetail)
{
  errno = errCode;
#ifdef rt_LIB_Win32_i386
  switch (errno) {
    case EWIN32API:
         SetLastError(errCodeDetail);
         break;
    case EWINSOCKET:
         WSASetLastError(errCodeDetail);
         break;
    default:
         break;
  }
#endif
}


/*== Context-free TL2 error text utilities ================================*/

void tosError_getText(int   errCode,
                      int   errCodeDetail,
                      char *msg,
                      int   msgSize)
{
#ifdef rt_LIB_Win32_i386
  LPVOID lpMsgBuf;
#endif
  char err[40];

  switch (errCode) {
    case EWIN32API:
#ifdef rt_LIB_Win32_i386
         FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                errCodeDetail,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                (LPTSTR) &lpMsgBuf,
                0,
                NULL
         );
         strncpy(msg, (char *)lpMsgBuf, msgSize - 1);
         LocalFree(lpMsgBuf);
#else
         strncpy(msg, "Win32 API error", msgSize - 1);
#endif
         msg[msgSize] = 0;
         break;
    case EWINSOCKET:
         sprintf(err, "WinSocket error %d", errCodeDetail);
         strncpy(msg, err, msgSize - 1);
         msg[msgSize] = 0;
         break;
    default:
         strncpy(msg, strerror(errno), msgSize - 1);
         msg[msgSize] = 0;
  }
}


void tosError_log(int errCode, int errCodeDetail)
{
#ifdef rt_LIB_Win32_i386
  LPVOID lpMsgBuf;
#endif
  char err[40];

  switch (errCode) {

    case EWIN32API:
#ifdef rt_LIB_Win32_i386
         FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                errCodeDetail,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                (LPTSTR) &lpMsgBuf,
                0,
                NULL
         );
         /* Delete tedious CR-LF */
         ((char *)lpMsgBuf)[strlen(lpMsgBuf)-2] = 0;
         tosLog_error("Win32API", "error", "%d - %s", (char *)lpMsgBuf);
         LocalFree(lpMsgBuf);
#else
         tosLog_error("Win32API", "error", "%d", errCodeDetail);
#endif
         break;

    case EWINSOCKET:
         sprintf(err, "%d", errCodeDetail);
         tosLog_error("WinSocket", "error", err);
         break;
    default:
         tosLog_error("c-runtime", "error", strerror(errno));
  }
}

