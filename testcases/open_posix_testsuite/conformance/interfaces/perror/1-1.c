/*
 * This file is licensed under the GPL license. For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 *	The function will map the error number accessed through the symbol
 *	errno to a language-dependent error message, which will be written to the standard
 *	error stream as follows:
 *		• First (if s is not a null pointer and the character pointed to by s
 *		is not the null byte), the string pointed to by s followed by a
 *		'colon' and a 'space'.
 *		• Then an error message string followed by a 'newline'.
 *
 * method:
 *	-Redirect error from stderr to a file using freopen.
 *	-Set errno to ERANGE.
 *	-Call perror() for "strtol" to get error message and it will be
 *	stored in file.
 *	-Restore stderr.
 *	-Use getline to read the error message from the file.
 *	-split the error line with delimeter space(' ') and check
 *	whether the error format is as specified in the assertion
 *	and store error message in perror_str.
 *	-Remove extra space in the begining and new line at the end of perror_str.
 *	-Store errno message from stderror with the errno got
 *	from strtol and store it in stderror_str.
 *	-compare perror_str and stderor_str.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include "posixtest.h"
#include <features.h>
#include <wchar.h>

#define TNAME "perror/1-1.c"

int main(void)
{
	FILE *fp;
	char *read_line = NULL;
	ssize_t read;
	size_t len;
	char *perror_str;
	char *strerr_str;
	char *token;
	int i;

	int temp_strerr = dup(2);

	/* Redirecting stderr to a file */
	fp = freopen("perror.txt", "w+", stderr);
	if (fp == NULL) {
		printf(TNAME " Error at freopen(), errno: %d\n", errno);
		exit(PTS_UNRESOLVED);
	}

	errno = ERANGE;
	perror("strtol");
	fclose(fp);
	//Restoring stderr
	stderr = fdopen(temp_strerr, "w");

	/* Opening file for reading error from file */
	fp = fopen("perror.txt", "r");
	read = getline(&read_line, &len, fp);
	if (read == -1) {
		printf(TNAME " Error at getline()\n");
		exit(PTS_UNRESOLVED);
	}
	fclose(fp);

	/* Get error string from error line */
	token = strtok_r(read_line, ":", &perror_str);
	if (token != NULL && (strcmp(read_line, "strtol") == 0) && (perror_str[0] == ' ')
					&& (perror_str[strlen(perror_str)-1] == '\n')) {
		printf(TNAME " Test passed, printed error as, string name followed by ':' and"
					" space. And a newline at the end.\n");
	} else {
		printf(TNAME " Test failed\n, Perror error display format was not as expecetd.\n");
		exit(PTS_FAIL);
	}

	/* Removing extra line at the end */
	perror_str[strlen(perror_str) - 1]  = '\0';

	/* Removing space at the begining */
	for (i = 0; perror_str[i]; i++)
		perror_str[i] = perror_str[i+1];

	strerr_str = strerror(errno);
	if (strcmp(perror_str, strerr_str) == 0) {
		printf(TNAME " Test Passed, mapped the error number accessed through the symbol"
					" errno to a language-dependent error message\n");
		exit(PTS_PASS);
	} else {
		printf(TNAME " Test failed, failed to map the error number accessed through the"
					" symbol errno to a language-dependent error message\n");
		exit(PTS_FAIL);
	}
}
