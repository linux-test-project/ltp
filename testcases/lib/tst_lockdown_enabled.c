// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023 Petr Vorel <pvorel@suse.cz>
 */

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"

int main(void)
{
	return !tst_lockdown_enabled();
}
