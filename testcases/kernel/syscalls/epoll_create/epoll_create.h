// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2021
 */

#ifndef EPOLL_CREATE_H__
#define EPOLL_CREATE_H__

#define EPOLL_CREATE_VARIANTS 2

static int do_epoll_create(int size)
{
	switch (tst_variant) {
	case 0:
		return tst_syscall(__NR_epoll_create, size);
	break;
	case 1:
		return epoll_create(size);
	break;
	}

	return -1;
}

static void variant_info(void)
{
	switch (tst_variant) {
	case 0:
		tst_res(TINFO, "Testing variant: syscall __NR_epoll_create");
	break;
	case 1:
		tst_res(TINFO, "Testing variant: libc epoll_create()");
	break;
	}
}

#endif /* EPOLL_CREATE_H__ */
