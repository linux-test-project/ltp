/*
 * Copyright (c) 2013 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __SAFE_STDIO_H__
#define __SAFE_STDIO_H__

#include <stdio.h>

FILE *safe_fopen(const char *file, const int lineno, void (cleanup_fn)(void),
                 const char *path, const char *mode);
#define SAFE_FOPEN(cleanup_fn, path, mode) \
	safe_fopen(__FILE__, __LINE__, cleanup_fn, path, mode)

int safe_fclose(const char *file, const int lineno, void (cleanup_fn)(void),
                FILE *f);
#define SAFE_FCLOSE(cleanup_fn, f) \
	safe_fclose(__FILE__, __LINE__, cleanup_fn, f)

int safe_asprintf(const char *file, const int lineno, void (cleanup_fn)(void),
                  char **strp, const char *fmt, ...);
#define SAFE_ASPRINTF(cleanup_fn, strp, fmt, ...) \
	safe_asprintf(__FILE__, __LINE__, cleanup_fn, strp, fmt, __VA_ARGS__)

#endif /* __SAFE_STDIO_H__ */
