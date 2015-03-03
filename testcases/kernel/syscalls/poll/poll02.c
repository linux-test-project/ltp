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
 * Check that poll() timeouts correctly.
 */
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/poll.h>

#include "test.h"
#include "safe_macros.h"

char *TCID = "poll02";
int TST_TOTAL = 1;

static char *opt_sleep_ms;

static option_t opts[] = {
	{"s:", NULL, &opt_sleep_ms},
	{NULL, NULL, NULL},
};

static void help(void);

int main(int ac, char **av)
{
	int lc, treshold;
	const char *msg;
	long long elapsed_ms, sleep_ms = 100;
	struct pollfd fds[] = {
		{.fd = 1, .events = POLLIN}
	};

	if ((msg = parse_opts(ac, av, opts, help)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	if (opt_sleep_ms) {
		sleep_ms = atoll(opt_sleep_ms);

		if (sleep_ms == 0)
			tst_brkm(TBROK, NULL, "Invalid timeout '%s'", opt_sleep_ms);
	}

	treshold = sleep_ms / 1000 + 10;

	tst_timer_check(CLOCK_MONOTONIC);

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_timer_start(CLOCK_MONOTONIC);
		TEST(poll(fds, 1, sleep_ms));
		tst_timer_stop();

		if (TEST_RETURN != 0) {
			tst_resm(TFAIL, "poll() haven't timeouted ret=%li",
				 TEST_RETURN);
			continue;
		}

		elapsed_ms = tst_timer_elapsed_ms();

		if (elapsed_ms < sleep_ms) {
			tst_resm(TFAIL,
			         "poll() woken up too early %llims, expected %llims",
				 elapsed_ms, sleep_ms);
			continue;
		}

		if (elapsed_ms - sleep_ms > treshold) {
			tst_resm(TFAIL,
			         "poll() slept too long %llims, expected %llims, threshold %i",
				 elapsed_ms, sleep_ms, treshold);
			continue;
		}

		tst_resm(TPASS, "poll() slept %llims, expected %llims, treshold %i",
		         elapsed_ms, sleep_ms, treshold);
	}

	tst_exit();
}

static void help(void)
{
	printf("  -s      poll() timeout lenght in ms\n");
}
