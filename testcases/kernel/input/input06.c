/*
 * Copyright (c) 2015 Cedric Hnyda <chnyda@suse.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

 /*
  *  Create a virtual device, activate auto-repeat and
  *  and check that auto repeat is working
  */

#include <linux/input.h>
#include <linux/uinput.h>
#include <linux/kd.h>

#include "test.h"
#include "safe_macros.h"
#include "lapi/fcntl.h"
#include "input_helper.h"

static void setup(void);
static void send_events(void);
static int check_events(void);
static void cleanup(void);

static int fd;
static int fd2;
struct input_event events[64];
static int num_events;
static int ev_iter;

char *TCID = "input06";

int main(int ac, char **av)
{
	int lc;
	int pid;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		pid = tst_fork();

		switch (pid) {
		case 0:
			send_events();
			exit(0);
		case -1:
			tst_brkm(TBROK | TERRNO, cleanup, "fork() failed");
		default:
			if (!check_events())
				tst_resm(TFAIL,
					"Wrong data received in eventX");
			else
				tst_resm(TPASS, "Data received in eventX");
		break;
		}

		SAFE_WAITPID(NULL, pid, NULL, 0);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_require_root();

	fd = open_uinput();

	SAFE_IOCTL(NULL, fd, UI_SET_EVBIT, EV_KEY);
	SAFE_IOCTL(NULL, fd, UI_SET_EVBIT, EV_REP);
	SAFE_IOCTL(NULL, fd, UI_SET_KEYBIT, KEY_X);

	create_device(fd);

	fd2 = open_device();
	SAFE_IOCTL(NULL, fd2, EVIOCGRAB, 1);
}

static void send_events(void)
{
	send_event(fd, EV_KEY, KEY_X, 1);
	send_event(fd, EV_SYN, 0, 0);

	/*
	 * Sleep long enough to keep the key pressed for some time
	 * (auto-repeat).  Default kernel delay to start auto-repeat is 250ms
	 * and the period is 33ms. So, we wait for a generous 500ms to make
	 * sure we get the auto-repeated keys
	 */
	usleep(500000);

	send_event(fd, EV_KEY, KEY_X, 0);
	send_event(fd, EV_SYN, 0, 0);
}

static int check_event(struct input_event *iev, int event, int code, int value)
{
	return iev->type == event && iev->code == code && iev->value == value;
}

static void read_events(void)
{
	int rd = read(fd2, events, sizeof(events));
	if (rd < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "read() failed");

	if (rd == 0)
		tst_brkm(TBROK, cleanup, "Failed to read events");

	if (rd % sizeof(struct input_event) != 0) {
		tst_brkm(TBROK, cleanup, "read size %i not multiple of %zu",
		         rd, sizeof(struct input_event));
	}

	ev_iter = 0;
	num_events = rd / sizeof(struct input_event);
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

static int parse_autorepeat_config(struct input_event *iev)
{
	if (!check_event_code(iev, EV_REP, REP_DELAY)) {
		tst_resm(TFAIL,
			 "Didn't get EV_REP configuration with code REP_DELAY");
		return 0;
	}

	if (!check_event_code(next_event(), EV_REP, REP_PERIOD)) {
		tst_resm(TFAIL,
			 "Didn't get EV_REP configuration with code REP_PERIOD");
		return 0;
	}

	return 1;
}

static int parse_key(struct input_event *iev)
{
	int autorep_count = 0;

	if (!check_event(iev, EV_KEY, KEY_X, 1) || !check_sync_event(next_event())) {
		tst_resm(TFAIL, "Didn't get expected key press for KEY_X");
		return 0;
	}

	iev = next_event();
	while (check_event(iev, EV_KEY, KEY_X, 2) && check_sync_event(next_event())) {
		autorep_count++;
		iev = next_event();
	}

	/* make sure we have at least one auto-repeated key event */
	if (!autorep_count) {
		tst_resm(TFAIL,
			 "Didn't get autorepeat events for the key - KEY_X");
		return 0;
	}

	if (!check_event(iev, EV_KEY, KEY_X, 0) || !check_sync_event(next_event())) {
		tst_resm(TFAIL,
			 "Didn't get expected key release for KEY_X");
		return 0;
	}

	tst_resm(TINFO,
		 "Received %d repititions for KEY_X", autorep_count);

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
			tst_resm(TFAIL,
				 "Unexpected event type '0x%04x' received",
				iev->type);
			ret = 0;
			break;
		}

		if (!ret || (rep_config_done && rep_keys_done))
			break;
	}

	return ret;
}

static void cleanup(void)
{
	if (fd2 > 0 && close(fd2))
		tst_resm(TWARN | TERRNO, "close(fd2) failed");

	destroy_device(fd);
}
