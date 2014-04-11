%{
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 *
 */
/* $Id: scan.l,v 1.1 2000/09/21 21:35:06 alaffin Exp $ */
/*
 * Lex rules for input processing.
 *
 * This handles all of the input parsing.  The rules liste here properly
 * store or process the pertenant input data in the proper ways.  The rules
 * for the various patterns maintains a "state" to determine if corrupted
 * input is seen (%Start keys + internal ones that only flag errors).
 *
 * See scanner.c for routines called from the actions.
 *
 * States:
 *	SCAN_OUTSIDE
 *		start-up state, inbetween tests
 *	SCAN_RTSKEY			valid from SCAN_OUTSIDE
 *		from rts_keyword_start to _end
 *		accompanied by lex KEY state.
 *	SCAN_TSTKEY			valid from SCAN_OUTSIDE
 *		test_start to test_output or test_end,
 *		execution_status to test_end
 *		accompanied by lex KEY state.
 *	SCAN_OUTPUT
 *		test_output to execution_status.
 *		accompanied by lex OUT or CUTS states.
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scan.h"
#include "reporter.h"
#include "symbol.h"
#include "tag_report.h"

int scan_mode = SCAN_OUTSIDE;	/* current mode */
char *key, *cont;	/* keyword pieces */
SYM keys=NULL;		/* stored keywords */
SYM ctag=NULL;		/* temporary - for storing current tag's info */
SYM alltags;		/* entire tag database.  set to scanner 'tags' param.*/
SYM k;			/* temporary sym pointer -- for key removal */
char info[KEYSIZE];	/* tmp string for inserting line numbers */
static int test_output( SYM, SYM);
static int check_mode(int, int, ...);

/*
 * Lex Definitions:
 * UI	Unsigned Integer
 * A	Alphabetic
 * W	"Word" characters (Alpha, Numeric, Hyphens, Underscores)
 * S    Space characters
 */
%}

%option noc++
%option noinput
%option nolex-compat
%option nounput
%option yylineno

UI      [0-9]+
A       [a-zA-Z]+
W	[a-zA-Z0-9_-]+
S	[ \t]+

%Start KEY OUT CUTS
%%
^<<<rts_keyword_start>>>$	{
    BEGIN KEY;
    check_mode(scan_mode, SCAN_OUTSIDE, 0);
    scan_mode = SCAN_RTSKEY;

    /* remove any keys that exist right now */
    if(keys != NULL)
	sym_rm(keys, RM_KEY | RM_DATA);
    /* start a new table of keys */
    keys = sym_open(0, 0, 0);
    return(KW_START);
    /* NOTREACHED */
}

^<<<rts_keyword_end>>>$		{
    BEGIN 0;
    check_mode(scan_mode, SCAN_RTSKEY, 0);
    scan_mode = SCAN_OUTSIDE;
#ifdef DEBUGGING
    DEBUG(D_SCAN_LEX, 10) {
	printf("RTS Keywords:\n");
	sym_dump_s(keys, 0);
    }
#endif
    /* remove _RTS key, if it exists, before replacing it */
    if( (k=(SYM)sym_get(alltags, "_RTS")) != NULL) {
	sym_rm(k, RM_KEY | RM_DATA);
    }

    sym_put(alltags, "_RTS", (void *)keys, PUT_REPLACE);
    keys = NULL;

    return(KW_END);
    /* NOTREACHED */
}

^<<<test_start>>>$		{
    BEGIN KEY;
    check_mode(scan_mode, SCAN_OUTSIDE, 0);
    scan_mode = SCAN_TSTKEY;

    /*
     * set up new "tag" and "keys" tables
     * to put the new data into.
     */

    /* remove any keys that exist right now */
    if(keys != NULL)
	sym_rm(keys, RM_KEY | RM_DATA);
    keys = sym_open(0, 0, 0);

    sprintf(info, "%d", yylineno);
    sym_put(keys, "_Start_line", strdup(info), 0);

    /* remove any tag info that exists right now */
    if(ctag != NULL)
	sym_rm(ctag, RM_KEY | RM_DATA);
    ctag = sym_open(0, 0, 0);

    return(TEST_START);
    /* NOTREACHED */
}

^<<<test_output>>>$		{
    BEGIN OUT;
    check_mode(scan_mode, SCAN_TSTKEY, 0);
    scan_mode = SCAN_OUTPUT;

    test_output(ctag, keys);

    return(TEST_OUTPUT);
    /* NOTREACHED */
}

^<<<execution_status>>>$	{
    BEGIN KEY;
    check_mode(scan_mode, SCAN_TSTKEY, SCAN_OUTPUT, 0);
    scan_mode = SCAN_TSTKEY;
    return(EXEC_STATUS);
    /* NOTREACHED */
}

^<<<test_end>>>$		{
    BEGIN 0;
    check_mode(scan_mode, SCAN_TSTKEY, 0);
    scan_mode = SCAN_OUTSIDE;

    sprintf(info, "%d", yylineno);

    sym_put(keys, "_End_line", strdup(info), 0);
#ifdef DEBUGGING
    DEBUG(D_SCAN_LEX, 10) {
	printf("Tag's Keywords:\n");
	sym_dump_s(keys, 0);
    }
#endif
    test_end(alltags, ctag, keys);
    ctag = keys = NULL;

    return(TEST_END);
    /* NOTREACHED */
}

<KEY>[a-zA-Z_-]+=\"[^\"\n]+\"	{
    key = yytext;
    cont = strchr(yytext, '=');
    *cont++ = '\0';
    if(*cont == '"') cont++;
    if(yytext[yyleng-1] == '"')
	yytext[yyleng-1] = '\0';
#ifdef DEBUGGING
    DEBUG(D_SCAN_LEX, 5)
	printf("A quoted keyword: %s = %s\n", key, cont);
#endif
    sym_put(keys, key, strdup(cont), 0);

    return(KEYWORD_QUOTED);
    /* NOTREACHED */
}

<KEY>[a-zA-Z_-]+=[^\t \n]+	{
    key = yytext;
    cont = strchr(yytext, '=');
    *cont++ = '\0';
#ifdef DEBUGGING
    DEBUG(D_SCAN_LEX, 5)
	printf("A keyword: %s = %s\n", key, cont);
#endif
    sym_put(keys, key, strdup(cont), 0);

    return(KEYWORD);
    /* NOTREACHED */
}

<KEY>[ \t\n]*			{
    return(SPACE);
    /* NOTREACHED */
}

<OUT>^.+$			{
#ifdef DEBUGGING
    DEBUG(D_SCAN_LEX, 5)
	printf("TEXT_LINE: %s\n", yytext);
#endif

    return(TEXT_LINE);
    /* NOTREACHED */
}

<CUTS>^{W}{S}{UI}{S}{A}{S}":"	   {
#ifdef DEBUGGING
    DEBUG(D_SCAN_LEX, 5)
	printf("CUTS Result: %s\n", yytext);
#endif
    cuts_testcase(ctag, keys);

    return(CUTS_RESULT);
    /* NOTREACHED */
}

<CUTS>^{W}{S}{UI}-{UI}{S}{A}{S}":" {
#ifdef DEBUGGING
    DEBUG(D_SCAN_LEX, 5)
	printf("CUTS Result: %s\n", yytext);
#endif
    cuts_testcase(ctag, keys);

    return(CUTS_RESULT_R);
    /* NOTREACHED */
}

.				{
    return(SPACE);
    /* NOTREACHED */

}
"\n"				{
    return(SPACE);
    /* NOTREACHED */
}
%%
/*
 * the BEGIN macro only exists in the lex file, so define a routine to
 * BEGIN the CUTS state.
 */
int
begin_cuts(void)
{
    BEGIN CUTS;
    return 0;
}

/*
 * Calls lex repeatedly until all input is seen.
 */
int
scanner(SYM tags)
{
    alltags = tags;		/* move into global scope for lex actions */

    while(yylex())
	;

    return 0;
}

/*
 * Test-Output record
 *  check if this is a CUTS test; if so, enter the lex "cuts" state;
 *  otherwise do nothing and lex will be in a "data" mode that will just
 *  toss all the output.
 */
static int
test_output(SYM tag, SYM keys)
{
    char *at;

    if((at=(char *)sym_get(keys, "analysis")) != NULL) {
	/* CUTS:number_of_testcases  || CUTS-1:number_of_testcases */
	if(strncasecmp("cuts", at, 4) == 0) {
	    begin_cuts();
	    /*printf("CUTS output expected\n");*/
	}
    }
    return 0;
}

/* Input Data State Check
 * RTS driver output goes thru specific
 * phases; this is used to verify that the new state is a legal state
 * to change to from the current state.
 * This accepts a variable number of arguments (valid states to be
 * in).  The last argument MUST be zero
 */
struct parse_states {
	char *name;
	int bits;
} parse_states[] = {
  { "outside",				SCAN_OUTSIDE },
  { "rts_keyword_start",		SCAN_RTSKEY },
  { "test_start | execution_status", 	SCAN_TSTKEY },
  { "test_output",			SCAN_OUTPUT },
  { "unknown",				0 }, /*end sentinel: bits = 0 */
};

static int
check_mode(int scan_mode, int fst, ...)
{
    va_list ap;			/* used for variable argument functions*/
    int found=0;		/* set to true if a valid state was found */
    int ckm;			/* Check Mode: the mode to look for */
    register struct parse_states *ps; /* for looking thru parse_states */
    char exp_mode[KEYSIZE];	/* expected mode list (for error message) */

    extern int yylineno;	/* Line number from Lex */

    /* look thru parse_states; end sentinel is "bits" = 0 */
    for(ps=parse_states; ps->bits && (ps->bits != fst);ps++)
	;
    strcpy(exp_mode, ps->name);

    /* look at first variable argument */
    if(fst == scan_mode)
	found++;
    else {
	/* not first... look at variable args */
	va_start(ap, fst);
	while(((ckm = va_arg(ap, int)) != 0) && (ckm != scan_mode)) {
	    for(ps=parse_states; ps->bits && (ps->bits != ckm);ps++)
		;
	    strcat(exp_mode, ", ");
	    strcat(exp_mode, ps->name);
	}
	va_end(ap);

	if(ckm == scan_mode)
	    found++;
    }

    if(!found) {
	for(ps=parse_states; ps->bits && (ps->bits != scan_mode);ps++)
	    ;

	fprintf(stderr, "PARSE ERROR -- Line %d found %s in mode %s[%d] expected { %s }\n",
		yylineno, yytext, ps->name, scan_mode, exp_mode);
    }

    return 0;
}

/*
 * This part of the file contains subroutines called by a lex scanner which
 * is parsing rts-driver-format input and putting it into a multi-level
 * symbol table.
 */

/*
 * References to lex variables
 */
/*extern char yytext[];		/ * text matched by last pattern */
/*extern long yyleng;		/ * length of above */

char **filenames;

int
lex_files(char **names)
{
    /* lex */
    extern FILE *yyin;

    filenames = names;

    if(*filenames != NULL) {
#ifdef DEBUGGING
	DEBUG(D_SCAN, 1)
	    printf("lex_files: first file is %s\n", *filenames);
#endif
	if((yyin = fopen(*filenames, "r")) == NULL) {
	    printf("Error opening %s for reading\n", *filenames);
	    exit(1);
	}
    }

    return 0;
}

/*
 * Called by lex's end-of-file processing.
 *  Open the next file on the command line.  If there is no next file,
 *  return "-1" and lex will end.
 */
int
yywrap(void)
{
    extern FILE *yyin;
    extern int yylineno;	/* Line number from Lex */

    if(*filenames != NULL)
	if(*++filenames != NULL) {
#ifdef DEBUGGING
	DEBUG(D_SCAN, 1)
	    printf("yywrap: next file is %s\n", *filenames);
#endif
	    yylineno=1;
	    if((yyin = fopen(*filenames, "r")) != NULL)
		return(0);
	    else {
		printf("Error opening %s for reading\n", *filenames);
		return(1);
	    }
	}

    return(-1);
}

