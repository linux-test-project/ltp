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
  *  and check that the events are well received in /dev/input/eventX
  */

#include <linux/input.h>

#include "input_helper.h"
#include "test.h"
#include "safe_macros.h"
#include "lapi/fcntl.h"

#define NB_TEST 20

static void setup(void);
static void send_events(void);
static int verify_data(struct input_event *iev, int nb);
static int check_events(void);
static void cleanup(void);

static int fd;
static int fd2;

char *TCID = "input01";

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
				tst_resm(TFAIL, "Wrong data read from eventX");
			else
				tst_resm(TPASS, "Data received from eventX");
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
	create_device(fd);

	fd2 = open_device();
}

static void send_events(void)
{
	int nb;

	for (nb = 0; nb < NB_TEST; ++nb) {
		send_rel_move(fd, 10, 1);
		usleep(1000);
	}
}

static int check_events(void)
{
	int nb, rd;
	unsigned int i;
	struct input_event iev[64];

	nb = 0;

	while (nb < NB_TEST * 3) {
		rd = read(fd2, iev, sizeof(iev));

		if (rd < 0)
			tst_brkm(TBROK | TERRNO, cleanup, "read()");

		if (rd == 0 || rd % sizeof(struct input_event)) {
			tst_resm(TINFO, "read() returned unexpected %i", rd);
			return 1;
		}

		for (i = 0; i < rd / sizeof(struct input_event); i++) {
			if (verify_data(&iev[i], nb++))
				return 1;
		}
	}

	return 0;
}

static int verify_data(struct input_event *iev, int nb)
{
	if (nb % 3 == 0) {
		if (iev->type != EV_REL) {
			tst_resm(TINFO,
			         "%i: Unexpected event type %i expected %i",
			         nb, iev->type, EV_REL);
			return 1;
		}

		if (iev->code != REL_X)
			return 1;

		if (iev->value != 10)
			return 1;

		return 0;
	}

	if (nb % 3 == 1) {
		if (iev->type != EV_REL) {
			tst_resm(TINFO,
			         "%i: Unexpected event type %i expected %i",
			         nb, iev->type, EV_REL);
			return 1;
		}

		if (iev->code != REL_Y)
			return 1;

		if (iev->value != 1)
			return 1;

		return 0;
	}

	if (nb % 3 == 2) {
		if (iev->type != EV_SYN) {
			tst_resm(TINFO,
			         "%i: Unexpected event type %i expected %i",
			         nb, iev->type, EV_SYN);
			return 1;
		}

		if (iev->code != 0)
			return 1;

		if (iev->value != 0)
			return 1;

		return 0;
	}
	return 1;
}

static void cleanup(void)
{
	if (fd2 > 0 && close(fd2))
		tst_resm(TWARN | TERRNO, "close(fd2)");

	destroy_device(fd);
}
