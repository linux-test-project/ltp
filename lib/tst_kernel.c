/*
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
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

#include <sys/personality.h>
#include <sys/utsname.h>
#include "test.h"
#include "tst_kernel.h"

static int get_kernel_bits_from_uname(struct utsname *buf)
{
	if (uname(buf)) {
		tst_brkm(TBROK | TERRNO, NULL, "uname()");
		return -1;
	}

	return strstr(buf->machine, "64") ? 64 : 32;
}

int tst_kernel_bits(void)
{
	struct utsname buf;
	int kernel_bits = get_kernel_bits_from_uname(&buf);

	if (kernel_bits == -1)
		return -1;

	/*
	 * ARM64 (aarch64) defines 32-bit compatibility modes as
	 * armv8l and armv8b (little and big endian).
	 * s390x is 64bit but not contain 64 in the words.
	 */
	if (!strcmp(buf.machine, "armv8l") || !strcmp(buf.machine, "armv8b")
			|| !strcmp(buf.machine, "s390x"))
		kernel_bits = 64;

#ifdef __ANDROID__
	/* Android's bionic libc sets the PER_LINUX32 personality for all 32-bit
	 * programs. This will cause buf.machine to report as i686 even though
	 * the kernel itself is 64-bit.
	 */
	if (!strcmp(buf.machine, "i686") &&
			(personality(0xffffffff) & PER_MASK) == PER_LINUX32) {
		/* Set the personality back to the default. */
		if (personality(PER_LINUX) == -1) {
			tst_brkm(TBROK | TERRNO, NULL, "personality()");
			return -1;
		}

		/* Redo the uname check without the PER_LINUX32 personality to
		 * determine the actual kernel bits value.
		 */
		kernel_bits = get_kernel_bits_from_uname(&buf);
		if (kernel_bits == -1)
			return -1;

		/* Set the personality back to PER_LINUX32. */
		if (personality(PER_LINUX32) == -1) {
			tst_brkm(TBROK | TERRNO, NULL, "personality()");
			return -1;
		}
	}
#endif  /* __ANDROID__ */

	tst_resm(TINFO, "uname.machine=%s kernel is %ibit",
	         buf.machine, kernel_bits);

	return kernel_bits;
}

int tst_check_driver(const char *name)
{
#ifndef __ANDROID__
	const char * const argv[] = { "modprobe", "-n", name, NULL };
	int res = tst_cmd_(NULL, argv, "/dev/null", "/dev/null",
			       TST_CMD_PASS_RETVAL);

	/* 255 - it looks like modprobe not available */
	return (res == 255) ? 0 : res;
#else
	/* Android modprobe may not have '-n', or properly installed
	 * module.*.bin files to determine built-in drivers. Assume
	 * all drivers are available.
	 */
	return 0;
#endif
}
