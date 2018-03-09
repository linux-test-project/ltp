/*
 * Copyright (C) 2014 Linux Test Project, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 */

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include "test.h"
#include "safe_macros.h"

#define OUTPUT_FNAME "output"

char *TCID = "trerrno";
int TST_TOTAL = 1;

static void setup(void)
{
	tst_tmpdir();
}

static void cleanup(void)
{
	tst_rmdir();
}

int main(void)
{
	int fd, stdout_fd;
	char msg[4096], *pos;
	FILE *f;

	setup();

	/* redirect stdout to file */
	stdout_fd = dup(fileno(stdout));
	fd = SAFE_OPEN(NULL, OUTPUT_FNAME, O_RDWR | O_CREAT, 0666);
	TEST(dup2(fd, fileno(stdout)));
	if (TEST_RETURN == -1)
		tst_brkm(TBROK | TTERRNO, cleanup, "dup2");

	errno = EPERM;
	TEST_ERRNO = EPERM;
	TEST_RETURN = EINVAL;
	tst_resm(TINFO | TRERRNO, "test");
	tst_old_flush();

	/* restore stdout */
	TEST(dup2(stdout_fd, fileno(stdout)));
	if (TEST_RETURN == -1)
		tst_brkm(TBROK | TTERRNO, cleanup, "dup2");

	/* read file and verify that output is as expected */
	SAFE_LSEEK(cleanup, fd, 0, SEEK_SET);
	f = fdopen(fd, "r");
	if (!f)
		tst_brkm(TBROK | TERRNO, cleanup, "fdopen");
	if (!fgets(msg, sizeof(msg), f))
		tst_brkm(TBROK, cleanup, "fgets");
	fclose(f);

	pos = strchr(msg, '\n');
	if (pos != NULL)
		*pos = '\0';

	tst_resm(TINFO, "%s", msg);
	if (strstr(msg, "EPERM"))
		tst_resm(TFAIL, "EPERM shouldn't be in TRERRNO output");
	if (strstr(msg, "EINVAL"))
		tst_resm(TPASS, "EINVAL should be in TRERRNO output");
	else
		tst_resm(TFAIL, "EINVAL not found in TRERRNO output");

	cleanup();
	tst_exit();
}
