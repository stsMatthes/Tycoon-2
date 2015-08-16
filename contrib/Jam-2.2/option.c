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
# include "option.h"

/*
 * option.c - command line option processing
 *
 * {o >o
 *  \<>) "Process command line options as defined in <option.h>.
 *		  Return the number of argv[] elements used up by options,
 *		  or -1 if an invalid option flag was given or an argument
 *		  was supplied for an option that does not require one."
 */

int
getoptions(argc, argv, opts, optv)
char **argv;
char *opts;
option *optv;
{
    int i;
    int optc = N_OPTS;

    memset( (char *)optv, '\0', sizeof( *optv ) * N_OPTS );

    for( i = 0; i < argc; i++ )
    {
	char *arg;

	if( argv[i][0] != '-' || !isalpha( argv[i][1] ) )
	    break;

	if( !optc-- )
	{
	    printf( "too many options (%d max)\n", N_OPTS );
	    return -1;
	}

	for( arg = &argv[i][1]; *arg; arg++ )
	{
	    char *f;

	    for( f = opts; *f; f++ )
		if( *f == *arg )
		    break;

	    if( !*f )
	    {
		printf( "Invalid option: -%c\n", *arg );
		return -1;
	    }

	    optv->flag = *f;

	    if( f[1] != ':' )
	    {
		optv++->val = "true";
	    }
	    else if( arg[1] )
	    {
		optv++->val = &arg[1];
		break;
	    }
	    else if( ++i < argc )
	    {
		optv++->val = argv[i];
		break;
	    }
	    else
	    {
		printf( "option: -%c needs argument\n", *f );
		return -1;
	    }
	}
    }

    return i;
}

/*
 * Name: getoptval() - find an option given its character
 */

char *
getoptval( optv, opt, subopt )
option *optv;
char opt;
int subopt;
{
	int i;

	for( i = 0; i < N_OPTS; i++, optv++ )
	    if( optv->flag == opt && !subopt-- )
		return optv->val;

	return 0;
}
