// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * 07/2001 Ported by Wayne Boyer
 */

/*\
 * Test that closing a file/pipe/socket works correctly.
 */

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "tst_test.h"

#define FILENAME "close01_testfile"

static int get_fd_file(void)
{
	return SAFE_OPEN(FILENAME, O_RDWR | O_CREAT, 0700);
}

static int get_fd_pipe(void)
{
	int pipefildes[2];
	SAFE_PIPE(pipefildes);
	SAFE_CLOSE(pipefildes[1]);
	return pipefildes[0];
}

static int get_fd_socket(void)
{
	return SAFE_SOCKET(AF_INET, SOCK_STREAM, 0);
}

struct test_case_t {
	int (*get_fd) ();
	char *type;
} tc[] = {
	{get_fd_file, "file"},
	{get_fd_pipe, "pipe"},
	{get_fd_socket, "socket"}
};

static void run(unsigned int i)
{
	TST_EXP_PASS(close(tc[i].get_fd()), "close a %s fd", tc[i].type);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tc),
	.needs_tmpdir = 1,
	.test = run,
};
