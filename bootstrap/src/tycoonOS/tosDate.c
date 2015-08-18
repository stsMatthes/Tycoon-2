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
  $File: //depot/tycoon2/stsmain/bootstrap/src/tycoonOS/tosDate.c $ $Revision: #4 $ $Date: 2003/11/03 $ Andreas Gawecki, Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Time and Date interface.
*/

#include "tos.h"
#include "tosLog.h"
#include "tosDate.h"
#include "tosError.h"
#include "tosMutex.h"


#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <locale.h>

#ifdef rt_LIB_Win32_i386
  #include <windows.h>
#endif


/*== Constants / Macros / Variables =======================================*/

#if defined(CLOCKS_PER_SEC)
  #define tosDate_CLOCKS_PER_SEC CLOCKS_PER_SEC
#else
  #if defined(CLK_TCK)
    #define tosDate_CLOCKS_PER_SEC CLK_TCK
  #else
    #error
  #endif
#endif

#if   defined(rt_LIB_Nextstep_i386) || defined(__BORLANDC__)
  /* borland C has problems accessing _tz... from a DLL */
  static int tosDate_timezone = 0;
  static char *tosDate_tzname[] = {"GMT", NULL};

#elif defined(rt_LIB_Win32_i386)
  #define tosDate_timezone (int)_timezone
  #define tosDate_tzname _tzname

#else
  #define tosDate_timezone (int)timezone
  #define tosDate_tzname tzname
#endif


#if defined(rt_LIB_Win32_i386)
  static tosMutex_T mxDate; /* needed for localtime */
#endif

/*== Timestamps ===========================================================*/

Long tosDate_time(void)
  {
  return (Long) time(NULL);
  }

Long tosDate_clock(void)
  {
  return (Long) clock();
  }

long tosDate_clocksPerSecond(void)
  {
  return tosDate_CLOCKS_PER_SEC;
  }


/*== Time structure manipulation ==========================================*/

void tosDate_normalize(struct tosDate_tm *ptm)
{
  /*
  If tm_isdst is negative, the correct timezone is determined and
  the components are not adjusted:
  */
  ptm->tm_isdst = -1;
  mktime(ptm);
}


void tosDate_fromTime(struct tosDate_tm *ptm, Long lTime)
{
  tosDate_time_t t = (tosDate_time_t) lTime;

#ifdef rt_LIB_Win32_i386
  struct tosDate_tm *newtime;
  /* sync needed (newtime is a static global buffer) */
  tosMutex_lock(&mxDate);
  newtime = localtime(&t);
  *ptm = *newtime;
  tosMutex_unlock(&mxDate);
#else
  localtime_r(&t, ptm);
#endif
}


int tosDate_fromString(struct tosDate_tm *ptm, char *buf, char *fmt)
{
#if defined(rt_LIB_Nextstep_i386) || defined(rt_LIB_Win32_i386)
  tosLog_error("tosDate",
               "fromString",
               "Operating system function 'strptime' not available");

  #ifdef rt_LIB_Win32_i386
    SetLastError(120); /* not implemented */
    errno = EWIN32API;
  #endif
  return -1;

#else
  if (!strptime(buf, fmt, ptm)) {
     /* General convention in tycoonOS: set errno on error */
     errno = EINVAL;
     return -1;
  }
  tosDate_normalize(ptm);
  return 0;
#endif
}


Long tosDate_asTime(struct tosDate_tm *ptm)
{
  return (Long) mktime(ptm);
}


/*== Time structure as handle =============================================*/

int tosDate_sizeOfHandle() { return sizeof(struct tosDate_tm); }

void tosDate_setHandle(struct tosDate_tm *ptm,
                       int year, int month, int day,
                       int hour, int minute, int second,
                       int offset,
                       int isDST)
{
  ptm->tm_sec = second;
  ptm->tm_min = minute;
  ptm->tm_hour = hour;
  ptm->tm_mday = day;
  ptm->tm_mon = month-1;
  ptm->tm_isdst = isDST;

  /* years since 1900 */
  ptm->tm_year = year-1900;

  /* honour offset: */
  ptm->tm_sec -= (offset - tosDate_timezone);

  /* fill in mday, yday: */
  tosDate_normalize(ptm);
}


/*== Retrieving date and time values ======================================*/

int tosDate_format(struct tosDate_tm *ptm, char *fmt, char *buffer, int n)
{
  return strftime(buffer, n, fmt, ptm);
}

int   tosDate_offset(struct tosDate_tm *ptm) { return tosDate_timezone; }
char *tosDate_zone  (struct tosDate_tm *ptm) { return tosDate_tzname[0]; }

int tosDate_second(struct tosDate_tm *t)  { return t->tm_sec; }
int tosDate_minute(struct tosDate_tm *t)  { return t->tm_min; }
int tosDate_hour  (struct tosDate_tm *t)  { return t->tm_hour; }

int tosDate_day  (struct tosDate_tm *t)  { return t->tm_mday; }
int tosDate_month(struct tosDate_tm *t)  { return t->tm_mon+1; }
int tosDate_year (struct tosDate_tm *t)
{
  /* years since 1900 */
  return 1900+t->tm_year;
}

int tosDate_weekDay(struct tosDate_tm *t)  { return t->tm_wday; }
int tosDate_yearDay(struct tosDate_tm *t)  { return t->tm_yday+1; }

int tosDate_isDST(struct tosDate_tm *t) { return t->tm_isdst; }


/*== Initializing =========================================================*/

void tosDate_init(void) {

    /* Set time zone from TZ environment variable. If TZ is not set,
     * the operating system is queried to obtain the default value
     * for the variable.
     */
#ifdef rt_LIB_Win32_i386
    Bool fSuccess;
    _tzset();
    fSuccess = tosMutex_init(&mxDate);
    assert(fSuccess == 0);
#else
    setlocale(LC_ALL, "POSIX");
    tzset();
#endif
}

