/*
 *   Copyright (c) International Business Machines Corp., 2001-2007
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
#ifndef _FFSB_STATS_H_
#define _FFSB_STATS_H_

#include <inttypes.h>

/* Latency statistics collection extension.
 *
 * For now, we are going to collect latency info on each (most)
 * syscalls using gettimeofday. Unfortunately, it is the
 * responsibility of each operation to collect this timing info.  We
 * try to make this easier by providing a function that does the
 * timing for supported syscalls.
 *
 * We want the ability to collect the average latency for a particular
 * call, and also to collect latency info for user specified intervals
 * -- called "buckets"
 */

struct stat_bucket {
	uint32_t min;
	uint32_t max;
	/* max = 0 indicates uninitialized bucket */
};

/* These are all the syscalls we currently support */
typedef enum { SYS_OPEN = 0,
	       SYS_READ,
	       SYS_WRITE,
	       SYS_CREATE,
	       SYS_LSEEK,
	       SYS_UNLINK,
	       SYS_CLOSE,
	       SYS_STAT
} syscall_t;

/* ASCII versions of the syscall names */
extern char *syscall_names[];

/* Return 1 on success, 0 on error */
int ffsb_stats_str2syscall(char *, syscall_t *);

/* Keep it in sync with syscall_t */
#define FFSB_NUM_SYSCALLS (8UL)

/* What stats to collect, shared among all threads  */
typedef struct ffsb_stats_config {
	unsigned num_buckets;
	struct stat_bucket *buckets;

	/* Ignore stats collection for some syscalls.
	 * Each bit corresponds to one syscall.
	 */
	uint32_t ignore_stats;
} ffsb_statsc_t;

void ffsb_statsc_init(ffsb_statsc_t *);
void ffsb_statsc_addbucket(ffsb_statsc_t *, uint32_t min, uint32_t max);
void ffsb_statsc_ignore_sys(ffsb_statsc_t *, syscall_t s);
void ffsb_statsc_destroy(ffsb_statsc_t *);

/* If we are collecting stats, then the config field is non-NULL */
typedef struct ffsb_stats_data {
	ffsb_statsc_t *config;
	uint32_t counts[FFSB_NUM_SYSCALLS];
	uint64_t totals[FFSB_NUM_SYSCALLS]; /* cumulative sums */
	uint64_t mins[FFSB_NUM_SYSCALLS];
	uint64_t maxs[FFSB_NUM_SYSCALLS];
	uint32_t *buckets[FFSB_NUM_SYSCALLS]; /* bucket counters */
} ffsb_statsd_t ;

/* constructor/destructor */
void ffsb_statsd_init(ffsb_statsd_t *, ffsb_statsc_t *);
void ffsb_statsd_destroy(ffsb_statsd_t *);

/* Add data to a stats data struct.  Value should be in microsecs
 * _NOT_ milli-secs
 */
void ffsb_add_data(ffsb_statsd_t *, syscall_t, uint32_t);

/* Make a copy of a stats config */
void ffsb_statsc_copy(ffsb_statsc_t *, ffsb_statsc_t *);

/* Add two statsd structs together */
void ffsb_statsd_add(ffsb_statsd_t *, ffsb_statsd_t *);

/* Print out statsd structure */
void ffsb_statsd_print(ffsb_statsd_t *fsd);

/* Do we want stats for the specified syscall */
int fsc_ignore_sys(ffsb_statsc_t *fsc, syscall_t s);

#endif /* _FFSB_STATS_H_ */
