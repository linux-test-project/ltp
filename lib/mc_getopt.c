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
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
 * 
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 * 
 * http://www.sgi.com 
 * 
 * For further information regarding this notice, see: 
 * 
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 */

/* $Id: mc_getopt.c,v 1.2 2000/07/30 19:34:14 alaffin Exp $ */

/**********************************************************
 * 
 *    OS Testing - Silicon Graphics, Inc.
 * 
 *    FUNCTION NAME 	: mc_getopt and mc_getoptv
 * 
 *    FUNCTION TITLE	: multiple character option parsing like getopt(3)
 * 
 *    SYNOPSIS:
 *	char *
 *	mc_getopt(argc, argv, flags, optstring)
 *	int argc;
 *	char *argv[];
 *	int flags;
 *	char *optstring;
 *
 *	mc_getoptv(argc, argv, flags, nopts, opt_arr)
 *	int argc;
 *	char *argv[];
 *	int flags;
 *      int nopts;
 *	char *opt_arr[];
 *
 *	extern int mc_optind;
 *	extern char *mc_optarg;
 *	extern char *mc_optopt;
 *
 * 
 *    AUTHOR		: Richard Logan
 * 
 *    DATE		: 01/96
 * 
 *    DESCRIPTION
 *
 *	The mc_getopt() and mc_getoptv() functions are command line parsers.
 *	The argc parameter specifies the argument count and the argv parameter
 *	specifies the argument array.  The optstring argument contains a string
 *      of recognized option strings; if an option string is followed by a
 *      colon, the option takes an argument.  Options are whitespace seperated.
 *	A option string can be a single character or a word.
 *
 *	The opt_arr array contains nopts elements.  Each option element in
 *      opt_arr must be NULL terminated string; if an option string is followed 
 *      by a colon, the option takes an argument.  A option string can be a 
 *	single character or a word.  opt_arr must be at least nopts elements
 *	in size.
 *
 *	The variable mc_optind specifies the index of the next element of the
 *	argv parameter to be processed.  It is initialized to 1, and the
 *	mc_getopt() function updates it as each element of argv is processed.
 *
 *	The mc_getopt() function returns the optstring string that
 *	uniquely matches the next element of the argv parameter.
 *	If the option string takes an argument, the mc_getopt()
 *      function sets the mc_optarg variable to point to the option string 
 *	argument according to the following rules:
 *
 *	o If the argv's option was the last option string in the string, 
 *	mc_optarg contains
 *	the next element of the argv array, and the mc_optind variable is
 *	incremented by 2.  If the resulting value of mc_optind is greater than
 *	or equal to argc, the mc_getopt() function returns an MC_MISSING_OPTARG
 *
 *	o Otherwise, mc_optarg points to the string following the option
 *	character in that element of the argv parameter, and the mc_optind
 *	variable is incremented by 1.
 *
 *	If any of the following conditions are true when the mc_getopt()
 *	function is called, the mc_getopt() function returns MC_DONE without
 *	changing the mc_optind variable:
 *
 *	o argv[mc_optind] is a null pointer
 *
 *	o *argv[mc_optind] is not the character '-'
 *
 *	o argv[mc_optind] points to the string "-"
 *
 *	If the argv[mc_optind] parameter points to the "--" string, the mc_getopt
 *	function returns MC_DONE after incrementing the mc_optind variable.
 *
 *	The flags argument can be used to change how an argv option string
 *	is compared to optstring.  The flags argument is a bitmask.
 *	The only two bits defined thus far are:
 *	
 *	o MC_FULL_TOKEN_MATCH
 *	o MC_CASE_INSENSITIVE
 *
 *	How flags bits can be used to affect mc_getopt()'s actions:
 *
 *	o Without the MC_FULL_TOKEN_MATCH bit set, if the argv string exactly
 *	matches one optstring option, that option will be returned.  
 *	Otherwise, if the argv string matches the beginning of only one
 *	optstring option, that optstring option is returned.
 *
 *	o With the MC_FULL_TOKEN_MATCH bit set, the argv string must exactly
 *	match the optstring, excluding the colon ":".  When word options are
 *	being used, the exact word option string must be in argv to be
 *	reconized as a match.
 *
 *	o Without the MC_CASE_INSENSITIVE bit set, case comparisons are used.
 *	The -I option is different than the -i option.
 *
 *	o With the MC_CASE_INSENSITIVE bit set, all strings are converted to
 *	lower case before comparisons are made.  This means the -I and -i
 *	option are the same.  The returned option string will be all lowercase.
 *	The mc_getoptv() function assumes the option string in the opt_arr
 *	is already converted to lowercase.
 *
 *	If the mc_getopt(v) function encounters an option string that is not
 *	contained in the optstring parameter, it returns MC_UNKNOWN_OPTION
 *	string.  If it detects a missing option argument, it returns 
 *	MC_MISSING_OPTARG string.  Unlike getopt, mc_getopt() will NOT print
 *	message if an argv option string is not found in optstring.
 *
 *	If the argv option string matches more than one optstring strings,
 *	MC_AMBIGUOUS_OPTION string is returned.
 *
 *      mc_getopt(v) will not modify argv or optstring.  It will strdup optstring
 *	and update some character pointers.  The optstring is parsed only
 *	once whenever a mc_optind is set to one or once a MC_DONE was returned.
 *      The strdup space is freed before a MC_DONE is returned.  This means that
 *	optstring is ignored on all subquent calls.
 *
 *
 *    RETURN VALUES
 *	The mc_getopt(v) function returns the next option string specified on
 *	the command line.  The string will be the string as listed in optstring.
 *
 *	MC_MISSING_OPTARG is returned if the mc_getopt(v) function detects a
 *	missing argument.
 *
 *	MC_UNKNOWN_OPTION is returned if the mc_getopt(v) function encounters an
 *	option string not in the optstring argument.
 *
 *	MC_AMBIGUOUS_OPTION is returned if the mc_getopt(v) function encounters
 *	option string that matches two or more optstring argument and there
 *	is not a single exact match.  This condition could occur if
 *	flags argument does not have the MC_FULL_TOKEN_MATCH bit set and
 *	the option string is not a unique string as defined in optstring.
 *	(i.e.  -it, when optsting contains " iterations:  italic")  It
 *	could also occur if an optstring option is duplicated
 *	(i.e.  -iterations, when optsting contains " iterations: iterations").
 *
 *	Otherwise, the mc_getopt(v) function returns MC_DONE when all command
 *	line options are parsed.
 *
 *    LIMITATIONS
 *	mc_getopt() has an internal limit of 256 options
 *	allowed in optstring.
 *
 *#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#**/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/param.h>

#include "mc_getopt.h"

#ifndef DEBUG
#define DEBUG 0		/* used for debugging */
#endif

/*
 * Externally accessable variables.
 */
int mc_optind = 1;	/* index to next argv token */
char *mc_optarg; 	/* pointer to option argument string */

char *mc_optopt;	/* pointer to argv or opt_arg option string */

/*
 * Ensure PATH_MAX is defined.
 */
#ifndef PATH_MAX
#define PATH_MAX	1023
#endif

/*
 * Define the max number of option strings that can exist in optstring.
 */
#define MAX_OPTIONS	256

/*******************************************************************************
 *
 *******************************************************************************/
#if __STDC__
char *
mc_getopt(int argc, char * const argv[], int flags, const char *optstring)
#else
char *
mc_getopt(argc, argv, flags, optstring)
int argc;
char * const argv[];
int flags;
const char *optstring;
#endif  /* __STDC__ */
{

    static char *opt_arr[MAX_OPTIONS]; /* array of pointers to option strings */

    static char *optstr=NULL;  /* strdup'ed optstring, ! NULL, means */
			/* space allocated */
    static int nopts;	/* number of opt_arr elements */

    char *token;	/* used in optstring parsing and string comparision */
    char *cc;		/* used in optstring parsing and converting to lower case */
    int optarr_ind=0;	/* index into opt_arr */
    

#if DEBUG > 0
printf(" Entering mc_getopt, argc = %d, mc_optind = %d, flags = %#o, optstring = '%s'\n",
    argc, mc_optind, flags, optstring);
#endif

    if ( mc_optind >= argc || argv[mc_optind] == NULL ||
	argv[mc_optind][0] != '-' || argv[mc_optind][1] == '\0' ) {

#if DEBUG > 1
printf(" no more options to process\n");
#endif
	if ( optstr != NULL )
	    free(optstr);
	return MC_DONE;

    } else if ( strcmp(argv[mc_optind], "--") == 0 ) {
#if DEBUG > 1
printf(" option --, no more options to process\n");
#endif
	mc_optind++;
	if ( optstr != NULL )
	    free(optstr);
	return MC_DONE;	/* we are done */
    }

    /*
     *  We have new argv - reset info for optstr.
     *  Otherwise old optstr is used.
     */
    if ( optstr == NULL || mc_optind == 1 ) {

        /*
         * copy string so that we don't modify user given string
         */
	if ( optstr != NULL )
	    free(optstr);	/* free old strdup'ed space */

	optstr=strdup(optstring);   /* copy string */
	nopts=0;
	optarr_ind=0;

	token=optstr;
        /*
         * walk through optstr option string (whitespace separated).
         */
        for (;;) {
	
            /* skip preceeding separator chars */
            for (; *token && isspace(*token); token++);
	
	    if ( ! *token )	/* no more tokens */
		break;

            cc = token;	/* start of option string */

            /*
             * find first separator character after token
             */
            for (; *cc && !isspace(*cc); cc++) {
		/*
		 * If doing case insensitive comparisons, make all
		 * optstr characters lowercase.
		 */
		if ( flags & MC_CASE_INSENSITIVE && isupper(*cc) )
		    *cc = tolower(*cc);
	    }

	    opt_arr[optarr_ind++] = token;

	    if ( ! *cc)	/* no more tokens */
		break;
    
            *cc = '\0';   /* null terminate */

	    token = ++cc;

        }  /* end of for loop */

	nopts=optarr_ind;
#if DEBUG > 1
	printf("nopts = %d\n", nopts);

	for(optarr_ind=0; optarr_ind<nopts; optarr_ind++)
   	    printf(" optstring option (index %2d) : %14s\n",
	        optarr_ind, opt_arr[optarr_ind]);
#endif
    }  /* end of processs optstring */

    /*
     * Call mc_getoptv with the optstring in opt_arr array format
     */

    cc=mc_getoptv(argc, argv, flags, nopts, opt_arr);

    if ( optstr != NULL && strcmp(cc, MC_DONE) == 0 )
        free(optstr);

    return cc;
}

/***********************************************************************
 *
 ***********************************************************************/
#if __STDC__
char *
mc_getoptv(int argc, char * const argv[], int flags, int nopts,
					char * const opt_arr[])
#else
char *
mc_getoptv(argc, argv, flags, nopts, opt_arr)
int argc;
char * const argv[];
int flags;
int nopts;
char * const opt_arr[];
#endif  /* __STDC__ */
{
    char *token;        /* used in string comparision */
    char *cc;           /* used in converting to lower case */
    char *cptr;
    int optarr_ind=0;   /* index into opt_arr */

    int found;          /* count of argv option partical matches to optstr options */
    int exact;          /* count of argv option exact matches */
    int cnt;            /* counter */
    int ind;            /* index into opt_arr for matched option */
    int reqarg;		/* if opt_arr[ind] requires an arg */

#if DEBUG > 1
printf(" In mc_getoptv: argv[%d] = %s\n", mc_optind, argv[mc_optind]);
#endif

    if ( mc_optind >= argc || argv[mc_optind] == NULL ||
        argv[mc_optind][0] != '-' || argv[mc_optind][1] == '\0' ) {

#if DEBUG > 1
printf(" no more options to process\n");
#endif
        return MC_DONE;

    } else if ( strcmp(argv[mc_optind], "--") == 0 ) {
#if DEBUG > 1
printf(" option --, no more options to process\n");
#endif
        mc_optind++;
        return MC_DONE; /* we are done */
    }

    if ( flags & MC_CASE_INSENSITIVE ) {
        mc_optopt = &argv[mc_optind][1];

        /*
         * If doing case insensitive comparison,
         * copy argv token and make it all lowercase.
         */
        cc = token = strdup(&argv[mc_optind++][1]);
        for( ;  *cc; cc++)
            if ( isupper(*cc) )
                *cc = tolower(*cc);

    } else {
        token = &argv[mc_optind++][1];
        mc_optopt = token;
    }

    found=0;
    exact=0;
    reqarg=0;

    /*
     * Compare token to each option string.
     * Do not stop once a match is found.  There maybe multiple matches.
     */
    for(optarr_ind=0; optarr_ind<nopts; optarr_ind++) {

	/*
	 * compare opt_arr[optarr_ind] to token
	 * character by character.
	 */

	cc=token;
	cptr=opt_arr[optarr_ind];
	cnt=0;	/* count of match characters */

	while ( *cc && *cptr && *cc++ == *cptr++ ) {
	    cnt++;
	}

        /*
	 * Determine if argv option partially matched opstring option.
	 */
	if ( cnt && *cc == '\0' ) { 
	    /*
	     * Determine if argv option fully matched opstring option.
             * Remember that opt_arr[optarr_ind] can have optional ":".
	     */
	    if ( *cptr == ':' || *cptr == '\0' ) {
                exact++;
                found++;
                ind=optarr_ind;
		if ( *cptr == ':' ) {
		    reqarg=1;
		} else {
		    reqarg=0;
		}

	    /*
	     * If not doing full token matches, count this partial match.
	     */
	    } else if ( ! (flags & MC_FULL_TOKEN_MATCH) ) {
	        found++;
                if ( ! exact ) { /* exact matches are used over partial */
                    ind=optarr_ind;

		    /*
		     * Determine if this option string ends with ":",
		     * requiring an argument.
		     * Find the end of the string first, this
		     * would allow a colon in the string.
		     */
		    while ( *cptr ) 
			cptr++;

		    if ( *(cptr-1) == ':' ) {
			reqarg=1;
		    } else {
			reqarg=0;
		    }
		}
	    }
	}    /* end partial match */
    }   /* end option loop */

#if DEBUG > 2
printf("optarr_ind = %d, ind = %d, found = %d, exact = %d\n",
    optarr_ind, ind, found, exact);
#endif

    /* if doing case insensitive comparison, free space */
    if ( flags & MC_CASE_INSENSITIVE ) {
        free(token);
    }

    if ( found == 0 ) {
#if DEBUG > 1
printf("argv option string not found in optstr returning MC_UNKNOWN_OPTION :%s\n", 
MC_UNKNOWN_OPTION);
#endif
        /* argv option string not found in optstr */
        return MC_UNKNOWN_OPTION;

    } else if ( exact == 1 || found == 1 ) {
        /* option is uniquely matched */
#if DEBUG > 1
printf("argv option found a string match\n");
#endif


        if ( reqarg ) { 	/* option requires argument */

            /* If option argument is missing, return MC_MISSING_OPTARG */
            if ( mc_optind >= argc ) {
#if DEBUG > 2
printf("returning MC_MISSING_OPTARG: %s\n", MC_MISSING_OPTARG);
#endif
                return MC_MISSING_OPTARG;
            }

            mc_optarg = argv[mc_optind++];

        }

#if DEBUG > 2
printf("returning the option (index %d) %s\n", ind, opt_arr[ind]);
#endif
        /* return the full mc_getopt option string */
        return opt_arr[ind];

    } else {

        /*
         * This condition will be encountered if
         * argv option string matches more than optstr option string.
         * The user could not have entered a unique option string or
         * the optstr has a duplicate option entered.
         */
#if DEBUG > 1
printf("argv option string not unique\n");
#endif
        return MC_AMBIGUOUS_OPTION;
    }
}



#ifdef UNIT_TEST

/****
 ****  The following code is provided as a basic unit test.
 ****  Compile this code with UNIT_TEST set and a executable
 ****  command will be generated.  The optstring is hardcoded,
 ****  but the programs argc and argv are given to mc_getopt.
 ****/

main(argc, argv)
int argc;
char **argv;
{

#define	OPTSTR1		" iterate: Testpause I: i "
    extern int mc_optind;
    extern char *mc_optarg;

    char *ret;
    int ind=0;

	mc_optind=1;
	mc_optarg=NULL;
	printf("*********** flags = 0, optstr = %s\n", OPTSTR1);
	while ( (ret=mc_getopt(argc, argv, 0, OPTSTR1)) != MC_DONE ) {
	   if ( mc_optarg != NULL )
	        printf("mc_getopt returned %16s, mc_optopt = %8s, mc_optind = %d, mc_optarg = %s\n",
		    ret, mc_optopt, mc_optind, mc_optarg);
	   else
	        printf("mc_getopt returned %16s, mc_optopt = %8s, mc_optind = %d, mc_optarg = NULL\n",
		    ret, mc_optopt, mc_optind);
	   ind++;
	   mc_optarg=NULL;
	}
	if ( mc_optind < argc )
	    printf("mc_getopt is done,  mc_optind = %d (argv[%d] = %s)\n",
		mc_optind, mc_optind, argv[mc_optind]);
	else
	    printf("mc_getopt is done,  mc_optind = %d (no arguments)\n", mc_optind);

	printf("******************************************************\n");

	
	mc_optind=1;
	mc_optarg=NULL;
	printf("*********** flags = MC_FULL_TOKEN_MATCH, optstr = %s\n", OPTSTR1);
	while ( (ret=mc_getopt(argc, argv, MC_FULL_TOKEN_MATCH, OPTSTR1)) != MC_DONE ) {
	   if ( mc_optarg != NULL )
	        printf("mc_getopt returned %16s, mc_optopt = %8s, mc_optind = %d, mc_optarg = %s\n",
		    ret, mc_optopt, mc_optind, mc_optarg);
	   else
	        printf("mc_getopt returned %16s, mc_optopt = %8s, mc_optind = %d, mc_optarg = NULL\n",
		    ret, mc_optopt, mc_optind);
	   mc_optarg=NULL;
	}
	if ( mc_optind < argc )
	    printf("mc_getopt is done,  mc_optind = %d (argv[%d] = %s)\n",
		mc_optind, mc_optind, argv[mc_optind]);
	else
	    printf("mc_getopt is done,  mc_optind = %d (no arguments)\n", mc_optind);

	mc_optind=1;
	mc_optarg=NULL;
	printf("*********** flags = MC_CASE_INSENSITIVE, optstr = %s\n", OPTSTR1);
	while ( (ret=mc_getopt(argc, argv, MC_CASE_INSENSITIVE, OPTSTR1)) != MC_DONE ) {
	   if ( mc_optarg != NULL )
	        printf("mc_getopt returned %16s, mc_optopt = %8s, mc_optind = %d, mc_optarg = %s\n",
		    ret, mc_optopt, mc_optind, mc_optarg);
	   else
	        printf("mc_getopt returned %16s, mc_optopt = %8s, mc_optind = %d, mc_optarg = NULL\n",
		    ret, mc_optopt, mc_optind);
	   mc_optarg=NULL;
	}
	if ( mc_optind < argc )
	    printf("mc_getopt is done,  mc_optind = %d (argv[%d] = %s)\n",
		mc_optind, mc_optind, argv[mc_optind]);
	else
	    printf("mc_getopt is done,  mc_optind = %d (no arguments)\n", mc_optind);

	return 0;
}

/**
Use the following unit test cmd line arguments:
 -i -itera -4 -I -5 -i -IteRate -6 arg

 ***/

#endif /* UNIT_TEST */
