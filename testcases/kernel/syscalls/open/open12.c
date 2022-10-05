/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Zeng Linggang <zenglg.jy@cn.fujitsu.com>
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
 * with this program.
 */
/*
 * DESCRIPTION
 *	This test case will verify basic function of open(2) with the flags
 *	O_APPEND, O_NOATIME, O_CLOEXEC and O_LARGEFILE.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <mntent.h>
#include <errno.h>
#include "test.h"
#include "safe_macros.h"
#include "lapi/fcntl.h"
#include "lapi/mount.h"

#define MNTPOINT	"mntpoint"
#define TEST_FILE	MNTPOINT"/test_file"
#define LARGE_FILE	"large_file"

#define DIR_MODE 0755

char *TCID = "open12";

static const char *device;
static unsigned int mount_flag, skip_noatime;

static void setup(void);
static void cleanup(void);
static void test_append(void);
static void test_noatime(void);
static void test_cloexec(void);
static void test_largefile(void);

static void (*test_func[])(void) = { test_append, test_noatime, test_cloexec,
				     test_largefile };

int TST_TOTAL = ARRAY_SIZE(test_func);

int main(int argc, char **argv)
{
	int lc;
	int i;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		for (i = 0; i < TST_TOTAL; i++)
			(*test_func[i])();
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	const char *mount_flags[] = {"noatime", "relatime", NULL};

	TEST_PAUSE;

	tst_sig(FORK, DEF_HANDLER, cleanup);

	tst_tmpdir();

	SAFE_MKDIR(cleanup, MNTPOINT, DIR_MODE);

	if (tst_path_has_mnt_flags(cleanup, NULL, mount_flags)) {
		const char *fs_type;

		if ((tst_kvercmp(2, 6, 30)) < 0) {
			tst_resm(TCONF,
				"MS_STRICTATIME flags for mount(2) needs kernel 2.6.30 "
				"or higher");
			skip_noatime = 1;
			return;
		}

		fs_type = tst_dev_fs_type();
		device = tst_acquire_device(cleanup);

		if (!device) {
			tst_resm(TINFO, "Failed to obtain block device");
			skip_noatime = 1;
			goto end;
		}

		tst_mkfs(cleanup, device, fs_type, NULL, NULL);

		SAFE_MOUNT(cleanup, device, MNTPOINT, fs_type, MS_STRICTATIME, NULL);
		mount_flag = 1;
	}

end:
	SAFE_FILE_PRINTF(cleanup, TEST_FILE, TEST_FILE);
}

static void test_append(void)
{
	off_t len1, len2;

	TEST(open(TEST_FILE, O_RDWR | O_APPEND, 0777));

	if (TEST_RETURN == -1) {
		tst_resm(TFAIL | TTERRNO, "open failed");
		return;
	}

	len1 = SAFE_LSEEK(cleanup, TEST_RETURN, 0, SEEK_CUR);
	SAFE_WRITE(cleanup, SAFE_WRITE_ALL, TEST_RETURN, TEST_FILE,
		sizeof(TEST_FILE));
	len2 = SAFE_LSEEK(cleanup, TEST_RETURN, 0, SEEK_CUR);
	SAFE_CLOSE(cleanup, TEST_RETURN);

	if (len2 > len1)
		tst_resm(TPASS, "test O_APPEND for open success");
	else
		tst_resm(TFAIL, "test O_APPEND for open failed");
}

static void test_noatime(void)
{
	char read_buf;
	struct stat old_stat, new_stat;

	if ((tst_kvercmp(2, 6, 8)) < 0) {
		tst_resm(TCONF,
			 "O_NOATIME flags test for open(2) needs kernel 2.6.8 "
			 "or higher");
		return;
	}

	if (skip_noatime) {
		tst_resm(TCONF,
		         "test O_NOATIME flag for open needs filesystems which "
		         "is mounted without noatime and relatime");
		return;
	}

	SAFE_STAT(cleanup, TEST_FILE, &old_stat);

	sleep(1);

	TEST(open(TEST_FILE, O_RDONLY | O_NOATIME, 0777));

	if (TEST_RETURN == -1) {
		tst_resm(TFAIL | TTERRNO, "open failed");
		return;
	}
	SAFE_READ(cleanup, 1, TEST_RETURN, &read_buf, 1);
	SAFE_CLOSE(cleanup, TEST_RETURN);
	SAFE_STAT(cleanup, TEST_FILE, &new_stat);

	if (old_stat.st_atime == new_stat.st_atime)
		tst_resm(TPASS, "test O_NOATIME for open success");
	else
		tst_resm(TFAIL, "test O_NOATIME for open failed");
}

static void test_cloexec(void)
{
	pid_t pid;
	int status;
	char buf[20];

	if ((tst_kvercmp(2, 6, 23)) < 0) {
		tst_resm(TCONF,
			 "O_CLOEXEC flags test for open(2) needs kernel 2.6.23 "
			 "or higher");
		return;
	}

	TEST(open(TEST_FILE, O_RDWR | O_APPEND | O_CLOEXEC, 0777));

	if (TEST_RETURN == -1) {
		tst_resm(TFAIL | TTERRNO, "open failed");
		return;
	}

	sprintf(buf, "%ld", TEST_RETURN);

	pid = tst_fork();
	if (pid < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "fork() failed");

	if (pid == 0) {
		if (execlp("open12_child", "open12_child", buf, NULL))
			exit(2);
	}

	SAFE_CLOSE(cleanup, TEST_RETURN);

	if (wait(&status) != pid)
		tst_brkm(TBROK | TERRNO, cleanup, "wait() failed");

	if (WIFEXITED(status)) {
		switch ((int8_t)WEXITSTATUS(status)) {
		case 0:
			tst_resm(TPASS, "test O_CLOEXEC for open success");
		break;
		case 1:
			tst_resm(TFAIL, "test O_CLOEXEC for open failed");
		break;
		default:
			tst_brkm(TBROK, cleanup, "execlp() failed");
		}
	} else {
		tst_brkm(TBROK, cleanup,
				 "open12_child exits with unexpected error");
	}
}

static void test_largefile(void)
{
	int fd;
	off64_t offset;

	fd = SAFE_OPEN(cleanup, LARGE_FILE,
				O_LARGEFILE | O_RDWR | O_CREAT, 0777);

	offset = lseek64(fd, 4.1*1024*1024*1024, SEEK_SET);
	if (offset == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "lseek64 failed");

	SAFE_WRITE(cleanup, SAFE_WRITE_ALL, fd, LARGE_FILE,
		sizeof(LARGE_FILE));

	SAFE_CLOSE(cleanup, fd);

	TEST(open(LARGE_FILE, O_LARGEFILE | O_RDONLY, 0777));

	if (TEST_RETURN == -1) {
		tst_resm(TFAIL, "test O_LARGEFILE for open failed");
	} else {
		tst_resm(TPASS, "test O_LARGEFILE for open success");
		SAFE_CLOSE(cleanup, TEST_RETURN);
	}
}

static void cleanup(void)
{
	if (mount_flag && tst_umount(MNTPOINT) == -1)
		tst_brkm(TWARN | TERRNO, NULL, "umount(2) failed");

	if (device)
		tst_release_device(device);

	tst_rmdir();
}
