// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Zeng Linggang <zenglg.jy@cn.fujitsu.com>
 * Copyright (c) 2025 SUSE LLC <mdoucha@suse.cz>
 */
/*\
 * This test case will verify basic function of open(2) with the flags
 * O_APPEND, O_NOATIME, O_CLOEXEC and O_LARGEFILE.
 */

#define _GNU_SOURCE

#include <sys/wait.h>
#include "tst_test.h"
#include "tst_safe_macros.h"
#include "lapi/fcntl.h"
#include "lapi/mount.h"

#define MNTPOINT	"mntpoint"
#define TEST_FILE	MNTPOINT "/test_file"
#define LARGE_FILE	MNTPOINT "/large_file"

static int fd = -1;

static void test_append(void);
static void test_noatime(void);
static void test_cloexec(void);
static void test_largefile(void);

static void (*test_func[])(void) = {
	test_append, test_noatime, test_cloexec, test_largefile
};

static void run(unsigned int n)
{
	test_func[n]();
}

static void setup(void)
{
	SAFE_FILE_PRINTF(TEST_FILE, TEST_FILE);
}

static void test_append(void)
{
	off_t len1, len2;

	tst_res(TINFO, "Testing O_APPEND");

	fd = TST_EXP_FD_SILENT(open(TEST_FILE, O_RDWR | O_APPEND, 0644));

	if (!TST_PASS)
		return;

	len1 = SAFE_LSEEK(fd, 0, SEEK_CUR);
	SAFE_WRITE(1, fd, TEST_FILE, strlen(TEST_FILE));
	len2 = SAFE_LSEEK(fd, 0, SEEK_CUR);
	SAFE_CLOSE(fd);

	if (len2 > len1)
		tst_res(TPASS, "O_APPEND works as expected");
	else
		tst_res(TFAIL, "O_APPEND did not move write cursor");
}

static void test_noatime(void)
{
	char read_buf;
	struct stat old_stat, new_stat;

	tst_res(TINFO, "Testing O_NOATIME");

	SAFE_STAT(TEST_FILE, &old_stat);
	sleep(1);
	fd = TST_EXP_FD_SILENT(open(TEST_FILE, O_RDONLY | O_NOATIME, 0644));

	if (!TST_PASS)
		return;

	SAFE_READ(1, fd, &read_buf, 1);
	SAFE_CLOSE(fd);
	SAFE_STAT(TEST_FILE, &new_stat);

	if (old_stat.st_atime == new_stat.st_atime)
		tst_res(TPASS, "File access time was not modified");
	else
		tst_res(TFAIL, "File access time changed");
}

static void test_cloexec(void)
{
	pid_t pid;
	int status;
	char buf[20];

	tst_res(TINFO, "Testing O_CLOEXEC");

	fd = TST_EXP_FD_SILENT(open(TEST_FILE, O_RDWR | O_APPEND | O_CLOEXEC,
		0644));

	if (!TST_PASS)
		return;

	snprintf(buf, sizeof(buf), "%d", fd);
	buf[sizeof(buf) - 1] = '\0';
	pid = SAFE_FORK();

	if (pid == 0) {
		if (execlp("open12_child", "open12_child", buf, NULL))
			exit(2);
	}

	SAFE_CLOSE(fd);

	if (wait(&status) != pid)
		tst_brk(TBROK | TERRNO, "wait() failed");

	if (!WIFEXITED(status))
		tst_brk(TBROK, "open12_child exited with unexpected error");

	switch (WEXITSTATUS(status)) {
	case 0:
		tst_res(TPASS, "File descriptor was closed by execlp()");
		break;
	case 1:
		tst_res(TFAIL, "File descriptor remained open after execlp()");
		break;
	default:
		tst_brk(TBROK, "execlp() failed");
	}
}

static void test_largefile(void)
{
	off_t offset;

	tst_res(TINFO, "Testing O_LARGEFILE");

	fd = TST_EXP_FD_SILENT(open(LARGE_FILE, O_LARGEFILE | O_RDWR | O_CREAT,
		0644));

	if (!TST_PASS)
		return;

	offset = lseek(fd, 4ULL * TST_GB + TST_MB, SEEK_SET);

	if (offset < 0) {
		tst_res(TFAIL | TERRNO, "lseek() past 4GB range failed");
		return;
	}

	SAFE_WRITE(1, fd, LARGE_FILE, strlen(LARGE_FILE));
	SAFE_CLOSE(fd);
	fd = open(LARGE_FILE, O_LARGEFILE | O_RDONLY, 0644);

	if (fd < 0) {
		tst_res(TFAIL | TERRNO, "Cannot open large file again");
		return;
	}

	tst_res(TPASS, "O_LARGEFILE works as expected");
	SAFE_CLOSE(fd);
}

static void cleanup(void)
{
	if (fd >= 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.setup = setup,
	.test = run,
	.cleanup = cleanup,
	.tcnt = ARRAY_SIZE(test_func),
	.forks_child = 1,
	.needs_root = 1,
	.all_filesystems = 1,
	.mntpoint = MNTPOINT,
	.filesystems = (struct tst_fs[]){
		{ .type = NULL, .mnt_flags = MS_STRICTATIME },
		{}
	}
};
