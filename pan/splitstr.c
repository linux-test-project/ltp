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
 *
 */
/* $Id: splitstr.c,v 1.1 2000/09/14 21:54:44 nstraz Exp $ */
/*
 * This is a heavily modified version of USC_parse_arg
 *
 * Synopsis
 *
 * char **splitstr(char *str, int *argcount, char *separator)
 *
 * Description
 * This function splits a string (str) into components that are separated by
 * one or more of the characters in the (separator) string.  An array of
 * strings is returned, along with argcount being set to the number of strings
 * found.
 *
 * To rid yourself of the memory allocated for the string:
 *	free( SplitstrReturn[0] );
 *	free( SplitstrReturn );
 *
 *#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#**/
#include <stdio.h>
#include <string.h>        /* for string functions */

#define ARG_ARRAY_SIZE 1000

char **
splitstr(char *str, int *argcount, char *separator)
{
    char *arg_string;
    char *arg_array[ARG_ARRAY_SIZE+1], **aa;

    int num_toks = 0;  /* number of tokens found */
    
    /* copy str to not destroy the original */
    arg_string = (char*)malloc( strlen(str) + 1 );
    strcpy( arg_string, str );

    if(separator==NULL)
	separator = " \t";

    /*
     * Use strtok() to parse 'arg_string', placing pointers to the
     * individual tokens into the elements of 'arg_array'.
     */
    arg_array[num_toks] = strtok(arg_string, separator);
    while ( num_toks < ARG_ARRAY_SIZE && 
	   (arg_array[++num_toks] = strtok(NULL, separator)) != NULL )
	;
    
    arg_array[++num_toks] = NULL;

    *argcount = num_toks-1;
    aa = (char **) malloc(sizeof(char *) * num_toks);
    memcpy(aa, arg_array, sizeof(char *) * num_toks);

    /*
     * Return the argument array.
     */
    return(aa);
}
