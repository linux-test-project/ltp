// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2015 Cedric Hnyda <chnyda@suse.com>
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that /dev/input/mice receive events sent from a virtual device,
 * that in our case is a mouse. The events are a sequence of mouse right click.
 */

#include <linux/uinput.h>

#include "input_common.h"

#define NUM_EVENTS 10
#define PS2_RIGHT_BTN 0x02
#define MOUSE_DEV "/dev/input/mice"

static int fd_send = -1;
static int fd_recv = -1;

static void recv_data(void)
{
	tst_res(TINFO, "Reading events back");

	char buf[30];
	int events = 0;
	int pressed = 0;
	int num_bytes = 0;

	TST_CHECKPOINT_WAKE(0);

	while (events < NUM_EVENTS) {
		memset(buf, 0, sizeof(buf));

		num_bytes = SAFE_READ(0, fd_recv, buf, sizeof(buf));

		for (int i = 0; i < num_bytes / 3; i++) {
			if (buf[3*i] & PS2_RIGHT_BTN)
				pressed = 1;

			if (pressed == 1 && !(buf[3*i] & PS2_RIGHT_BTN)) {
				pressed = 0;
				events++;
			}
		}
	}

	TST_EXP_EQ_LI(events, NUM_EVENTS);
}

static void send_mouse_events(void)
{
	tst_res(TINFO, "Sending right click");

	TST_CHECKPOINT_WAIT(0);

	for (int i = 0; i < NUM_EVENTS; i++) {
		send_event(fd_send, EV_KEY, BTN_RIGHT, 1);
		send_event(fd_send, EV_SYN, 0, 0);
		usleep(1000);

		send_event(fd_send, EV_KEY, BTN_RIGHT, 0);
		send_event(fd_send, EV_SYN, 0, 0);
		usleep(1000);
	}
}

static void run(void)
{
	if (!SAFE_FORK()) {
		send_mouse_events();
		exit(0);
	}

	recv_data();
}

static void setup(void)
{
	fd_send = open_uinput();

	setup_mouse_events(fd_send);
	SAFE_IOCTL(fd_send, UI_SET_EVBIT, EV_KEY);
	SAFE_IOCTL(fd_send, UI_SET_KEYBIT, BTN_RIGHT);

	create_input_device(fd_send);

	fd_recv = SAFE_OPEN(MOUSE_DEV, O_RDONLY);
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
