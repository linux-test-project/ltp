// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2015 Cedric Hnyda <chnyda@suse.com>
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify that /dev/input/eventX won't receive any event sent from a virtual
 * device, that in our case is a mouse, when the event device has been grabbed
 * by an another process.
 */

#include "input_common.h"

#define MOVE_X 10
#define MOVE_Y 1

static int fd_send = -1;
static int fd_recv = -1;

static void send_events(void)
{
	int fd;

	fd = open_event_device();

	SAFE_IOCTL(fd, EVIOCGRAB, 1);
	tst_res(TINFO, "The virtual device was grabbed");

	send_relative_move(fd_send, MOVE_X, MOVE_Y);

	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	SAFE_CLOSE(fd);
}

static void run(void)
{
	if (!SAFE_FORK()) {
		send_events();
		exit(0);
	}

	TST_CHECKPOINT_WAIT(0);

	verify_no_events_queued(fd_recv);

	TST_CHECKPOINT_WAKE(0);
}

static void setup(void)
{
	fd_send = open_uinput();
	if (fd_send == -1)
		tst_brk(TCONF, "Virtual device is not available");

	setup_mouse_events(fd_send);
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
	.forks_child = 1,
	.needs_root = 1,
	.needs_checkpoints = 1,
};
