/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2016 Linux Test Project
 */

#ifndef TST_SIG_PROC_H__
#define TST_SIG_PROC_H__

#include <sys/types.h>

pid_t create_sig_proc(int sig, int count, unsigned int usec);

#endif	/* TST_SIG_PROC_H__ */
