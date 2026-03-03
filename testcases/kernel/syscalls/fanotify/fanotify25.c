// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 SUSE LLC
 * Author: Petr Pavlu <ppavlu@suse.cz>
 * LTP port: Martin Doucha <mdoucha@suse.cz>
 */

/*\
 * Verify that fanotify monitoring can be applied to the tracing filesystem
 * and write events will be correctly delivered.
 */

#define _GNU_SOURCE
#include "tst_test.h"
#include "lapi/mount.h"

#ifdef HAVE_SYS_FANOTIFY_H
#include "fanotify.h"

#define MNTPOINT "/sys/kernel/tracing"
#define EVENTS_SYSFILE MNTPOINT "/kprobe_events"

static const struct traceconfig {
	const char *filename;
	const char *wdata;
} trace_cmds[] = {
	{EVENTS_SYSFILE, "p:ltp_oom_kill_process_0 oom_kill_process"},
	{MNTPOINT "/events/kprobes/ltp_oom_kill_process_0/enable", "1"},
	{MNTPOINT "/events/kprobes/ltp_oom_kill_process_0/enable", "0"},
	{EVENTS_SYSFILE, "-:ltp_oom_kill_process_0"},
	{}
};

static int fan_fd = -1;
static int unmount_needed;

static void setup(void)
{
	if (tst_fs_type(MNTPOINT) != TST_TRACEFS_MAGIC) {
		SAFE_MOUNT("tracefs", MNTPOINT, "tracefs",
			MS_NODEV | MS_NOEXEC | MS_NOSUID, NULL);
		unmount_needed = 1;
	}

	if (access(EVENTS_SYSFILE, F_OK))
		tst_brk(TCONF, "Kprobe events not supported by kernel");

	fan_fd = SAFE_FANOTIFY_INIT(FAN_CLASS_NOTIF | FAN_NONBLOCK, O_RDONLY);
	SAFE_FANOTIFY_MARK(fan_fd, FAN_MARK_ADD | FAN_MARK_MOUNT, FAN_MODIFY,
		-1, MNTPOINT);
}

static void do_child(void)
{
	int i, fd, events, ret;
	pid_t pid = getpid();
	struct fanotify_event_metadata buf;

	for (i = 0, events = 0; trace_cmds[i].filename; i++) {
		fd = SAFE_OPEN(trace_cmds[i].filename, O_WRONLY, 0644);
		SAFE_WRITE(1, fd, trace_cmds[i].wdata,
			strlen(trace_cmds[i].wdata));
		SAFE_CLOSE(fd);

		while ((ret = read(fan_fd, &buf, sizeof(buf))) > 0) {
			if (buf.pid != pid)
				continue;

			if (!(buf.mask & FAN_MODIFY)) {
				tst_res(TFAIL, "Unexpected event %llx",
					buf.mask);
				continue;
			}

			events++;
		}

		if (ret < 0 && errno != EAGAIN)
			tst_res(TFAIL | TERRNO, "fanotify read() failed");
	}

	if (events == i)
		tst_res(TPASS, "Received %d events", events);
	else
		tst_res(TFAIL, "Received %d events, expected %d", events, i);
}

static void run(void)
{
	/*
	 * Fork a child to do the actual trace writes, otherwise tracefs
	 * would be busy until the current process exits and it would become
	 * impossible to unmount in cleanup().
	 */
	if (!SAFE_FORK()) {
		do_child();
		SAFE_CLOSE(fan_fd);
		exit(0);
	}
}

static void cleanup(void)
{
	if (fan_fd >= 0)
		SAFE_CLOSE(fan_fd);

	if (unmount_needed)
		SAFE_UMOUNT(MNTPOINT);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.forks_child = 1,
	.taint_check = TST_TAINT_W | TST_TAINT_D,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_TRACING",
		NULL
	}
};

#else
	TST_TEST_TCONF("system doesn't have required fanotify support");
#endif
