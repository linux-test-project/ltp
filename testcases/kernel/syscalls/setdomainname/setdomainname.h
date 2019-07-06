// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Petr Vorel <petr.vorel@gmail.com>
 */

#ifndef SETDOMAINNAME_H__
#define SETDOMAINNAME_H__

#include <string.h>
#include "lapi/utsname.h"
#include "lapi/syscalls.h"
#include "tst_test.h"

#define TST_VALID_DOMAIN_NAME "test_dom"

static char backup[_UTSNAME_DOMAIN_LENGTH];

#define TEST_VARIANTS 2

static void setdomainname_info(void)
{
	switch (tst_variant) {
	case 0:
		tst_res(TINFO, "Testing libc setdomainname()");
		break;
	case 1:
		tst_res(TINFO, "Testing __NR_setdomainname syscall");
		break;
	}
}

static int do_setdomainname(char *new, size_t len)
{
	switch (tst_variant) {
	case 0:
		return setdomainname(new, len);
	break;
	case 1:
		return tst_syscall(__NR_setdomainname, new, len);
	}

	return -1;
}

static void setup(void)
{
	setdomainname_info();
	if ((getdomainname(backup, sizeof(backup))) < 0)
		tst_brk(TBROK | TERRNO, "getdomainname() failed");
}

static void cleanup(void)
{
	if ((setdomainname(backup, strlen(backup))) < 0)
		tst_res(TWARN | TERRNO, "setdomainname() failed ('%s')", backup);
}

#endif /* SETDOMAINNAME_H__ */
