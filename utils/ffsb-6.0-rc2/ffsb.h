/*
 *   Copyright (c) International Business Machines Corp., 2001-2004
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
#ifndef _FFSB_H_
#define _FFSB_H_

#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "config.h"

#include "ffsb_op.h"
#include "ffsb_tg.h"
#include "ffsb_fs.h"

/*
 * The main thread wakes up once in so many seconds to check elapsed
 * time this is a tunable for that sleep interval in seconds
 */

#define FFSB_TG_WAIT_TIME (1)

#define MARK printf("MARK FUNC: %s() @ %s:%d\n", __FUNCTION__, __FILE__, __LINE__);

struct results {
	struct rusage before;
	struct rusage after;
	double runtime;
	double cputime;
	double cpu_before;
	double cpu_after;
	double cpu_total;
};

struct ffsb_tg;
struct ffsb_fs;

typedef struct profile_config {
	struct config_options *global;
	struct container *fs_container;
	struct container *tg_container;
} profile_config_t;

typedef struct ffsb_config {
	unsigned time;

	unsigned num_filesys;
	unsigned num_threadgroups;

	int num_totalthreads;		/* gets calculated after init() */

	struct ffsb_tg *groups;
	struct ffsb_fs *filesystems;

	struct profile_config *profile_conf;
	char *callout;			/* we will try and exec this */

	struct results results;
} ffsb_config_t;

void init_ffsb_config(ffsb_config_t *fc, unsigned num_fs, unsigned num_tg);

/*
 * this is kind of like a special case "constructor" which is only
 * used by fs-aging code to build a fake config for the aging tg
 */
void init_ffsb_config_1fs(ffsb_config_t *fc, struct ffsb_fs *fs,
			   struct ffsb_tg *tg);

void destroy_ffsb_config(ffsb_config_t *fc);

/* getters/setters, parser only should use setters */

void fc_set_time(ffsb_config_t *fc, unsigned time);

void fc_set_num_totalthreads(ffsb_config_t *fc, int num);

/* num is zero-based */
/* get a particular threadgroup object */
struct ffsb_tg *fc_get_tg(ffsb_config_t *fc, unsigned num);

/* get a particular filesystem object */
struct ffsb_fs *fc_get_fs(ffsb_config_t *fc, unsigned num);

void fc_set_callout(ffsb_config_t *fc, char *callout);
char *fc_get_callout(ffsb_config_t *fc);

#endif
