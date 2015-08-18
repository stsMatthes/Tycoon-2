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
 * Copyright 1993, 1995 Christopher Seiwald.
 *
 * This file is part of Jam - see jam.c for Copyright information.
 */

# include "jam.h"
# include "lists.h"
# include "search.h"
#ifdef FATFS
# include "timestam.h"
#else
# include "timestamp.h"
#endif
# include "filesys.h"
# include "variable.h"
# include "newstr.h"

/*
 * search.c - find a target along $(SEARCH) or $(LOCATE) 
 */

char *
search( target, time )
char	*target;
time_t	*time;
{
	FILENAME f[1];
	LIST	*varlist;
	char	buf[ MAXJPATH ];

	/* Parse the filename */

	file_parse( target, f );

	f->f_grist.ptr = 0;
	f->f_grist.len = 0;

	if( varlist = var_get( "LOCATE" ) )
	{
	    f->f_root.ptr = varlist->string;
	    f->f_root.len = strlen( varlist->string );

	    file_build( f, buf, 1 );

	    if( DEBUG_SEARCH )
		printf( "locate %s: %s\n", target, buf );

	    timestamp( buf, time );

	    return newstr( buf );
	}
	else if( varlist = var_get( "SEARCH" ) )
	{
	    while( varlist )
	    {
		f->f_root.ptr = varlist->string;
		f->f_root.len = strlen( varlist->string );

		file_build( f, buf, 1 );

		if( DEBUG_SEARCH )
		    printf( "search %s: %s\n", target, buf );

		timestamp( buf, time );

		if( *time )
		    return newstr( buf );

		varlist = list_next( varlist );
	    }
	}

	/* Look for the obvious */
	/* This is a questionable move.  Should we look in the */
	/* obvious place if SEARCH is set? */

	f->f_root.ptr = 0;
	f->f_root.len = 0;

	file_build( f, buf, 1 );

	if( DEBUG_SEARCH )
	    printf( "search %s: %s\n", target, buf );

	timestamp( buf, time );

	return newstr( buf );
}
