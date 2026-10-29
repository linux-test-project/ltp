// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *	07/2001 Ported by Wayne Boyer
 *	10/2008 Suzuki K P <suzuki@in.ibm.com>
 *
 * Copyright (C) 2026 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify that a forked child can close all the files which have been open by
 * the parent process, after closing and re-opening.
 */

#include "tst_test.h"
#include "tst_safe_stdio.h"

#define FILE_PREFIX "ltp_file"

static FILE **open_files;
static long file_open_max;

static void run(void)
{
	FILE *f;
	long nfiles;
	long totfiles;
	int name_len;
	char name[PATH_MAX];
	char reopen[PATH_MAX];

	memset(reopen, 0, PATH_MAX);

	tst_res(TINFO, "Opening files from parent");

	for (nfiles = 0; nfiles < file_open_max; nfiles++) {
		name_len = snprintf(name, PATH_MAX, "%s%lu", FILE_PREFIX, nfiles);
		if (!nfiles)
			memcpy(reopen, name, name_len);

		f = fopen(name, "a");
		if (!f) {
			/* raised if we reached OPEN_MAX */
			if (errno == EMFILE)
				break;

			tst_brk(TBROK | TERRNO, "fopen() error");
		}

		open_files[nfiles] = f;
	}

	totfiles = nfiles;

	if (!totfiles)
		tst_brk(TBROK, "Parent couldn't open any file");

	tst_res(TINFO, "Closing %lu files from child", totfiles);

	if (!SAFE_FORK()) {
		SAFE_FCLOSE(open_files[0]);
		open_files[0] = SAFE_FOPEN(reopen, "a");

		for (nfiles = nfiles - 1; nfiles >= 0; nfiles--)
			SAFE_FCLOSE(open_files[nfiles]);

		_exit(0);
	}

	tst_reap_children();

	tst_res(TPASS, "Child closed all parent's files");

	for (nfiles = 0; nfiles < totfiles; nfiles++) {
		snprintf(name, PATH_MAX, "%s%lu", FILE_PREFIX, nfiles);

		SAFE_FCLOSE(open_files[nfiles]);
		SAFE_UNLINK(name);
	}
}

static void setup(void)
{
	file_open_max = sysconf(_SC_OPEN_MAX);
	open_files = SAFE_MALLOC(sizeof(FILE *) * file_open_max);
}

static void cleanup(void)
{
	free(open_files);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
	.needs_tmpdir = 1,
};
