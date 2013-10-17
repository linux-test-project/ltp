/*
 * Copyright (C) 2013 Linux Test Project, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it
 * is free of the rightful claim of any third person regarding
 * infringement or the like.  Any license provided herein, whether
 * implied or otherwise, applies only to this software file.  Patent
 * licenses, if any, provided herein do not apply to combinations of
 * this program with other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

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
		fd = open(tmp, O_RDONLY);
		if (fd == -1)
			tst_brkm(TBROK|TERRNO, NULL, "failed to open %s", tmp);
	} else {
		if (errno != ENOENT)
			tst_brkm(TBROK|TERRNO, NULL, "failed to stat %s", tmp);
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
		tst_resm(TINFO, "ns_name=%s, ns_fds[%d]=%d, ns_types[%d]=0x%x",
			 proc_name, ns_total, fd, ns_total, clone_type);
		ns_total++;
	}
}

static void init_available_ns(void)
{
#if defined(CLONE_NEWIPC)
	init_ns_type(CLONE_NEWIPC, "ipc");
#endif
#if defined(CLONE_NEWNS)
	init_ns_type(CLONE_NEWNS, "mnt");
#endif
#if defined(CLONE_NEWNET)
	init_ns_type(CLONE_NEWNET, "net");
#endif
#if defined(CLONE_NEWPID)
	init_ns_type(CLONE_NEWPID, "pid");
#endif
#if defined(CLONE_NEWUTS)
	init_ns_type(CLONE_NEWUTS, "uts");
#endif
}

static void close_ns_fds(void)
{
	int i;

	for (i = 0; i < ns_total; i++)
		if (ns_fds[i] != -1)
			close(ns_fds[i]);
}
