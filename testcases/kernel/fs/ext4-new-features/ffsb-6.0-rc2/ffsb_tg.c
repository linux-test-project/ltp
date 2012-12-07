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
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>

#include "ffsb_tg.h"
#include "util.h"

void init_ffsb_tg(ffsb_tg_t * tg, unsigned num_threads, unsigned tg_num)
{
	int i;
	memset(tg, 0, sizeof(ffsb_tg_t));

	tg->threads = ffsb_malloc(sizeof(ffsb_thread_t) * num_threads);
	tg->tg_num = tg_num;
	tg->num_threads = num_threads;

	tg->bindfs = -1;	/* default is not bound */

	tg->thread_bufsize = 0;
	for (i = 0; i < num_threads; i++)
		init_ffsb_thread(tg->threads + i, tg, 0, tg_num, i);
}

void destroy_ffsb_tg(ffsb_tg_t * tg)
{
	int i;
	for (i = 0; i < tg->num_threads; i++)
		destroy_ffsb_thread(tg->threads + i);
	free(tg->threads);
	if (tg_needs_stats(tg))
		ffsb_statsc_destroy(&tg->fsc);
}

void *tg_run(void *data)
{
	tg_run_params_t *params = (tg_run_params_t *) data;
	ffsb_tg_t *tg = params->tg;
	int i;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

	tg->start_barrier = params->thread_barrier;

	/* Sum up the weights for use later by tg_get_op() */
	tg->sum_weights = 0;
	for (i = 0; i < FFSB_NUMOPS; i++)
		tg->sum_weights += tg->op_weights[i];

	tg->fc = params->fc;
	tg->flagval = -1;
	tg->stopval = 1;

	/* spawn threads */
	for (i = 0; i < tg->num_threads; i++) {
		ffsb_thread_t *ft = &tg->threads[i];
		pthread_create(&ft->ptid, &attr, ft_run, ft);
	}

	if (params->tg_barrier)
		ffsb_barrier_wait(params->tg_barrier);

	/* wait for termination condition to be true */
	do {
		ffsb_sleep(params->wait_time);
	} while (params->poll_fn(params->poll_data) == 0);

	/* set flag value */
	tg->flagval = tg->stopval;

	/* wait on theads to finish */
	for (i = 0; i < tg->num_threads; i++)
		pthread_join(tg->threads[i].ptid, NULL);

	return NULL;
}

/* Needs to set params->opnum and params->fs */
void tg_get_op(ffsb_tg_t * tg, randdata_t * rd, tg_op_params_t * params)
{
	unsigned curop;
	int num;
	int fsnum;

	num = 1 + getrandom(rd, tg->sum_weights);
	curop = 0;

	while (tg->op_weights[curop] < num) {
		num -= tg->op_weights[curop];
		curop++;
	}

	params->opnum = curop;

	/* If we're bound to a particular filesystem, use that,
	 * otherwise, pick one at random.
	 */
	fsnum = tg->bindfs;
	if (fsnum < 0)
		fsnum = getrandom(rd, tg->fc->num_filesys);

	params->fs = fc_get_fs(tg->fc, fsnum);
}

void tg_set_op_weight(ffsb_tg_t * tg, char *opname, unsigned weight)
{
	int opnum = ops_find_op(opname);
	assert(opnum >= 0);
	tg->op_weights[opnum] = weight;
}

unsigned tg_get_op_weight(ffsb_tg_t * tg, char *opname)
{
	int opnum = ops_find_op(opname);
	assert(opnum >= 0);
	return tg->op_weights[opnum];
}

void tg_set_bindfs(ffsb_tg_t * tg, int fsnum)
{
	tg->bindfs = fsnum;
}

int tg_get_bindfs(ffsb_tg_t * tg)
{
	return tg->bindfs;
}

unsigned tg_get_numthreads(ffsb_tg_t * tg)
{
	return tg->num_threads;
}

static void update_bufsize(ffsb_tg_t * tg)
{
	int i;
	uint32_t newmax = max(tg->read_blocksize, tg->write_blocksize);

	if (newmax == max(newmax, tg->thread_bufsize))
		for (i = 0; i < tg->num_threads; i++)
			ft_alter_bufsize(tg->threads + i, newmax);
}

void tg_set_read_random(ffsb_tg_t * tg, int rr)
{
	tg->read_random = rr;
}

void tg_set_write_random(ffsb_tg_t * tg, int wr)
{
	tg->write_random = wr;
}

void tg_set_fsync_file(ffsb_tg_t * tg, int fsync)
{
	tg->fsync_file = fsync;
}

void tg_set_read_size(ffsb_tg_t * tg, uint64_t rs)
{
	tg->read_size = rs;
}

void tg_set_read_blocksize(ffsb_tg_t * tg, uint32_t rs)
{
	tg->read_blocksize = rs;
	update_bufsize(tg);
}

void tg_set_read_skip(ffsb_tg_t * tg, int rs)
{
	tg->read_skip = rs;
}

void tg_set_read_skipsize(ffsb_tg_t * tg, uint32_t rs)
{
	tg->read_skipsize = rs;
}

void tg_set_write_size(ffsb_tg_t * tg, uint64_t ws)
{
	tg->write_size = ws;
}

void tg_set_write_blocksize(ffsb_tg_t * tg, uint32_t ws)
{
	tg->write_blocksize = ws;
	update_bufsize(tg);
}

int tg_get_read_random(ffsb_tg_t * tg)
{
	return tg->read_random;
}

int tg_get_write_random(ffsb_tg_t * tg)
{
	return tg->write_random;
}

int tg_get_fsync_file(ffsb_tg_t * tg)
{
	return tg->fsync_file;
}

uint64_t tg_get_read_size(ffsb_tg_t * tg)
{
	return tg->read_size;
}

uint32_t tg_get_read_blocksize(ffsb_tg_t * tg)
{
	return tg->read_blocksize;
}

int tg_get_read_skip(ffsb_tg_t * tg)
{
	return tg->read_skip;
}

uint32_t tg_get_read_skipsize(ffsb_tg_t * tg)
{
	return tg->read_skipsize;
}

uint64_t tg_get_write_size(ffsb_tg_t * tg)
{
	return tg->write_size;
}

uint32_t tg_get_write_blocksize(ffsb_tg_t * tg)
{
	return tg->write_blocksize;
}

int tg_get_stopval(ffsb_tg_t * tg)
{
	return tg->stopval;
}

ffsb_barrier_t *tg_get_start_barrier(ffsb_tg_t * tg)
{
	return tg->start_barrier;
}

static void tg_print_config_helper(ffsb_tg_t * tg)
{
	int i;
	int sumweights = 0;
	char buf[256];

	printf("\t num_threads      = %d\n", tg->num_threads);
	printf("\t\n");
	printf("\t read_random      = %s\n", (tg->read_random) ? "on" : "off");
	printf("\t read_size        = %llu\t(%s)\n", tg->read_size,
	       ffsb_printsize(buf, tg->read_size, 256));
	printf("\t read_blocksize   = %u\t(%s)\n", tg->read_blocksize,
	       ffsb_printsize(buf, tg->read_blocksize, 256));
	printf("\t read_skip        = %s\n", (tg->read_skip) ? "on" : "off");
	printf("\t read_skipsize    = %u\t(%s)\n", tg->read_skipsize,
	       ffsb_printsize(buf, tg->read_skipsize, 256));
	printf("\t\n");
	printf("\t write_random     = %s\n", (tg->write_random) ? "on" : "off");
	printf("\t write_size       = %llu\t(%s)\n", tg->write_size,
	       ffsb_printsize(buf, tg->write_size, 256));
	printf("\t fsync_file       = %d\n", tg->fsync_file);
	printf("\t write_blocksize  = %u\t(%s)\n", tg->write_blocksize,
	       ffsb_printsize(buf, tg->write_blocksize, 256));
	printf("\t wait time        = %u\n", tg->wait_time);
	if (tg->bindfs >= 0) {
		printf("\t\n");
		printf("\t bound to fs %d\n", tg->bindfs);
	}
	printf("\t\n");
	printf("\t op weights\n");

	for (i = 0; i < FFSB_NUMOPS; i++)
		sumweights += tg->op_weights[i];

	for (i = 0; i < FFSB_NUMOPS; i++)
		printf("\t %20s = %d (%.2f%%)\n", op_get_name(i),
		       tg->op_weights[i], 100 * (float)tg->op_weights[i] /
		       (float)sumweights);
	printf("\t\n");
}

void tg_print_config(ffsb_tg_t * tg)
{
	printf("ThreadGroup %d\n", tg->tg_num);
	printf("================\n");
	tg_print_config_helper(tg);
}

void tg_print_config_aging(ffsb_tg_t * tg, char *fsname)
{
	printf("\t Aging ThreadGroup for fs %s\n", fsname);
	printf("\t ================\n");
	tg_print_config_helper(tg);
}

void tg_collect_results(ffsb_tg_t * tg, ffsb_op_results_t * r)
{
	int i;
	for (i = 0; i < tg_get_numthreads(tg); i++)
		add_results(r, ft_get_results(tg->threads + i));
}

void tg_set_waittime(ffsb_tg_t * tg, unsigned time)
{
	tg->wait_time = time;
}

unsigned tg_get_waittime(ffsb_tg_t * tg)
{
	return tg->wait_time;
}

int tg_get_flagval(ffsb_tg_t * tg)
{
	return tg->flagval;
}

void tg_set_statsc(ffsb_tg_t * tg, ffsb_statsc_t * fsc)
{
	if (fsc) {
		int i;

		tg->need_stats = 1;
		ffsb_statsc_copy(&tg->fsc, fsc);

		for (i = 0; i < tg->num_threads; i++)
			ft_set_statsc(tg->threads + i, &tg->fsc);
	}
}

void tg_collect_stats(ffsb_tg_t * tg, ffsb_statsd_t * fsd)
{
	int i;

	assert(tg->need_stats);
	ffsb_statsd_init(fsd, &tg->fsc);

	for (i = 0; i < tg_get_numthreads(tg); i++)
		ffsb_statsd_add(fsd, ft_get_stats_data(tg->threads + i));
}

int tg_needs_stats(ffsb_tg_t * tg)
{
	return tg->need_stats;
}
