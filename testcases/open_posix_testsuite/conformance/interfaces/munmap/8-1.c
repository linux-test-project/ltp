/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * The munmap() function shall fail if:
 * [EINVAL] Addresses in the range [addr,addr+len)
 * are outside the valid range for the
 * address space of a process.
 *
 */


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "posixtest.h"

#define TNAME "munmap/8-1.c"

int main(void)
{
	int rc;
	void *pa;

	/* -1 should be an invalid address */
	pa = (void *)-1;
	rc = munmap(pa, 1);
	if (rc == -1 && errno == EINVAL) {
		printf("Got EINVAL\n");
		printf("Test PASSED\n");
		exit(PTS_PASS);
	} else {
		printf("Test FAILED: Expect EINVAL but get: %s\n",
		       strerror(errno));
		return PTS_FAIL;
	}
}
