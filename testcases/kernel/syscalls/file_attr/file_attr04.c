// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2025 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify that `file_getattr` and `file_setattr` are correctly raising an error
 * when the wrong file descriptors types are passed to them.
 */

#include "tst_test.h"
#include "lapi/fs.h"

#define FILENAME "ltp_file"

static struct file_attr *attr;

static void test_invalid_fd(struct tst_fd *fd)
{
	if (fd->type == TST_FD_DIR || fd->type == TST_FD_OPEN_TREE) {
		tst_res(TCONF, "Skipping DIR fd");
		return;
	}

	memset(attr, 0, sizeof(*attr));

	if (tst_variant) {
		TST_EXP_FAIL(file_getattr(
			fd->fd, FILENAME,
			attr, FILE_ATTR_SIZE_LATEST,
			0), ENOTDIR);
	} else {
		TST_EXP_FAIL(file_setattr(
			fd->fd, FILENAME,
			attr, FILE_ATTR_SIZE_LATEST,
			0), ENOTDIR);
	}
}

static void run(void)
{
	TST_FD_FOREACH(fd) {
		tst_res(TINFO, "%s -> ...", tst_fd_desc(&fd));
		test_invalid_fd(&fd);
	}
}

static void setup(void)
{
	SAFE_TOUCH(FILENAME, 0640, NULL);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.test_variants = 2,
	.needs_tmpdir = 1,
	.bufs = (struct tst_buffers []) {
		{&attr, .size = sizeof(struct file_attr)},
		{}
	}
};
