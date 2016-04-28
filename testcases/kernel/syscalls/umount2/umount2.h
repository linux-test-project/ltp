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

#ifndef UMOUNT2_H__
#define UMOUNT2_H__

static inline int umount2_retry(const char *target, int flags)
{
	int i, ret;

	for (i = 0; i < 50; i++) {
		ret = umount2(target, flags);

		if (ret == 0 || errno != EBUSY)
			return ret;

		tst_resm(TINFO, "umount('%s', %i) failed with EBUSY, try %2i...",
			 target, flags, i);

		usleep(100000);
	}

	tst_resm(TWARN, "Failed to umount('%s', %i) after 50 retries",
	         target, flags);

	errno = EBUSY;
	return -1;
}

#endif	/* UMOUNT2_H__ */
