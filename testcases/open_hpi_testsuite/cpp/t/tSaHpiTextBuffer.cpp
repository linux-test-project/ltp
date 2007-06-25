/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2005
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *    W. David Ashley <dashley@us.ibm.com>
 */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
extern "C"
{
#include <SaHpi.h>
}
#include "oSaHpiTextBuffer.hpp"


int main(int argc, char *argv[]) {
    oSaHpiTextBuffer *tb1, *tb2;

    // create the first text buffer
    tb1 = new oSaHpiTextBuffer;
    if (tb1 == NULL) {
        printf("Error: Unable to create a oSaHpiTextBuffer.\n");
        return -1;
    }

    // append a string to the first text buffer
    if (tb1->append("My string")) {
        printf("Error: Unable to append a string to the buffer.\n");
        return -1;
    }

    // copy the first text buffer to the second text buffer
    tb2 = new oSaHpiTextBuffer(*tb1);
    if (tb2 == NULL) {
        printf("Error: Unable to copy a oSaHpiTextBuffer.\n");
        return -1;
    }

    // append a string to the second text buffer
    if (tb2->append(", more string")) {
        printf("Error: Unable to append a string to the new buffer.\n");
        return -1;
    }

    // print the contents of the first text buffer
    fprintf(stdout, "\nSaHpiTextBuffer\n");
    if (tb1->fprint(stdout, 3)) {
        printf("Error: Unable to print the buffer.\n");
        return -1;

    }
    fprintf(stdout, "\n");

    // print the contents of the second text buffer
    fprintf(stdout, "\nSaHpiTextBuffer\n");
    if (tb2->fprint(stdout, 3)) {
        printf("Error: Unable to print the new buffer.\n");
        return -1;

    }
    fprintf(stdout, "\n");





    printf("Success!\n");
    return 0;
}

