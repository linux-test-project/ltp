// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022 SUSE LLC <rpalethorpe@suse.com>
 */
#define _GNU_SOURCE
#define TST_NO_DEFAULT_MAIN

#include "tst_test.h"
#include "tst_epoll.h"

int safe_epoll_create1(const char *const file, const int lineno,
		       const int flags)
{
	const char *flags_str;
	int ret = epoll_create1(flags);

	switch (flags) {
	case EPOLL_CLOEXEC:
		flags_str = "EPOLL_CLOEXEC";
		break;
	case 0:
		flags_str = "";
		break;
	default:
		flags_str = "???";
	}

	if (ret == -1) {
		tst_brk_(file, lineno,
			 TBROK | TERRNO, "epoll_create1(%s)", flags_str);
	}

	return ret;
}

int safe_epoll_ctl(const char *const file, const int lineno,
		   int epfd, int op, int fd, struct epoll_event *ev)
{
	const char *op_str;
	int ret;

	switch (op) {
	case EPOLL_CTL_ADD:
		op_str = "EPOLL_CTL_ADD";
		break;
	case EPOLL_CTL_DEL:
		op_str = "EPOLL_CTL_DEL";
		break;
	case EPOLL_CTL_MOD:
		op_str = "EPOLL_CTL_MOD";
		break;
	default:
		op_str = "???";
	}

	ret = epoll_ctl(epfd, op, fd, ev);

	if (ret == -1) {
		tst_brk_(file, lineno,
			 TBROK | TERRNO,
			 "epoll_ctl(%d, %s, %d, ...", epfd, op_str, fd);
	}

	return ret;
}

int safe_epoll_wait(const char *const file, const int lineno,
		    int epfd, struct epoll_event *events,
		    int maxevents, int timeout)
{
	int ret = epoll_wait(epfd, events, maxevents, timeout);

	if (ret == -1) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			 "epoll_wait(%d, ..., %d, %d)",
			 epfd, maxevents, timeout);
	}

	return ret;
}

