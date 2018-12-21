// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2009 FUJITSU LIMITED
 * Author: Li Zefan <lizf@cn.fujitsu.com>
 */

#define _GNU_SOURCE

#include <sched.h>
#include <stdlib.h>
#include <unistd.h>
#include "test.h"

#define DEFAULT_USEC	30000

int foo(void __attribute__ ((unused)) * arg)
{
	return 0;
}

int main(int argc, char **argv)
{
	int usec;

	if (argc == 2)
		usec = atoi(argv[1]);
	else
		usec = DEFAULT_USEC;

	while (1) {
		usleep(usec);
		ltp_clone_quick(CLONE_NEWNS, foo, NULL);
	}

	tst_exit();
}
