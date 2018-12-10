/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Copyright (c) 2012, Cyril Hrubis <chrubis@suse.cz>
 *
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * The st_ctime and st_mtime fields of a file that is mapped with
 * MAP_SHARED and PROT_WRITE shall be marked for update at some point
 * in the interval between a write reference to the
 * mapped region and the next call to msync() with MS_ASYNC or MS_SYNC
 * for that portion of the file by any process.
 * If there is no such call and if the underlying file is modified
 * as a result of a write reference, then these fields shall be marked
 * for update at some time after the write reference.
 *
 */


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
#include <time.h>
#include "posixtest.h"

int main(void)
{
	char tmpfname[256];
	ssize_t size = 1024;
	char data[size];
	void *pa;
	int fd;
	struct stat stat_buff;

	time_t mtime1, mtime2, ctime1, ctime2;

	char *ch;

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_mmap_14_1_%d", getpid());
	unlink(tmpfname);
	fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf("Error at open(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}

	memset(data, 'a', size);
	printf("Time before write(): %ld\n", time(NULL));
	if (write(fd, data, size) != size) {
		printf("Error at write(): %s\n", strerror(errno));
		unlink(tmpfname);
		return PTS_UNRESOLVED;
	}
	sleep(1);
	printf("Time before mmap(): %ld\n", time(NULL));
	pa = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (pa == MAP_FAILED) {
		printf("Error at mmap: %s\n", strerror(errno));
		unlink(tmpfname);
		return PTS_FAIL;
	}
	sleep(1);
	printf("Time before write reference: %ld\n", time(NULL));
	/* Before write reference */
	if (stat(tmpfname, &stat_buff) == -1) {
		printf("Error at 1st stat(): %s\n", strerror(errno));
		unlink(tmpfname);
		return PTS_UNRESOLVED;
	}

	ctime1 = stat_buff.st_ctime;
	mtime1 = stat_buff.st_mtime;

	ch = pa;
	*ch = 'b';

	/* Wait a while in case the precision of the sa_time
	 * is not acurate enough to reflect the update
	 */
	sleep(1);
	printf("Time before msync(): %ld\n", time(NULL));
	msync(pa, size, MS_SYNC);

	/* FIXME: Update the in-core meta data to the disk */
	fsync(fd);

	if (stat(tmpfname, &stat_buff) == -1) {
		printf("Error at stat(): %s\n", strerror(errno));
		unlink(tmpfname);
		return PTS_UNRESOLVED;
	}

	ctime2 = stat_buff.st_ctime;
	mtime2 = stat_buff.st_mtime;

	printf("ctime1: %ld, ctime2: %ld\nmtime1: %ld, mtime2: %ld\n",
	       ctime1, ctime2, mtime1, mtime2);
	if (ctime2 == ctime1 || mtime2 == mtime1) {
		printf("Test FAILED: "
		       "st_ctime and st_mtime were not updated properly\n");
		unlink(tmpfname);
		return PTS_FAIL;
	}

	munmap(pa, size);
	close(fd);
	unlink(tmpfname);
	printf("Test PASSED\n");
	return PTS_PASS;
}
