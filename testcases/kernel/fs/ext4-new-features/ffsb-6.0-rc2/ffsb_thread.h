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
#ifndef _FFSB_THREAD_H_
#define _FFSB_THREAD_H_

#include <pthread.h>
#include <inttypes.h>

#include "rand.h"
#include "ffsb_op.h"
#include "ffsb_tg.h"
#include "ffsb_stats.h"

#include "util.h" /* for barrier stuff */

struct ffsb_tg;
struct ffsb_op_results;

/* FFSB thread object
 *
 * The thread object doesn't store any configuration information, it
 * mostly just holds per-thread state information such as the random
 * data store and the per-thread buffer to copy data to/from disk
 */

typedef struct ffsb_thread {
	unsigned thread_num;
	unsigned tg_num;
	pthread_t ptid;
	struct randdata rd;
	struct ffsb_tg *tg; /* owning thread group */

	/* If we are using Direct IO, then we must only use a 4k
	 * aligned buffer so, alignedbuf_4k is a pointer into
	 * "mallocbuf" which is what malloc gave us.
	 */
	char *alignedbuf;
	char *mallocbuf;

	struct ffsb_op_results results;

	/* stats */
	ffsb_statsd_t fsd;
} ffsb_thread_t ;

void init_ffsb_thread(ffsb_thread_t *, struct ffsb_tg *, unsigned,
		       unsigned, unsigned);
void destroy_ffsb_thread(ffsb_thread_t *);

/* Owning thread group will start thread with this, thread runs until
 * *ft->checkval == ft->stopval.  Yes this is not strictly
 * synchronized, but that is okay for our purposes, and it limits (IMO
 * expensive) bus-locking.
 *
 * pthread_create() is called by tg with this function as a parameter
 * data is a (ffsb_thread_t*)
 */
void *ft_run(void *);

void ft_alter_bufsize(ffsb_thread_t *, unsigned);
char *ft_getbuf(ffsb_thread_t *);

int ft_get_read_random(ffsb_thread_t *);
uint32_t ft_get_read_size(ffsb_thread_t *);
uint32_t ft_get_read_blocksize(ffsb_thread_t *);

int ft_get_write_random(ffsb_thread_t *);
uint32_t ft_get_write_size(ffsb_thread_t *);
uint32_t ft_get_write_blocksize(ffsb_thread_t *);

int ft_get_fsync_file(ffsb_thread_t *);

randdata_t *ft_get_randdata(ffsb_thread_t *);

void ft_incr_op(ffsb_thread_t *ft, unsigned opnum, unsigned increment, uint64_t bytes);

void ft_add_readbytes(ffsb_thread_t *, uint32_t);
void ft_add_writebytes(ffsb_thread_t *, uint32_t);

int ft_get_read_skip(ffsb_thread_t *);
uint32_t ft_get_read_skipsize(ffsb_thread_t *);

ffsb_op_results_t *ft_get_results(ffsb_thread_t *);

void ft_set_statsc(ffsb_thread_t *, ffsb_statsc_t *);

/* for these two, ft == NULL is OK */
int ft_needs_stats(ffsb_thread_t *, syscall_t);
void ft_add_stat(ffsb_thread_t *, syscall_t, uint32_t);

ffsb_statsd_t *ft_get_stats_data(ffsb_thread_t *);

#endif /* _FFSB_THREAD_H_ */
