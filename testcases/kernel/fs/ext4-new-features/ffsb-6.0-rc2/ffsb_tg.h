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
#ifndef _FFSB_TG_H_
#define _FFSB_TG_H_
#include <inttypes.h>
#include <stdlib.h>
#include <sys/types.h>

#include <pthread.h>

#include "ffsb.h"
#include "ffsb_op.h"
#include "ffsb_thread.h"
#include "ffsb_fs.h"
#include "ffsb_stats.h"

#include "util.h" /* for barrier obj */

/* "Thread Group" object defs.
 *
 *  A thread group contains configuration information and can run its
 *  "gang" of threads performing a particular mix of operations.
 *
 * The thread group is responsible for creating the ffsb_thread
 * objects which must ask the thread group object to select an
 * operation and a filesystem to run that operation on.  The thread
 * objects don't contain any of the configuration information.
 *
 * Thread groups are also abstracted so they can be "run" arbitrarily
 * which is useful because we can reuse them for aging.  The running
 * is a bit complex, the thread group has to have a callback function
 * which is runs at a specified interval to determine when to
 * terminate.
 *
 * To synchronize starting across many thread groups there are two
 * barriers used, the first one "tg_barrier" in the run_params is to
 * synchronize multiple thread-groups being ready to start, meaning
 * that all their threads have been spawned The second one
 * "thread_barrier" synchronizes all threads across multiple thread
 * groups, so that they all start at once.
*/

struct ffsb_thread;
struct ffsb_config;

typedef struct ffsb_tg {
	unsigned tg_num;
	unsigned num_threads;
	unsigned op_weights[FFSB_NUMOPS];
	struct ffsb_thread *threads;

	/* A threadgroup can be bound to just one filesystem.
	 * A value * < 0 , means we aren't bound to any.
	*/
	int bindfs;

	int read_random;	/* boolean */
	uint64_t read_size;
	uint32_t read_blocksize;

	int read_skip;		/* boolean */
	uint32_t read_skipsize;

	int write_random;	/* boolean */
	uint64_t write_size;
	uint32_t write_blocksize;

	int fsync_file;		/* boolean */

	/* Should be max(write_blocksize, read_blocksize) */
	uint32_t thread_bufsize;

	/* these fields are calculated/set by tg_run() */
	unsigned sum_weights;
	struct ffsb_config *fc;
	ffsb_barrier_t *start_barrier;

	/* Used for stopping the threads */
	int flagval;
	int stopval;

	/* Delay between every operation, in milliseconds*/
	unsigned wait_time;

	/* stats configuration */
	int need_stats;
	ffsb_statsc_t fsc;
} ffsb_tg_t;

/* Init should be the very first thing called on the tg.  After that,
 * the user can call the set methods and finally run.
 */
void init_ffsb_tg(ffsb_tg_t *tg, unsigned num_threads, unsigned tg_num);
void destroy_ffsb_tg(ffsb_tg_t *tg);

/* Parameters needed to fire off a thread group.  The main thread will
 * evaluate poll_fn(poll_data) until it gets a nonzero return value.
 * It will sleep for wait_time secs between calls The ffsb_config
 * struct is needed for fs selection.  Barriers are to synchronize
 * multiple tgs and all threads pt is for pthread_create()
 */
typedef struct tg_run_params {
	ffsb_tg_t *tg;
	int (*poll_fn)(void *);
	void *poll_data;
	unsigned wait_time; /* in sec */
	struct ffsb_config *fc;
	ffsb_barrier_t *tg_barrier;

	/* Should be initialized by caller to tg_run() */
	ffsb_barrier_t *thread_barrier;
	pthread_t  pt;
} tg_run_params_t;

/* This function is meant to be called as a parameter to
 * pthread_create()
 */
void *tg_run(void *);

void tg_print_config(ffsb_tg_t *tg);
void tg_print_config_aging(ffsb_tg_t *tg, char *fsname);

/* Adds all of this tg's results to res */
void tg_collect_results(ffsb_tg_t *tg, ffsb_op_results_t *res);

/* Adds up all this tg's stats to totals */
void tg_collect_stats(ffsb_tg_t *tg, ffsb_statsd_t *totals);

/* getters/setters, setters should not be called after a run has begun */

void tg_set_statsc(ffsb_tg_t *tg, ffsb_statsc_t *fsc);

void tg_set_bindfs(ffsb_tg_t *tg, int fsnum);
int  tg_get_bindfs(ffsb_tg_t *tg);

unsigned tg_get_numthreads(ffsb_tg_t *tg);

void tg_set_op_weight(ffsb_tg_t *tg, char *opname, unsigned weight);
unsigned tg_get_op_weight(ffsb_tg_t *tg, char *opname);

void tg_set_read_random(ffsb_tg_t *tg, int rr);
void tg_set_write_random(ffsb_tg_t *tg, int wr);
void tg_set_fsync_file(ffsb_tg_t *tg, int fsync);

int tg_get_read_random(ffsb_tg_t *tg);
int tg_get_write_random(ffsb_tg_t *tg);
int tg_get_fsync_file(ffsb_tg_t *tg);

void tg_set_read_size(ffsb_tg_t *tg, uint64_t rs);
void tg_set_read_blocksize(ffsb_tg_t *tg, uint32_t rs);

void tg_set_read_skipsize(ffsb_tg_t *tg, uint32_t rs);
void tg_set_read_skip(ffsb_tg_t *tg, int rs);

void tg_set_write_size(ffsb_tg_t *tg, uint64_t ws);
void tg_set_write_blocksize(ffsb_tg_t *tg, uint32_t ws);

uint64_t tg_get_read_size(ffsb_tg_t *tg);
uint32_t tg_get_read_blocksize(ffsb_tg_t *tg);

int tg_get_read_skip(ffsb_tg_t *tg);
uint32_t tg_get_read_skipsize(ffsb_tg_t *tg);

uint64_t tg_get_write_size(ffsb_tg_t *tg);
uint32_t tg_get_write_blocksize(ffsb_tg_t *tg);

void tg_set_waittime(ffsb_tg_t *tg, unsigned time);
unsigned tg_get_waittime(ffsb_tg_t *tg);

/* The threads in the tg should be the only ones using these (below)
 * funcs.
 */
ffsb_barrier_t *tg_get_start_barrier(ffsb_tg_t *tg);
int tg_get_stopval(ffsb_tg_t *tg);
int tg_get_flagval(ffsb_tg_t *tg);

/* The threads in this tg will use this function to get an op to run,
 * so all configuration specific information is kept in this object.
 */
typedef struct tg_op_params {
	struct ffsb_fs *fs;     /* out parameter */
	unsigned opnum;         /* out parameter */
} tg_op_params_t;

/* tg and rd and in parameters, everything in params is out */
void  tg_get_op(ffsb_tg_t *tg, randdata_t *rd, tg_op_params_t *params);

/* want stats for this tg ? */
int tg_needs_stats(ffsb_tg_t *tg);

#endif /* _FFSB_TG_H_ */
