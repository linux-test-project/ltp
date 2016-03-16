/*
 * Copyright (c) 2013-2016 Cyril Hrubis <chrubis@suse.cz>
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

#ifndef SAFE_STDIO_FN_H__
#define SAFE_STDIO_FN_H__

#include <stdio.h>

FILE *safe_fopen(const char *file, const int lineno, void (cleanup_fn)(void),
                 const char *path, const char *mode);

int safe_fclose(const char *file, const int lineno, void (cleanup_fn)(void),
                FILE *f);

int safe_asprintf(const char *file, const int lineno, void (cleanup_fn)(void),
                  char **strp, const char *fmt, ...);

FILE *safe_popen(const char *file, const int lineno, void (cleanup_fn)(void),
		 const char *command, const char *type);

#endif /* SAFE_STDIO_FN_H__ */
