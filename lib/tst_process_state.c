/*
 * Copyright (C) 2012 Cyril Hrubis chrubis@suse.cz
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "tst_process_state.h"

void tst_process_state_wait(const char *file, const int lineno,
                            void (*cleanup_fn)(void),
                            pid_t pid, const char state)
{
	char proc_path[128], cur_state;

	snprintf(proc_path, sizeof(proc_path), "/proc/%i/stat", pid);

	do {
		safe_file_scanf(file, lineno, cleanup_fn, proc_path,
		                "%*i %*s %c", &cur_state);
		usleep(10000);
	} while (state != cur_state);
}
