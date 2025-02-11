// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Author: David Fenner, Jon Hendrickson
 * Copyright (C) 2024 Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * This test verifies that open() is working correctly on symlink()
 * generated files. We generate a file via symlink, then we read both from file
 * and symlink, comparing that data has been correctly written.
 */

#include "tst_test.h"

#define FILENAME "myfile.txt"
#define SYMBNAME "myfile_symlink"
#define BIG_STRING "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz"

static char buff_file[128];
static char buff_symb[128];
static int str_size;

static void run(void)
{
	int fd_file, fd_symb;

	memset(buff_file, 0, sizeof(buff_file));
	memset(buff_symb, 0, sizeof(buff_symb));

	tst_res(TINFO, "Create symlink");
	SAFE_TOUCH(FILENAME, 0777, NULL);
	SAFE_SYMLINK(FILENAME, SYMBNAME);

	fd_file = SAFE_OPEN(FILENAME, O_RDONLY, 0777);
	fd_symb = SAFE_OPEN(SYMBNAME, O_RDWR, 0777);

	tst_res(TINFO, "Write data via symlink");
	SAFE_WRITE(SAFE_WRITE_ALL, fd_symb, BIG_STRING, str_size);
	SAFE_LSEEK(fd_symb, 0, 0);

	tst_res(TINFO, "Read data via file");
	SAFE_READ(1, fd_file, buff_file, str_size);
	SAFE_LSEEK(fd_file, 0, 0);

	tst_res(TINFO, "Read data via symlink");
	SAFE_READ(1, fd_symb, buff_symb, str_size);
	SAFE_LSEEK(fd_symb, 0, 0);

	TST_EXP_EXPR(!strncmp(buff_file, BIG_STRING, str_size),
		"file data has been correctly written");

	TST_EXP_EXPR(!strncmp(buff_file, buff_symb, str_size),
		"file data is the equivalent to symlink generated file data");

	SAFE_CLOSE(fd_file);
	SAFE_CLOSE(fd_symb);

	SAFE_UNLINK(SYMBNAME);
	SAFE_UNLINK(FILENAME);
}

static void setup(void)
{
	str_size = strlen(BIG_STRING);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.needs_tmpdir = 1,
};
