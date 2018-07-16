/*
 * This file is licensed under the GPL license. For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 *	The function will compare not more than n bytes from the array pointed to
 *	by s1 to the array pointed to by s2. Upon succesful completion, function
 *	will return an integer greater than 0, if possibly null-terminated array
 *	pointed to by s1 is greater than the possibly null-terminated array
 *	pointed to by s2.
 *
 *  method:
 *	-Generate sample string s1 with all upper case characters.
 *	-Generate sample string s2 with all lower case characters.
 *	-Compare the both strings using strncmp() function for given number of bytes.
 *	-Check the return value.
 *	-Repeat the above steps for given number of iterations.
 *
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include "posixtest.h"

#define STRING_MAX_LEN 50000
#define STEP_COUNT 2000
#define TRUE 1
#define FALSE 0
#define TNAME "strncmp/1-1.c"

char *random_string(int len, bool is_uppercase)
{
	int i;
	char *output_string = malloc(len + 1);
	if (output_string == NULL) {
		printf(TNAME " failed to allocate memory\n");
		exit(PTS_UNRESOLVED);
	}
	for (i = 0; i < len; i++) {
		/* Limiting characters from 1-255 */
		output_string[i] = rand() % 254 + 1;
		if (output_string == NULL) {
			printf(TNAME " Failed to allocate memory\n");
			exit(PTS_UNRESOLVED);
		}
		if (is_uppercase == 1) {
			//Generates a uppercase character
			if (!isupper(output_string[i]))
				i--;
		} else {
			//Generates a lowercase character
			if (!islower(output_string[i]))
				i--;
		}
	}
	output_string[len] = '\0';
	return output_string;
}

int main(void)
{
	int ret, i, num_bytes;

	for (i = 1; i < STRING_MAX_LEN; i += STEP_COUNT) {
		char *sample_str_1;
		char *sample_str_2;
		num_bytes = rand() % i + 1;
		sample_str_2 = random_string(i, true);
		sample_str_1 = random_string(i, false);

		ret = strncmp(sample_str_1, sample_str_2, num_bytes);

		if (ret <= 0) {
			printf(TNAME " Test Failed, return value is not as expected, "
						"for string1 greater than string2. Expected > 0,"
						"but got %d.\n", ret);
			printf("str1=%s\n str2=%s\n", sample_str_1, sample_str_2);
			exit(PTS_FAIL);
		}
		free(sample_str_1);
		free(sample_str_2);
	}
	printf(TNAME " Test Passed, return value is as expected, "
				"for string1 greater than string2.\n");
	exit(PTS_PASS);
}
