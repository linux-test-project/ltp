// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wayne Boyer, International Business Machines  Corp., 2001
 * Copyright (c) 2019 SUSE LLC, Jozef Pupava <jpupava@suse.com>
 */

/*
 * Test Description:
 *  Test fsync() return value on test file
 *  fsync() has to finish within TIME_LIMIT.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <time.h>
#include "tst_test.h"

#define BLOCKSIZE 8192
#define MAXBLKS 262144
#define TIME_LIMIT 120

char tempfile[40] = "";
char pbuf[BUFSIZ];
int fd;
off_t max_blks = MAXBLKS;

struct statvfs stat_buf;

static void setup(void) {
	/* free blocks avail to non-superuser */
	unsigned long f_bavail;

	fd = SAFE_OPEN("tempfile", O_RDWR | O_CREAT | O_TRUNC, 0777);

	if (fstatvfs(fd, &stat_buf) != 0) {
		tst_brk(TBROK, "fstatvfs failed");
	}

	f_bavail = (stat_buf.f_bavail * stat_buf.f_frsize) / BLOCKSIZE;
	if (f_bavail && (f_bavail < MAXBLKS)) {
		max_blks = f_bavail;
	}

#ifdef LARGEFILE
	SAFE_FCNTL(fd, F_SETFL, O_LARGEFILE);
	SAFE_WRITE(1, fd, pbuf, BUFSIZ);
#endif
}

static void run(void) {
	off_t offset;
	int i;
	int max_block = 0;
	int data_blocks = 0;
	time_t time_start, time_end;
	double time_delta;
	long int random_number;

	while (max_block <= data_blocks) {
		random_number = rand();
		max_block = random_number % max_blks;
		data_blocks = random_number % 1000 + 1;
	}

	for (i = 1; i <= data_blocks; i++) {
		offset = i * ((BLOCKSIZE * max_block) / data_blocks);
		offset -= BUFSIZ;
		SAFE_LSEEK(fd, offset, SEEK_SET);
		SAFE_WRITE(1, fd, pbuf, BUFSIZ);
	}
	time_start = time(0);

	TEST(fsync(fd));

	time_end = time(0);

	if (time_end == -1) {
		tst_res(TFAIL | TTERRNO, "getting end time failed");
	} else if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "fsync failed");
	} else if (TST_RET != 0) {
		tst_res(TFAIL | TTERRNO,
		"fsync failed with unexpected return value");
	} else if (time_end < time_start) {
		tst_res(TFAIL,
		"timer broken end %ld < start %ld",
		time_end, time_start);
	} else if ((time_delta =
		difftime(time_end, time_start)) > TIME_LIMIT) {
		tst_res(TFAIL,
		"fsync took too long: %lf seconds; "
		"max_block: %d; data_blocks: %d",
		time_delta, max_block, data_blocks);
	} else {
		tst_res(TPASS,
		"fsync succeeded in an acceptable amount of time");
	}
	SAFE_FTRUNCATE(fd, 0);
}

static void cleanup(void) {
	SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1
};
