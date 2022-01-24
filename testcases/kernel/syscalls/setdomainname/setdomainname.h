// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Petr Vorel <petr.vorel@gmail.com>
 */

#ifndef SETDOMAINNAME_H__
#define SETDOMAINNAME_H__

#include <string.h>
#include "tst_test.h"
#include "lapi/utsname.h"
#include "lapi/syscalls.h"

#define TST_VALID_DOMAIN_NAME "test_dom"

#ifdef TEST_SETHOSTNAME
# define GET_SYSCALL gethostname
# define SET_SYSCALL sethostname
# define SYSCALL_NAME "hostname"
# define SYSCALL_NR __NR_sethostname
#else
# define GET_SYSCALL getdomainname
# define SET_SYSCALL setdomainname
# define SYSCALL_NAME "domainname"
# define SYSCALL_NR __NR_setdomainname
#endif

static char backup[_UTSNAME_DOMAIN_LENGTH];

#define TEST_VARIANTS 2

static void setdomainname_info(void)
{
	switch (tst_variant) {
	case 0:
		tst_res(TINFO, "Testing libc set" SYSCALL_NAME "()");
		break;
	case 1:
		tst_res(TINFO, "Testing __NR_set" SYSCALL_NAME " syscall");
		break;
	}
}

static int do_setdomainname(char *new, size_t len)
{
	switch (tst_variant) {
	case 0:
		return SET_SYSCALL(new, len);
	break;
	case 1:
		return tst_syscall(SYSCALL_NR, new, len);
	}

	return -1;
}

static void setup(void)
{
	setdomainname_info();
	if ((GET_SYSCALL(backup, sizeof(backup))) < 0)
		tst_brk(TBROK | TERRNO, "get" SYSCALL_NAME "() failed");
}

static void cleanup(void)
{
	if ((SET_SYSCALL(backup, strlen(backup))) < 0)
		tst_res(TWARN | TERRNO, "set" SYSCALL_NAME "() failed ('%s')", backup);
}

#endif /* SETDOMAINNAME_H__ */
