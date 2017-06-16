/*
 * Copyright (c) 2017 Xiao Yang <yangx.jy@cn.fujitsu.com>
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

#ifndef SPLICE_H__
#define SPLICE_H__

#include <unistd.h>
#include "tst_safe_file_ops.h"
#include "tst_minmax.h"

static inline int get_max_limit(int default_len_data)
{
	int pipe_max_unpriv;

	if (!access("/proc/sys/fs/pipe-max-size", F_OK)) {
		SAFE_FILE_SCANF("/proc/sys/fs/pipe-max-size", "%d", &pipe_max_unpriv);
		return MIN(pipe_max_unpriv, default_len_data);
	}

	if (!access("/proc/sys/fs/pipe-max-pages", F_OK)) {
		SAFE_FILE_SCANF("/proc/sys/fs/pipe-max-pages", "%d", &pipe_max_unpriv);
		return MIN(pipe_max_unpriv * getpagesize(), default_len_data);
	}

	return default_len_data;
}

#endif  /* SPLICE_H__ */
