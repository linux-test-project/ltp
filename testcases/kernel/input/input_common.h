/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

#ifndef INPUT_COMMON_H__
#define INPUT_COMMON_H__

#include <linux/input.h>
#include <poll.h>

#include "tst_test.h"
#include "tst_uinput.h"

static inline int open_event_device(void)
{
	int fd;
	char path[1024];
	char *device;
	char *handlers;

	memset(path, 0, sizeof(path));

	handlers = get_input_field_value('H');
	device = strtok(handlers, " ");

	while (device) {
		if (strstr(device, "event") != NULL) {
			memset(path, 0, sizeof(path));
			snprintf(path, sizeof(path), "/dev/input/%s", device);

			if (!access(path, F_OK)) {
				tst_res(TINFO, "Found event device: %s", path);
				break;
			}
		}

		device = strtok(NULL, " ");
	}

	free(handlers);

	if (path[0] == '\0')
		tst_brk(TBROK, "Can't find event device");

	fd = SAFE_OPEN(path, O_RDONLY);

	return fd;
}

static inline void send_event(
	const int fd, const int event,
	const int code, const int value)
{
	struct input_event ev = {
		.type = event,
		.code = code,
		.value = value,
	};

	SAFE_WRITE(SAFE_WRITE_ALL, fd, &ev, sizeof(ev));
}

static inline void send_relative_move(const int fd, const int x, const int y)
{
	send_event(fd, EV_REL, REL_X, x);
	send_event(fd, EV_REL, REL_Y, y);
	send_event(fd, EV_SYN, 0, 0);
}

static inline void verify_no_events_queued(const int fd_recv)
{
	int num_bytes;
	int num_events;
	struct input_event ev;
	struct pollfd fds = {
		.fd = fd_recv,
		.events = POLLIN
	};

	num_events = poll(&fds, 1, 30);

	TST_EXP_EQ_LI(num_events, 0);
	if (!num_events)
		return;

	num_bytes = SAFE_READ(0, fd_recv, &ev, sizeof(ev));
	if (!num_bytes)
		return;

	tst_res(TFAIL, "Received unexpected event: "
		"type=%i, code=%i, value=%i",
		ev.type,
		ev.code,
		ev.value);
}
#endif
