// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 SUSE LLC
 * Author: Christian Amann <camann@suse.com>
 */

/*
 * Tests basic error handling of the
 * copy_file_range syscall
 *
 * 1) Try to copy contents to file open as readonly
 *    -> EBADF
 * 2) Try to copy contents to directory -> EISDIR
 * 3) Try to copy contents to a file opened with the
 *    O_APPEND flag -> EBADF
 * 4) Try to copy contents to closed filedescriptor
 *    -> EBADF
 * 5) Try to copy contents with invalid 'flags' value
 *    -> EINVAL
 * 6) Try to copy contents to a file chattred with +i
 *    flag -> EPERM
 * 7) Try to copy contents to a swapfile ->ETXTBSY
 * 8) Try to copy contents to the samefile with overlapping
 *    ->EINVAL
 * 9) Try to copy contents to a blkdev ->EINVAL
 * 10) Try to copy contents to a chardev ->EINVAL
 * 11) Try to copy contents to a FIFO ->EINVAL
 * 12) Try to copy contents to a file with length beyond
 *     16EiB wraps around 0 -> EOVERFLOW
 * 13) Try to copy contents to a file with target file range
 *     beyond maximum supported file size ->EFBIG
 */

#define _GNU_SOURCE

#include "tst_test.h"
#include "copy_file_range.h"

static int fd_src;
static int fd_dest;
static int fd_rdonly;
static int fd_dir;
static int fd_closed;
static int fd_append;
static int fd_immutable;
static int fd_swapfile;
static int fd_dup;
static int fd_blkdev;
static int fd_chrdev;
static int fd_fifo;
static int fd_copy;

static int chattr_i_nsup;
static int swap_nsup;
static int loop_devn;

static struct tcase {
	int	*copy_to_fd;
	int	flags;
	int	exp_err;
	loff_t  dst;
	loff_t     len;
} tcases[] = {
	{&fd_rdonly,	0,   EBADF,      0,     CONTSIZE},
	{&fd_dir,	0,   EISDIR,     0,     CONTSIZE},
	{&fd_append,	0,   EBADF,      0,     CONTSIZE},
	{&fd_closed,	0,   EBADF,      0,     CONTSIZE},
	{&fd_dest,	-1,  EINVAL,     0,     CONTSIZE},
	{&fd_immutable, 0,   EPERM,      0,     CONTSIZE},
	{&fd_swapfile,  0,   ETXTBSY,    0,     CONTSIZE},
	{&fd_dup,       0,   EINVAL,     0,     CONTSIZE/2},
	{&fd_blkdev,    0,   EINVAL,     0,     CONTSIZE},
	{&fd_chrdev,    0,   EINVAL,     0,     CONTSIZE},
	{&fd_fifo,      0,   EINVAL,     0,     CONTSIZE},
	{&fd_copy,      0,   EOVERFLOW,  MAX_OFF, ULLONG_MAX},
	{&fd_copy,      0,   EFBIG,      MAX_OFF, MIN_OFF},
};

static int run_command(char *command, char *option, char *file)
{
	const char *const cmd[] = {command, option, file, NULL};
	int ret;

	ret = tst_run_cmd(cmd, NULL, NULL, 1);
	switch (ret) {
	case 0:
	return 0;
	case 255:
		tst_res(TCONF, "%s binary not installed", command);
	return 1;
	default:
		tst_res(TCONF, "%s exited with %i", command, ret);
	return 2;
	}
}

static void verify_copy_file_range(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	if (tc->copy_to_fd == &fd_immutable && chattr_i_nsup) {
		tst_res(TCONF, "filesystem doesn't support chattr +i, skip it");
		return;
	}
	if (tc->copy_to_fd == &fd_swapfile && swap_nsup) {
		tst_res(TCONF, "filesystem doesn't support swapfile, skip it");
		return;
	}
	if (tc->copy_to_fd == &fd_blkdev && loop_devn == -1) {
		tst_res(TCONF, "filesystem doesn't have free loopdev, skip it");
		return;
	}
	TEST(sys_copy_file_range(fd_src, 0, *tc->copy_to_fd,
				&tc->dst, tc->len, tc->flags));

	if (TST_RET == -1) {
		if (tc->exp_err == TST_ERR) {
			tst_res(TPASS | TTERRNO,
					"copy_file_range failed as expected");
		} else {
			tst_res(TFAIL | TTERRNO,
				"copy_file_range failed unexpectedly; expected %s, but got",
				tst_strerrno(tc->exp_err));
			return;
		}
	} else {
		tst_res(TFAIL,
			"copy_file_range returned wrong value: %ld", TST_RET);
	}
}

static void cleanup(void)
{
	if (fd_append > 0)
		SAFE_CLOSE(fd_append);
	if (fd_dir > 0)
		SAFE_CLOSE(fd_dir);
	if (fd_rdonly > 0)
		SAFE_CLOSE(fd_rdonly);
	if (fd_dest > 0)
		SAFE_CLOSE(fd_dest);
	if (fd_src > 0)
		SAFE_CLOSE(fd_src);
	if (fd_immutable > 0) {
		run_command("chattr", "-i", FILE_IMMUTABLE_PATH);
		SAFE_CLOSE(fd_immutable);
	}
	if (fd_swapfile > 0) {
		run_command("swapoff", FILE_SWAP_PATH, NULL);
		SAFE_CLOSE(fd_swapfile);
	}
	if (fd_dup > 0)
		SAFE_CLOSE(fd_dup);
	if (fd_copy > 0)
		SAFE_CLOSE(fd_copy);
	SAFE_UNLINK(FILE_FIFO);
}

static void setup(void)
{
	syscall_info();
	char dev_path[1024];

	if (access(FILE_DIR_PATH, F_OK) == -1)
		SAFE_MKDIR(FILE_DIR_PATH, 0777);
	/*
	 * call tst_find_free_loopdev(), avoid overwriting its
	 * content on used loopdev.
	 */
	loop_devn = tst_find_free_loopdev(dev_path, sizeof(dev_path));

	SAFE_MKNOD(FILE_FIFO, S_IFIFO | 0777, 0);

	fd_src    = SAFE_OPEN(FILE_SRC_PATH, O_RDWR | O_CREAT, 0664);
	fd_dest   = SAFE_OPEN(FILE_DEST_PATH, O_RDWR | O_CREAT, 0664);
	fd_rdonly = SAFE_OPEN(FILE_RDONL_PATH, O_RDONLY | O_CREAT, 0664);
	fd_dir    = SAFE_OPEN(FILE_DIR_PATH, O_DIRECTORY);
	fd_closed = -1;
	fd_append = SAFE_OPEN(FILE_DEST_PATH,
			O_RDWR | O_CREAT | O_APPEND, 0664);
	fd_immutable = SAFE_OPEN(FILE_IMMUTABLE_PATH, O_RDWR | O_CREAT, 0664);
	fd_swapfile = SAFE_OPEN(FILE_SWAP_PATH, O_RDWR | O_CREAT, 0600);

	if (loop_devn == -1)
		fd_blkdev = SAFE_OPEN(dev_path, O_RDWR, 0600);

	fd_chrdev = SAFE_OPEN(FILE_CHRDEV, O_RDWR, 0600);
	fd_fifo = SAFE_OPEN(FILE_FIFO, O_RDWR, 0600);

	SAFE_WRITE(1, fd_src, CONTENT, CONTSIZE);
	close(fd_src);
	fd_src = SAFE_OPEN(FILE_SRC_PATH, O_RDONLY, 0664);
	fd_dup = SAFE_OPEN(FILE_SRC_PATH, O_WRONLY|O_CREAT, 0666);

	fd_copy = SAFE_OPEN(FILE_COPY_PATH, O_RDWR | O_CREAT | O_TRUNC, 0664);
	chattr_i_nsup = run_command("chattr", "+i", FILE_IMMUTABLE_PATH);

	if (!tst_fs_has_free(".", sysconf(_SC_PAGESIZE) * 10, TST_BYTES)) {
		tst_res(TCONF, "Insufficient disk space to create swap file");
		swap_nsup = 3;
		return;
	}

	if (tst_fill_file(FILE_SWAP_PATH, 0, sysconf(_SC_PAGESIZE), 10) != 0) {
		tst_res(TCONF, "Failed to create swapfile");
		swap_nsup = 4;
		return;
	}

	swap_nsup = run_command("mkswap", FILE_SWAP_PATH, NULL);
	swap_nsup = run_command("swapon", FILE_SWAP_PATH, NULL);
}

static struct tst_test test = {
	.test = verify_copy_file_range,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.mount_device = 1,
	.mntpoint = MNTPOINT,
	.all_filesystems = 1,
	.test_variants = TEST_VARIANTS,
};
