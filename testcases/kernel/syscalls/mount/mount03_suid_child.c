// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * Copyright (c) 2022 Petr Vorel <pvorel@suse.cz>
 */

/*
 * SUID to root program invoked by a non-root process to validate the mount
 * flag MS_NOSUID.
 */

#include <errno.h>
#include <unistd.h>

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"

int main(void)
{
	tst_reinit();

	TST_EXP_FAIL(setreuid(getuid(), 0), EPERM);

	return 0;
}
