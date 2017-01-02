/*
 * Copyright (c) 2010-2017 Linux Test Project
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

#ifndef TST_SAFE_PRW_H__
#define TST_SAFE_PRW_H__

static inline ssize_t safe_pread(const char *file, const int lineno,
		char len_strict, int fildes, void *buf, size_t nbyte,
		off_t offset)
{
	ssize_t rval;

	rval = pread(fildes, buf, nbyte, offset);

	if (rval == -1 || (len_strict && (size_t)rval != nbyte)) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			 "pread(%d,%p,%zu,%lld) failed",
			 fildes, buf, nbyte, (long long)offset);
	}

	return rval;
}
#define SAFE_PREAD(len_strict, fildes, buf, nbyte, offset) \
	safe_pread(__FILE__, __LINE__, (len_strict), (fildes), \
	           (buf), (nbyte), (offset))

static inline ssize_t safe_pwrite(const char *file, const int lineno,
		char len_strict, int fildes, const void *buf, size_t nbyte,
		off_t offset)
{
	ssize_t rval;

	rval = pwrite(fildes, buf, nbyte, offset);
	if (rval == -1 || (len_strict && (size_t)rval != nbyte)) {
		tst_brk_(file, lineno, TBROK | TERRNO,
			 "pwrite(%d,%p,%zu,%lld) failed",
			 fildes, buf, nbyte, (long long)offset);
	}

	return rval;
}
#define SAFE_PWRITE(len_strict, fildes, buf, nbyte, offset) \
	safe_pwrite(__FILE__, __LINE__, (len_strict), (fildes), \
	            (buf), (nbyte), (offset))

#endif /* SAFE_PRW_H__ */
