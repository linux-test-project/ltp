/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Copyright (c) 2012, Cyril Hrubis <chrubis@suse.cz>
 *
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * MPR References within the address range starting at pa and
 * continuing for len bytes to whole pages following the end
 * of an object shall result in delivery of a SIGBUS signal.
 *
 * Test Steps:
 * 1. Map a file with size = 1/2 page_size, while len = 2 * page_size
 * 2. If Memory Protection option is supported, read the second page
 *    beyond the object (mapped file) size (NOT the patial page),
 *    should get SIGBUS;
 */


#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "posixtest.h"

#define WRITE(str) write(STDOUT_FILENO, str, sizeof(str) - 1)

void sigbus_handler(int signum)
{
	if (signum == SIGBUS) {
		WRITE("SIGBUS triggered\n");
		WRITE("Test PASSED\n");
		_exit(PTS_PASS);
	}
}

int main(void)
{
#ifndef _POSIX_MEMORY_PROTECTION
	printf("_POSIX_MEMORY_PROTECTION is not defined\n");
	return PTS_UNTESTED;
#endif
	char tmpfname[256];
	long page_size;
	long total_size;

	void *pa;
	size_t len;
	int fd;

	char *ch;
	struct sigaction sa;

	page_size = sysconf(_SC_PAGE_SIZE);

	/* Size of the file to be mapped */
	total_size = page_size / 2;

	/* mmap 2 pages */
	len = page_size * 2;

	sigfillset(&sa.sa_mask);
	sa.sa_handler = sigbus_handler;
	sigaction(SIGBUS, &sa, NULL);

	/* Create tmp file */
	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_mmap_11_2_%d", getpid());
	unlink(tmpfname);
	fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf("Error at open(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}
	unlink(tmpfname);

	if (ftruncate(fd, total_size) == -1) {
		printf("Error at ftruncate(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}

	pa = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (pa == MAP_FAILED) {
		printf("Error at mmap(): %s\n", strerror(errno));
		return PTS_FAIL;
	}

	/* Read the second page */
	ch = pa + page_size + 1;

	/* This reference should trigger SIGBUS */
	*ch = 0;

	/* wait for a while */
	sleep(1);

	printf("Test FAILED: SIGBUS not triggered, "
	       "while Memory Protection is enabled\n");
	return PTS_FAIL;
}
