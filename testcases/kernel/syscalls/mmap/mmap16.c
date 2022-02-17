// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2015 Fujitsu Ltd. Xiaoguang Wang <wangxg.fnst@cn.fujitsu.com>
 * Copyright (C) 2021 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * This is a regression test for a silent data corruption for a mmaped file
 * when filesystem gets out of space.
 *
 * Fixed by commits:
 *
 *  commit 0572639ff66dcffe62d37adfe4c4576f9fc398f4
 *  Author: Xiaoguang Wang <wangxg.fnst@cn.fujitsu.com>
 *  Date:   Thu Feb 12 23:00:17 2015 -0500
 *
 *      ext4: fix mmap data corruption in nodelalloc mode when blocksize < pagesize
 *
 *  commit d6320cbfc92910a3e5f10c42d98c231c98db4f60
 *  Author: Jan Kara <jack@suse.cz>
 *  Date:   Wed Oct 1 21:49:46 2014 -0400
 *
 *      ext4: fix mmap data corruption when blocksize < pagesize
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include "tst_test.h"

#define MNTPOINT "mntpoint"
#define FILE_PARENT "mntpoint/testfilep"
#define FILE_CHILD "mntpoint/testfilec"
#define FS_BLOCKSIZE 1024
#define LOOPS 10

static int parentfd = -1;
static int childfd = -1;

static void do_child(void)
{
	int offset;
	int page_size;
	char buf[FS_BLOCKSIZE];
	char *addr = NULL;

	page_size = getpagesize();

	childfd = SAFE_OPEN(FILE_CHILD, O_RDWR | O_CREAT, 0666);

	memset(buf, 'a', FS_BLOCKSIZE);
	SAFE_WRITE(1, childfd, buf, FS_BLOCKSIZE);

	/*
	 * In case mremap() may fail because that memory area can not be
	 * expanded at current virtual address(MREMAP_MAYMOVE is not set),
	 * we first do a mmap(page_size * 2) operation to reserve some
	 * free address space.
	 */
	addr = SAFE_MMAP(NULL, page_size * 2, PROT_WRITE | PROT_READ,
			 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	SAFE_MUNMAP(addr, page_size * 2);

	addr = SAFE_MMAP(addr, FS_BLOCKSIZE, PROT_WRITE | PROT_READ, MAP_SHARED, childfd, 0);

	addr[0] = 'a';

	SAFE_FTRUNCATE(childfd, page_size * 2);

	addr = mremap(addr, FS_BLOCKSIZE, 2 * page_size, 0);
	if (addr == MAP_FAILED)
		tst_brk(TBROK | TERRNO, "mremap failed unexpectedly");

	/*
	 * Let parent process consume FS free blocks as many as possible, then
	 * there'll be no free blocks allocated for this new file mmaping for
	 * offset starting at 1024, 2048, or 3072. If this above kernel bug
	 * has been fixed, usually child process will killed by SIGBUS signal,
	 * if not, the data 'A', 'B', 'C' will be silently discarded later when
	 * kernel writepage is called, that means data corruption.
	 */
	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	for (offset = FS_BLOCKSIZE; offset < page_size; offset += FS_BLOCKSIZE)
		addr[offset] = 'a';

	SAFE_MUNMAP(addr, 2 * page_size);
	SAFE_CLOSE(childfd);

	exit(1);
}

static void run_single(void)
{
	int ret, status;
	pid_t child;
	char buf[FS_BLOCKSIZE];
	int bug_reproduced = 0;

	child = SAFE_FORK();
	if (!child) {
		do_child();
		return;
	}

	parentfd = SAFE_OPEN(FILE_PARENT, O_RDWR | O_CREAT, 0666);

	memset(buf, 'a', FS_BLOCKSIZE);

	TST_CHECKPOINT_WAIT(0);

	while (1) {
		ret = write(parentfd, buf, FS_BLOCKSIZE);
		if (ret < 0) {
			if (errno == ENOSPC)
				break;

			tst_brk(TBROK | TERRNO, "write failed unexpectedly");
		}
	}

	SAFE_CLOSE(parentfd);

	TST_CHECKPOINT_WAKE(0);

	SAFE_WAITPID(child, &status, 0);
	if (WIFEXITED(status) && WEXITSTATUS(status) == 1) {
		bug_reproduced = 1;
	} else {
		/*
		 * If child process was killed by SIGBUS, bug is not reproduced.
		 */
		if (!WIFSIGNALED(status) || WTERMSIG(status) != SIGBUS) {
			tst_brk(TBROK | TERRNO, "child process terminate unexpectedly with status %s",
				tst_strstatus(status));
		}
	}

	SAFE_UNLINK(FILE_PARENT);
	SAFE_UNLINK(FILE_CHILD);

	if (bug_reproduced)
		tst_res(TFAIL, "bug is reproduced");
	else
		tst_res(TPASS, "bug is not reproduced");
}

static void run(void)
{
	int i;

	for (i = 0; i < LOOPS; i++)
		run_single();
}

static void cleanup(void)
{
	if (childfd >= 0)
		SAFE_CLOSE(childfd);

	if (parentfd >= 0)
		SAFE_CLOSE(parentfd);
}

static struct tst_test test = {
	.test_all = run,
	.cleanup = cleanup,
	.forks_child = 1,
	.needs_root = 1,
	.needs_checkpoints = 1,
	.mount_device = 1,
	.mntpoint = MNTPOINT,
	.dev_fs_type = "ext4",
	.dev_fs_opts = (const char *const[]){
		"-b",
		"1024",
		NULL,
	},
	.dev_extra_opts = (const char *const[]){
		"10240",
		NULL,
	},
	.needs_cmds = (const char *const[]){
		"mkfs.ext4",
		NULL,
	},
	.tags = (const struct tst_tag[]){
		{"linux-git", "d6320cbfc929"},
		{"linux-git", "0572639ff66d"},
		{},
	},
};
