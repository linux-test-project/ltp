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
#include "tso_safe_macros.h"

#define OUTPUT_FNAME "output"
#define LTPROOT "/opt/ltp"

char *TCID = "dataroot";
int TST_TOTAL = 1;

static void cmp_paths(const char *p1, const char *p2, const char *s)
{
	if (strncmp(p1, p2, PATH_MAX) == 0)
		tst_resm(TPASS, "%s", s);
	else
		tst_resm(TFAIL, "%s, %s != %s", s, p1, p2);
}

int main(void)
{
	const char *dataroot;
	char tmp[PATH_MAX];

	/* LTPROOT */
	setenv("LTPROOT", LTPROOT, 1);
	dataroot = tst_dataroot();
	snprintf(tmp, PATH_MAX, "%s/testcases/data/%s", LTPROOT, TCID);
	cmp_paths(dataroot, tmp, "LTPROOT, no tmpdir, "
		"dataroot == $LTPROOT/testcases/data/$TCID");

	tst_exit();
}

