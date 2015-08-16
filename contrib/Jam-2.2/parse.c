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
# include "parse.h"
# include "scan.h"
# include "newstr.h"

/*
 * parse.c - make and destroy parse trees as driven by the parser
 */

static PARSE *yypsave;

void
parse_file( f )
char *f;
{
	/* Suspend scan of current file */
	/* and push this new file in the stream */

	yyfparse(f);

	/* Now parse each block of rules and execute it. */
	/* Execute it outside of the parser so that recursive */
	/* calls to yyrun() work (no recursive yyparse's). */

	for(;;)
	{
	    LOL l;
	    PARSE *p;

	    lol_init( &l );

	    yypsave = 0;

	    if( yyparse() || !( p = yypsave ) )
		break;

	    (*(p->func))( p, &l );

	    parse_free( p );
	}
}

void
parse_save( p )
PARSE *p;
{
	yypsave = p;
}

PARSE *
parse_make( func, left, right, string, string1, llist, rlist, num )
void	(*func)();
PARSE	*left;
PARSE	*right;
char	*string;
char	*string1;
LIST	*llist;
LIST	*rlist;
int	num;
{
	PARSE	*p = (PARSE *)malloc( sizeof( PARSE ) );

	p->func = func;
	p->left = left;
	p->right = right;
	p->string = string;
	p->string1 = string1;
	p->llist = llist;
	p->rlist = rlist;
	p->num = num;

	return p;
}

void
parse_free( p )
PARSE	*p;
{
	if( p->string )
	    freestr( p->string );
	if( p->string1 )
	    freestr( p->string1 );
	if( p->llist )
	    list_free( p->llist );
	if( p->rlist )
	    list_free( p->rlist );
	if( p->left )
	    parse_free( p->left );
	if( p->right )
	    parse_free( p->right );
	
	free( (char *)p );
}
