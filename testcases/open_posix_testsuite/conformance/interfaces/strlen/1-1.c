/*
 * This file is licensed under the GPL license. For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 *	The function will compute the number of bytes in the string to which s points,
 *	not including the terminating NUL character. and will return the length of s.
 *
 * method:
 *	-Generate sample string s1 of variable lengths in a loop.
 *	-Use strlen() for generated strings and check whether return value is as expected.
 *
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "posixtest.h"

#define STRING_MAX_LEN 50000
#define STEP_COUNT 2000
#define TNAME "strlen/1-1.c"

char *random_string(int len)
{
	int i;
	char *output_string;
	output_string = malloc(len + 1);
	if (output_string == NULL) {
		printf(" Failed to allocate memory\n");
		exit(PTS_UNRESOLVED);
	}
	for (i = 0; i < len; i++)
		output_string[i] = rand() % 254 + 1;
	output_string[len] = '\0';
	return output_string;
}

int main(void)
{
	char *ret_str;
	int i;
	size_t obtained_len;

	for (i = 0; i < STRING_MAX_LEN; i += STEP_COUNT) {
		ret_str = random_string(i);
		obtained_len = strlen(ret_str);

		if (obtained_len != (size_t)i) {
			printf(TNAME " Test Failed, return value is not as expected, "
					"for the given string. Expected string length: %d,"
					" But got: %zu.\n", i, obtained_len);
			exit(PTS_FAIL);
		}
		free(ret_str);
	}
	printf(TNAME " Test Passed, strlen() return value is as expected, "
			"for the given strings.\n");
	exit(PTS_PASS);
}
