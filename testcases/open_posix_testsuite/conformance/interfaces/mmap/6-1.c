/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Copyright (c) 2012, Cyril Hrubis <chrubis@suse.cz>
 *
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * MPR An implementation may permit accesses other than those specified by prot;
 * however, if the Memory Protection option is supported, the implementation
 * shall not permit a write to succeed where PROT_WRITE has not been set or
 * shall not permit any access where PROT_NONE alone has been set.
 * The implementation shall support at least the following values of prot:
 * PROT_NONE, PROT_READ, PROT_WRITE, and the bitwise-inclusive OR of PROT_READ
 * and PROT_WRITE.
 *
 * Test Steps:
 *
 * If Memory Protection option is supported:
 * 1. Spawn a child process.
 * 2. The child process mmap a memory region setting prot as PROT_READ.
 * 3. Try to write the mapped memory.
 * 4. If the writing triger SIGSEGV, the PASS.
 *
 * Please refer to IEEE_1003.1-2001. 2.8.3.3 Memory Protection.
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

int main(void)
{
#ifdef _POSIX_MEMORY_PROTECTION
	char tmpfname[256];
	void *pa;
	size_t size = 1024;
	int fd;

	pid_t child;
	int status;
	int sig_num;

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_mmap_6_1_%d", getpid());
	unlink(tmpfname);
	fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf("Error at open(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}
	unlink(tmpfname);

	child = fork();

	switch (child) {
	case 0:
		if (ftruncate(fd, size) == -1) {
			printf("Error at ftruncate(): %s\n", strerror(errno));
			return PTS_UNRESOLVED;
		}

		pa = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
		if (pa == MAP_FAILED) {
			printf("Error at mmap: %s\n", strerror(errno));
			return PTS_FAIL;
		}

		*(char *)pa = 'b';
		return 0;
		break;
	case -1:
		printf("Error at fork(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
		break;
	default:
		break;
	}

	waitpid(child, &status, WUNTRACED);
	close(fd);

	if (WIFSIGNALED(status)) {
		sig_num = WTERMSIG(status);
		printf("Child process terminated by signal %d\n", sig_num);
		if (sig_num == SIGSEGV) {
			printf("Got SIGSEGV when writing to the mapped memory, "
			       "without setting PROT_WRITE\n" "Test PASSED\n");
			return PTS_PASS;
		}
	}

	if (WIFEXITED(status)) {
		if (WEXITSTATUS(status) == 0) {
			printf
			    ("Did not got SIGSEGV when writing to the mapped memory,"
			     " without setting PROT_WRITE\n" "Test FAILED\n");
			return PTS_FAIL;
		}
	}

	printf("Test Unresolved\n");
	return PTS_UNRESOLVED;
#else
	printf("Test Unsupported, _POSIX_MEMORY_PROTECTION not defined\n");
	return PTS_UNSUPPORTED;
#endif
}
