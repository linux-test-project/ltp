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
#define MAXBLKS 65536
#define BUF_SIZE 2048

char tempfile[40] = "";
char pbuf[BUF_SIZE];
int fd;
off_t max_blks = MAXBLKS;
int time_limit = 120;

struct statvfs stat_buf;

static void setup(void) {
	/* free blocks avail to non-superuser */
	unsigned long f_bavail;

	if (tst_is_virt(VIRT_ANY)) {
		tst_res(TINFO, "Running in a VM, multiply the time_limit by 2");
		time_limit *= 2;
	}

	fd = SAFE_OPEN("tempfile", O_RDWR | O_CREAT | O_TRUNC, 0777);

	if (fstatvfs(fd, &stat_buf) != 0) {
		tst_brk(TBROK, "fstatvfs failed");
	}

	f_bavail = (stat_buf.f_bavail * stat_buf.f_bsize) / BLOCKSIZE;
	if (f_bavail && (f_bavail < MAXBLKS)) {
		max_blks = f_bavail;
	}

#ifdef LARGEFILE
	SAFE_FCNTL(fd, F_SETFL, O_LARGEFILE);
	SAFE_WRITE(1, fd, pbuf, BUF_SIZE);
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

	random_number = rand();
	max_block = random_number % max_blks + 1;
	data_blocks = random_number % max_block;

	for (i = 1; i <= data_blocks; i++) {
		offset = i * ((BLOCKSIZE * max_block) / data_blocks);
		offset -= BUF_SIZE;
		SAFE_LSEEK(fd, offset, SEEK_SET);
		SAFE_WRITE(1, fd, pbuf, BUF_SIZE);
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
		difftime(time_end, time_start)) > time_limit) {
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
	.needs_tmpdir = 1,
	.max_runtime = 300,
};
