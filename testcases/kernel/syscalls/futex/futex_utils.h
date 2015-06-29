/*
 * Copyright (C) 2015 Cyril Hrubis <chrubis@suse.cz>
 *
 * Licensed under the GNU GPLv2 or later.
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

#ifndef FUTEX_UTILS_H__
#define FUTEX_UTILS_H__

/*
 * Wait for nr_threads to be sleeping
 */
static int wait_for_threads(unsigned int nr_threads)
{
	char thread_state, name[1024];
	DIR *dir;
	struct dirent *dent;
	unsigned int cnt = 0;

	snprintf(name, sizeof(name), "/proc/%i/task/", getpid());

	dir = SAFE_OPENDIR(NULL, name);

	while ((dent = SAFE_READDIR(NULL, dir))) {
		/* skip ".", ".." and the main thread */
		if (atoi(dent->d_name) == getpid() || atoi(dent->d_name) == 0)
			continue;

		snprintf(name, sizeof(name), "/proc/%i/task/%s/stat",
		         getpid(), dent->d_name);

		SAFE_FILE_SCANF(NULL, name, "%*i %*s %c", &thread_state);

		if (thread_state != 'S') {
			tst_resm(TINFO, "Thread %s not sleeping yet", dent->d_name);
			SAFE_CLOSEDIR(NULL, dir);
			return 1;
		}
		cnt++;
	}

	SAFE_CLOSEDIR(NULL, dir);

	if (cnt != nr_threads) {
		tst_resm(TINFO, "%u threads sleeping, expected %u",
	                  cnt, nr_threads);
	}

	return 0;
}

#endif /* FUTEX_UTILS_H__ */
