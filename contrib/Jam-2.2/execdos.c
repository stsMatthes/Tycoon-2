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

# if defined ( WIN95 ) || defined ( WIN98 )

# include "jam.h"
# include "execcmd.h"
# include "lists.h"
# include <errno.h>

# include <process.h>

/*
 * execunix.c - execute a shell script on UNIX/WinNT/OS2
 *
 * If $(JAMSHELL) is defined, uses that to formulate execvp()/spawnvp().
 * The default is:
 *
 *	/bin/sh -c %		[ on UNIX ]
 *	cmd.exe /c %		[ on OS2/WinNT ]
 *	command.exe /c %	[ on WIN95 ]
 *
 * Each word must be an individual element in a jam variable value.
 *
 * In $(JAMSHELL), % expands to the command string and ! expands to 
 * the slot number (starting at 1) for multiprocess (-j) invocations.
 * If $(JAMSHELL) doesn't include a %, it is tacked on as the last
 * argument.
 *
 * Don't just set JAMSHELL to /bin/sh or cmd.exe - it won't work!
 *
 * External routines:
 *	execcmd() - launch an async command execution
 * 	execwait() - wait and drive at most one execution completion
 *
 * Internal routines:
 *	onintr() - bump intr to note command interruption
 *
 * 04/08/94 (seiwald) - Coherent/386 support added.
 * 05/04/94 (seiwald) - async multiprocess interface
 * 01/22/95 (seiwald) - $(JAMSHELL) support
 * 06/02/97 (gsar)    - full async multiprocess support for Win32
 */

static void (*istat)( int );
void onintr( int );

/*
 * onintr() - bump intr to note command interruption
 */

static int intr ;

void
onintr( disp )
int disp;
{
	intr++;
	printf( "...interrupted\n" );
}

/*
 * execcmd() - launch an async command execution
 */

void
execcmd( string, func, closure, shell )
char *string;
void (*func)();
void *closure;
LIST *shell;
{
	int ret;
	int rstat;
	char *argv[ MAXARGC + 1 ];	/* +1 for NULL */

	static char *comspec = 0;
	static char *tempfile = 0;
	char *p;

	/* XXX this is questionable practice, since COMSPEC has
	 * a high degree of variability, resulting in Jamfiles
	 * that frequently won't work.  COMSPEC also denotes a shell
	 * fit for interative use, not necessarily/merely a shell
	 * capable of launching commands.  Besides, people can
	 * just set JAMSHELL instead.
	 */
	if ( comspec == 0) {
	  comspec = getenv( "COMSPEC" );

	  if ( comspec == 0) {
	    comspec = "command.com";
	  }
	}

        if (tempfile == 0) {
            char *tempdir;

            if( !( tempdir = getenv( "TEMP" ) ) &&
                !( tempdir = getenv( "TMP" ) ) )
                    tempdir = "\\temp";

	    tempfile = malloc( strlen( tempdir ) + 14 );

	    sprintf(tempfile, "%s\\jamtmp.bat", tempdir);
	}

	/* Trim leading, ending white space */

	while( isspace( *string ) )
		++string;

	p = strchr( string, '\n' );

	while( p && isspace( *p ) )
		++p;

	/* If multi line or too long, write to bat file. */
	/* Otherwise, exec directly. */
	/* Frankly, if it is a single long line I don't think the */
	/* command interpreter will do any better -- it will fail. */

	if( p && *p || strlen( string ) > MAXLINE )
	{
	    FILE *f;

	    /* Write command to bat file. */

	    f = fopen( tempfile, "w" );
	    fputs( string, f );
	    fclose( f );

	    string = tempfile;
	}


	/* Forumulate argv */
	{
        int i = 0;
	    argv[i++] = comspec;
	    argv[i++] = "/C";		/* anything more is non-portable */
            while (*string) {
              char *pt;
              while (isspace(*string)) {
                string++;
              }
              if (!*string) {
				break;
			  }
              pt = strchr(string, ' ');
              argv[i++] = string;
              if (pt) {
                *pt++ = 0;
                string = pt;
              }
              else {
                break;
              }
            }
            argv[i] = 0;
	}

	/* Catch interrupts whenever commands are running. */

       istat = signal( SIGINT, onintr );

	/* Start the command */

	ret = spawnvp( P_WAIT, argv[0], argv );

	if (ret < 0 )
	{
	    perror( "spawn" );
	    exit( EXITBAD );
	}

        signal( SIGINT, istat );

	if ( intr )
	    rstat = EXEC_CMD_INTR;
	else if ( ret != 0 )
	    rstat = EXEC_CMD_FAIL;
	else
	    rstat = EXEC_CMD_OK;

	(*func)( closure, rstat );
}

int execwait()
{
  return(0);
}

# endif

