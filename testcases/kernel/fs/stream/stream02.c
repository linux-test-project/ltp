// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2002
 *	ported from SPIE section2/filesuite/stream2.c, by Airong Zhang
 *
 * Copyright (c) 2026 Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify that it's possible to `fopen()` a file that has been created by
 * `mknod()` using different modes.
 */

#include "tst_test.h"
#include "tst_safe_stdio.h"

#define FILENAME "ltp_file_node"

static const char *const modes[] = {
	"r+",
	"w+",
	"a+",
};

static void run(void)
{
	FILE *stream;

	SAFE_MKNOD(FILENAME, (S_IFIFO | 0666), 0);

	for (size_t i = 0; i < ARRAY_SIZE(modes); i++) {
		TST_EXP_PASS_PTR_NULL(fopen(FILENAME, modes[i]),
			"fopen(%s, %s)", FILENAME, modes[i]);

		if (TST_PASS)
			SAFE_FCLOSE(TST_RET_PTR);
	}

	SAFE_UNLINK(FILENAME);
}

static struct tst_test test = {
	.test_all = run,
	.needs_tmpdir = 1,
};
