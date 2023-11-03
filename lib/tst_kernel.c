/*
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) 2020-2021 Petr Vorel <pvorel@suse.cz>
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
#include <limits.h>

#include "test.h"
#include "tst_kernel.h"
#include "old_safe_stdio.h"

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
	static int kernel_bits;

	if (kernel_bits)
		return kernel_bits;

	kernel_bits = get_kernel_bits_from_uname(&buf);

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

static int tst_search_driver_(const char *driver, const char *file)
{
	struct stat st;
	char buf[PATH_MAX];
	char *path = NULL, *search = NULL, *sep = NULL;
	FILE *f;
	int ret = -1;

	struct utsname uts;

	if (uname(&uts)) {
		tst_brkm(TBROK | TERRNO, NULL, "uname() failed");
		return -1;
	}
	SAFE_ASPRINTF(NULL, &path, "/lib/modules/%s/%s", uts.release, file);

	if (stat(path, &st) || !(S_ISREG(st.st_mode) || S_ISLNK(st.st_mode))) {
		tst_resm(TWARN, "expected file %s does not exist or not a file", path);
		return -1;
	}

	if (access(path, R_OK)) {
		tst_resm(TWARN, "file %s cannot be read", path);
		return -1;
	}

	/* always search for x86_64 */
	char *fix = strstr(driver, "x86-64");

	if (fix)
		fix[3] = '_';

	SAFE_ASPRINTF(NULL, &search, "/%s.ko", driver);

	f = SAFE_FOPEN(NULL, path, "r");

	while (fgets(buf, sizeof(buf), f)) {
		/* cut dependencies after : */
		if ((sep = strchr(buf, ':')))
			*sep = 0;

		/* driver found */
		if (strstr(buf, search) != NULL) {
			ret = 0;
			break;
		}
	}

	SAFE_FCLOSE(NULL, f);
	free(search);
	free(path);

	return ret;
}

static int tst_search_driver(const char *driver, const char *file)
{
#ifdef __ANDROID__
	/*
	 * Android may not have properly installed modules.* files. We could
	 * search modules in /system/lib/modules, but to determine built-in
	 * drivers we need modules.builtin. Therefore assume all drivers are
	 * available.
	 */
	return 0;
#endif

	if (!tst_search_driver_(driver, file))
		return 0;

	int ret = -1;

	if (strrchr(driver, '-') || strrchr(driver, '_')) {
		char *driver2 = strdup(driver);
		char *ix = driver2;
		char find = '-', replace = '_';

		if (strrchr(driver, '_')) {
			find = '_';
			replace = '-';
		}

		while ((ix = strchr(ix, find)))
			*ix++ = replace;

		ret = tst_search_driver_(driver2, file);
		free(driver2);
	}

	return ret;
}

int tst_check_builtin_driver(const char *driver)
{
	if (!tst_search_driver(driver, "modules.builtin"))
		return 0;

	return -1;
}

int tst_check_driver(const char *driver)
{
	if (!tst_search_driver(driver, "modules.dep") ||
		!tst_check_builtin_driver(driver))
		return 0;

	return -1;
}
