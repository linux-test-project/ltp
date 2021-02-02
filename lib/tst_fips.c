// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 Petr Vorel <pvorel@suse.cz>
 */

#define TST_NO_DEFAULT_MAIN

#define PATH_FIPS	"/proc/sys/crypto/fips_enabled"

#include "tst_test.h"
#include "tst_safe_macros.h"
#include "tst_fips.h"

int tst_fips_enabled(void)
{
	int fips = 0;

	if (access(PATH_FIPS, R_OK) == 0) {
		SAFE_FILE_SCANF(PATH_FIPS, "%d", &fips);
	}

	tst_res(TINFO, "FIPS: %s", fips ? "on" : "off");
	return fips;
}
