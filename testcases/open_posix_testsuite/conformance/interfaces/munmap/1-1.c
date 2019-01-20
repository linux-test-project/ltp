/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * The munmap() function shall remove any mappings for those
 * entire pages containing any part of the address space of
 * the process starting at addr and continuing for len bytes.
 * Further references to these pages shall result in the
 * generation of a SIGSEGV signal to the process.
 *
 * Test Step:
 * 1. map a file into memory;
 * 2. unmap;
 * 3. Try to reference the unmapped memory, outside the
 *    mapped file, test whether SIGSEGV is triggered.
 */


#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "posixtest.h"

#define TNAME "munmap/1-1.c"

void sigsegv_handler(int signum LTP_ATTRIBUTE_UNUSED)
{
	printf("Got SIGSEGV\n");
	printf("Test PASSED\n");
	exit(PTS_PASS);
}

int main(void)
{
	char tmpfname[256];
	long file_size;

	void *pa = NULL;
	void *addr = NULL;
	size_t len;
	int flag;
	int fd;
	off_t off = 0;
	int prot;

	int page_size;

	char *ch1;

	struct sigaction sa;

	sigfillset(&sa.sa_mask);
	sa.sa_handler = sigsegv_handler;
	sigaction(SIGSEGV, &sa, NULL);

	page_size = sysconf(_SC_PAGE_SIZE);
	file_size = 2 * page_size;

	/* We hope to map 2 pages */
	len = page_size + 1;

	/* Create tmp file */
	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_munmap_1_1_%d",
		 getpid());
	unlink(tmpfname);
	fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf(TNAME " Error at open(): %s\n", strerror(errno));
		exit(PTS_UNRESOLVED);
	}
	unlink(tmpfname);

	if (ftruncate(fd, file_size) == -1) {
		printf("Error at ftruncate: %s\n", strerror(errno));
		exit(PTS_UNRESOLVED);
	}

	flag = MAP_SHARED;
	prot = PROT_READ | PROT_WRITE;
	pa = mmap(addr, len, prot, flag, fd, off);
	if (pa == MAP_FAILED) {
		printf("Test UNRESOLVED: " TNAME " Error at mmap: %s\n",
		       strerror(errno));
		exit(PTS_UNRESOLVED);
	}

	/* ch1 is outside the mapped object, but in the mapped file */
	ch1 = pa + len + 1;
	*ch1 = 'b';

	close(fd);
	if (munmap(pa, len) == -1) {
		printf("Test FAILED: " TNAME " Error at munmap: %s\n",
		       strerror(errno));
		exit(PTS_FAIL);
	}

	/* Try to reference the unmapped area, should trigger SIGSEGV */
	*ch1 = 'a';

	/* If reach this point, test fail */
	printf("Test FAILED: Did not trigger SIGSEGV\n");
	return PTS_FAIL;
}
