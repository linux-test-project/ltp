/*   
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

   Testing passing an invalid signals to sigrelse().
   After sighold is called on an invalid signal, sigrelse() should 
return -1 and set errno to EINVAL
   
   The invalid signal passed to sigrelse() depends on the argument 
passed to this program.
   There are currently 4 invalid signals.
 */

#define _XOPEN_SOURCE 600

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <stdint.h>
#include <setjmp.h>
#include "posixtest.h"

jmp_buf sig11_recover;
void sig11_handler(int sig);

int main(int argc, char *argv[])
{
	int signo, TEST_RETURN;
	struct sigaction sa, osa;

	if (argc < 2) {
        	printf("Usage:  %s [1|2|3|4]\n", argv[0]);
		return PTS_UNRESOLVED;
	}

	/*
		Various error conditions
	*/
	switch (argv[1][0]) {
		case '1':
			signo=-1;
			break;
		case '2':
			signo=-10000;
			break;
		case '3':
			signo=INT32_MIN+1;
			break;
		case '4':
			signo=INT32_MIN;
			break;
		default:
			printf("Usage:  %s [1|2|3|4]\n", argv[0]);
			return PTS_UNRESOLVED;
	}

	/* special sig11 case */
        sa.sa_handler = &sig11_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;

        sigaction(SIGSEGV, NULL, &osa);
        sigaction(SIGSEGV, &sa, NULL);

        if (setjmp(sig11_recover)) {
              errno = EINVAL;
              TEST_RETURN=-2;
        } else {
              TEST_RETURN=sigrelse(signo);
        }
        sigaction(SIGSEGV, &osa, NULL);

        if (TEST_RETURN == -1) {
                if (EINVAL == errno) {
                        printf ("errno set to EINVAL\n");
                        return PTS_PASS;
                } else {
                        printf ("errno not set to EINVAL\n");
                        return PTS_FAIL;
                }
        }
	if (TEST_RETURN == -2) {
                printf ("test received SIGSEGV\n");
                return PTS_UNRESOLVED;
        }

        printf("sigrelse did not return -1\n");
        return PTS_FAIL;

}

/******************************************************************
 * sig11_handler() - our segfault recover hack
 ******************************************************************/
void
sig11_handler(int sig)
{
    longjmp(sig11_recover, 1);
}


