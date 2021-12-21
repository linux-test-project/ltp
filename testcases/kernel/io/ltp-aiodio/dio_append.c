// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2004 Daniel McNeil <daniel@osdl.org>
 *				 2004 Open Source Development Lab
 *				 2004  Marty Ridgeway <mridge@us.ibm.com>
 * Copyright (C) 2021 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Appends zeroed data to a file using O_DIRECT while a child processes are
 * doing buffered reads after seeking to the end of the file and checks if the
 * buffer reads always see zero.
 */

#define _GNU_SOURCE

#include "tst_test.h"
#include "common.h"

static int *run_child;

static char *str_numchildren;
static char *str_writesize;
static char *str_appends;

static int numchildren;
static long long writesize;
static int appends;

static void setup(void)
{
	numchildren = 16;
	writesize = 64 * 1024;
	appends = 1000;

	if (tst_parse_int(str_numchildren, &numchildren, 1, INT_MAX))
		tst_brk(TBROK, "Invalid number of children '%s'", str_numchildren);

	if (tst_parse_filesize(str_writesize, &writesize, 1, LLONG_MAX))
		tst_brk(TBROK, "Invalid write file size '%s'", str_writesize);

	if (tst_parse_int(str_appends, &appends, 1, INT_MAX))
		tst_brk(TBROK, "Invalid number of appends '%s'", str_appends);

	run_child = SAFE_MMAP(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
}

static void cleanup(void)
{
	SAFE_MUNMAP(run_child, sizeof(int));
}

static void run(void)
{
	char *filename = "dio_append";
	int status;
	int i;

	*run_child = 1;

	for (i = 0; i < numchildren; i++) {
		if (!SAFE_FORK()) {
			io_read_eof(filename, run_child);
			return;
		}
	}

	tst_res(TINFO, "Parent append to file");

	io_append(filename, 0, O_DIRECT | O_WRONLY | O_CREAT, writesize, appends);

	if (SAFE_WAITPID(-1, &status, WNOHANG))
		tst_res(TFAIL, "Non zero bytes read");
	else
		tst_res(TPASS, "All bytes read were zeroed");

	*run_child = 0;
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
	.forks_child = 1,
	.options = (struct tst_option[]) {
		{"n:", &str_numchildren, "Number of processes (default 16)"},
		{"w:", &str_writesize, "Write size for each append (default 64K)"},
		{"c:", &str_appends, "Number of appends (default 1000)"},
		{}
	},
};
