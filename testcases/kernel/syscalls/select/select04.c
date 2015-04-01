/*
 * Copyright (C) 2015 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Check that select() timeouts correctly.
 */
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>

#include "test.h"
#include "safe_macros.h"

char *TCID = "select04";
int TST_TOTAL = 1;

static char *opt_sleep_us;

static option_t opts[] = {
	{"s:", NULL, &opt_sleep_us},
	{NULL, NULL, NULL},
};

static void help(void);
static void setup(void);
static void cleanup(void);

static int fds[2];

int main(int ac, char **av)
{
	int lc, treshold;
	long long elapsed_us, sleep_us = 100000;
	struct timeval timeout;
	fd_set sfds;

	tst_parse_opts(ac, av, opts, help);

	if (opt_sleep_us) {
		sleep_us = atoll(opt_sleep_us);

		if (sleep_us == 0) {
			tst_brkm(TBROK, NULL, "Invalid timeout '%s'",
			         opt_sleep_us);
		}
	}

	treshold = sleep_us / 100 + 20000;

	setup();

	FD_ZERO(&sfds);

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		FD_SET(fds[0], &sfds);
		timeout = tst_us_to_timeval(sleep_us);

		tst_timer_start(CLOCK_MONOTONIC);
		TEST(select(1, &sfds, NULL, NULL, &timeout));
		tst_timer_stop();

		if (TEST_RETURN != 0) {
			tst_resm(TFAIL, "select() haven't timeouted ret=%li",
				 TEST_RETURN);
			continue;
		}

		elapsed_us = tst_timer_elapsed_us();

		if (elapsed_us < sleep_us) {
			tst_resm(TFAIL,
			         "select() woken up too early %llius, expected %llius",
				 elapsed_us, sleep_us);
			continue;
		}

		if (elapsed_us - sleep_us > treshold) {
			tst_resm(TFAIL,
			         "select() slept too long %llius, expected %llius, threshold %i",
				 elapsed_us, sleep_us, treshold);
			continue;
		}

		tst_resm(TPASS, "select() slept %llius, expected %llius, treshold %i",
		         elapsed_us, sleep_us, treshold);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_timer_check(CLOCK_MONOTONIC);

	SAFE_PIPE(NULL, fds);
}

static void cleanup(void)
{
	if (close(fds[0]))
		tst_resm(TWARN | TERRNO, "close(fds[0]) failed");

	if (close(fds[1]))
		tst_resm(TWARN | TERRNO, "close(fds[1]) failed");
}

static void help(void)
{
	printf("  -s      select() timeout lenght in us\n");
}
