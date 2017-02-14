/*
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) 2013 Stanislav Kholmanskikh <stanislav.kholmanskikh@oracle.com>
 * Copyright (c) 2010 Ngie Cooper <yaneurabeya@gmail.com>
 * Copyright (c) 2008 Mike Frysinger <vapier@gmail.com>
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

#ifndef TST_COMMON_H__
#define TST_COMMON_H__

#define LTP_ATTRIBUTE_NORETURN		__attribute__((noreturn))
#define LTP_ATTRIBUTE_UNUSED		__attribute__((unused))
#define LTP_ATTRIBUTE_UNUSED_RESULT	__attribute__((warn_unused_result))

#ifndef ARRAY_SIZE
# define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

/* Round x to the next multiple of a.
 * a should be a power of 2.
 */
#define LTP_ALIGN(x, a)    __LTP_ALIGN_MASK(x, (typeof(x))(a) - 1)
#define __LTP_ALIGN_MASK(x, mask)  (((x) + (mask)) & ~(mask))

#endif /* TST_COMMON_H__ */
