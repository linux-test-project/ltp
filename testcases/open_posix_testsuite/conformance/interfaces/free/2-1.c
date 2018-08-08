/*
 * This file is licensed under the GPL license. For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 *	If ptr is null pointer
 *		No action will ocuur.
 *
 * method:
 *	In child process:
 * 		Initialise a pointer to NULL
 *		Use free on NULL pointer
 *		exit from child
 *	In parent process
 *		wait till child terminates
 *		If child is terminated normally
 *		test is passed.
*/

#define _XOPEN_SOURCE 700
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

#define TNAME "free/2-1.c "

int main(void)
{

	int status = 0;
	int pid = fork();

	if (pid == 0) {
		int *ptr = NULL;
		free(ptr);
		exit(0);
	} else {
		waitpid(0, &status, pid);
		if (WIFEXITED(status)) {
			printf(TNAME "Test Passed\n");
			exit(PTS_PASS);
		}
	}
	printf(TNAME "Test Failed\n");
	exit(PTS_FAIL);
}
