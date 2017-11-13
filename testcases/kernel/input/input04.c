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
  *  Create a virtual device (mouse), send empty events to /dev/uinput
  *  and check that the events are not received in /dev/inputX
  */

#include <linux/input.h>

#include "test.h"
#include "safe_macros.h"
#include "lapi/fcntl.h"
#include "input_helper.h"

#define NB_TEST 20

static void setup(void);
static void send_events(void);
static void cleanup(void);

static int fd, fd2;

char *TCID = "input04";

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
			if (no_events_queued(fd2, 1))
				tst_resm(TPASS,
					"No data received in /dev/inputX");
			else
				tst_resm(TFAIL,
					"Data received /dev/inputX");
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
		send_rel_move(fd, 0, 0);
		usleep(1000);
	}
}

static void cleanup(void)
{
	if (fd2 > 0 && close(fd2))
		tst_resm(TWARN | TERRNO, "close(fd2)");

	destroy_device(fd);
}
