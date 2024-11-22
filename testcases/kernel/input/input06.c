// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2015 Cedric Hnyda <chnyda@suse.com>
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that auto-repeat is working on a virtual device, that in our case
 * it's a keyboard.
 */

#include <linux/uinput.h>

#include "input_common.h"

struct input_event events[64];
static int num_events;
static int ev_iter;
static int fd_send = -1;
static int fd_recv = -1;

static void send_events(void)
{
	send_event(fd_send, EV_KEY, KEY_X, 1);
	send_event(fd_send, EV_SYN, 0, 0);

	/*
	 * Sleep long enough to keep the key pressed for some time
	 * (auto-repeat).  Default kernel delay to start auto-repeat is 250ms
	 * and the period is 33ms. So, we wait for a generous 500ms to make
	 * sure we get the auto-repeated keys
	 */
	usleep(500000);

	send_event(fd_send, EV_KEY, KEY_X, 0);
	send_event(fd_send, EV_SYN, 0, 0);
}

static int check_event(struct input_event *iev, int event, int code, int value)
{
	return iev->type == event && iev->code == code && iev->value == value;
}

static void read_events(void)
{
	int num_bytes = SAFE_READ(0, fd_recv, events, sizeof(events));

	if (!num_bytes)
		tst_brk(TBROK, "Failed to read events");

	if (num_bytes % sizeof(struct input_event) != 0) {
		tst_brk(TBROK, "Read size %i is not multiple of %zu",
			num_bytes, sizeof(struct input_event));
	}

	ev_iter = 0;
	num_events = num_bytes / sizeof(struct input_event);
}

static int have_events(void)
{
	return num_events && ev_iter < num_events;
}

static struct input_event *next_event(void)
{
	if (!have_events())
		read_events();

	return &events[ev_iter++];
}

static int check_event_code(struct input_event *iev, int event, int code)
{
	return iev->type == event && iev->code == code;
}

static int parse_autorepeat_config(struct input_event *iev)
{
	if (!check_event_code(iev, EV_REP, REP_DELAY)) {
		tst_res(TFAIL, "Didn't get EV_REP type with REP_DELAY code");
		return 0;
	}

	if (!check_event_code(next_event(), EV_REP, REP_PERIOD)) {
		tst_res(TFAIL, "Didn't get EV_REP type with REP_PERIOD code");
		return 0;
	}

	return 1;
}

static int check_sync_event(struct input_event *iev)
{
	return check_event_code(iev, EV_SYN, SYN_REPORT);
}

static int parse_key(struct input_event *iev)
{
	int autorep_count = 0;

	if (!check_event(iev, EV_KEY, KEY_X, 1) || !check_sync_event(next_event())) {
		tst_res(TFAIL, "Didn't get expected key press for KEY_X");
		return 0;
	}

	iev = next_event();
	while (check_event(iev, EV_KEY, KEY_X, 2) && check_sync_event(next_event())) {
		autorep_count++;
		iev = next_event();
	}

	/* make sure we have at least one auto-repeated key event */
	if (!autorep_count) {
		tst_res(TFAIL, "Didn't get autorepeat events for the key - KEY_X");
		return 0;
	}

	if (!check_event(iev, EV_KEY, KEY_X, 0) || !check_sync_event(next_event())) {
		tst_res(TFAIL, "Didn't get expected key release for KEY_X");
		return 0;
	}

	tst_res(TINFO, "Received %d repetitions for KEY_X", autorep_count);

	return 1;
}

static int check_events(void)
{
	struct input_event *iev;
	int ret = 0;
	int rep_config_done = 0;
	int rep_keys_done = 0;

	read_events();

	while (have_events()) {
		iev = next_event();
		switch (iev->type) {
		case EV_REP:
			ret = parse_autorepeat_config(iev);
			rep_config_done = 1;
			break;
		case EV_KEY:
			ret = parse_key(iev);
			rep_keys_done = 1;
			break;
		default:
			tst_res(TFAIL, "Unexpected event type '0x%04x' received",
				iev->type);
			ret = 0;
			break;
		}

		if (!ret || (rep_config_done && rep_keys_done))
			break;
	}

	return ret;
}

static void run(void)
{
	if (!SAFE_FORK()) {
		send_events();
		exit(0);
	}

	if (!check_events())
		tst_res(TFAIL, "Wrong data received from input device");
	else
		tst_res(TPASS, "Data received from input device");
}

static void setup(void)
{
	fd_send = open_uinput();

	SAFE_IOCTL(fd_send, UI_SET_EVBIT, EV_KEY);
	SAFE_IOCTL(fd_send, UI_SET_EVBIT, EV_REP);
	SAFE_IOCTL(fd_send, UI_SET_KEYBIT, KEY_X);

	create_input_device(fd_send);

	fd_recv = open_event_device();
	SAFE_IOCTL(fd_recv, EVIOCGRAB, 1);
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
	.forks_child = 1,
};
