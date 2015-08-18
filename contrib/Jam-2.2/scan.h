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

/*
 * scan.h - the jam yacc scanner
 *
 * External functions:
 *
 *	yyerror( char *s ) - print a parsing error message
 *	yyfparse( char *s ) - scan include file s
 *	yylex() - parse the next token, returning its type
 *	yymode() - adjust lexicon of scanner
 *	yyparse() - declaration for yacc parser
 *	yyanyerrors() - indicate if any parsing errors occured
 *
 * The yymode() function is for the parser to adjust the lexicon of the
 * scanner.  Aside from normal keyword scanning, there is a mode to
 * handle action strings (look only for the closing }) and a mode to 
 * ignore most keywords when looking for a punctuation keyword.  This 
 * allows non-punctuation keywords to be used in lists without quoting.
 */

/*
 * YYSTYPE - value of a lexical token
 */

# define YYSTYPE YYSYMBOL

typedef struct _YYSTYPE {
	int		type;
	char		*string;
	PARSE		*parse;
	LIST		*list;
	int		number;
} YYSTYPE;

extern YYSTYPE yylval;

void yyerror();
void yyfparse();
int yylex();
void yymode();
int yyparse();
int yyanyerrors();

# define SCAN_NORMAL	0	/* normal parsing */
# define SCAN_STRING	1	/* look only for matching } */
# define SCAN_PUNCT	2	/* only punctuation keywords */
