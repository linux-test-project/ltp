/*
 * Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 This is the assistant program for assertion 9. Basically, here we verify
 that no alternate stack has been carried over from the program that used
 the execl() function to call this program.

*/

#define _XOPEN_SOURCE 600

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "posixtest.h"

int main(void) {
        stack_t alternate_s;

        if (sigaltstack((stack_t *)0, &alternate_s) == -1) {
                perror("Unexpected error while attempting to setup test pre-conditions");
                exit(PTS_UNRESOLVED);
        }

        if (0 != alternate_s.ss_sp) {
                printf("Test FAILED: ss_sp of the stack is not same as the defined one\n");
                exit(PTS_FAIL);
        }

        if (0 != alternate_s.ss_size) {
                printf("Test FAILED: ss_size of the stack is not same as the defined one\n");
                exit(PTS_FAIL);
        }

}
