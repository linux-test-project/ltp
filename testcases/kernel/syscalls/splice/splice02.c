// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Jens Axboe <axboe@kernel.dk>, 2009
 * http://lkml.org/lkml/2009/4/2/55
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "tst_test.h"
#include "lapi/splice.h"

#define SPLICE_SIZE (1024)

static void splice_test(void)
{
	int pipefd[2];
	FILE* fptr;

	fptr = fopen("splice02-temp", "w+");
	if (fptr == NULL) {
		tst_brk(TBROK|TERRNO, "Openning of file is failed");
	}

	SAFE_PIPE(pipefd);

	TEST(splice(fileno(fptr), NULL, pipefd[1], NULL, SPLICE_SIZE, 0));
	if (TST_RET < 0) {
		tst_res(TFAIL, "splice failed - errno = %d : %s",
			TST_ERR, strerror(TST_ERR));
	} else {
		tst_res(TPASS, "splice() system call Passed");
	}

	SAFE_CLOSE(pipefd[0]);
	SAFE_CLOSE(pipefd[1]);

	fclose(fptr);
}

static struct tst_test test = {
	.test_all = splice_test,
	.needs_tmpdir = 1,
	.min_kver = "2.6.17",
};
