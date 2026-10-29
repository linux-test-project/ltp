// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2010-2020
 * Copyright (c) International Business Machines Corp., 2009
 *
 * Authors:
 * Mimi Zohar <zohar@us.ibm.com>
 */

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"

#define MMAPSIZE 1024

int main(int argc, char *argv[])
{
	int fd;
	void *file;

	tst_reinit();

	if (argc != 2)
		tst_brk(TBROK, "usage: ima_mmap <filename>");

	fd = SAFE_OPEN(argv[1], O_CREAT | O_RDWR, S_IRWXU);
	file = SAFE_MMAP(NULL, MMAPSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	SAFE_CLOSE(fd);

	/* Waiting until ima_violations.sh open and close file */
	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	SAFE_MUNMAP(file, MMAPSIZE);
	tst_res(TPASS, "test completed");

	return 0;
}
