/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Xing Gu <gux.fnst@cn.fujitsu.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
/*
 * Description:
 *   Verify that,
 *   1)openat() succeeds to open a file in append mode, when
 *     'flags' is set to O_APPEND.
 *   2)openat() succeeds to enable the close-on-exec flag for a
 *     file descriptor, when 'flags' is set to O_CLOEXEC.
 *   3)openat() succeeds to allow files whose sizes cannot be
 *     represented in an off_t but can be represented in an off64_t
 *     to be opened, when 'flags' is set to O_LARGEFILE.
 *   4)openat() succeeds to not update the file last access time
 *     (st_atime in the inode) when the file is read, when 'flags'
 *     is set to O_NOATIME.
 *   5)openat() succeeds to open the file failed if the file is a
 *     symbolic link, when 'flags' is set to O_NOFOLLOW.
 *   6)openat() succeeds to truncate the file to length 0 if the file
 *     already exists and is a regular file and the open mode allows
 *     writing, when 'flags' is set to O_TRUNC.
 */

#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <mntent.h>

#include "test.h"
#include "safe_macros.h"
#include "lapi/fcntl.h"
#include "openat.h"

#define TEST_APP "openat02_child"

#define TEST_FILE "test_file"
#define SFILE "sfile"
#define LARGE_FILE "large_file"

#define STR "abcdefg"

static void setup(void);
static void cleanup(void);

static void testfunc_append(void);
static void testfunc_cloexec(void);
static void testfunc_largefile(void);
static void testfunc_noatime(void);
static void testfunc_nofollow(void);
static void testfunc_trunc(void);

static void (*testfunc[])(void) = {
	testfunc_append,
	testfunc_cloexec,
	testfunc_largefile,
	testfunc_noatime,
	testfunc_nofollow,
	testfunc_trunc,
};

char *TCID = "openat02";
int TST_TOTAL = ARRAY_SIZE(testfunc);

int main(int ac, char **av)
{
	int lc;
	int i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++)
			(*testfunc[i])();
	}

	cleanup();
	tst_exit();
}

void setup(void)
{
	TEST_PAUSE;

	tst_sig(FORK, DEF_HANDLER, cleanup);

	tst_tmpdir();

	SAFE_FILE_PRINTF(cleanup, TEST_FILE, "test file");

	SAFE_SYMLINK(cleanup, TEST_FILE, SFILE);
}

void testfunc_append(void)
{
	off_t file_offset;

	SAFE_FILE_PRINTF(cleanup, TEST_FILE, "test file");

	TEST(openat(AT_FDCWD, TEST_FILE, O_APPEND | O_RDWR, 0777));

	if (TEST_RETURN == -1) {
		tst_resm(TFAIL | TTERRNO, "openat failed");
		return;
	}

	SAFE_WRITE(cleanup, 1, TEST_RETURN, STR, sizeof(STR) - 1);

	file_offset = SAFE_LSEEK(cleanup, TEST_RETURN, 0, SEEK_CUR);

	if (file_offset > (off_t)(sizeof(STR) - 1))
		tst_resm(TPASS, "test O_APPEND for openat success");
	else
		tst_resm(TFAIL, "test O_APPEND for openat failed");

	SAFE_CLOSE(cleanup, TEST_RETURN);
}

void testfunc_cloexec(void)
{
	pid_t pid;
	int status;
	char buf[20];

	if ((tst_kvercmp(2, 6, 23)) < 0) {
		tst_resm(TCONF, "test O_CLOEXEC flags for openat "
						"needs kernel 2.6.23 or higher");
		return;
	}

	TEST(openat(AT_FDCWD, TEST_FILE, O_CLOEXEC | O_RDWR, 0777));

	if (TEST_RETURN == -1) {
		tst_resm(TFAIL | TTERRNO, "openat failed");
		return;
	}

	sprintf(buf, "%ld", TEST_RETURN);

	pid = tst_fork();

	if (pid < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "fork() failed");

	if (pid == 0) {
		if (execlp(TEST_APP, TEST_APP, buf, NULL))
			exit(2);
	}

	SAFE_CLOSE(cleanup, TEST_RETURN);

	SAFE_WAIT(cleanup, &status);

	if (WIFEXITED(status)) {
		switch ((int8_t)WEXITSTATUS(status)) {
		case 0:
			tst_resm(TPASS, "test O_CLOEXEC for openat success");
		break;
		case 1:
			tst_resm(TFAIL, "test O_CLOEXEC for openat failed");
		break;
		default:
			tst_brkm(TBROK, cleanup, "execlp() failed");
		}
	} else {
		tst_brkm(TBROK, cleanup,
				 "openat2_exec exits with unexpected error");
	}
}

void testfunc_largefile(void)
{
	int fd;
	off64_t offset;

	fd = SAFE_OPEN(cleanup, LARGE_FILE,
				O_LARGEFILE | O_RDWR | O_CREAT, 0777);

	offset = lseek64(fd, 4.1*1024*1024*1024, SEEK_SET);
	if (offset == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "lseek64 failed");

	SAFE_WRITE(cleanup, 1, fd, STR, sizeof(STR) - 1);

	SAFE_CLOSE(cleanup, fd);

	TEST(openat(AT_FDCWD, LARGE_FILE, O_LARGEFILE | O_RDONLY, 0777));

	if (TEST_RETURN == -1) {
		tst_resm(TFAIL, "test O_LARGEFILE for openat failed");
	} else {
		tst_resm(TPASS, "test O_LARGEFILE for openat success");
		SAFE_CLOSE(cleanup, TEST_RETURN);
	}
}

void testfunc_noatime(void)
{
	struct stat file_stat, file_newstat;
	char buf;
	const char *flags[] = {"noatime", "relatime", NULL};
	int ret;

	if ((tst_kvercmp(2, 6, 8)) < 0) {
		tst_resm(TCONF, "test O_NOATIME flags for openat "
						"needs kernel 2.6.8 or higher");
		return;
	}

	ret = tst_path_has_mnt_flags(cleanup, NULL, flags);
	if (ret > 0) {
		tst_resm(TCONF, "test O_NOATIME flag for openat needs "
			"filesystems which are mounted without "
			"noatime and relatime");
		return;
	}

	SAFE_STAT(cleanup, TEST_FILE, &file_stat);

	sleep(1);

	TEST(openat(AT_FDCWD, TEST_FILE, O_NOATIME | O_RDONLY, 0777));

	if (TEST_RETURN == -1) {
		tst_resm(TFAIL | TTERRNO, "openat failed");
		return;
	}

	SAFE_READ(cleanup, 1, TEST_RETURN, &buf, 1);

	SAFE_CLOSE(cleanup, TEST_RETURN);

	SAFE_STAT(cleanup, TEST_FILE, &file_newstat);

	if (file_stat.st_atime == file_newstat.st_atime)
		tst_resm(TPASS, "test O_NOATIME for openat success");
	else
		tst_resm(TFAIL, "test O_NOATIME for openat failed");
}

void testfunc_nofollow(void)
{
	TEST(openat(AT_FDCWD, SFILE, O_NOFOLLOW | O_RDONLY, 0777));

	if (TEST_RETURN == -1 && TEST_ERRNO == ELOOP) {
		tst_resm(TPASS, "test O_NOFOLLOW for openat success");
	} else {
		tst_resm(TFAIL, "test O_NOFOLLOW for openat failed");
		SAFE_CLOSE(cleanup, TEST_RETURN);
	}
}

void testfunc_trunc(void)
{
	struct stat file_stat;

	TEST(openat(AT_FDCWD, TEST_FILE, O_TRUNC | O_RDWR, 0777));

	if (TEST_RETURN == -1) {
		tst_resm(TFAIL | TTERRNO, "openat failed");
		return;
	}

	SAFE_FSTAT(cleanup, TEST_RETURN, &file_stat);

	if (file_stat.st_size == 0)
		tst_resm(TPASS, "test O_TRUNC for openat success");
	else
		tst_resm(TFAIL, "test O_TRUNC for openat failed");

	SAFE_CLOSE(cleanup, TEST_RETURN);
}

void cleanup(void)
{
	tst_rmdir();
}
