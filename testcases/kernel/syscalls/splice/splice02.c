/******************************************************************************/
/* Copyright (c) Jens Axboe <axboe@kernel.dk>, 2009                           */
/*                                                                            */
/* LKML Reference: http://lkml.org/lkml/2009/4/2/55                           */
/*                                                                            */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                  */
/* the GNU General Public License for more details.                           */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program;  if not, write to the Free Software Foundation,   */
/* Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA           */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/* Description: This tests the splice() syscall                               */
/******************************************************************************/
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "tst_test.h"
#include "lapi/splice.h"

#define SPLICE_SIZE (64*1024)

static void splice_test(void)
{
	int fd;

	fd = SAFE_OPEN("splice02-temp", O_WRONLY | O_CREAT | O_TRUNC, 0644);

	TEST(splice(STDIN_FILENO, NULL, fd, NULL, SPLICE_SIZE, 0));
	if (TEST_RETURN < 0) {
		tst_res(TFAIL, "splice failed - errno = %d : %s",
			TEST_ERRNO, strerror(TEST_ERRNO));
	} else {
		tst_res(TPASS, "splice() system call Passed");
	}

	SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test_all = splice_test,
	.needs_tmpdir = 1,
	.min_kver = "2.6.17",
};
