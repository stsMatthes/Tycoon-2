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
  tosNet.h 1.0 final  24-OCT-1998  Andre Willomat

  Interface to Portable Tycoon-2 operating system (TycoonOS)

  We support IP (Internet Protocol) only.
*/


#ifndef TOSNET_H
#define TOSNET_H


#include "tos.h"


#ifdef __cplusplus
extern "C" {
#endif


/*== Constants ============================================================*/

#define tosNet_IPADDR_LENGTH 4

extern int tosNet_getIPAddrLen(void);
extern int tosNet_getMaxHostNameLen(void);


/*== Obtain host informations =============================================*/

extern int tosNet_getHostName(char * pszName, int cbName);

extern int tosNet_getHostByName(const char *pszName,
                                Byte       *pbAddr,
                                int         cbAddr);

/*== Init =================================================================*/

extern void tosNet_init(void);


#ifdef __cplusplus
}
#endif

#endif /* TOSNET_H */

