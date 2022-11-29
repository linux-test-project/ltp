// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2014-2020
 */

#include <stdio.h>
#include "lapi/sched.h"

#define NS_MAX 5
static int ns_types[NS_MAX];
static int ns_fds[NS_MAX];
static int ns_total;

static int get_ns_fd(int pid, const char *ns)
{
	char tmp[PATH_MAX];
	struct stat st;
	int fd = -1;

	sprintf(tmp, "/proc/%d/ns/%s", pid, ns);
	if (stat(tmp, &st) == 0) {
		fd = SAFE_OPEN(tmp, O_RDONLY);
	} else {
		if (errno != ENOENT)
			tst_brk(TBROK|TERRNO, "failed to stat %s", tmp);
	}
	return fd;
}

static void init_ns_type(int clone_type, const char *proc_name)
{
	int fd;

	fd = get_ns_fd(getpid(), proc_name);
	if (fd != -1) {
		ns_types[ns_total] = clone_type;
		ns_fds[ns_total] = fd;
		tst_res(TINFO, "ns_name=%s, ns_fds[%d]=%d, ns_types[%d]=0x%x",
			 proc_name, ns_total, fd, ns_total, clone_type);
		ns_total++;
	}
}

static void init_available_ns(void)
{
	init_ns_type(CLONE_NEWIPC, "ipc");
	init_ns_type(CLONE_NEWNS, "mnt");
	init_ns_type(CLONE_NEWNET, "net");
	init_ns_type(CLONE_NEWPID, "pid");
	init_ns_type(CLONE_NEWUTS, "uts");
}

static void close_ns_fds(void)
{
	int i;

	for (i = 0; i < ns_total; i++)
		if (ns_fds[i] != -1)
			SAFE_CLOSE(ns_fds[i]);
}
