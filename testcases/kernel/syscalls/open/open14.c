// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2015-2016 Oracle and/or its affiliates. All Rights Reserved.
 * Author: Alexey Kodanev <alexey.kodanev@oracle.com>
 * Copyright (c) 2025 SUSE LLC <mdoucha@suse.cz>
 */

/*\
 * Check the functionality of O_TMPFILE flag for open() syscall:
 *
 * 1) Creation and linking (naming) of a single temp file
 * 2) Creation of multiple unlinked temp files in a hierarchy of directories
 * 3) Access permissions of linked temp files match creation mode argument
 */

#include "tst_test.h"
#include "tst_safe_macros.h"
#include "lapi/fcntl.h"

#define FILE_COUNT 100
#define MNTPOINT "mntpoint"

static char buf[1024];
static int fds[FILE_COUNT];
static const ssize_t size = sizeof(buf);
static const ssize_t blocks_num = 4;

static void setup(void)
{
	int i;

	for (i = 0; i < FILE_COUNT; i++)
		fds[i] = -1;

	memset(buf, 1, size);
	SAFE_CHDIR(MNTPOINT);
	fds[0] = open(".", O_TMPFILE | O_RDWR, 0600);

	if (fds[0] == -1) {
		if (errno == EISDIR || errno == ENOTSUP)
			tst_brk(TCONF, "O_TMPFILE not supported");

		tst_brk(TBROK | TERRNO, "open() failed");
	}

	SAFE_CLOSE(fds[0]);
}

static void write_file(int fd)
{
	int i;

	for (i = 0; i < blocks_num; ++i)
		SAFE_WRITE(1, fd, buf, size);
}

static void test01(void)
{
	struct stat st;
	char path[PATH_MAX];

	tst_res(TINFO, "Testing creation and linking of single temp file");
	fds[0] = SAFE_OPEN(".", O_TMPFILE | O_RDWR, 0600);
	write_file(fds[0]);
	SAFE_FSTAT(fds[0], &st);

	if (st.st_size != blocks_num * size) {
		tst_res(TFAIL, "Unexpected test file size: %li != %zu",
			 (long)st.st_size, blocks_num * size);
	} else {
		tst_res(TPASS, "Test file size is %li", (long)st.st_size);
	}

	if (!tst_dir_is_empty(".", 1))
		tst_res(TFAIL, "Test directory is not empty");
	else
		tst_res(TPASS, "Test directory is empty");

	snprintf(path, PATH_MAX, "/proc/self/fd/%d", fds[0]);
	tst_res(TINFO, "Linking unnamed test file to 'tmpfile'");
	SAFE_LINKAT(AT_FDCWD, path, AT_FDCWD, "tmpfile", AT_SYMLINK_FOLLOW);

	if (tst_dir_is_empty(".", 1)) {
		tst_res(TFAIL, "Test directory is still empty");
		SAFE_CLOSE(fds[0]);
		return;
	}

	if (access("tmpfile", F_OK)) {
		tst_res(TFAIL | TERRNO, "Linked test file not found");
		SAFE_CLOSE(fds[0]);
		return;
	}

	tst_res(TPASS, "Test file was linked correctly");
	SAFE_UNLINK("tmpfile");
	SAFE_CLOSE(fds[0]);
}

static int read_file(int fd)
{
	int i;
	char tmp[size];

	SAFE_LSEEK(fd, 0, SEEK_SET);

	for (i = 0; i < blocks_num; ++i) {
		SAFE_READ(0, fd, tmp, size);

		if (memcmp(buf, tmp, size)) {
			tst_res(TFAIL, "got unexepected data");
			return 1;
		}
	}

	return 0;
}

static void test02(void)
{
	int i, fails = 0;
	char path[PATH_MAX];

	tst_res(TINFO, "Testing temp files in multiple directories");
	for (i = 0; i < FILE_COUNT; ++i) {
		snprintf(path, PATH_MAX, "tst02_%d", i);
		SAFE_MKDIR(path, 0700);
		SAFE_CHDIR(path);
		fds[i] = SAFE_OPEN(".", O_TMPFILE | O_RDWR, 0600);
	}

	tst_res(TINFO, "Removing test directories");
	for (i = FILE_COUNT - 1; i >= 0; --i) {
		SAFE_CHDIR("..");
		snprintf(path, PATH_MAX, "tst02_%d", i);
		SAFE_RMDIR(path);
	}

	tst_res(TINFO, "Writing and reading temporary files");
	for (i = 0; i < FILE_COUNT; ++i) {
		write_file(fds[i]);
		fails += read_file(fds[i]);
	}

	tst_res(TINFO, "Closing temporary files");
	for (i = 0; i < FILE_COUNT; ++i)
		SAFE_CLOSE(fds[i]);

	if (!fails)
		tst_res(TPASS, "Multiple files test passed");
}

static void link_tmp_file(int fd)
{
	char path1[PATH_MAX], path2[PATH_MAX];

	snprintf(path1, PATH_MAX, "/proc/self/fd/%d", fd);
	snprintf(path2, PATH_MAX, "tmpfile_%d", fd);
	SAFE_LINKAT(AT_FDCWD, path1, AT_FDCWD, path2, AT_SYMLINK_FOLLOW);
}

static void test03(void)
{
	const mode_t test_perms[] = { 0, 07777, 001, 0755, 0644, 0440 };

	int i, fails = 0;
	char path[PATH_MAX];
	struct stat st;
	mode_t mask = umask(0), perm;

	umask(mask);
	tst_res(TINFO, "Testing linked temp files access mode");

	for (i = 0; i < FILE_COUNT; ++i) {
		snprintf(path, PATH_MAX, "tst03_%d", i);
		SAFE_MKDIR(path, 0700);
		SAFE_CHDIR(path);

		perm = test_perms[i % ARRAY_SIZE(test_perms)];
		fds[i] = SAFE_OPEN(".", O_TMPFILE | O_RDWR, perm);
		write_file(fds[i]);
		read_file(fds[i]);
		link_tmp_file(fds[i]);

		snprintf(path, PATH_MAX, "tmpfile_%d", fds[i]);
		SAFE_LSTAT(path, &st);
		perm &= ~mask;

		if ((st.st_mode & ~S_IFMT) != perm) {
			tst_res(TFAIL, "Unexpected access mode: %04o != %04o",
				st.st_mode & ~S_IFMT, perm);
			fails++;
		}
	}

	if (!fails)
		tst_res(TPASS, "File access modes are correct");

	tst_res(TINFO, "Removing files and directories");

	for (i = FILE_COUNT - 1; i >= 0; --i) {
		snprintf(path, PATH_MAX, "tmpfile_%d", fds[i]);
		SAFE_UNLINK(path);
		SAFE_CLOSE(fds[i]);

		SAFE_CHDIR("..");
		snprintf(path, PATH_MAX, "tst03_%d", i);
		SAFE_RMDIR(path);
	}
}

static void run(void)
{
	test01();
	test02();
	test03();
}

static void cleanup(void)
{
	int i;

	for (i = 0; i < FILE_COUNT; i++) {
		if (fds[i] >= 0)
			SAFE_CLOSE(fds[i]);
	}

	SAFE_CHDIR("..");
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.cleanup = cleanup,
	.needs_root = 1,
	.mntpoint = MNTPOINT,
	.all_filesystems = 1
};
