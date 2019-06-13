// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * Copyright (c) 2012 Cyril Hrubis <chrubis@suse.cz>
 */

#include <unistd.h>
#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"

int main(void)
{
	tst_reinit();

	TST_CHECKPOINT_WAKE(0);

	pause();

	return 0;
}
