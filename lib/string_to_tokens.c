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
/**********************************************************
 * 
 *    OS Testing - Silicon Graphics, Inc.
 * 
 *    FUNCTION NAME     : string_to_tokens
 * 
 *    FUNCTION TITLE    : Break a string into its tokens
 * 
 *    SYNOPSIS:
 *
 * int string_to_tokens(arg_string, arg_array, array_size, separator)
 *    char *arg_string;
 *    char *arg_array[];
 *    int array_size;
 *    char *separator;
 * 
 *    AUTHOR            : Richard Logan
 *
 *    DATE		: 10/94
 *
 *    INITIAL RELEASE   : UNICOS 7.0
 * 
 *    DESCRIPTION
 * This function parses the string 'arg_string', placing pointers to
 * the 'separator' separated tokens into the elements of 'arg_array'.
 * The array is terminated with a null pointer.
 * 'arg_array' must contains at least 'array_size' elements.
 * Only the first 'array_size' minus one tokens will be placed into
 * 'arg_array'.  If there are more than 'array_size'-1 tokens, the rest are
 * ignored by this routine.
 *
 *    RETURN VALUE
 * This function returns the number of 'separator' separated tokens that
 * were found in 'arg_string'.
 * If 'arg_array' or 'separator' is NULL or 'array_size' is less than 2, -1 is returned.
 * 
 *    WARNING
 * This function uses strtok() to parse 'arg_string', and thus
 * physically alters 'arg_string' by placing null characters where the
 * separators originally were.
 *
 *
 *#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#**/
#include <stdio.h>         
#include <string.h>        /* for string functions */
#include "string_to_tokens.h"

int
string_to_tokens(char *arg_string, char *arg_array[], int array_size, char *separator)
{
   int num_toks = 0;  /* number of tokens found */
   char *strtok();
	
   if ( arg_array == NULL || array_size <= 1 || separator == NULL )
	return -1;

   /*
    * Use strtok() to parse 'arg_string', placing pointers to the
    * individual tokens into the elements of 'arg_array'.
    */
   if ( (arg_array[num_toks] = strtok(arg_string, separator)) == NULL ) {
	return 0;
   }

   for (num_toks=1;num_toks<array_size; num_toks++) {
	if ( (arg_array[num_toks] = strtok(NULL, separator)) == NULL )
	    break;
   }

   if ( num_toks == array_size )
	arg_array[num_toks] = NULL;

   /*
    * Return the number of tokens that were found in 'arg_string'.
    */
   return(num_toks);

} /* end of string_to_tokens */
