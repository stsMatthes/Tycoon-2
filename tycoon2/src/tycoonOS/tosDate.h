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
  tosDate.h 1.0 final  22-OCT-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  Time and Date interface.
*/

#ifndef TOSDATE_H
#define TOSDATE_H


#include "tos.h"
#include <time.h>


#ifdef __cplusplus
extern "C" {
#endif


/*== Timestamps ===========================================================*/

extern Long tosDate_time(void);
extern Long tosDate_clock(void);
extern long tosDate_clocksPerSecond(void);


/*== Time structure manipulation ==========================================*/

#define tosDate_tm     tm
#define tosDate_time_t time_t

extern void tosDate_normalize(struct tosDate_tm *ptm);

extern void tosDate_fromTime(struct tosDate_tm *ptm, Long lTime);
extern Long tosDate_asTime(struct tosDate_tm *ptm);

extern int tosDate_fromString(struct tosDate_tm *ptm, char *buf, char *fmt);


/*== Time structure as handle =============================================*/

extern int tosDate_sizeOfHandle(void);

extern void tosDate_setHandle(struct tosDate_tm *ptm,
                              int year, int month, int day,
                              int hour, int minute, int second,
                              int offset,
                              int isDST);


/*== Retrieving date and time values ======================================*/

extern int tosDate_format(struct tosDate_tm *ptm,
                                       char *fmt,
                                       char *buffer,
                                        int  n);

extern int   tosDate_offset(struct tosDate_tm *ptm);
extern char *tosDate_zone  (struct tosDate_tm *ptm);

extern int tosDate_second(struct tosDate_tm *t);
extern int tosDate_minute(struct tosDate_tm *t);
extern int tosDate_hour  (struct tosDate_tm *t);

extern int tosDate_day  (struct tosDate_tm *t);
extern int tosDate_month(struct tosDate_tm *t);
extern int tosDate_year (struct tosDate_tm *t);

extern int tosDate_weekDay(struct tosDate_tm *t);
extern int tosDate_yearDay(struct tosDate_tm *t);

extern int tosDate_isDST(struct tosDate_tm *t);


/*== Initializing =========================================================*/

extern void tosDate_init(void);


#ifdef __cplusplus
}
#endif

#endif /* TOSDATE_H */
