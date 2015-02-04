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
/*                                                                            */
/* File:        splice02.c                                                    */
/*                                                                            */
/* Description: This tests the splice() syscall                               */
/*                                                                            */
/* Usage:  <for command-line>                                                 */
/* echo "Test splice()" > <outfile>; splice02 <outfile>                       */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   splice02                                                      */
/******************************************************************************/
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "test.h"
#include "safe_macros.h"
#include "lapi/splice.h"

char *TCID = "splice02";
int TST_TOTAL = 1;

static void cleanup(void)
{
	tst_rmdir();
}

static void setup(void)
{
	if ((tst_kvercmp(2, 6, 17)) < 0) {
		tst_brkm(TCONF, cleanup, "This test can only run on kernels "
			"that are 2.6.17 or higher");
	}

	TEST_PAUSE;
	tst_tmpdir();
}

#define SPLICE_SIZE (64*1024)

int main(int ac, char **av)
{
	int fd;

	setup();

	if (ac < 2) {
		tst_brkm(TFAIL, NULL, "%s failed - Usage: %s outfile", TCID,
			 av[0]);
	}

	fd = SAFE_OPEN(cleanup, av[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);

	TEST(splice(STDIN_FILENO, NULL, fd, NULL, SPLICE_SIZE, 0));
	if (TEST_RETURN < 0) {
		tst_resm(TFAIL, "splice failed - errno = %d : %s",
			 TEST_ERRNO, strerror(TEST_ERRNO));
	} else {
		tst_resm(TPASS, "splice() system call Passed");
	}

	SAFE_CLOSE(cleanup, fd);

	cleanup();
	tst_exit();
}
