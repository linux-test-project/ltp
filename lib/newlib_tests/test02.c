// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * The tst_resm() and tst_brkm() should be rerouted to the new lib.
 */

#include "tst_test.h"

void tst_resm_(char *, int, int, char *);
void tst_brkm_(char *, int, int, void (*)(void), char *);

static void cleanup(void)
{
}

static void do_test(unsigned int i)
{
	switch (i) {
	case 0:
		tst_resm_(__FILE__, __LINE__, TINFO, "info message");
		tst_resm_(__FILE__, __LINE__, TPASS, "passed message");
	break;
	case 1:
		tst_brkm_(__FILE__, __LINE__, TCONF, cleanup, "Non-NULL cleanup");
	break;
	}
}

static struct tst_test test = {
	.tcnt = 2,
	.test = do_test,
};
