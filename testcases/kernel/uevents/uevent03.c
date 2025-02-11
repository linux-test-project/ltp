// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2019 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) Linux Test Project, 2019-2023
 */

/*\
 * Very simple uevent netlink socket test.
 *
 * We fork a child that listens for a kernel events while parents creates and
 * removes a virtual mouse which produces add and remove event for the device
 * itself and for two event handlers called eventX and mouseY.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/sysmacros.h>
#include <linux/uinput.h>
#include "tst_test.h"
#include "tst_uinput.h"
#include "uevent.h"

static int mouse_fd;

static void create_uinput_mouse(void)
{
	mouse_fd = open_uinput();
	if (mouse_fd == -1)
		tst_brk(TCONF, "Virtual device is not available");

	setup_mouse_events(mouse_fd);
	create_input_device(mouse_fd);
}

static void destroy_uinput_mouse(void)
{
	destroy_input_device(mouse_fd);
}

static void get_minor_major(char *device, char *minor, char *major, size_t buf_sizes)
{
	char path[1024];
	struct stat stbuf;

	snprintf(path, sizeof(path), "/dev/input/%s", device);

	SAFE_STAT(path, &stbuf);

	snprintf(major, buf_sizes, "MAJOR=%i", major(stbuf.st_rdev));
	snprintf(minor, buf_sizes, "MINOR=%i", minor(stbuf.st_rdev));
}

#define MINOR_MAJOR_SIZE 32

static void verify_uevent(void)
{
	int pid, fd;
	char add_msg[1024];
	char rem_msg[1024];
	char dev_path[1024];
	char add_msg_event1[1024];
	char rem_msg_event1[1024];
	char dev_path_event1[1024];
	char add_msg_event2[1024];
	char rem_msg_event2[1024];
	char dev_path_event2[1024];
	char dev_name1[1024];
	char dev_name2[1024];

	char minor_event1[MINOR_MAJOR_SIZE];
	char minor_event2[MINOR_MAJOR_SIZE];
	char major_event1[MINOR_MAJOR_SIZE];
	char major_event2[MINOR_MAJOR_SIZE];

	char *handlers, *handler1, *handler2, *sysname;
	struct uevent_desc add = {
		.msg = add_msg,
		.value_cnt = 7,
		.values = (const char*[]) {
			"ACTION=add",
			dev_path,
			"SUBSYSTEM=input",
			"NAME=\"virtual-device-ltp\"",
			"PROP=0",
			"EV=7",
			"REL=3",
		}
	};

	struct uevent_desc add_event1 = {
		.msg = add_msg_event1,
		.value_cnt = 6,
		.values = (const char*[]) {
			"ACTION=add",
			"SUBSYSTEM=input",
			dev_name1,
			dev_path_event1,
			minor_event1,
			major_event1,
		}
	};

	struct uevent_desc add_event2 = {
		.msg = add_msg_event2,
		.value_cnt = 6,
		.values = (const char*[]) {
			"ACTION=add",
			"SUBSYSTEM=input",
			dev_name2,
			dev_path_event2,
			minor_event2,
			major_event2,
		}
	};

	struct uevent_desc rem_event1 = {
		.msg = rem_msg_event1,
		.value_cnt = 6,
		.values = (const char*[]) {
			"ACTION=remove",
			"SUBSYSTEM=input",
			dev_name1,
			dev_path_event1,
			minor_event1,
			major_event1,
		}
	};

	struct uevent_desc rem_event2 = {
		.msg = rem_msg_event2,
		.value_cnt = 6,
		.values = (const char*[]) {
			"ACTION=remove",
			"SUBSYSTEM=input",
			dev_name2,
			dev_path_event2,
			minor_event2,
			major_event2,
		}
	};

	struct uevent_desc rem = {
		.msg = rem_msg,
		.value_cnt = 7,
		.values = (const char*[]) {
			"ACTION=remove",
			dev_path,
			"SUBSYSTEM=input",
			"NAME=\"virtual-device-ltp\"",
			"PROP=0",
			"EV=7",
			"REL=3",
		}
	};

	const struct uevent_desc *const uevents[] = {
		&add,
		&add_event1,
		&add_event2,
		&rem_event1,
		&rem_event2,
		&rem,
		NULL
	};

	fd = open_uevent_netlink();

	create_uinput_mouse();

	sysname = get_input_field_value('S');
	handlers = get_input_field_value('H');

	if (!sysname)
		tst_brk(TBROK, "Expected /devices/virtual/input/inputN sysname!");

	tst_res(TINFO, "Sysname: %s", sysname);
	tst_res(TINFO, "Handlers: %s", handlers);

	handler1 = strtok(handlers, " ");
	if (!handler1)
		tst_brk(TBROK, "Expected mouseX and eventY handlers!");

	get_minor_major(handler1, minor_event1, major_event1, MINOR_MAJOR_SIZE);

	handler2 = strtok(NULL, " ");
	if (!handler2)
		tst_brk(TBROK, "Expected mouseX and eventY handlers!");

	get_minor_major(handler2, minor_event2, major_event2, MINOR_MAJOR_SIZE);

	destroy_uinput_mouse();

	snprintf(add_msg, sizeof(add_msg), "add@%s", sysname);

	snprintf(rem_msg, sizeof(rem_msg), "remove@%s", sysname);

	snprintf(dev_path, sizeof(dev_path), "DEVPATH=%s", sysname);

	snprintf(add_msg_event1, sizeof(add_msg_event1),
		"add@%s/%s", sysname, handler1);

	snprintf(rem_msg_event1, sizeof(rem_msg_event1),
		"remove@%s/%s", sysname, handler1);

	snprintf(dev_path_event1, sizeof(dev_path_event1),
		"DEVPATH=%s/%s", sysname, handler1);

	snprintf(dev_name1, sizeof(dev_name1),
		"DEVNAME=input/%s", handler1);


	snprintf(add_msg_event2, sizeof(add_msg_event2),
		"add@%s/%s", sysname, handler2);

	snprintf(rem_msg_event2, sizeof(rem_msg_event2),
		"remove@%s/%s", sysname, handler2);

	snprintf(dev_path_event2, sizeof(dev_path_event2),
		"DEVPATH=%s/%s", sysname, handler2);

	snprintf(dev_name2, sizeof(dev_name2),
		"DEVNAME=input/%s", handler2);

	free(sysname);
	free(handlers);

	pid = SAFE_FORK();
	if (!pid) {
		wait_for_uevents(fd, uevents);
		exit(0);
	}

	SAFE_CLOSE(fd);
	wait_for_pid(pid);
}

static struct tst_test test = {
	.test_all = verify_uevent,
	.forks_child = 1,
	.needs_checkpoints = 1,
	.needs_drivers = (const char *const[]) {
		"uinput",
		NULL
	},
	.needs_root = 1,
};
