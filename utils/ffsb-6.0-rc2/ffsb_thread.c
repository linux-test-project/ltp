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
#include "ffsb_tg.h"
#include "ffsb_thread.h"
#include "ffsb_op.h"
#include "util.h"

void init_ffsb_thread(ffsb_thread_t * ft, struct ffsb_tg *tg, unsigned bufsize,
		      unsigned tg_num, unsigned thread_num)
{
	memset(ft, 0, sizeof(ffsb_thread_t));

	ft->tg = tg;
	ft->tg_num = tg_num;
	ft->thread_num = thread_num;

	if (bufsize)
		ft_alter_bufsize(ft, bufsize);

	init_random(&ft->rd, MAX_RANDBUF_SIZE);
}

void destroy_ffsb_thread(ffsb_thread_t * ft)
{
	free(ft->mallocbuf);
	destroy_random(&ft->rd);
	if (ft->fsd.config)
		ffsb_statsd_destroy(&ft->fsd);
}

void ft_set_statsc(ffsb_thread_t * ft, ffsb_statsc_t * fsc)
{
	ffsb_statsd_init(&ft->fsd, fsc);
}

void *ft_run(void *data)
{
	ffsb_thread_t *ft = (ffsb_thread_t *) data;
	tg_op_params_t params;
	unsigned wait_time = tg_get_waittime(ft->tg);
	int stopval = tg_get_stopval(ft->tg);

	ffsb_barrier_wait(tg_get_start_barrier(ft->tg));

	while (tg_get_flagval(ft->tg) != stopval) {
		tg_get_op(ft->tg, &ft->rd, &params);
		do_op(ft, params.fs, params.opnum);
		ffsb_milli_sleep(wait_time);
	}
	return NULL;
}

void ft_alter_bufsize(ffsb_thread_t * ft, unsigned bufsize)
{
	if (ft->mallocbuf != NULL)
		free(ft->mallocbuf);
	ft->mallocbuf = ffsb_malloc(bufsize + 4096);
	ft->alignedbuf = ffsb_align_4k(ft->mallocbuf + (4096 - 1));
}

char *ft_getbuf(ffsb_thread_t * ft)
{
	return ft->alignedbuf;
}

int ft_get_read_random(ffsb_thread_t * ft)
{
	return tg_get_read_random(ft->tg);
}

uint32_t ft_get_read_size(ffsb_thread_t * ft)
{
	return tg_get_read_size(ft->tg);
}

uint32_t ft_get_read_blocksize(ffsb_thread_t * ft)
{
	return tg_get_read_blocksize(ft->tg);
}

int ft_get_write_random(ffsb_thread_t * ft)
{
	return tg_get_write_random(ft->tg);
}

uint32_t ft_get_write_size(ffsb_thread_t * ft)
{
	return tg_get_write_size(ft->tg);
}

uint32_t ft_get_write_blocksize(ffsb_thread_t * ft)
{
	return tg_get_write_blocksize(ft->tg);
}

int ft_get_fsync_file(ffsb_thread_t * ft)
{
	return tg_get_fsync_file(ft->tg);
}

randdata_t *ft_get_randdata(ffsb_thread_t * ft)
{
	return &ft->rd;
}

void ft_incr_op(ffsb_thread_t * ft, unsigned opnum, unsigned increment,
		uint64_t bytes)
{
	ft->results.ops[opnum] += increment;
	ft->results.op_weight[opnum]++;
	ft->results.bytes[opnum] += bytes;
}

void ft_add_readbytes(ffsb_thread_t * ft, uint32_t bytes)
{
	ft->results.read_bytes += bytes;
}

void ft_add_writebytes(ffsb_thread_t * ft, uint32_t bytes)
{
	ft->results.write_bytes += bytes;
}

ffsb_op_results_t *ft_get_results(ffsb_thread_t * ft)
{
	return &ft->results;
}

int ft_get_read_skip(ffsb_thread_t * ft)
{
	return tg_get_read_skip(ft->tg);
}

uint32_t ft_get_read_skipsize(ffsb_thread_t * ft)
{
	return tg_get_read_skipsize(ft->tg);
}

int ft_needs_stats(ffsb_thread_t * ft, syscall_t sys)
{
	int ret = 0;
	if (ft && ft->fsd.config && !fsc_ignore_sys(ft->fsd.config, sys))
		ret = 1;
	return ret;
}

void ft_add_stat(ffsb_thread_t * ft, syscall_t sys, uint32_t val)
{
	if (ft)
		ffsb_add_data(&ft->fsd, sys, val);
}

ffsb_statsd_t *ft_get_stats_data(ffsb_thread_t * ft)
{
	return &ft->fsd;
}
