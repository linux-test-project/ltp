// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2015 Cedric Hnyda <chnyda@suse.com>
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that /dev/input/eventX doesn't receive any event sent from a virtual
 * device, that in our case is a mouse, when events not advertised in the input
 * device bits are filtered.
 */

#include <linux/uinput.h>

#include "input_common.h"

#define NUM_EVENTS 20
#define MOVE_X 10
#define MOVE_Y 10

static int fd_send = -1;
static int fd_recv = -1;

static void run(void)
{
	tst_res(TINFO, "Sending relative mouse move (%i, %i)", MOVE_X, MOVE_Y);

	for (int i = 0; i < NUM_EVENTS; i++) {
		send_relative_move(fd_send, MOVE_X, MOVE_Y);
		usleep(1000);
	}

	verify_no_events_queued(fd_recv);
}

static void setup(void)
{
	fd_send = open_uinput();
	if (fd_send == -1)
		tst_brk(TCONF, "Virtual device is not available");

	SAFE_IOCTL(fd_send, UI_SET_EVBIT, EV_KEY);
	SAFE_IOCTL(fd_send, UI_SET_KEYBIT, BTN_LEFT);
	create_input_device(fd_send);

	fd_recv = open_event_device();
}

static void cleanup(void)
{
	if (fd_send != -1)
		destroy_input_device(fd_send);

	if (fd_recv != -1)
		SAFE_CLOSE(fd_recv);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
};
