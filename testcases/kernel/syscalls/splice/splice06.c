// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023 SUSE LLC <wegao@suse.com>
 */

/*\
 * This test is cover splice() on proc files.
 *
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <fcntl.h>
#include <ctype.h>

#include "tst_test.h"
#include "lapi/splice.h"

#define BUF_SIZE 100
#define PIPE_MAX_INIT_SIZE 65536
#define DOMAIN_INIT_NAME "LTP_INIT"
#define DOMAIN_TEST_NAME "LTP_TEST"
#define INTEGER_PROCFILE "/proc/sys/fs/pipe-max-size"
#define STRING_PROCFILE "/proc/sys/kernel/domainname"
static int pipe_max_test_size;

static void format_str(char *str)
{
	int i;

	for (i = 0; i < BUF_SIZE ; i++) {
		if (!isdigit(str[i])) {
			str[i] = '\0';
			break;
		}
	}
	if (i == BUF_SIZE)
		tst_brk(TBROK, "can not find nonnumeric character from input string");
}

static int splice_read_num(const char *file)
{
	int pipes[2];
	int fd_in;
	int ret;
	int num;
	char buf[BUF_SIZE];

	memset(buf, '\0', sizeof(buf));
	fd_in = SAFE_OPEN(file, O_RDONLY);
	SAFE_PIPE(pipes);

	ret = splice(fd_in, NULL, pipes[1], NULL, BUF_SIZE - 1, 0);
	if (ret < 0)
		tst_brk(TBROK | TERRNO, "splice(fd_in, pipe) failed");

	SAFE_READ(0, pipes[0], buf, BUF_SIZE);

	/* Search for the first nonnumeric character and replace it with \0 */
	format_str(buf);

	if (tst_parse_int(buf, &num, 0, INT_MAX))
		tst_brk(TBROK, "Invalid buffer num %s", buf);

	SAFE_CLOSE(fd_in);
	SAFE_CLOSE(pipes[0]);
	SAFE_CLOSE(pipes[1]);

	return num;
}

static char *splice_read_str(const char *file, char *dest)
{
	int pipes[2];
	int fd_in;
	int ret;

	fd_in = SAFE_OPEN(file, O_RDONLY);
	SAFE_PIPE(pipes);

	ret = splice(fd_in, NULL, pipes[1], NULL, BUF_SIZE, 0);
	if (ret < 0)
		tst_brk(TBROK | TERRNO, "splice(fd_in, pipe) failed");

	SAFE_READ(0, pipes[0], dest, BUF_SIZE);

	SAFE_CLOSE(fd_in);
	SAFE_CLOSE(pipes[0]);
	SAFE_CLOSE(pipes[1]);

	return dest;
}


static void splice_write_num(const char *file, int num)
{
	int pipes[2];
	int fd_out;
	int ret;
	char buf[BUF_SIZE];

	memset(buf, '\0', sizeof(buf));

	fd_out = SAFE_OPEN(file, O_WRONLY, 0777);
	SAFE_PIPE(pipes);
	sprintf(buf, "%d", num);

	SAFE_WRITE(SAFE_WRITE_ALL, pipes[1], buf, strlen(buf));

	ret = splice(pipes[0], NULL, fd_out, NULL, BUF_SIZE, 0);
	if (ret < 0)
		tst_brk(TBROK | TERRNO, "splice write failed");

	SAFE_CLOSE(fd_out);
	SAFE_CLOSE(pipes[0]);
	SAFE_CLOSE(pipes[1]);
}

static void splice_write_str(const char *file, char *dest)
{
	int pipes[2];
	int fd_out;
	int ret;

	fd_out = SAFE_OPEN(file, O_WRONLY, 0777);
	SAFE_PIPE(pipes);

	SAFE_WRITE(SAFE_WRITE_ALL, pipes[1], dest, strlen(dest));

	ret = splice(pipes[0], NULL, fd_out, NULL, BUF_SIZE, 0);
	if (ret < 0)
		tst_brk(TBROK | TERRNO, "splice write failed");

	SAFE_CLOSE(fd_out);
	SAFE_CLOSE(pipes[0]);
	SAFE_CLOSE(pipes[1]);
}

static void file_write_num(const char *file, int num)
{
	SAFE_FILE_PRINTF(file, "%d", num);
}

static void file_write_str(const char *file, char *dest)
{
	SAFE_FILE_PRINTF(file, "%s", dest);
}

static int file_read_num(const char *file)
{
	int num;

	SAFE_FILE_SCANF(file, "%d", &num);

	return num;
}

static char *file_read_str(const char *file, char *dest)
{
	SAFE_FILE_SCANF(file, "%s", dest);
	return dest;
}

static void splice_test(void)
{

	char buf_file[BUF_SIZE];
	char buf_splice[BUF_SIZE];

	if (file_read_num(INTEGER_PROCFILE) == splice_read_num(INTEGER_PROCFILE))
		tst_res(TPASS, "Read num through splice correctly");
	else
		tst_brk(TBROK | TERRNO, "Read num through splice failed");

	splice_write_num(INTEGER_PROCFILE, pipe_max_test_size);

	if (file_read_num(INTEGER_PROCFILE) == pipe_max_test_size)
		tst_res(TPASS, "Write num through splice correctly");
	else
		tst_brk(TBROK | TERRNO, "Write num through splice failed");

	memset(buf_file, '\0', sizeof(buf_file));
	memset(buf_splice, '\0', sizeof(buf_splice));

	file_read_str(STRING_PROCFILE, buf_file);
	splice_read_str(STRING_PROCFILE, buf_splice);

	if (!strncmp(buf_file, buf_splice, strlen(buf_file)))
		tst_res(TPASS, "Read string through splice correctly");
	else
		tst_brk(TBROK | TERRNO, "Read string through splice failed");

	memset(buf_file, '\0', sizeof(buf_file));

	splice_write_str(STRING_PROCFILE, DOMAIN_TEST_NAME);
	file_read_str(STRING_PROCFILE, buf_file);

	if (!strncmp(buf_file, DOMAIN_TEST_NAME, strlen(buf_file)))
		tst_res(TPASS, "Write string through splice correctly");
	else
		tst_brk(TBROK | TERRNO, "Write string through splice failed");
}

static void setup(void)
{
	pipe_max_test_size = getpagesize();
	file_write_str(STRING_PROCFILE, DOMAIN_INIT_NAME);
	file_write_num(STRING_PROCFILE, PIPE_MAX_INIT_SIZE);
}

static struct tst_test test = {
	.min_kver = "5.11",
	.setup = setup,
	.test_all = splice_test,
	.needs_tmpdir = 1,
	.save_restore = (const struct tst_path_val[]) {
		{INTEGER_PROCFILE, NULL, TST_SR_TCONF},
		{STRING_PROCFILE, NULL, TST_SR_TCONF},
		{}
	},
};
