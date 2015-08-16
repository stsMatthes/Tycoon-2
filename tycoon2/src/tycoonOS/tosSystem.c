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
  tosSystem.c 1.0 final  08-OCT-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Detection of base host operating system parameters.
*/

#include <stdio.h>
#include <assert.h>
#include <errno.h>

#ifdef rt_LIB_Win32_i386
  #include <windows.h>
#endif


#include "tosSystem.h"
#include "tos.h"
#include "tosError.h"
#include "tosLog.h"


/*== Multiple architecture support ========================================*/

typedef struct {
  String pszName;
  Int    iByteOrder;
} SystemTable;

static Int _hostID = tosSystem_ID_UNKNOWN;

static SystemTable _systemInfo[] = {
  {"Invalid",        -1},
  {"Solaris2_SPARC", tosSystem_BIG_ENDIAN},
  {"Linux_i386",     tosSystem_LITTLE_ENDIAN},
  {"HPUX_PARISC",    tosSystem_BIG_ENDIAN},
  {"Nextstep_i386",  tosSystem_LITTLE_ENDIAN},
  {"WinNT_i386",     tosSystem_LITTLE_ENDIAN},
  {"Win95_i386",     tosSystem_LITTLE_ENDIAN},
  {"Win98_i386",     tosSystem_LITTLE_ENDIAN},
  {"Darwin_PPC",     tosSystem_BIG_ENDIAN},
  {"Darwin_i386",    tosSystem_LITTLE_ENDIAN},
  {"Unknown",        -1}
};


Int tosSystem_getID(void)
{
  return _hostID;
}

String tosSystem_getName(void)
{
  assert((_hostID > 0) && (_hostID <= tosSystem_ID_UNKNOWN));
  return (String) _systemInfo[_hostID].pszName;
}


/*== Byte order ===========================================================*/

Int tosSystem_getByteOrderOf(Int systemId)
{
  assert((systemId > 0) && (systemId <= tosSystem_ID_UNKNOWN));
  return (Int) _systemInfo[systemId].iByteOrder;
}

Int tosSystem_getByteOrder(void)
{
  return tosSystem_getByteOrderOf(_hostID);
}


/*== Initializing =========================================================*/

void tosSystem_init()
{
#if   defined(rt_LIB_Solaris2_SPARC)
  _hostID = tosSystem_ID_SOLARIS2;
  #ifdef tos_DEBUG
     tosLog_debug("tosSystem", "init", "Starting on Host %s\n", _systemInfo[_hostID].pszName);
  #endif
 
#elif defined(rt_LIB_Linux_i386)
  _hostID = tosSystem_ID_LINUX;
  #ifdef tos_DEBUG
     tosLog_debug("tosSystem", "init", "Starting on Host %s\n", _systemInfo[_hostID].pszName);
  #endif

#elif defined(rt_LIB_Darwin_PPC)
  _hostID = tosSystem_ID_MACOSX;
  #ifdef tos_DEBUG
     tosLog_debug("tosSystem", "init", "Starting on Host %s\n", _systemInfo[_hostID].pszName);
  #endif

#elif defined(rt_LIB_Darwin_i386)
  _hostID = tosSystem_ID_MACOSX_386;
  #ifdef tos_DEBUG
     tosLog_debug("tosSystem", "init", "Starting on Host %s\n", _systemInfo[_hostID].pszName);
  #endif

#elif defined(rt_LIB_HPUX_PARISC)
  _hostID = tosSystem_ID_HPUX;
  #ifdef tos_DEBUG
     tosLog_debug("tosSystem", "init", "Starting on Host %s\n", _systemInfo[_hostID].pszName);
  #endif

#elif defined(rt_LIB_Nextstep_i386)
  _hostID = tosSystem_ID_NEXTSTEP;
  #ifdef tos_DEBUG
     tosLog_debug("tosSystem", "init", "Starting on Host %s\n", _systemInfo[_hostID].pszName);
  #endif

#elif defined(rt_LIB_Win32_i386)
   BOOL          rc;
   OSVERSIONINFO osVersion;
   osVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

   rc = GetVersionEx(&osVersion);
   if (rc == 0) {
      tosLog_error("tosSystem", "init", "Error on Win32API call: GetVersionEx");
      errno = EWIN32API;
      tosError_ABORT();
   }

   if (osVersion.dwPlatformId == VER_PLATFORM_WIN32_NT) {
      _hostID = tosSystem_ID_WINNT;
      #ifdef tos_DEBUG
         tosLog_debug("tosSystem", "init",
                 "Starting on Host %s, V%d.%d, Build:  %u, CSD Version:  %s\n",
                 _systemInfo[_hostID].pszName,
                 osVersion.dwMajorVersion,
                 osVersion.dwMinorVersion,
                 osVersion.dwBuildNumber,
                 osVersion.szCSDVersion);
      #endif
   }
   else
     if (osVersion.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
        if ((osVersion.dwMajorVersion == 4) && (osVersion.dwMinorVersion >= 10))
           _hostID = tosSystem_ID_WIN98;
        else
           _hostID = tosSystem_ID_WIN95;

        #ifdef tos_DEBUG
          tosLog_debug("tosSystem", "init",
                 "Starting on Host %s, V%d.%d, Build:  %d.%d.%d\n",
                  _systemInfo[_hostID].pszName,
                  osVersion.dwMajorVersion,
                  osVersion.dwMinorVersion,
                  HIBYTE(HIWORD(osVersion.dwBuildNumber)),
                  LOBYTE(HIWORD(osVersion.dwBuildNumber)),
                  LOWORD(osVersion.dwBuildNumber));
        #endif
     }
     else {
        tosLog_error("tosSystem",
                     "init",
                     "Illegal platform ID (Win32s ?): %d",
                     osVersion.dwPlatformId);
        tosError_ABORT();
     }


#endif
}

