/*
 * This file is licensed under the GPL license. For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 *  If the array pointed to by s1 is a string that is shorter than n bytes,
 *  NUL characters will be appended to the copy in the array pointed to
 *  by s2, until n bytes in all are written
 *
 * method:
 *  -Generate sample string s1 of variable lengths in a loop.
 *  -Define number of bytes you want to copy,
 *  which is more than the length of s1.
 *  -Use strncpy to copy string from s1 to s2.
 *  -Check whether s2 is appeneded by '\0' after
 *  writing string s1, till number of bytes given.
 *  -Repeat the above steps for given number of iterations.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

#define STRING_MAX_LEN 50000
#define STEP_COUNT 2000
#define TNAME "strncpy/2-1.c"
#define EXTRA_BYTES 10

char *random_string(int len)
{
    int i;
    char *output_string;
    output_string = malloc(len + 1);
    if (output_string == NULL) {
        printf(TNAME " Failed to allocate memory\n");
        exit(PTS_UNRESOLVED);
    }
    for (i = 0; i < len; i++)
        /* Limiting characters from 1-255 */
        output_string[i] = rand() % 254 + 1;
    output_string[len] = '\0';
    return output_string;
}
int main(void)
{
    int i, j, c;
    char *ret_str;

    for (i = 1; i < STRING_MAX_LEN; i += STEP_COUNT) {
        c = 0;
        char *sample_str_1;
        char *sample_str_2 = malloc(i + EXTRA_BYTES);
        if (sample_str_2 == NULL) {
            printf(TNAME " Failed to allocate memory\n");
            exit(PTS_UNRESOLVED);
        }
        sample_str_1 = random_string(i);
        ret_str = strncpy(sample_str_2, sample_str_1, i + EXTRA_BYTES);
        sample_str_2[i + EXTRA_BYTES] = '\0';

        if (strcmp(sample_str_1, sample_str_2) != 0) {
            printf(TNAME " Test failed, string copy failed. "
                        "Expected string: %s, But got: %s\n"
                        , sample_str_1, sample_str_2);
            exit(PTS_UNRESOLVED);
        } else if (ret_str != sample_str_2) {
            printf(TNAME " Test Failed, return is not as expected. "
                        "Expected return: %p, But obtained: %p\n"
                        , sample_str_2, ret_str);
            exit(PTS_FAIL);
        }

        for (j = strlen(sample_str_1); j < i + EXTRA_BYTES; j++) {
            if (sample_str_2[j] == '\0') {
                c++;
            }
        }

        if (c != EXTRA_BYTES) {
            printf(TNAME " Test Failed, The difference in the number of bytes"
                        " for two strings (s1 and s2) have not been appended"
                        " with NULL bytes\n");
            exit(PTS_FAIL);
        }
        free(sample_str_1);
        free(sample_str_2);
    }
    printf(TNAME "  Test Passed, The difference in the number of bytes for two "
                    "strings (s1 and s2) have been appended with NULL bytes in s2\n");
    exit(PTS_PASS);
}
