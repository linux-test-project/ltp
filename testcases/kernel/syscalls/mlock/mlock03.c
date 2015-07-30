/*
 * Stack size mapping is decreased through mlock/munlock call.
 *
 * This is to test kernel if it has a problem with shortening [stack]
 * mapping through several loops of mlock/munlock of /proc/self/maps.
 *
 * From:
 * munlock     76KiB bfef2000-bff05000 rw-p 00000000 00:00 0          [stack]
 *
 * To:
 * munlock     44KiB bfefa000-bff05000 rw-p 00000000 00:00 0          [stack]
 *
 * with more iterations - could drop to 0KiB.
 *
 * Copyright (C) 2010  Red Hat, Inc.
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it
 * is free of the rightful claim of any third person regarding
 * infringement or the like.  Any license provided herein, whether
 * implied or otherwise, applies only to this software file.  Patent
 * licenses, if any, provided herein do not apply to combinations of
 * this program with other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include "test.h"

#define KB 1024

char *TCID = "mlock03";
int TST_TOTAL = 1;

static void setup(void);
static void cleanup(void);

int main(int argc, char *argv[])
{
	int lc;
	long from, to;
	long first = -1, last = -1;
	char b[KB];
	FILE *fp;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		fp = fopen("/proc/self/maps", "r");
		if (fp == NULL)
			tst_brkm(TBROK | TERRNO, cleanup, "fopen");
		while (!feof(fp)) {
			if (!fgets(b, KB - 1, fp))
				break;
			b[strlen(b) - 1] = '\0';
			sscanf(b, "%lx-%lx", &from, &to);

			/* Record the initial stack size. */
			if (lc == 0 && strstr(b, "[stack]") != NULL)
				first = (to - from) / KB;

			switch (lc & 1) {
			case 0:
				if (mlock((const void *)from, to - from) == -1)
					tst_resm(TINFO | TERRNO,
						 "mlock failed");
				break;
			case 1:
				if (munlock((void *)from, to - from) == -1)
					tst_resm(TINFO | TERRNO,
						 "munlock failed");
				break;
			default:
				break;
			}
			tst_resm(TINFO, "%s from %lx to %0lx",
				 (lc & 1) ? "munlock" : "mlock", from, to);

			/* Record the final stack size. */
			if (strstr(b, "[stack]") != NULL)
				last = (to - from) / KB;
		}
		fclose(fp);
	}
	tst_resm(TINFO, "starting stack size is %ld", first);
	tst_resm(TINFO, "final stack size is %ld", last);
	if (last < first)
		tst_resm(TFAIL, "stack size is decreased.");
	else
		tst_resm(TPASS, "stack size is not decreased.");

	cleanup();
	tst_exit();
}

void setup(void)
{
	tst_require_root();

	tst_sig(FORK, DEF_HANDLER, cleanup);
	TEST_PAUSE;
}

void cleanup(void)
{
}
