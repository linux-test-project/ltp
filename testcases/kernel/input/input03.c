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
  *  Create a virtual device (mouse), send events to /dev/uinput
  *  and check that the events are well received in /dev/input/mice
  */

#include <linux/input.h>
#include <linux/uinput.h>

#include "test.h"
#include "safe_macros.h"
#include "lapi/fcntl.h"
#include "input_helper.h"

#define NB_TEST 10
#define PS2_RIGHT_BTN 0x02

static void setup(void);
static void send_events(void);
static int check_events(void);
static void cleanup(void);

static int fd, fd2;

char *TCID = "input03";

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
			if (check_events())
				tst_resm(TFAIL, "Wrong data received");
			else
				tst_resm(TPASS,
					"Data received in /dev/input/mice");
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

	setup_mouse_events(fd);
	SAFE_IOCTL(NULL, fd, UI_SET_EVBIT, EV_KEY);
	SAFE_IOCTL(NULL, fd, UI_SET_KEYBIT, BTN_RIGHT);

	create_device(fd);

	fd2 = SAFE_OPEN(NULL, "/dev/input/mice", O_RDONLY);
}

static void send_events(void)
{
	int nb;

	for (nb = 0; nb < NB_TEST; ++nb) {
		send_event(fd, EV_KEY, BTN_RIGHT, 1);
		send_event(fd, EV_SYN, 0, 0);
		usleep(1000);
		send_event(fd, EV_KEY, BTN_RIGHT, 0);
		send_event(fd, EV_SYN, 0, 0);
		usleep(1000);
	}
}

static int check_events(void)
{
	int nb, rd, i, pressed = 0;
	char buf[30];

	nb = 0;

	while (nb < NB_TEST) {
		rd = read(fd2, buf, sizeof(buf));

		if (rd < 0)
			tst_brkm(TBROK | TERRNO, NULL, "read() failed");

		if (rd % 3) {
			tst_resm(TINFO, "read() returned %i", rd);
			return 1;
		}

		for (i = 0; i < rd / 3; i++) {
			if (buf[3*i] & PS2_RIGHT_BTN)
				pressed = 1;

			if (pressed == 1 && !(buf[3*i] & PS2_RIGHT_BTN)) {
				pressed = 0;
				nb++;
			}
		}
	}

	return nb != NB_TEST;
}

static void cleanup(void)
{
	if (fd2 > 0 && close(fd2))
		tst_resm(TWARN, "close(fd2) failed");

	destroy_device(fd);
}
