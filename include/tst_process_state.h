/*
 * Copyright (C) 2012-2014 Cyril Hrubis chrubis@suse.cz
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

 /*

   These functions helps you wait till a process with given pid changes state.
   This is for example useful when you need to wait in parent until child
   blocks.

  */

#ifndef TST_PROCESS_STATE__
#define TST_PROCESS_STATE__

#include <unistd.h>

/*
 * Waits for process state change.
 *
 * The state is one of the following:
 *
 * R - process is running
 * S - process is sleeping
 * D - process sleeping uninterruptibly
 * Z - zombie process
 * T - process is traced
 */
#ifdef TST_TEST_H__

#define TST_PROCESS_STATE_WAIT(pid, state, msec_timeout) \
	tst_process_state_wait(__FILE__, __LINE__, NULL, \
	                       (pid), (state), (msec_timeout))
#else
/*
 * The same as above but does not use tst_brkm() interface.
 *
 * This function is intended to be used from child processes.
 *
 * Returns zero on success, non-zero on failure.
 */
int tst_process_state_wait2(pid_t pid, const char state);

# define TST_PROCESS_STATE_WAIT(cleanup_fn, pid, state) \
	 tst_process_state_wait(__FILE__, __LINE__, (cleanup_fn), \
	                        (pid), (state), 0)
#endif

int tst_process_state_wait(const char *file, const int lineno,
                            void (*cleanup_fn)(void), pid_t pid,
			    const char state, unsigned int msec_timeout);

#endif /* TST_PROCESS_STATE__ */
