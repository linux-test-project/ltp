/*
 * Copyright (c) 2017 Carlo Marcelo Arenas Belón <carenas@gmail.com>
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
 * Very basic test for the RND* ioctls.
 *
 * Reads the entropy available from both /proc and the ioctl and compares
 * they are similar enough (within a configured fuzz factor)
 *
 */

#include <asm/types.h>
#include <linux/random.h>
#include <stdlib.h>
#include "tst_test.h"

static char *s_fuzz;
static int fuzz = 2;
static struct tst_option options[] = {
	{"f:", &s_fuzz, "-f c     Fuzz factor for valid match (default 2)"},
	{NULL, NULL, NULL}
};
static int fd;

static void verify_ioctl(void)
{
	int cnt, pcnt;

	SAFE_IOCTL(fd, RNDGETENTCNT, &cnt);
	SAFE_FILE_SCANF("/proc/sys/kernel/random/entropy_avail", "%d", &pcnt);
	tst_res(TINFO, "entropy value from ioctl: %d, proc: %d", cnt, pcnt);

	if (abs(pcnt - cnt) <= fuzz)
		tst_res(TPASS, "entropy value within expected parameters");
	else
		tst_res(TFAIL, "incorrect entropy value from ioctl");
}

static void setup(void)
{
	fd = SAFE_OPEN("/dev/urandom", O_RDONLY);
	if (s_fuzz)
		fuzz = SAFE_STRTOL(s_fuzz, 0, 4096);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.options = options,
	.test_all = verify_ioctl,
};
