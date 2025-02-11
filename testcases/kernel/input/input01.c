// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2015 Cedric Hnyda <chnyda@suse.com>
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify that /dev/input/eventX receive events sent from a virtual device,
 * that in our case is a mouse.
 */

#include "input_common.h"

#define NUM_EVENTS 20
#define MOVE_X 10
#define MOVE_Y 1

static int fd_send = -1;
static int fd_recv = -1;

static void run(void)
{
	struct input_event iev[3];

	tst_res(TINFO, "Sending relative move: (%i, %i)", MOVE_X, MOVE_Y);

	for (int i = 0; i < NUM_EVENTS; i++) {
		send_relative_move(fd_send, MOVE_X, MOVE_Y);
		usleep(1000);
	}

	tst_res(TINFO, "Reading events back");

	for (int i = 0; i < NUM_EVENTS; i++) {
		SAFE_READ(0, fd_recv, iev, 3 * sizeof(struct input_event));

		TST_EXP_EQ_LI(iev[0].type, EV_REL);
		TST_EXP_EQ_LI(iev[0].code, REL_X);
		TST_EXP_EQ_LI(iev[0].value, MOVE_X);

		TST_EXP_EQ_LI(iev[1].type, EV_REL);
		TST_EXP_EQ_LI(iev[1].code, REL_Y);
		TST_EXP_EQ_LI(iev[1].value, MOVE_Y);

		TST_EXP_EQ_LI(iev[2].type, EV_SYN);
		TST_EXP_EQ_LI(iev[2].code, 0);
		TST_EXP_EQ_LI(iev[2].value, 0);
	}
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
	.needs_root = 1,
};
