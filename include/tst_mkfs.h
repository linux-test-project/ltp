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

#ifndef TST_MKFS_H__
#define TST_MKFS_H__

/*
 * @dev: path to a device
 * @fs_type: filesystem type
 * @fs_opts: NULL or NULL terminated array of extra mkfs options
 */
void tst_mkfs_(const char *file, const int lineno, void (cleanup_fn)(void),
	       const char *dev, const char *fs_type,
	       const char *const fs_opts[], const char *extra_opt);

#define SAFE_MKFS(device, fs_type, fs_opts, extra_opt) \
	tst_mkfs_(__FILE__, __LINE__, NULL, device, fs_type, \
		  fs_opts, extra_opt)

#endif	/* TST_MKFS_H__ */
