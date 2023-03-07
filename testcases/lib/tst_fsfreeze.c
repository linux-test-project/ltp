// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2010 Hajime Taira <htaira@redhat.com>
 *                    Masatake Yamato <yamato@redhat.com>
 * Copyright (c) 2023 Petr Vorel <pvorel@suse.cz>
 *
 * Based on fsfreeze from util-linux.
 */

#include <linux/fs.h>
#include <stdio.h>

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tst_safe_macros.h"

static void help(void)
{
	printf("Freeze and unfreeze the device.\n");
	printf("Usage: tst_fsfreeze device\n");
}

int main(int argc, char *argv[])
{
	int fd;

	if (argc < 2) {
		help();
		return 1;
	}

	fd = SAFE_OPEN(argv[1], O_RDONLY);
	SAFE_IOCTL(fd, FIFREEZE, 0);
	SAFE_IOCTL(fd, FITHAW, 0);
	SAFE_CLOSE(fd);

	return 0;
}
