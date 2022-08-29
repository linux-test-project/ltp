// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2019 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef UEVENT_H__
#define UEVENT_H__

#include "tst_netlink.h"

/*
 * There are two broadcast groups defined for the NETLINK_KOBJECT_UEVENT. The
 * primary consument of the KERNEL group is udev which handles the hotplug
 * events and then, once udev does it's magic the events are rebroadcasted to
 * the UDEV group which is consumed by various daemons in the userspace.
 */
enum monitor_netlink_group {
	MONITOR_GROUP_NONE,
	MONITOR_GROUP_KERNEL,
	MONITOR_GROUP_UDEV,
};

/*
 * The messages received from the NETLINK_KOBJECT_UEVENT socket are stored as a
 * sequence of a null-terminated strings. First in the buffer is a summary of a
 * action i.e. "$ACTION@$DEVPATH" which is then followed by a bunch of
 * key-value pairs.
 *
 * For example attaching a file to loopback device generates event:
 *
 * "change@/devices/virtual/block/loop0\0
 *  ACTION=change\0
 *  DEVPATH=/devices/virtual/block/loop0\0
 *  SUBSYSTEM=block\0
 *  MAJOR=7\0
 *  MINOR=0\0
 *  DEVNAME=loop0\0
 *  DEVTYPE=disk\0
 *  SEQNUM=2677\0"
 */

/*
 * Prints uevent.
 */
static inline void print_uevent(const char *event, int len)
{
	int consumed = 0;

	tst_res(TINFO, "Got uevent:");

	while (consumed < len) {
		tst_res(TINFO, "%s", event);
		int l = strlen(event) + 1;
		consumed += l;
		event += l;
	}
}

/*
 * Uevents read from the socket are matched against this description.
 *
 * The msg is the overall action description e.g.
 * "add@/class/input/input4/mouse1" which has to be matched exactly before we
 * event attempt to check the key-value pairs stored in the values array. The
 * event is considered to match if all key-value pairs in the values has been
 * found in the received event.
 */
struct uevent_desc {
	const char *msg;
	int value_cnt;
	const char **values;
};

static inline int uevent_match(const char *event, int len,
                               const struct uevent_desc *uevent)
{
	int consumed = 0;
	int val_matches = 0;

	if (memcmp(event, uevent->msg, strlen(uevent->msg)))
		return 0;

	int l = strlen(event) + 1;

	consumed += l;
	event += l;

	while (consumed < len) {
		int i;
		for (i = 0; i < uevent->value_cnt; i++) {
			if (!strcmp(event, uevent->values[i])) {
				val_matches++;
				break;
			}
		}

		l = strlen(event) + 1;
		consumed += l;
		event += l;
	}

	return val_matches == uevent->value_cnt;
}

static inline int open_uevent_netlink(void)
{
	int fd;
	struct sockaddr_nl nl_addr = {
		.nl_family = AF_NETLINK,
		.nl_groups = MONITOR_GROUP_KERNEL,
	};

	fd = SAFE_SOCKET(AF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT);

	SAFE_BIND(fd, (struct sockaddr *)&nl_addr, sizeof(nl_addr));

	return fd;
}

/*
 * Reads events from uevent netlink socket until all expected events passed in
 * the uevent array are matched.
 */
static inline void wait_for_uevents(int fd, const struct uevent_desc *const uevents[])
{
	int i = 0;

	while (1) {
		int len;
		char buf[4096];

		len = recv(fd, &buf, sizeof(buf), 0);

		if (len == 0)
			continue;

		print_uevent(buf, len);

		if (uevent_match(buf, len, uevents[i])) {
			tst_res(TPASS, "Got expected UEVENT");
			if (!uevents[++i]) {
				close(fd);
				return;
			}
		}
	}
}

/*
 * Waits 5 seconds for a child to exit, kills the child after a timeout.
 */
static inline void wait_for_pid(int pid)
{
	int status, ret;
	int retries = 5000;

	do {
		ret = waitpid(pid, &status, WNOHANG);
		usleep(1000);
	} while (ret == 0 && retries--);

	if (ret == pid) {
		if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
			return;

		tst_res(TFAIL, "Child exited with %s", tst_strstatus(status));
	}

	SAFE_KILL(pid, SIGKILL);

	SAFE_WAITPID(pid, NULL, 0);

	tst_res(TFAIL, "Did not get all expected UEVENTS");
}

#endif /* UEVENT_H__ */
