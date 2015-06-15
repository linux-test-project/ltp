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
/* $Id: splitstr.c,v 1.2 2000/09/21 20:42:31 nstraz Exp $ */
/*
 * Synopsis
 *
 * const char **splitstr(const char *str, const char *separator, int *argcount)
 *
 * Description
 * This function splits a string (str) into components that are separated by
 * one or more of the characters in the (separator) string.  An array of
 * strings is returned, along with argcount being set to the number of strings
 * found.  Argcount can be NULL.  There will always be a NULL element in the
 * array after the last valid element.  If an error occurs, NULL will be
 * returned and argcount will be set to zero.
 *
 * To rid yourself of the memory allocated for splitstr(), pass the return
 * value from splitstr() unmodified to splitstr_free():
 *
 * void splitstr_free( const char ** return_from_splitstr );
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>		/* for string functions */
#ifdef UNIT_TEST
#include <assert.h>
#endif /* UNIT_TEST */
#include "splitstr.h"

const char **splitstr(const char *str, const char *separator, int *argcount)
{
	char *arg_string = NULL, **arg_array = NULL, *cur_tok = NULL;

	int num_toks = 0, max_toks = 20, i;

	/*
	 * In most recoverable errors, if argcount is not NULL,
	 * set argcount to 0. Then return NULL.
	 */
	if (str == NULL) {
		if (argcount != NULL)
			*argcount = 0;
		return (NULL);
	}

	/*
	 * set aside temporary space to work on the string.
	 */
	arg_string = strdup(str);

	if (arg_string == NULL) {
		if (argcount != NULL)
			*argcount = 0;
		return (NULL);
	}

	/*
	 * set aside an initial char ** array for string array.
	 */
	arg_array = malloc(sizeof(char *) * max_toks);

	if (arg_array == NULL) {
		if (argcount != NULL)
			*argcount = 0;
		free(arg_string);
		return (NULL);
	}

	if (separator == NULL)
		separator = " \t";

	/*
	 * Use strtok() to parse 'arg_string', placing pointers to the
	 * individual tokens into the elements of 'arg_array'.  Expand
	 * 'arg_array' if necessary.
	 */
	cur_tok = strtok(arg_string, separator);
	while (cur_tok != NULL) {
		arg_array[num_toks++] = cur_tok;
		cur_tok = strtok(NULL, separator);
		if (num_toks == max_toks) {
			max_toks += 20;
			arg_array =
			    (char **)realloc((void *)arg_array,
					     sizeof(char *) * max_toks);
			if (arg_array == NULL) {
				fprintf(stderr, "realloc: New memory allocation failed \n");
				free(arg_string);
				exit(1);
			}
		}
	}
	arg_array[num_toks] = NULL;

	/*
	 * If there are any spaces left in our array, make them NULL
	 */
	for (i = num_toks + 1; i < max_toks; i++)
		arg_array[i] = NULL;

	/* This seems nice, but since memory is allocated on a page basis, this
	 * isn't really helpful:
	 * arg_array = (char **)realloc((void *)arg_array, sizeof(char *)*num_toks+1 );*/

	if (argcount != NULL)
		*argcount = num_toks;

	/*
	 * Return the argument array.
	 */
	return ((const char **)arg_array);
}

/*
 * splitster_free( const char ** )
 *
 * This takes the return value from splitster() and free()s memory
 * allocated by splitster.  Assuming: ret=splitster(...), this
 * requires that ret and *ret returned from splitster() have not
 * been modified.
 */
void splitstr_free(const char **p_return)
{
	if (*p_return != NULL)
		free((char *)*p_return);
	if (p_return != NULL)
		free((char **)p_return);
}

#ifdef UNIT_TEST

int main()
{
	int i, y, test_size = 1000, size_ret;
	char test_str[32768];
	char buf[16];
	char *test_str_array[test_size];
	const char **ret;

	for (i = 0; i < test_size; i++) {
		snprintf(buf, 16, "arg%d", i);
		test_str_array[i] = strdup(buf);
	}

	for (i = 0; i < test_size; i++) {
		test_str[0] = '\0';
		for (y = 0; y < i; y++) {
			snprintf(buf, 16, "arg%d ", y);
			strncat(test_str, buf, 16);
		}
		ret = splitstr(test_str, NULL, &size_ret);
		assert(size_ret == i);
		for (y = 0; y < i; y++)
			assert(strcmp(ret[y], test_str_array[y]) == 0);

		splitstr_free(ret);
	}
	return 0;
}

#endif
