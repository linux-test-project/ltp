/*
 * This file is licensed under the GPL license. For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 *
 *	The function will compare the string pointed to by s1 to the string pointed
 *	to by s2. Upon completion, function will return an integer equal to 0,
 *	if the string pointed to by s1 is equal to the string pointed to by s2.
 *
 *  method:
 *	-Generate two equal sample strings s1 and s2.
 *	-Compare the both strings using strcmp() function
 *	-Check the return value.
 *	-Repeat the above steps for given number of iterations.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

#define STRING_MAX_LEN 50000
#define STEP_COUNT 2000
#define TNAME "strcmp/2-1.c"

void random_string(char *sample_str_1, char *sample_str_2, int len)
{
	int i;
	char output_char;
	for (i = 0; i < len; i++) {
		/* Limiting characters from 1-255 */
		output_char = rand() % 254 + 1;
		sample_str_1[i] = output_char;
		sample_str_2[i] = output_char;
	}
	sample_str_1[i] = '\0';
	sample_str_2[i] = '\0';
}

int main(void)
{
	int ret, i;

	for (i = 1; i < STRING_MAX_LEN; i += STEP_COUNT) {
		char *sample_str_1 = malloc(i);
		if (sample_str_1 == NULL) {
			printf(TNAME " Failed to allocate memory\n");
			exit(PTS_UNRESOLVED);
		}
		char *sample_str_2 = malloc(i);
		if (sample_str_2 == NULL) {
			printf(TNAME " Failed to allocate memory\n");
			exit(PTS_UNRESOLVED);
		}
		random_string(sample_str_1, sample_str_2, i);

		ret = strcmp(sample_str_1, sample_str_2);
		if (ret != 0) {
			printf(TNAME " Test Failed, return value is not as expected, "
						"for string1 equal to string2. Expected 0,"
						"but got %d.\n", ret);
			exit(PTS_FAIL);
		}
		free(sample_str_1);
		free(sample_str_2);
	}
	printf(TNAME " Test Passed, return value is as expected, "
				"for string1 equal to string2.\n");
	exit(PTS_PASS);
}
