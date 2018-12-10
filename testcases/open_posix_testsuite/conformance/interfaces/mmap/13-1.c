/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Copyright (c) 2012, Cyril Hrubis <chrubis@suse.cz>
 *
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * The st_atime field of the mapped file may be marked for update
 * at any time between the mmap() call and the corresponding munmap()
 * call. The initial read or write reference to a mapped region
 * shall cause the file st_atime field to be marked for update if
 * it has not already been marked for update.
 *
 * Test Steps:
 * 1. Do stat before mmap() and after munmap(),
 *    also after writing the mapped region.
 * 2. Compare whether st_atime has been updated.
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
#include "noatime.h"
#include "posixtest.h"

int main(void)
{
	char tmpfname[256];
	ssize_t size = 1024;
	char data[size];
	void *pa;
	int fd;

	struct stat stat_buff, stat_buff2;
	time_t atime1, atime2, atime3;

	char *ch;

	if (mounted_noatime("/tmp") == 1) {
		printf("UNTESTED: The /tmp is mounted noatime\n");
		return PTS_UNTESTED;
	}

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_mmap_13_1_%d", getpid());
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

	if (stat(tmpfname, &stat_buff) == -1) {
		printf("Error at 1st stat(): %s\n", strerror(errno));
		unlink(tmpfname);
		return PTS_UNRESOLVED;
	}
	/* atime1: write */
	atime1 = stat_buff.st_atime;

	sleep(1);

	printf("Time before mmap(): %ld\n", time(NULL));
	pa = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (pa == MAP_FAILED) {
		printf("Error at mmap: %s\n", strerror(errno));
		unlink(tmpfname);
		return PTS_FAIL;
	}

	if (stat(tmpfname, &stat_buff2) == -1) {
		printf("Error at 2nd stat(): %s\n", strerror(errno));
		unlink(tmpfname);
		return PTS_UNRESOLVED;
	}
	/* for mmap */
	atime2 = stat_buff2.st_atime;

	/* Wait a while in case the precision of the sa_time
	 * is not acurate enough to reflect the change
	 */
	sleep(1);

	/* write reference to mapped memory */
	ch = pa;
	*ch = 'b';

	printf("Time before munmap(): %ld\n", time(NULL));
	munmap(pa, size);

	/* FIXME: Update the in-core meta data to the disk */
	fsync(fd);
	close(fd);
	if (stat(tmpfname, &stat_buff) == -1) {
		printf("Error at 3rd stat(): %s\n", strerror(errno));
		unlink(tmpfname);
		return PTS_UNRESOLVED;
	}
	/* atime3: write to memory */
	atime3 = stat_buff.st_atime;

	printf("atime1: %d, atime2: %d, atime3: %d\n",
	       (int)atime1, (int)atime2, (int)atime3);
	if (atime1 != atime3 || atime1 != atime2) {
		printf("Test PASSED\n");
		unlink(tmpfname);
		return PTS_PASS;
	}

	printf("Test FAILED: st_atime was not updated properly\n");
	unlink(tmpfname);
	return PTS_FAIL;
}
