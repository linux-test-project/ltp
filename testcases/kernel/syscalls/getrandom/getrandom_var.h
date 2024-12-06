/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2024 Jan Stancek <jstancek@redhat.com>
 */

#ifndef GETRANDOM_VAR__
#define GETRANDOM_VAR__

#include "lapi/syscalls.h"

static inline int do_getrandom(void *buf, size_t buflen, unsigned int flags)
{
	switch (tst_variant) {
	case 0:
		return tst_syscall(__NR_getrandom, buf, buflen, flags);
	case 1:
		return getrandom(buf, buflen, flags);
	}
	return -1;
}

static void getrandom_info(void)
{
	switch (tst_variant) {
	case 0:
		tst_res(TINFO, "Testing SYS_getrandom syscall");
		break;
	case 1:
		tst_res(TINFO, "Testing libc getrandom()");
		break;
	}
}

/* if we don't have libc getrandom() test only syscall version */
#ifdef HAVE_SYS_RANDOM_H
# define TEST_VARIANTS 2
#else
# define TEST_VARIANTS 1
#endif

#endif /* GETRANDOM_VAR__ */
