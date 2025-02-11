// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2019 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) Linux Test Project, 2019-2023
 */

/*\
 * Very simple uevent netlink socket test.
 *
 * We fork a child that listens for a kernel events while parents attaches and
 * detaches a loop device which should produce two change events.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "tst_test.h"

#include "uevent.h"

static void generate_device_events(const char *dev_path)
{
	tst_fill_file("loop.img", 0, 1024, 1024);

	tst_res(TINFO, "Attaching device %s", dev_path);
	tst_attach_device(dev_path, "loop.img");
	tst_res(TINFO, "Detaching device %s", dev_path);
	tst_detach_device(dev_path);
}

static void verify_uevent(void)
{
	int pid, fd, dev_num;
	char dev_path[1024];
	char ev_msg[1024];
	char ev_dev_path[1024];
	char ev_dev_minor[128];
	char ev_dev_name[128];

	struct uevent_desc desc = {
		.msg = ev_msg,
		.value_cnt = 7,
		.values = (const char*[]) {
			"ACTION=change",
			ev_dev_path,
			"SUBSYSTEM=block",
			"MAJOR=7",
			ev_dev_minor,
			ev_dev_name,
			"DEVTYPE=disk",
		}
	};

	dev_num = tst_find_free_loopdev(dev_path, sizeof(dev_path));

	if (dev_num < 0)
		tst_brk(TBROK, "Failed to find free loop device");

	snprintf(ev_msg, sizeof(ev_msg),
	         "change@/devices/virtual/block/loop%i", dev_num);

	snprintf(ev_dev_path, sizeof(ev_dev_path),
	         "DEVPATH=/devices/virtual/block/loop%i", dev_num);

	snprintf(ev_dev_minor, sizeof(ev_dev_minor), "MINOR=%i", dev_num);
	snprintf(ev_dev_name, sizeof(ev_dev_name), "DEVNAME=loop%i", dev_num);

	const struct uevent_desc *const uevents[] = {
		&desc,
		&desc,
		NULL
	};

	pid = SAFE_FORK();
	if (!pid) {
		fd = open_uevent_netlink();
		TST_CHECKPOINT_WAKE(0);
		wait_for_uevents(fd, uevents);
		exit(0);
	}

	TST_CHECKPOINT_WAIT(0);

	generate_device_events(dev_path);

	wait_for_pid(pid);
}

static struct tst_test test = {
	.test_all = verify_uevent,
	.forks_child = 1,
	.needs_checkpoints = 1,
	.needs_drivers = (const char *const []) {
		"loop",
		NULL
	},
	.needs_root = 1
};
