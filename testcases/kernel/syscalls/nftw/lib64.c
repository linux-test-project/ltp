/*
 *
 *   Copyright (c) International Business Machines  Corp., 2002
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* 10/31/2002   Port to LTP     robbiew@us.ibm.com */
/* 06/30/2001   Port to Linux   nsharoff@us.ibm.com */

/*
 * NAME
 *      lib64.c - This file contains code for common failure conditions 
 */


#include "nftw64.h"

extern int 		s2;

static char     	*tmp_path = "./tmp";
static const char       *no_file = "./tmp/no_such_file";
static const char       *is_a_file = "./tmp/is_a_file";

FILE *temp;
/* 
 * Cleanup the ./tmp
 */
void
remove_test_ENOTDIR_files()
{
	(void)unlink(is_a_file);
}


/* 
 * Cleanup the ./tmp
 */
void
remove_test_ENOENT_files(void)
{
	(void)unlink(no_file);
}

/*
 *  get_long_name_buffer()
 * 	Create a path containing a component of length pathconf(NAME_MAX) + 1
 *      characters.
 */

static char * 
get_long_name_buffer(size_t *length, size_t extra)
{
	char	*buffer;
	size_t	path_length, name_length;

	temp=stderr;
	if ((path_length = pathconf(tmp_path, _PC_PATH_MAX)) == -1) {
		perror("pathconf");
		cleanup_function();
		fail_exit();
	}

	if ((name_length = pathconf(tmp_path, _PC_NAME_MAX)) == -1) {
		perror("pathconf");
		cleanup_function();
		fail_exit();
	}

	if ((strlen(tmp_path) + name_length + extra) > path_length) {
		fprintf(temp, "ERROR: pathconf(_PC_NAME_MAX) too large relative to pathconf(_PC_PATH_MAX)");
		cleanup_function();
		fail_exit();
	}

	if ((buffer = (char *)malloc(path_length + extra)) == NULL) {
		perror("malloc");
		cleanup_function();
		fail_exit();
	}
	*length = name_length;
	return buffer;
}

static void
execute_function(char *name, int (*callback)(const char *), char *buffer,
	int expected)
{
	int	result;

	temp=stderr;
#ifdef DEBUG
	fprintf(temp, "TEST: %s fails with ENAMETOOLONG\n", name);
#endif

	errno = 0;
	result = (*callback)(buffer);

	/*callback found an error, fail*/
	if (result == -752) {
		cleanup_function();
		fail_exit();
	}
	if (result != expected) {
		fprintf(temp, "ERROR: %s did not return value as expected\n", 
			name);
		fprintf(temp, "       Expected: %d  Received: %d\n", 
			expected, result);
		cleanup_function();
		fail_exit();
	}
	if (errno != ENAMETOOLONG) {
		perror(name);
		cleanup_function();
		fail_exit();
	}
}


static void
test_long_file_name(char *name, int (*callback)(const char *), int expected)
{
	char	*ptr, *ptr_end, *buffer;
	size_t	name_length;

	buffer = get_long_name_buffer(&name_length, 1); 

	strcpy(buffer, tmp_path);
	ptr = buffer + strlen(buffer);
	ptr_end = ptr + name_length + 1;

	*(ptr++) = '/';
	while(ptr <= ptr_end)
		*(ptr++) = 'E';
	*ptr = '\000';

	execute_function(name, callback, buffer, expected);
	free(buffer);
}



static void
test_long_component_name(char *name, int (*callback)(const char *), 
	int expected)
{
	char	*ptr, *ptr_end, *buffer;
	size_t	name_length;

	buffer = get_long_name_buffer(&name_length, 3);

	strcpy(buffer, tmp_path);
	ptr = buffer + strlen(buffer);
	ptr_end = ptr + name_length + 2;
	*(ptr++) = '/';
	for (; ptr < ptr_end; ptr++)
		*ptr = 'E';
	*(ptr++) = '/';
	*(ptr++) = 'F';
	*ptr = '\000';
	execute_function(name, callback, buffer, expected);
	free(buffer);
}


void
test_ENAMETOOLONG_path(char *name, int (*callback)(const char *), int expected)
{
	size_t	pcPathMax;
	char	*path, *tmpPtr;
	int	pathLength, tempPathLength;


	if ((pcPathMax = pathconf(tmp_path, _PC_PATH_MAX)) == -1) {
		perror("pathconf");
		cleanup_function();
		fail_exit();
	}
	temp=stderr;
#ifdef DEBUG
	fprintf(temp, "INFO: pathconf(_PC_PATH_MAX) for %s is %lu\n", 
		tmp_path, pcPathMax);
#endif

	if ((path = (char *)malloc(pcPathMax + 2)) == NULL) {
		perror("malloc for path");
		cleanup_function();
		fail_exit();
	}
	path = strcpy(path, tmp_path);
	pathLength = (int)strlen(path);
	tempPathLength = pathLength + 1;

	/* leave some chars for element that pushes path over PC_PATH_MAX */
	pcPathMax = pcPathMax - tempPathLength - 5;

	while (pathLength < pcPathMax) { 
		sprintf(path, "%s/%s", path, tmp_path); 
		pathLength += tempPathLength; 
	}

	/* reinstate pcPathMax correct value */
	pcPathMax = pcPathMax + tempPathLength + 5;

	tmpPtr = path + pathLength;
	*tmpPtr++ = '/' ; 
	pathLength++;
	while (pathLength <= pcPathMax) { 
		*tmpPtr++ = 'z'; 
		pathLength++; 
	}
	*tmpPtr = (char)NULL;

	pathLength = (int) strlen(path);
	if (pathLength != pcPathMax + 1) {
		fprintf(temp, "ERROR: test logic failure, path length is %d, should be %lu\n", pathLength, (long unsigned int)pcPathMax + 1);
		free(path);
		cleanup_function();
		fail_exit();
	}
	execute_function(name, callback, path, expected);
	free(path);
}


void
test_ENAMETOOLONG_name(char *name, int (*callback)(const char *), int expected)
{
	test_long_file_name(name, callback, expected);
	test_long_component_name(name, callback, expected);
}



void
test_ENOENT_empty(char *name, int (*callback)(const char *), int expected)
{
	char		*empty_string;

	empty_string = "";

	errno = 0;

	temp=stderr;
#ifdef DEBUG
	fprintf(temp, "TEST: ENOENT when empty string is passed\n");
#endif

	if ((s2 = (*callback)(empty_string)) == expected) {
		if (errno != ENOENT) {
			perror(name);
			cleanup_function();
			fail_exit();
		}
	} else {
		fprintf(temp, "ERROR: %s did not return correct value\n", name);
		fprintf(temp, "       Expected: %d  Received: %d\n", 
			expected, s2); 
		cleanup_function(); 
		fail_exit(); 
	} 
}


void
test_ENOTDIR(char *name, int (*callback)(const char *), int expected)
{
	int	fd;

	if ((fd = creat(is_a_file, (mode_t)(S_IRWXU | S_IRWXG | S_IRWXO))) 
			== -1) {
		perror("creat");
		cleanup_function();
		fail_exit();
	}

	if (close(fd) == -1) {
		perror("close");
		remove_test_ENOTDIR_files();
		cleanup_function();
		fail_exit();
	}

	errno = 0;

	temp=stderr;
#ifdef DEBUG
	fprintf(temp, "TEST: ENOTDIR when a component is not a directory\n");
#endif

	s2 = (*callback)("./tmp/is_a_file/no_file");
	/*callback found an error, bail*/
	if (s2 == -752) {
		remove_test_ENOTDIR_files();
		cleanup_function();
		fail_exit();
	}
	if (s2 == expected) {
		if (errno != ENOTDIR) {
			perror(name);
			cleanup_function();
			fail_exit();
		}
	} else {
		fprintf(temp, "ERROR: %s did not return correct value\n", name);
		fprintf(temp, "       Expected: %d  Received: %d\n", 
			expected, s2); 
		cleanup_function();
		fail_exit();
	}
	remove_test_ENOTDIR_files();
}



void
test_ENOENT_nofile(char *name, int (*callback)(const char *), int expected)
{

	remove_test_ENOENT_files();

	temp=stderr;
#ifdef DEBUG
	fprintf(temp, "TEST: ENOENT when file does not exist\n");
#endif

	if ((s2 = (*callback)(no_file)) == expected) {
		if (errno != ENOENT) {
			perror(name);
			cleanup_function();
			fail_exit();
		}
	} else {
		fprintf(temp, "ERROR: %s did not return correct value\n", name);
		fprintf(temp, "       Expected: %d  Received: %d\n", 
			expected, s2);
		cleanup_function();
		fail_exit();
	}
}

