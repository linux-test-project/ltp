// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2019-2023
 */

#include <sys/ioctl.h>
#include <linux/fs.h>

#define TST_NO_DEFAULT_MAIN

#include "tst_test.h"
#include "tst_fs.h"

int tst_fibmap(const char *filename)
{
	int fd, block = 0;

	fd = SAFE_OPEN(filename, O_RDWR | O_CREAT, 0666);

	if (ioctl(fd, FIBMAP, &block)) {
		tst_res(TINFO | TERRNO, "FIBMAP ioctl is NOT supported");
		SAFE_CLOSE(fd);
		return 1;
	}

	tst_res(TINFO, "FIBMAP ioctl is supported");
	SAFE_CLOSE(fd);

	return 0;
}
