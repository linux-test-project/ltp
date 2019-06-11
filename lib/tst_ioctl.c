// SPDX-License-Identifier: GPL-2.0-or-later

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/fs.h>

#define TST_NO_DEFAULT_MAIN

#include "tst_test.h"

int tst_fibmap(const char *filename)
{
	/* test if FIBMAP ioctl is supported */
	int fd, block = 0;

	fd = open(filename, O_RDWR | O_CREAT, 0666);
	if (fd < 0) {
		tst_res(TWARN | TERRNO,
			 "open(%s, O_RDWR | O_CREAT, 0666) failed", filename);
		return -1;
	}

	if (ioctl(fd, FIBMAP, &block)) {
		tst_res(TINFO | TERRNO, "FIBMAP ioctl is NOT supported");
		close(fd);
		return 1;
	}
	tst_res(TINFO, "FIBMAP ioctl is supported");

	if (close(fd)) {
		tst_res(TWARN | TERRNO, "close(fd) failed");
		return -1;
	}
	return 0;
}
