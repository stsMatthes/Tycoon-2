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
  tosSecurity.c 1.0        11-JAN-1999  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Support for a very basicly security support.

  Status: No ready for win32
*/

#include <errno.h>
#include <string.h>

#ifdef rt_LIB_Win32_i386
  #include <windows.h>
#else
  #include <sys/types.h>
  #include <unistd.h>
  #include <grp.h>
  #include <pwd.h>
#endif


#include "tosSecurity.h"
#include "tos.h"
#include "tosLog.h"
#include "tosError.h"
#include "tosFilename.h"


#ifdef __cplusplus
extern "C" {
#endif


/*== Basic types ==========================================================*/

#ifdef rt_LIB_WinNT_i386
  typedef PSID tosSecurity_UID;
  typedef PSID tosSecurity_GID;
#else
  typedef uid_t tosSecurity_UID;
  typedef gid_t tosSecurity_GID;
#endif


/*== Basic type conversion ================================================*/

#ifdef rt_LIB_Win32_i386

  static int __SIDToString(PSID sid, char *res, Int *sizeOfRes)
  {
      SID_NAME_USE snu;
      DWORD sizeDom = 0;
      char  sDomain[120];

      sizeDom = sizeof(sDomain);
      if (!LookupAccountSid(NULL, sid, res, sizeOfRes, sDomain, &sizeDom, &snu)) {
         res[0] = 0;
         return -1;
      }
      return 0;
  }

  static int __stringToSID(char *account, PSID *res, Int *sizeOfRes)
  {
      SID_NAME_USE snu;
      DWORD sizeDom = 0;
      char  sDomain[120];

      sizeDom = sizeof(sDomain);
      if (!LookupAccountName(NULL, account, res, sizeOfRes, sDomain, &sizeDom, &snu)) {
         return -1;
      }
      return 0;
  }

#endif


Int tosSecurity_uidToString(tosSecurity_SID *uid,
                            char            *resUid,
                            Int             *sizeOfRes)
{
  Int res = 0;
  
#ifdef rt_LIB_Win32_i386
  res = __SIDToString((PSID)uid, resUid, sizeOfRes);
  if (res != 0)
     errno = EWIN32API;
#else
  struct passwd  pwd;
  char   buffer[512];

#ifdef rt_LIB_Linux_i386
  struct passwd *resPwd;
#endif

  *resUid = 0;
#ifdef rt_LIB_Linux_i386
  if (getpwuid_r((tosSecurity_UID)*uid, &pwd, buffer, 512, &resPwd) == 0)
#else
  if (getpwuid_r((tosSecurity_UID)*uid, &pwd, buffer, 512) != NULL)
#endif
  {
     if (strlen(pwd.pw_name)+1 > *sizeOfRes) {
        *sizeOfRes = strlen(pwd.pw_name)+1;
        res        = -1;
        errno      = ERANGE;
     }
     else
        strcpy(resUid, pwd.pw_name);
  }
  else res = -1;
#endif

  #ifdef tosSecurity_DEBUG
     tosLog_debug("tosSecurity",
                  "uidToString",
                  "UID: %d ==> %s, res=(%d:%d:%d)",
                  *uid,
                  resUid,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}


Int tosSecurity_gidToString(tosSecurity_SID *gid,
                            char            *resGid,
                            Int             *sizeOfRes)
{
  Int res = 0;

#ifdef rt_LIB_Win32_i386
  res = __SIDToString((PSID)gid, resGid, sizeOfRes);
  if (res != 0)
     errno = EWIN32API;
#else
  struct group  grp;
  char   buffer[512];

#ifdef rt_LIB_Linux_i386
  struct group *resGrp;
#endif

  *resGid = 0;
#ifdef rt_LIB_Linux_i386
  if (getgrgid_r((tosSecurity_GID)*gid, &grp, buffer, 512, &resGrp) == 0)
#else
  if (getgrgid_r((tosSecurity_GID)*gid, &grp, buffer, 512) != NULL)
#endif
  {
     if (strlen(grp.gr_name)+1 > *sizeOfRes) {
        *sizeOfRes = strlen(grp.gr_name)+1;
        res        = -1;
        errno      = ERANGE;
     }
     else
        strcpy(resGid, grp.gr_name);
  }
  else res = -1;
#endif

  #ifdef tosSecurity_DEBUG
     tosLog_debug("tosSecurity",
                  "gidToString",
                  "GID: %d ==> %s, res=(%d:%d:%d)",
                  *gid,
                  resGid,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}


Int tosSecurity_uidFromString(char            *uname,
                              tosSecurity_SID *resUid,
                              Int             *sizeOfSID)
{
  int res = 0;

#ifdef rt_LIB_Win32_i386
  res = __stringToSID(uname, &((PSID)resUid), sizeOfSID);
  if (res != 0)
     errno = EWIN32API;
#else
  struct passwd * pwd;
  if (!(pwd = getpwnam(uname)))
     res = -1;
  else {
     if (*sizeOfSID != sizeof(uid_t)) {
        *sizeOfSID = sizeof(uid_t);
        res        = -1;
        errno      = ERANGE;
     }
     else
        *resUid = (tosSecurity_UID)(pwd->pw_uid);
  }
#endif

  #ifdef tosSecurity_DEBUG
     if (resUid)
        tosLog_debug("tosSecurity",
                     "uidFromString",
                     "UID: %s ==> %d, res=(%d:%d:%d)",
                     uname,
                     *resUid,
                     res,
                     tosError_getCode(),
                     tosError_getCodeDetail());
     else
        tosLog_debug("tosSecurity",
                     "uidFromString",
                     "Detecting correct size of uid: %d",
                     *sizeOfSID);
  #endif
  return res;
}

Int tosSecurity_gidFromString(char            *gname,
                              tosSecurity_SID *resGid,
                              Int             *sizeOfSID)
{
  Int res = 0;

#ifdef rt_LIB_Win32_i386
  res = __stringToSID(gname, &((PSID)resGid), sizeOfSID);
  if (res != 0)
     errno = EWIN32API;
#else
  struct group * grp;
  if (!(grp = getgrnam(gname)))
     res = -1;
  else {
     if (*sizeOfSID != sizeof(gid_t)) {
        *sizeOfSID = sizeof(gid_t);
        res        = -1;
        errno      = ERANGE;
     }
     else
        *resGid = (tosSecurity_GID)(grp->gr_gid);
  }
#endif

  #ifdef tosSecurity_DEBUG
     if (resGid)
        tosLog_debug("tosSecurity",
                     "gidFromString",
                     "GID: %s ==> %d, res=(%d:%d:%d)",
                     gname,
                     *resGid,
                     res,
                     tosError_getCode(),
                     tosError_getCodeDetail());
     else
        tosLog_debug("tosSecurity",
                     "gidFromString",
                     "Detecting correct size of gid: %d",
                     *sizeOfSID);
  #endif
  return res;
}


/*== Current access id tokens =============================================*/

Int tosSecurity_getCurrentUID(tosSecurity_SID *resUid,
                              Int             *sizeOfSID)
{
  Int res = 0;

#ifdef rt_LIB_Win32_i386
  /*
   * Gets the owner of the process access token.
   */
  unsigned char tubuf[1024];
  TOKEN_USER *ptu = (TOKEN_USER *) tubuf;
  DWORD cbDummy;
  HANDLE hAccessToken;

  if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hAccessToken)) {
     res   = -1;
     errno = EWIN32API;
  }
  else {
    if (!GetTokenInformation(hAccessToken, TokenUser, ptu, sizeof tubuf, &cbDummy)) {
       res   = -1;
       errno = EWIN32API;
    }
    else {
       if (*sizeOfSID != GetLengthSid(ptu->User.Sid)) {
          *sizeOfSID = GetLengthSid(ptu->User.Sid);
          res        = -1;
          errno      = ERANGE;
       }
       else
          CopySid(*sizeOfSID, (PSID)resUid, ptu->User.Sid);
    }
    CloseHandle(hAccessToken);
  }

#else
  if (*sizeOfSID != sizeof(uid_t)) {
      *sizeOfSID = sizeof(uid_t);
      res        = -1;
      errno      = ERANGE;
  }
  else
      *resUid = (tosSecurity_SID)getuid();
#endif

  #ifdef tosSecurity_DEBUG
     if (resUid)
        tosLog_debug("tosSecurity",
                     "getCurrentUID",
                     "Current user-id is %d, res=(%d:%d:%d)",
                     *resUid,
                     res,
                     tosError_getCode(),
                     tosError_getCodeDetail());
     else
        tosLog_debug("tosSecurity",
                     "getCurrentUID",
                     "Detecting correct size of uid: %d",
                     *sizeOfSID);
  #endif
  return res;
}


Int tosSecurity_getCurrentGID(tosSecurity_SID *resGid,
                              Int             *sizeOfSID)
{
  Int res = 0;

#ifdef rt_LIB_Win32_i386
  /*
   * Gets the primary group of the process access token.
   */
  unsigned char tubuf[1024];
  TOKEN_PRIMARY_GROUP *ptu = (TOKEN_PRIMARY_GROUP *) tubuf;
  DWORD cbDummy;
  HANDLE hAccessToken;

  if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hAccessToken)) {
     res   = -1;
     errno = EWIN32API;
  }
  else {
    if (!GetTokenInformation(hAccessToken, TokenPrimaryGroup, ptu, sizeof tubuf, &cbDummy)) {
       res   = -1;
       errno = EWIN32API;
    }
    else {
       if (*sizeOfSID != GetLengthSid(ptu->PrimaryGroup)) {
          *sizeOfSID = GetLengthSid(ptu->PrimaryGroup);
          res        = -1;
          errno      = ERANGE;
       }
       else
          CopySid(*sizeOfSID, (PSID)resGid, ptu->PrimaryGroup);
    }
    CloseHandle(hAccessToken);
  }

#else
  if (*sizeOfSID != sizeof(gid_t)) {
      *sizeOfSID = sizeof(gid_t);
      res        = -1;
      errno      = ERANGE;
  }
  else
      *resGid = (tosSecurity_SID)getgid();
#endif
  
  #ifdef tosSecurity_DEBUG
     if (resGid)
        tosLog_debug("tosSecurity",
                     "getCurrentGID",
                     "Current group-id is %d, res=(%d:%d:%d)",
                     *resGid,
                     res,
                     tosError_getCode(),
                     tosError_getCodeDetail());
     else
        tosLog_debug("tosSecurity",
                     "getCurrentGID",
                     "Detecting correct size of gid: %d",
                     *sizeOfSID);
  #endif
  return res;
}


/*== Unix specific functions ==============================================*/

Int tosSecurity_setCurrentUID(tosSecurity_SID *uid)
{
  Int res = 0;

#ifdef rt_LIB_WinNT_i386
  tosLog_error("tosSecurity",
               "setCurrentUID",
               "Not implemented on Win32 API");

  SetLastError(120); /* not implemented */
  errno = EWIN32API;
  res   = -1;
#else
  res = setuid((tosSecurity_UID)*uid);
#endif
  
  #ifdef tosSecurity_DEBUG
     tosLog_debug("tosSecurity",
                  "setCurrentUID",
                  "Set current user-id to %d, res=(%d:%d:%d)",
                  *uid,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}


Int tosSecurity_setCurrentGID(tosSecurity_SID *gid)
{
  Int res = 0;

#ifdef rt_LIB_WinNT_i386
  tosLog_error("tosSecurity",
               "setCurrentGID",
               "Not implemented on Win32 API");

  SetLastError(120); /* not implemented */
  errno = EWIN32API;
  res   = -1;
#else
  res = setgid((tosSecurity_GID)*gid);
#endif
  
  #ifdef tosSecurity_DEBUG
     tosLog_debug("tosSecurity",
                  "setCurrentGID",
                  "Set current group-id to %d, res=(%d:%d:%d)",
                  *gid,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}


/*== Change file access tokens ============================================*/

Int tosSecurity_chown(const char      *path,
                      tosSecurity_SID *owner,
                      tosSecurity_SID *group)
{
  Int res = 0;
  char systemPath[tosFilename_MAXLEN];

  tosFilename_normalize(path, systemPath, tosFilename_MAXLEN);

#ifdef rt_LIB_Win32_i386
  /* Waiting until soon ... */
  tosLog_error("tosSecurity",
               "chown",
               "File security not yet implemented on Windows NT");

  SetLastError(120); /* not implemented */
  errno = EWIN32API;
  res   = -1;
#else
  res = chown(systemPath, (tosSecurity_UID)*owner, (tosSecurity_GID)*group);
#endif

  #ifdef tosSecurity_DEBUG
     tosLog_debug("tosSecurity",
                  "chown",
                  "Change file owner of %s, user=%u, group=%u, res=(%d:%d:%d)",
                  systemPath,
                  *owner,
                  *group,
                  res,
                  tosError_getCode(),
                  tosError_getCodeDetail());
  #endif
  return res;
}


/*== Init =================================================================*/

void tosSecurity_init(void)
{
#ifdef tos_DEBUG
  tosSecurity_SID *id;
  int sizeOfSID;

  char user[120];
  char group[120];
  int size = 120;

  sizeOfSID = 0;
  tosSecurity_getCurrentUID(NULL, &sizeOfSID);
  id = malloc(sizeOfSID);
  tosSecurity_getCurrentUID(id, &sizeOfSID);
  tosSecurity_uidToString(id, user, &size);
  free(id);

  sizeOfSID = 0;
  tosSecurity_getCurrentGID(NULL, &sizeOfSID);
  id = malloc(sizeOfSID);
  tosSecurity_getCurrentGID(id, &sizeOfSID);
  tosSecurity_gidToString(id, group, &size);
  free(id);

  fprintf(stderr, "Current user and group: %s.%s\n", user, group);

  /* If we got security errors (Win95) ... */
  tosError_reset();
#endif
}

