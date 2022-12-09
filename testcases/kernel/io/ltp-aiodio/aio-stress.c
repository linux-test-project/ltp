// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2004 SuSE, Inc.  All Rights Reserved.
 *               Written by: Chris Mason <mason@suse.com>
 * Copyright (C) 2022 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Test creates a series of files and start AIO operations on them.
 * AIO is done in a rotating loop: first file1.bin gets 8 requests, then
 * file2.bin, then file3.bin etc. As each file finishes writing, test switches
 * to reads. IO buffers are aligned in case we want to do direct IO.
 */

#define _FILE_OFFSET_BITS 64

#define _GNU_SOURCE
#include "tst_test.h"

#ifdef HAVE_LIBAIO
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <string.h>
#include <pthread.h>
#include <libaio.h>
#include "tst_safe_pthread.h"
#include "tst_safe_sysv_ipc.h"

#define IO_FREE 0
#define IO_PENDING 1

enum {
	WRITE,
	READ,
	RWRITE,
	RREAD,
	LAST_STAGE,
};

#define USE_MALLOC 0
#define USE_SHM 1
#define USE_SHMFS 2

static char *str_num_files;
static char *str_max_io_submit;
static char *str_num_contexts;
static char *str_context_offset;
static char *str_file_size;
static char *str_rec_len;
static char *str_depth;
static char *str_io_iter;
static char *str_iterations;
static char *str_o_flag;
static char *str_stages;
static char *str_use_shm;
static char *str_num_threads;

static int num_files = 1;
static long long file_size = 1024 * 1024 * 1024;
static long stages;
static unsigned long page_size_mask;
static int o_flag;
static char *latency_stats;
static char *completion_latency_stats;
static int io_iter = 8;
static int iterations = 500;
static int max_io_submit;
static long long rec_len = 64 * 1024;
static int depth = 64;
static int num_threads = 1;
static int num_contexts = 1;
static long long context_offset = 2 * 1024 * 1024;
static char *no_fsync_stages;
static int use_shm;
static int shm_id;
static char *unaligned_buffer;
static char *aligned_buffer;
static int padded_reclen;
static char *no_stonewall;
static char *verify;
static char *verify_buf;
static char *unlink_files;

/*
 * latencies during io_submit are measured, these are the
 * granularities for deviations
 */
#define DEVIATIONS 6
static int deviations[DEVIATIONS] = { 100, 250, 500, 1000, 5000, 10000 };

struct io_latency {
	double max;
	double min;
	double total_io;
	double total_lat;
	double deviations[DEVIATIONS];
};

/* container for a series of operations to a file */
struct io_oper {
	/* already open file descriptor, valid for whatever operation you want
	 */
	int fd;

	/* starting byte of the operation */
	off_t start;

	/* ending byte of the operation */
	off_t end;

	/* size of the read/write buffer */
	int reclen;

	/* max number of pending requests before a wait is triggered */
	int depth;

	/* current number of pending requests */
	int num_pending;

	/* last error, zero if there were none */
	int last_err;

	/* total number of errors hit. */
	int num_err;

	/* read,write, random, etc */
	int rw;

	/* number of I/O that will get sent to aio */
	int total_ios;

	/* number of I/O we've already sent */
	int started_ios;

	/* last offset used in an io operation */
	off_t last_offset;

	/* stonewalled = 1 when we got cut off before submitting all our I/O */
	int stonewalled;

	/* list management */
	struct io_oper *next;
	struct io_oper *prev;

	struct timeval start_time;

	char *file_name;
};

/* a single io, and all the tracking needed for it */
struct io_unit {
	/* note, iocb must go first! */
	struct iocb iocb;

	/* pointer to parent io operation struct */
	struct io_oper *io_oper;

	/* aligned buffer */
	char *buf;

	/* size of the aligned buffer (record size) */
	int buf_size;

	/* state of this io unit (free, pending, done) */
	int busy;

	/* result of last operation */
	long res;

	struct io_unit *next;

	struct timeval io_start_time; /* time of io_submit */
};

struct thread_info {
	io_context_t io_ctx;
	pthread_t tid;

	/* allocated array of io_unit structs */
	struct io_unit *ios;

	/* list of io units available for io */
	struct io_unit *free_ious;

	/* number of io units in the I/O array */
	int num_global_ios;

	/* number of io units in flight */
	int num_global_pending;

	/* preallocated array of iocb pointers, only used in run_active */
	struct iocb **iocbs;

	/* preallocated array of events */
	struct io_event *events;

	/* size of the events array */
	int num_global_events;

	/* latency stats for io_submit */
	struct io_latency io_submit_latency;

	/* list of operations still in progress, and of those finished */
	struct io_oper *active_opers;
	struct io_oper *finished_opers;

	/* number of files this thread is doing io on */
	int num_files;

	/* how much io this thread did in the last stage */
	double stage_mb_trans;

	/* latency completion stats i/o time from io_submit until io_getevents */
	struct io_latency io_completion_latency;
};

/* pthread mutexes and other globals for keeping the threads in sync */
static pthread_cond_t stage_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t stage_mutex = PTHREAD_MUTEX_INITIALIZER;
static int threads_ending;
static int threads_starting;
static struct timeval global_stage_start_time;
static struct thread_info *global_thread_info;

/*
 * return seconds between start_tv and stop_tv in double precision
 */
static double time_since(struct timeval *start_tv, struct timeval *stop_tv)
{
	double sec, usec;
	double ret;

	sec = stop_tv->tv_sec - start_tv->tv_sec;
	usec = stop_tv->tv_usec - start_tv->tv_usec;
	if (sec > 0 && usec < 0) {
		sec--;
		usec += 1000000;
	}

	ret = sec + usec / (double)1000000;
	if (ret < 0)
		ret = 0;

	return ret;
}

/*
 * return seconds between start_tv and now in double precision
 */
static double time_since_now(struct timeval *start_tv)
{
	struct timeval stop_time;

	gettimeofday(&stop_time, NULL);

	return time_since(start_tv, &stop_time);
}

/*
 * Add latency info to latency struct
 */
static void calc_latency(struct timeval *start_tv, struct timeval *stop_tv,
			 struct io_latency *lat)
{
	double delta;
	int i;

	delta = time_since(start_tv, stop_tv);
	delta = delta * 1000;

	if (delta > lat->max)
		lat->max = delta;

	if (!lat->min || delta < lat->min)
		lat->min = delta;

	lat->total_io++;
	lat->total_lat += delta;

	for (i = 0; i < DEVIATIONS; i++) {
		if (delta < deviations[i]) {
			lat->deviations[i]++;
			break;
		}
	}
}

static void oper_list_add(struct io_oper *oper, struct io_oper **list)
{
	if (!*list) {
		*list = oper;
		oper->prev = oper->next = oper;
		return;
	}

	oper->prev = (*list)->prev;
	oper->next = *list;

	(*list)->prev->next = oper;
	(*list)->prev = oper;
}

static void oper_list_del(struct io_oper *oper, struct io_oper **list)
{
	if ((*list)->next == (*list)->prev && *list == (*list)->next) {
		*list = NULL;
		return;
	}

	oper->prev->next = oper->next;
	oper->next->prev = oper->prev;

	if (*list == oper)
		*list = oper->next;
}

/* worker func to check error fields in the io unit */
static int check_finished_io(struct io_unit *io)
{
	int i;

	if (io->res != io->buf_size) {
		struct stat s;

		SAFE_FSTAT(io->io_oper->fd, &s);

		/*
		 * If file size is large enough for the read, then this short
		 * read is an error.
		 */
		if ((io->io_oper->rw == READ || io->io_oper->rw == RREAD) &&
		    s.st_size > (io->iocb.u.c.offset + io->res)) {

			tst_res(TINFO, "io err %lu (%s) op %d, off %llu size %d",
				io->res, tst_strerrno(-io->res), io->iocb.aio_lio_opcode,
				io->iocb.u.c.offset, io->buf_size);
			io->io_oper->last_err = io->res;
			io->io_oper->num_err++;
			return -1;
		}
	}

	if (verify && io->io_oper->rw == READ) {
		if (memcmp(io->buf, verify_buf, io->io_oper->reclen)) {
			tst_res(TINFO, "verify error, file %s offset %llu contents (offset:bad:good):",
				io->io_oper->file_name, io->iocb.u.c.offset);

			for (i = 0; i < io->io_oper->reclen; i++) {
				if (io->buf[i] != verify_buf[i]) {
					tst_res(TINFO, "%d:%c:%c ", i,
						io->buf[i], verify_buf[i]);
				}
			}
		}
	}

	return 0;
}

/* worker func to check the busy bits and get an io unit ready for use */
static int grab_iou(struct io_unit *io, struct io_oper *oper)
{
	if (io->busy == IO_PENDING)
		return -1;

	io->busy = IO_PENDING;
	io->res = 0;
	io->io_oper = oper;

	return 0;
}

static char *stage_name(int rw)
{
	switch (rw) {
	case WRITE:
		return "write";
	case READ:
		return "read";
	case RWRITE:
		return "random write";
	case RREAD:
		return "random read";
	}

	return "unknown";
}

static inline double oper_mb_trans(struct io_oper *oper)
{
	return ((double)oper->started_ios * (double)oper->reclen) / (double)(1024 * 1024);
}

static void print_time(struct io_oper *oper)
{
	double runtime;
	double tput;
	double mb;

	runtime = time_since_now(&oper->start_time);
	mb = oper_mb_trans(oper);
	tput = mb / runtime;

	tst_res(TINFO, "%s on %s (%.2f MB/s) %.2f MB in %.2fs",
		stage_name(oper->rw), oper->file_name, tput, mb, runtime);
}

static void print_lat(char *str, struct io_latency *lat)
{
	char out[4 * 1024];
	char *ptr = out;
	double avg = lat->total_lat / lat->total_io;
	int i;
	double total_counted = 0;

	tst_res(TINFO, "%s min %.2f avg %.2f max %.2f", str, lat->min, avg, lat->max);

	for (i = 0; i < DEVIATIONS; i++) {
		ptr += sprintf(ptr, "%.0f < %d", lat->deviations[i], deviations[i]);
		total_counted += lat->deviations[i];
	}

	if (total_counted && lat->total_io - total_counted)
		ptr += sprintf(ptr, " < %.0f", lat->total_io - total_counted);

	tst_res(TINFO, "%s", out);

	memset(lat, 0, sizeof(*lat));
}

static void print_latency(struct thread_info *t)
{
	struct io_latency *lat = &t->io_submit_latency;

	print_lat("latency", lat);
}

static void print_completion_latency(struct thread_info *t)
{
	struct io_latency *lat = &t->io_completion_latency;

	print_lat("completion latency", lat);
}

/*
 * updates the fields in the io operation struct that belongs to this
 * io unit, and make the io unit reusable again
 */
static void finish_io(struct thread_info *t, struct io_unit *io, long result,
		      struct timeval *tv_now)
{
	struct io_oper *oper = io->io_oper;

	calc_latency(&io->io_start_time, tv_now, &t->io_completion_latency);
	io->res = result;
	io->busy = IO_FREE;
	io->next = t->free_ious;
	t->free_ious = io;
	oper->num_pending--;
	t->num_global_pending--;
	check_finished_io(io);

	if (oper->num_pending == 0 &&
	    (oper->started_ios == oper->total_ios || oper->stonewalled)) {
		print_time(oper);
	}
}

static int read_some_events(struct thread_info *t)
{
	struct io_unit *event_io;
	struct io_event *event;
	int nr;
	int i;
	int min_nr = io_iter;
	struct timeval stop_time;

	if (t->num_global_pending < io_iter)
		min_nr = t->num_global_pending;

	nr = io_getevents(t->io_ctx, min_nr, t->num_global_events, t->events, NULL);
	if (nr <= 0)
		return nr;

	gettimeofday(&stop_time, NULL);

	for (i = 0; i < nr; i++) {
		event = t->events + i;
		event_io = (struct io_unit *)((unsigned long)event->obj);
		finish_io(t, event_io, event->res, &stop_time);
	}

	return nr;
}

/*
 * finds a free io unit, waiting for pending requests if required.  returns
 * null if none could be found
 */
static struct io_unit *find_iou(struct thread_info *t, struct io_oper *oper)
{
	struct io_unit *event_io;
	int nr;

retry:
	if (t->free_ious) {
		event_io = t->free_ious;
		t->free_ious = t->free_ious->next;

		if (grab_iou(event_io, oper))
			tst_brk(TBROK, "io unit on free list but not free");

		return event_io;
	}

	nr = read_some_events(t);
	if (nr > 0)
		goto retry;
	else
		tst_res(TINFO, "no free ious after read_some_events");

	return NULL;
}

/*
 * wait for all pending requests for this io operation to finish
 */
static int io_oper_wait(struct thread_info *t, struct io_oper *oper)
{
	struct io_event event;
	struct io_unit *event_io;

	if (!oper)
		return 0;

	if (oper->num_pending == 0)
		goto done;

		/* this func is not speed sensitive, no need to go wild reading
		 * more than one event at a time
		 */
	while (io_getevents(t->io_ctx, 1, 1, &event, NULL) > 0) {
		struct timeval tv_now;

		event_io = (struct io_unit *)((unsigned long)event.obj);

		gettimeofday(&tv_now, NULL);
		finish_io(t, event_io, event.res, &tv_now);

		if (oper->num_pending == 0)
			break;
	}
done:
	if (oper->num_err)
		tst_res(TINFO, "%u errors on oper, last %u", oper->num_err, oper->last_err);

	return 0;
}

static off_t random_byte_offset(struct io_oper *oper)
{
	off_t num;
	off_t rand_byte = oper->start;
	off_t range;
	off_t offset = 1;

	range = (oper->end - oper->start) / (1024 * 1024);

	if ((page_size_mask + 1) > (1024 * 1024))
		offset = (page_size_mask + 1) / (1024 * 1024);

	if (range < offset)
		range = 0;
	else
		range -= offset;

	/* find a random mb offset */
	num = 1 + (int)((double)range * rand() / (RAND_MAX + 1.0));
	rand_byte += num * 1024 * 1024;

	/* find a random byte offset */
	num = 1 + (int)((double)(1024 * 1024) * rand() / (RAND_MAX + 1.0));

	/* page align */
	num = (num + page_size_mask) & ~page_size_mask;
	rand_byte += num;

	if (rand_byte + oper->reclen > oper->end)
		rand_byte -= oper->reclen;

	return rand_byte;
}

/*
 * build an aio iocb for an operation, based on oper->rw and the
 * last offset used.  This finds the struct io_unit that will be attached
 * to the iocb, and things are ready for submission to aio after this
 * is called.
 *
 * returns null on error
 */
static struct io_unit *build_iocb(struct thread_info *t, struct io_oper *oper)
{
	struct io_unit *io;
	off_t rand_byte;

	io = find_iou(t, oper);
	if (!io)
		tst_brk(TBROK, "unable to find io unit");

	switch (oper->rw) {
	case WRITE:
		io_prep_pwrite(&io->iocb, oper->fd, io->buf, oper->reclen, oper->last_offset);
		oper->last_offset += oper->reclen;
		break;
	case READ:
		io_prep_pread(&io->iocb, oper->fd, io->buf, oper->reclen, oper->last_offset);
		oper->last_offset += oper->reclen;
		break;
	case RREAD:
		rand_byte = random_byte_offset(oper);
		oper->last_offset = rand_byte;
		io_prep_pread(&io->iocb, oper->fd, io->buf, oper->reclen, rand_byte);
		break;
	case RWRITE:
		rand_byte = random_byte_offset(oper);
		oper->last_offset = rand_byte;
		io_prep_pwrite(&io->iocb, oper->fd, io->buf, oper->reclen, rand_byte);

		break;
	}

	return io;
}

/*
 * wait for any pending requests, and then free all ram associated with
 * an operation.  returns the last error the operation hit (zero means none)
 */
static int finish_oper(struct thread_info *t, struct io_oper *oper)
{
	unsigned long last_err;

	io_oper_wait(t, oper);

	last_err = oper->last_err;

	if (oper->num_pending > 0)
		tst_res(TINFO, "oper num_pending is %d", oper->num_pending);

	SAFE_CLOSE(oper->fd);
	free(oper);

	return last_err;
}

/*
 * allocates an io operation and fills in all the fields.  returns
 * null on error
 */
static struct io_oper *create_oper(int fd, int rw, off_t start, off_t end,
				   int reclen, int depth, char *file_name)
{
	struct io_oper *oper;

	oper = SAFE_MALLOC(sizeof(*oper));
	memset(oper, 0, sizeof(*oper));

	oper->depth = depth;
	oper->start = start;
	oper->end = end;
	oper->last_offset = oper->start;
	oper->fd = fd;
	oper->reclen = reclen;
	oper->rw = rw;
	oper->total_ios = (oper->end - oper->start) / oper->reclen;
	oper->file_name = file_name;

	return oper;
}

/*
 * does setup on num_ios worth of iocbs, but does not actually
 * start any io
 */
static int build_oper(struct thread_info *t, struct io_oper *oper, int num_ios,
		      struct iocb **my_iocbs)
{
	int i;
	struct io_unit *io;

	if (oper->started_ios == 0)
		gettimeofday(&oper->start_time, NULL);

	if (num_ios == 0)
		num_ios = oper->total_ios;

	if ((oper->started_ios + num_ios) > oper->total_ios)
		num_ios = oper->total_ios - oper->started_ios;

	for (i = 0; i < num_ios; i++) {
		io = build_iocb(t, oper);
		if (!io)
			return -1;

		my_iocbs[i] = &io->iocb;
	}

	return num_ios;
}

/*
 * runs through the iocbs in the array provided and updates
 * counters in the associated oper struct
 */
static void update_iou_counters(struct iocb **my_iocbs, int nr, struct timeval *tv_now)
{
	struct io_unit *io;
	int i;

	for (i = 0; i < nr; i++) {
		io = (struct io_unit *)(my_iocbs[i]);
		io->io_oper->num_pending++;
		io->io_oper->started_ios++;
		io->io_start_time = *tv_now; /* set time of io_submit */
	}
}

/* starts some io for a given file, returns zero if all went well */
static int run_built(struct thread_info *t, int num_ios, struct iocb **my_iocbs)
{
	int ret;
	struct timeval start_time;
	struct timeval stop_time;

resubmit:
	gettimeofday(&start_time, NULL);
	ret = io_submit(t->io_ctx, num_ios, my_iocbs);

	gettimeofday(&stop_time, NULL);
	calc_latency(&start_time, &stop_time, &t->io_submit_latency);

	if (ret != num_ios) {
		/* some I/O got through */
		if (ret > 0) {
			update_iou_counters(my_iocbs, ret, &stop_time);
			my_iocbs += ret;
			t->num_global_pending += ret;
			num_ios -= ret;
		}
		/*
		 * we've used all the requests allocated in aio_init, wait and
		 * retry
		 */
		if (ret > 0 || ret == -EAGAIN) {
			int old_ret = ret;

			ret = read_some_events(t);
			if (ret <= 0)
				tst_brk(TBROK, "ret was %d and now is %d", ret, old_ret);

			goto resubmit;
		}

		tst_res(TINFO, "ret %d (%s) on io_submit", ret, tst_strerrno(-ret));
		return -1;
	}

	update_iou_counters(my_iocbs, ret, &stop_time);
	t->num_global_pending += ret;

	return 0;
}

/*
 * changes oper->rw to the next in a command sequence, or returns zero
 * to say this operation is really, completely done for
 */
static int restart_oper(struct io_oper *oper)
{
	int new_rw = 0;

	if (oper->last_err)
		return 0;

	if (oper->rw == WRITE && (stages & (1 << READ)))
		new_rw = READ;

	if (oper->rw == READ && (!new_rw && stages & (1 << RWRITE)))
		new_rw = RWRITE;

	if (oper->rw == RWRITE && (!new_rw && stages & (1 << RREAD)))
		new_rw = RREAD;

	if (new_rw) {
		oper->started_ios = 0;
		oper->last_offset = oper->start;
		oper->stonewalled = 0;

		/*
		 * we're restarting an operation with pending requests, so the
		 * timing info won't be printed by finish_io.  Printing it here
		 */
		if (oper->num_pending)
			print_time(oper);

		oper->rw = new_rw;
		return 1;
	}

	return 0;
}

static int oper_runnable(struct io_oper *oper)
{
	struct stat buf;

	/* first context is always runnable, if started_ios > 0, no need to
	 * redo the calculations
	 */
	if (oper->started_ios || oper->start == 0)
		return 1;

	/* only the sequential phases force delays in starting */
	if (oper->rw >= RWRITE)
		return 1;

	SAFE_FSTAT(oper->fd, &buf);
	if (S_ISREG(buf.st_mode) && buf.st_size < oper->start)
		return 0;

	return 1;
}

/*
 * runs through all the io operations on the active list, and starts
 * a chunk of io on each.  If any io operations are completely finished,
 * it either switches them to the next stage or puts them on the
 * finished list.
 *
 * this function stops after max_io_submit iocbs are sent down the
 * pipe, even if it has not yet touched all the operations on the
 * active list.  Any operations that have finished are moved onto
 * the finished_opers list.
 */
static int run_active_list(struct thread_info *t, int io_iter, int max_io_submit)
{
	struct io_oper *oper;
	struct io_oper *built_opers = NULL;
	struct iocb **my_iocbs = t->iocbs;
	int ret = 0;
	int num_built = 0;

	oper = t->active_opers;

	while (oper) {
		if (!oper_runnable(oper)) {
			oper = oper->next;
			if (oper == t->active_opers)
				break;
			continue;
		}

		ret = build_oper(t, oper, io_iter, my_iocbs);
		if (ret >= 0) {
			my_iocbs += ret;
			num_built += ret;
			oper_list_del(oper, &t->active_opers);
			oper_list_add(oper, &built_opers);
			oper = t->active_opers;
			if (num_built + io_iter > max_io_submit)
				break;
		} else
			break;
	}

	if (num_built) {
		ret = run_built(t, num_built, t->iocbs);
		if (ret < 0)
			tst_brk(TBROK, "error %d on run_built", ret);

		while (built_opers) {
			oper = built_opers;
			oper_list_del(oper, &built_opers);
			oper_list_add(oper, &t->active_opers);
			if (oper->started_ios == oper->total_ios) {
				oper_list_del(oper, &t->active_opers);
				oper_list_add(oper, &t->finished_opers);
			}
		}
	}

	return 0;
}

static void aio_setup(io_context_t *io_ctx, int n)
{
	int res = io_queue_init(n, io_ctx);

	if (res != 0)
		tst_brk(TBROK, "io_queue_setup(%d) returned %d (%s)", n, res, tst_strerrno(-res));
}

/*
 * allocate io operation and event arrays for a given thread
 */
static void setup_ious(struct thread_info *t, int num_files, int depth, int reclen, int max_io_submit)
{
	int i;
	size_t bytes = num_files * depth * sizeof(*t->ios);

	t->ios = SAFE_MALLOC(bytes);

	memset(t->ios, 0, bytes);

	for (i = 0; i < depth * num_files; i++) {
		t->ios[i].buf = aligned_buffer;
		aligned_buffer += padded_reclen;
		t->ios[i].buf_size = reclen;
		if (verify)
			memset(t->ios[i].buf, 'b', reclen);
		else
			memset(t->ios[i].buf, 0, reclen);
		t->ios[i].next = t->free_ious;
		t->free_ious = t->ios + i;
	}

	if (verify) {
		verify_buf = aligned_buffer;
		memset(verify_buf, 'b', reclen);
	}

	t->iocbs = SAFE_MALLOC(sizeof(struct iocb *) * max_io_submit);
	memset(t->iocbs, 0, max_io_submit * sizeof(struct iocb *));

	t->events = SAFE_MALLOC(sizeof(struct io_event) * depth * num_files);
	memset(t->events, 0, num_files * sizeof(struct io_event) * depth);

	t->num_global_ios = num_files * depth;
	t->num_global_events = t->num_global_ios;
}

/*
 * The buffers used for file data are allocated as a single big
 * malloc, and then each thread and operation takes a piece and uses
 * that for file data.  This lets us do a large shm or bigpages alloc
 * and without trying to find a special place in each thread to map the
 * buffers to
 */
static int setup_shared_mem(int num_threads, int num_files, int depth, int reclen)
{
	char *p = NULL;
	size_t total_ram;

	padded_reclen = (reclen + page_size_mask) / (page_size_mask + 1);
	padded_reclen = padded_reclen * (page_size_mask + 1);
	total_ram = num_files * depth * padded_reclen + num_threads;

	if (verify)
		total_ram += padded_reclen;

	/* for aligning buffer after the allocation */
	total_ram += page_size_mask;

	if (use_shm == USE_MALLOC) {
		p = SAFE_MALLOC(total_ram);
	} else if (use_shm == USE_SHM) {
		SAFE_SHMGET(IPC_PRIVATE, total_ram, IPC_CREAT | 0700);
		p = SAFE_SHMAT(shm_id, (char *)0x50000000, 0);
	} else if (use_shm == USE_SHMFS) {
		char mmap_name[16]; /* /dev/shm/ + null + XXXXXX */
		int fd;

		strcpy(mmap_name, "/dev/shm/XXXXXX");
		fd = mkstemp(mmap_name);
		if (fd < 0)
			tst_brk(TBROK, "mkstemp error");

		SAFE_UNLINK(mmap_name);
		SAFE_FTRUNCATE(fd, total_ram);

		shm_id = fd;

		p = SAFE_MMAP((char *)0x50000000, total_ram,
			      PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	}

	unaligned_buffer = p;
	p = (char *)((intptr_t)(p + page_size_mask) & ~page_size_mask);
	aligned_buffer = p;

	return 0;
}

/*
 * runs through all the thread_info structs and calculates a combined
 * throughput
 */
static void global_thread_throughput(struct thread_info *t, char *this_stage)
{
	int i;
	double runtime = time_since_now(&global_stage_start_time);
	double total_mb = 0;
	double min_trans = 0;

	for (i = 0; i < num_threads; i++) {
		total_mb += global_thread_info[i].stage_mb_trans;

		if (!min_trans || t->stage_mb_trans < min_trans)
			min_trans = t->stage_mb_trans;
	}

	if (total_mb) {
		tst_res(TINFO, "%s throughput (%.2f MB/s)", this_stage, total_mb / runtime);
		tst_res(TINFO, "%.2f MB in %.2fs", total_mb, runtime);

		if (no_stonewall)
			tst_res(TINFO, "min transfer %.2fMB", min_trans);
	}
}

/* this is the meat of the state machine.  There is a list of
 * active operations structs, and as each one finishes the required
 * io it is moved to a list of finished operations.  Once they have
 * all finished whatever stage they were in, they are given the chance
 * to restart and pick a different stage (read/write/random read etc)
 *
 * various timings are printed in between the stages, along with
 * thread synchronization if there are more than one threads.
 */
static int *worker(struct thread_info *t)
{
	struct io_oper *oper;
	char *this_stage = NULL;
	struct timeval stage_time;
	int status = 0;
	int iteration = 0;
	int cnt;

	aio_setup(&t->io_ctx, 512);

restart:
	if (num_threads > 1) {
		pthread_mutex_lock(&stage_mutex);
		threads_starting++;

		if (threads_starting == num_threads) {
			threads_ending = 0;
			gettimeofday(&global_stage_start_time, NULL);
			pthread_cond_broadcast(&stage_cond);
		}

		while (threads_starting != num_threads)
			pthread_cond_wait(&stage_cond, &stage_mutex);
		pthread_mutex_unlock(&stage_mutex);
	}

	if (t->active_opers) {
		this_stage = stage_name(t->active_opers->rw);
		gettimeofday(&stage_time, NULL);
		t->stage_mb_trans = 0;
	}

	cnt = 0;

	/* first we send everything through aio */
	while (t->active_opers && cnt < iterations) {
		if (!no_stonewall && threads_ending) {
			oper = t->active_opers;
			oper->stonewalled = 1;
			oper_list_del(oper, &t->active_opers);
			oper_list_add(oper, &t->finished_opers);
		} else {
			run_active_list(t, io_iter, max_io_submit);
		}
		cnt++;
	}

	if (latency_stats)
		print_latency(t);

	if (completion_latency_stats)
		print_completion_latency(t);

	/* then we wait for all the operations to finish */
	oper = t->finished_opers;
	do {
		if (!oper)
			break;
		io_oper_wait(t, oper);
		oper = oper->next;
	} while (oper != t->finished_opers);

	/* then we do an fsync to get the timing for any future operations
	 * right, and check to see if any of these need to get restarted
	 */
	oper = t->finished_opers;
	while (oper) {
		if (!no_fsync_stages)
			SAFE_FSYNC(oper->fd);

		t->stage_mb_trans += oper_mb_trans(oper);

		if (restart_oper(oper)) {
			oper_list_del(oper, &t->finished_opers);
			oper_list_add(oper, &t->active_opers);
			oper = t->finished_opers;
			continue;
		}

		oper = oper->next;

		if (oper == t->finished_opers)
			break;
	}

	if (t->stage_mb_trans && t->num_files > 0) {
		double seconds = time_since_now(&stage_time);

		tst_res(TINFO, "thread %td %s totals (%.2f MB/s) %.2f MB in %.2fs",
			t - global_thread_info, this_stage,
			t->stage_mb_trans / seconds, t->stage_mb_trans, seconds);
	}

	if (num_threads > 1) {
		pthread_mutex_lock(&stage_mutex);
		threads_ending++;

		if (threads_ending == num_threads) {
			threads_starting = 0;
			pthread_cond_broadcast(&stage_cond);
			global_thread_throughput(t, this_stage);
		}

		while (threads_ending != num_threads)
			pthread_cond_wait(&stage_cond, &stage_mutex);
		pthread_mutex_unlock(&stage_mutex);
	}

	/* someone got restarted, go back to the beginning */
	if (t->active_opers && cnt < iterations) {
		iteration++;
		goto restart;
	}

	/* finally, free all the ram */
	while (t->finished_opers) {
		oper = t->finished_opers;
		oper_list_del(oper, &t->finished_opers);
		status = finish_oper(t, oper);
	}

	if (t->num_global_pending)
		tst_res(TINFO, "global num pending is %d", t->num_global_pending);

	io_queue_release(t->io_ctx);

	return (void *)(intptr_t)status;
}

typedef void *(*start_routine)(void *);
static int run_workers(struct thread_info *t, int num_threads)
{
	void *retval;
	int ret = 0;
	int i;

	for (i = 0; i < num_threads; i++)
		SAFE_PTHREAD_CREATE(&t[i].tid, NULL, (start_routine)worker, t + i);

	for (i = 0; i < num_threads; i++) {
		SAFE_PTHREAD_JOIN(t[i].tid, &retval);
		ret |= (intptr_t)retval;
	}

	return ret;
}

static void setup(void)
{
	int maxaio;
	int stages_i;

	page_size_mask = getpagesize() - 1;

	SAFE_FILE_SCANF("/proc/sys/fs/aio-max-nr", "%d", &maxaio);
	tst_res(TINFO, "Maximum AIO blocks: %d", maxaio);

	if (tst_parse_int(str_num_files, &num_files, 1, INT_MAX))
		tst_brk(TBROK, "Invalid number of files to generate '%s'", str_num_files);

	if (tst_parse_int(str_max_io_submit, &max_io_submit, 0, INT_MAX))
		tst_brk(TBROK, "Invalid number of iocbs '%s'", str_max_io_submit);

	if (max_io_submit > maxaio)
		tst_res(TCONF, "Number of async IO blocks passed the maximum (%d)", maxaio);

	if (tst_parse_int(str_num_contexts, &num_contexts, 1, INT_MAX))
		tst_brk(TBROK, "Invalid number of contexts per file '%s'", str_num_contexts);

	if (tst_parse_filesize(str_context_offset, &context_offset, 1, LLONG_MAX))
		tst_brk(TBROK, "Invalid offset between contexts '%s'", str_context_offset);

	if (tst_parse_filesize(str_file_size, &file_size, 1, LLONG_MAX))
		tst_brk(TBROK, "Invalid file size '%s'", str_file_size);

	if (tst_parse_filesize(str_rec_len, &rec_len, 1, LONG_MAX))
		tst_brk(TBROK, "Invalid record size '%s'", str_rec_len);

	if (tst_parse_int(str_depth, &depth, 1, INT_MAX))
		tst_brk(TBROK, "Invalid number of pending aio requests '%s'", str_depth);

	if (tst_parse_int(str_io_iter, &io_iter, 1, INT_MAX))
		tst_brk(TBROK, "Invalid number of I/O per file '%s'", str_io_iter);

	if (tst_parse_int(str_iterations, &iterations, 1, INT_MAX))
		tst_brk(TBROK, "Invalid number of total ayncs I/O '%s'", str_iterations);

	if (tst_parse_int(str_stages, &stages_i, 0, INT_MAX))
		tst_brk(TBROK, "Invalid stage number '%s'", str_stages);

	if (stages_i) {
		stages |= 1 << stages_i;
		tst_res(TINFO, "Adding stage %s", stage_name(stages_i));
	}

	if (tst_parse_int(str_num_threads, &num_threads, 1, INT_MAX))
		tst_brk(TBROK, "Invalid number of threads '%s'", str_num_threads);

	if (str_o_flag)
		o_flag = O_DIRECT;
	else
		o_flag = O_SYNC;

	if (str_use_shm) {
		if (!strcmp(str_use_shm, "shm")) {
			tst_res(TINFO, "using ipc shm");
			use_shm = USE_SHM;
		} else if (!strcmp(str_use_shm, "shmfs")) {
			tst_res(TINFO, "using /dev/shm for buffers");
			use_shm = USE_SHMFS;
		} else {
			tst_brk(TBROK, "Invalid shm option '%s'", str_use_shm);
		}
	}
}

static void run(void)
{
	char files[num_files][265];
	int first_stage = WRITE;
	struct io_oper *oper;
	int status = 0;
	int open_fds = 0;
	struct thread_info *t;
	int rwfd;
	int i;
	int j;

	/*
	 * make sure we don't try to submit more I/O than we have allocated
	 * memory for
	 */
	if (depth < io_iter) {
		io_iter = depth;
		tst_res(TINFO, "dropping io_iter to %d", io_iter);
	}

	if (num_threads > (num_files * num_contexts)) {
		num_threads = num_files * num_contexts;
		tst_res(TINFO, "Dropping thread count to the number of contexts %d", num_threads);
	}

	t = SAFE_MALLOC(num_threads * sizeof(*t));
	memset(t, 0, num_threads * sizeof(*t));
	global_thread_info = t;

	/* by default, allow a huge number of iocbs to be sent towards
	 * io_submit
	 */
	if (!max_io_submit)
		max_io_submit = num_files * io_iter * num_contexts;

	/*
	 * make sure we don't try to submit more I/O than max_io_submit allows
	 */
	if (max_io_submit < io_iter) {
		io_iter = max_io_submit;
		tst_res(TINFO, "dropping io_iter to %d", io_iter);
	}

	if (!stages) {
		stages = (1 << WRITE) | (1 << READ) | (1 << RREAD) | (1 << RWRITE);
	} else {
		for (i = 0; i < LAST_STAGE; i++) {
			if (stages & (1 << i)) {
				first_stage = i;
				tst_res(TINFO, "starting with %s", stage_name(i));
				break;
			}
		}
	}

	if (file_size < num_contexts * context_offset) {
		tst_brk(TBROK, "file size %ld too small for %d contexts",
			(long)file_size, num_contexts);
	}

	tst_res(TINFO, "file size %ldMB, record size %lldKB, depth %d, I/O per iteration %d",
		(long)(file_size / (1024 * 1024)), rec_len / 1024, depth, io_iter);
	tst_res(TINFO, "max io_submit %d, buffer alignment set to %luKB",
		max_io_submit, (page_size_mask + 1) / 1024);
	tst_res(TINFO, "threads %d files %d contexts %d context offset %ldMB verification %s",
		num_threads, num_files, num_contexts,
		(long)(context_offset / (1024 * 1024)), verify ? "on" : "off");

	/* open all the files and do any required setup for them */
	for (i = 0; i < num_files; i++) {
		int thread_index;

		snprintf(files[i], sizeof(files[i]), "file%d.bin", i);

		for (j = 0; j < num_contexts; j++) {
			thread_index = open_fds % num_threads;
			open_fds++;

			rwfd = SAFE_OPEN(files[i], O_CREAT | O_RDWR | o_flag, 0600);

			oper = create_oper(rwfd, first_stage, j * context_offset,
					   file_size - j * context_offset,
					   rec_len, depth, files[i]);
			if (!oper)
				tst_brk(TBROK, "error in create_oper");

			oper_list_add(oper, &t[thread_index].active_opers);
			t[thread_index].num_files++;
		}
	}

	if (setup_shared_mem(num_threads, num_files * num_contexts, depth, rec_len))
		tst_brk(TBROK, "error in setup_shared_mem");

	for (i = 0; i < num_threads; i++)
		setup_ious(&t[i], t[i].num_files, depth, rec_len, max_io_submit);

	if (num_threads > 1) {
		tst_res(TINFO, "Running multi thread version num_threads: %d", num_threads);
		status = run_workers(t, num_threads);
	} else {
		tst_res(TINFO, "Running single thread version");
		status = (intptr_t)worker(t);
	}

	for (i = 0; i < num_files; i++)
		SAFE_UNLINK(files[i]);

	if (status)
		tst_res(TFAIL, "Test did not pass");
	else
		tst_res(TPASS, "Test passed");
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.needs_tmpdir = 1,
	.needs_root = 1,
	.max_runtime = 1800,
	.options = (struct tst_option[]){
		{ "a:", &str_iterations, "Total number of ayncs I/O the program will run (default 500)" },
		{ "b:", &str_max_io_submit, "Max number of iocbs to give io_submit at once" },
		{ "c:", &str_num_contexts, "Number of io contexts per file" },
		{ "d:", &str_depth, "Number of pending aio requests for each file (default 64)" },
		{ "e:", &str_io_iter, "Number of I/O per file sent before switching to the next file (default 8)" },
		{ "f:", &str_num_files, "Number of files to generate" },
		{ "g:", &str_context_offset, "Offset between contexts (default 2M)" },
		{ "l", &latency_stats, "Print io_submit latencies after each stage" },
		{ "L", &completion_latency_stats, "Print io completion latencies after each stage" },
		{ "m", &str_use_shm, "SHM use ipc shared memory for io buffers instead of malloc" },
		{ "n", &no_fsync_stages, "No fsyncs between write stage and read stage" },
		{ "o:", &str_stages, "Add an operation to the list: write=0, read=1, random write=2, random read=3" },
		{ "O", &str_o_flag, "Use O_DIRECT" },
		{ "r:", &str_rec_len, "Record size in KB used for each io (default 64K)" },
		{ "s:", &str_file_size, "Size in MB of the test file(s) (default 1024M)" },
		{ "t:", &str_num_threads, "Number of threads to run" },
		{ "u", &unlink_files, "Unlink files after completion" },
		{ "v", &verify, "Verification of bytes written" },
		{ "x", &no_stonewall, "Turn off thread stonewalling" },
		{},
	},
};
#else
TST_TEST_TCONF("test requires libaio and its development packages");
#endif
