/*
 * This file is licensed under the GPL license. For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 *
 *  The function will copy not more than n bytes (bytes that follow a NUL
 *  character are not copied) from the array pointed to by s1 to the array
 *  pointed to by s2. And it will return s2.

 * method:
 *  -Generate sample string s1 of variable lengths in a loop.
 *  -Use strncpy() to copy s1 into sample string s2 in each iteration
 *  with given n value.
 *  -Truncate last n characters of s1 and
 *  -Compare both strings(s1 and s2) and Also compare returned pointer with s2.
 *  -Repeat the above steps for given number of iterations.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

#define STRING_MAX_LEN 50000
#define STEP_COUNT 2000
#define TNAME "strncpy/1-1.c"

char *random_string(int len)
{
    int i;
    char *output_string = NULL;
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
    char *ret_str;
    int i, num_bytes;

    for (i = 1; i < STRING_MAX_LEN; i += STEP_COUNT) {
        char *sample_str_1;
        char *sample_str_2 = malloc(i - 1);
        if (sample_str_2 == NULL) {
            printf(TNAME " Failed to allocate memory\n");
            exit(PTS_UNRESOLVED);
        }
        sample_str_1 = random_string(i);
        num_bytes = rand() % i;

        ret_str = strncpy(sample_str_2, sample_str_1, num_bytes);

        sample_str_2[num_bytes] = '\0';
        sample_str_1[num_bytes] = '\0';

        if (strcmp(sample_str_2, sample_str_1) != 0
                    && ret_str == sample_str_2) {
            printf(TNAME " Test Failed, string copy failed. "
                            "Expected string: %s\n, But got: %s\n"
                            , sample_str_1, sample_str_2);
            exit(PTS_FAIL);
        } else if (ret_str != sample_str_2) {
            printf(TNAME " Test Failed, return is not as expected. "
                            "Expected: %p, But obtained: %p\n"
                            , sample_str_2, ret_str);
            exit(PTS_FAIL);
        }
        free(sample_str_1);
        free(sample_str_2);
    }
    printf(TNAME " Test Passed, string copied successfully\n");
    exit(PTS_PASS);
}
