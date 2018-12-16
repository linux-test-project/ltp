/*
 * This file is licensed under the GPL license. For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 *
 *	The function will locate the first occurrence of c (converted to a char)
 *	in the string pointed to by s. The terminating NUL character is
 *	considered to be part of the string. Upon completion, the function will
 *	return a pointer to the byte, or a null pointer if the byte was not found.
 *
 * method:
 *	-Generate sample string s1
 *	-Define one char, taking a character from the sample string(char1), one char as NULL(char2)
	and third character out of the sample string(char3).
 *	-Use strchr for string with char1 and store result in result1.
 *	-Use strchr for string with char2 and store result in result2.
 *	-Use strchr for string with char3 and store result in result3.
 *	-Compare the result1 with pointer to the defined char(char1).
 *	-Compare the result2 with pointer to the defined char(char2).
 *	-Compare result3 with NULL.
 *	-Repeat the above all steps for given number of iterations.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

#define STRING_MAX_LEN 50000
#define STEP_COUNT 2000
#define TNAME "strchr/1-1.c"
#define SKIP_CHAR 'n'
#define MATCH_CHAR 's'

char *random_string(int len, int char_pos)
{
	int i;
	char *output_string = malloc(len + 1);
	if (output_string == NULL) {
		printf(TNAME " failed to allocate memory\n");
		exit(PTS_UNRESOLVED);
	}
	for (i = 0; i < len; i++) {
		output_string[i] = rand() % 254 + 1;
		/*Add character MATCH_CHAR at char_pos*/
		if (i == char_pos)
			output_string[i] = MATCH_CHAR;
		/*Avoid adding SKIP_CHAR and MATCH_CHAR in the string*/
		else if (output_string[i] == SKIP_CHAR || output_string[i] == MATCH_CHAR)
			i--;
	}
	output_string[len] = '\0';
	return output_string;
}

int main(void)
{
	int i, char_pos;

	for (i = 1; i < STRING_MAX_LEN; i += STEP_COUNT) {
		char *sample_str;
		char sample_char_1;
		char sample_char_2 = '\0';
		char sample_char_3 = 'n';
		char_pos = rand() % i;

		sample_str = random_string(i, char_pos);
		sample_char_1  = sample_str[char_pos];

		char *ret_str_1 = strchr(sample_str, sample_char_1);
		char *ret_str_2 = strchr(sample_str, sample_char_2);
		char *ret_str_3 = strchr(sample_str, sample_char_3);

		if (ret_str_1 != &sample_str[char_pos]) {
			printf(TNAME " Test Failed, Failed to return pointer to the byte, when matching"
					" character is found. Expected: %p, but returned: %p\n",
					&sample_str[char_pos], ret_str_1);
			exit(PTS_FAIL);
		} else if (ret_str_2 != &sample_str[i]) {
			printf(TNAME " Test Failed, Failed to consider NUL character as a part of the"
					" string,\n\t\t\t\t   so failed to return pointer to the NUL character,"
					" when matching character is found.\n\t\t\t\t   Expected: %p,"
					" but returned: %p\n", &sample_str[i - 1], ret_str_2);
			exit(PTS_FAIL);
		} else if (ret_str_3 != NULL) {
			printf(TNAME " Test Failed, Failed to return NULL when character is not found"
					" in the string. Expected: NULL, but returned: %p\n", ret_str_3);
			exit(PTS_FAIL);
		}
		free(sample_str);
	}
	printf(TNAME " Test Passed, First case: character is found, "
				"so returned pointer to the byte.\n\t\t\t  Second case: The"
				" terminating NUL character is considered as part of the string,"
				" \n\t\t\t\t\tso returned pointer to the NUL character.\n\t\t\t  "
				"Third case: character is not found, so"
				" NULL pointer is returned.\n");
	exit(PTS_PASS);
}
