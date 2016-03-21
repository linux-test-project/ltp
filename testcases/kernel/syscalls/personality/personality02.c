/*
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * If personality with STICKY_TIMEOUTS is used select() timeout is not updated.
 */

#include "test.h"
#include <sys/personality.h>
#include <sys/select.h>

char *TCID = "personality02";
int TST_TOTAL = 1;

#define USEC 10

static void verify_personality(void)
{
	struct timeval tv = {.tv_sec = 0, .tv_usec = USEC};
	int ret;
	fd_set rfds;

	FD_ZERO(&rfds);
	FD_SET(1, &rfds);

	personality(PER_LINUX | STICKY_TIMEOUTS);
	ret = select(2, &rfds, NULL, NULL, &tv);
	personality(PER_LINUX);
	if (ret < 0)
		tst_resm(TBROK | TERRNO, "select()");

	if (tv.tv_usec != USEC)
		tst_resm(TFAIL, "Timeout was modified");
	else
		tst_resm(TPASS, "Timeout wasn't modified");
}

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	for (lc = 0; TEST_LOOPING(lc); lc++)
		verify_personality();

	tst_exit();
}
