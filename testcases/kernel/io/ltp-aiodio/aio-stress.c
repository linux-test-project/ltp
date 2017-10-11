/*
 * Copyright (c) 2004 SuSE, Inc.  All Rights Reserved.
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
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 *
 * aio-stress
 *
 * will open or create each file on the command line, and start a series
 * of aio to it.
 *
 * aio is done in a rotating loop.  first file1 gets 8 requests, then
 * file2, then file3 etc.  As each file finishes writing, it is switched
 * to reads
 *
 * io buffers are aligned in case you want to do raw io
 *
 * compile with gcc -Wall -laio -lpthread -o aio-stress aio-stress.c
 *
 * run aio-stress -h to see the options
 *
 * Please mail Chris Mason (mason@suse.com) with bug reports or patches
 */
#define _FILE_OFFSET_BITS 64
#define PROG_VERSION "0.21"
#define NEW_GETEVENTS

#define _GNU_SOURCE
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

#include "config.h"
#include "tst_res_flags.h"

#ifdef HAVE_LIBAIO
#include <libaio.h>

#define IO_FREE 0
#define IO_PENDING 1
#define RUN_FOREVER -1

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

/*
 * various globals, these are effectively read only by the time the threads
 * are started
 */
long stages = 0;
unsigned long page_size_mask;
int o_direct = 0;
int o_sync = 0;
int latency_stats = 0;
int completion_latency_stats = 0;
int io_iter = 8;
int iterations = RUN_FOREVER;
int max_io_submit = 0;
long rec_len = 64 * 1024;
int depth = 64;
int num_threads = 1;
int num_contexts = 1;
off_t context_offset = 2 * 1024 * 1024;
int fsync_stages = 1;
int use_shm = 0;
int shm_id;
char *unaligned_buffer = NULL;
char *aligned_buffer = NULL;
int padded_reclen = 0;
int stonewall = 1;
int verify = 0;
char *verify_buf = NULL;
int unlink_files = 0;

struct io_unit;
struct thread_info;

/* pthread mutexes and other globals for keeping the threads in sync */
pthread_cond_t stage_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t stage_mutex = PTHREAD_MUTEX_INITIALIZER;
int threads_ending = 0;
int threads_starting = 0;
struct timeval global_stage_start_time;
struct thread_info *global_thread_info;

/*
 * latencies during io_submit are measured, these are the
 * granularities for deviations
 */
#define DEVIATIONS 6
int deviations[DEVIATIONS] = { 100, 250, 500, 1000, 5000, 10000 };

struct io_latency {
	double max;
	double min;
	double total_io;
	double total_lat;
	double deviations[DEVIATIONS];
};

/* container for a series of operations to a file */
struct io_oper {
	/* already open file descriptor, valid for whatever operation you want */
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

	struct timeval io_start_time;	/* time of io_submit */
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
	return;
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
		fstat(io->io_oper->fd, &s);

		/*
		 * If file size is large enough for the read, then this short
		 * read is an error.
		 */
		if ((io->io_oper->rw == READ || io->io_oper->rw == RREAD) &&
		    s.st_size > (io->iocb.u.c.offset + io->res)) {

			fprintf(stderr,
				"io err %lu (%s) op %d, off %Lu size %d\n",
				io->res, strerror(-io->res),
				io->iocb.aio_lio_opcode, io->iocb.u.c.offset,
				io->buf_size);
			io->io_oper->last_err = io->res;
			io->io_oper->num_err++;
			return -1;
		}
	}
	if (verify && io->io_oper->rw == READ) {
		if (memcmp(io->buf, verify_buf, io->io_oper->reclen)) {
			fprintf(stderr,
				"verify error, file %s offset %Lu contents (offset:bad:good):\n",
				io->io_oper->file_name, io->iocb.u.c.offset);

			for (i = 0; i < io->io_oper->reclen; i++) {
				if (io->buf[i] != verify_buf[i]) {
					fprintf(stderr, "%d:%c:%c ", i,
						io->buf[i], verify_buf[i]);
				}
			}
			fprintf(stderr, "\n");
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

char *stage_name(int rw)
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
	return ((double)oper->started_ios * (double)oper->reclen) /
	    (double)(1024 * 1024);
}

static void print_time(struct io_oper *oper)
{
	double runtime;
	double tput;
	double mb;

	runtime = time_since_now(&oper->start_time);
	mb = oper_mb_trans(oper);
	tput = mb / runtime;
	fprintf(stderr, "%s on %s (%.2f MB/s) %.2f MB in %.2fs\n",
		stage_name(oper->rw), oper->file_name, tput, mb, runtime);
}

static void print_lat(char *str, struct io_latency *lat)
{
	double avg = lat->total_lat / lat->total_io;
	int i;
	double total_counted = 0;
	fprintf(stderr, "%s min %.2f avg %.2f max %.2f\n\t",
		str, lat->min, avg, lat->max);

	for (i = 0; i < DEVIATIONS; i++) {
		fprintf(stderr, " %.0f < %d", lat->deviations[i],
			deviations[i]);
		total_counted += lat->deviations[i];
	}
	if (total_counted && lat->total_io - total_counted)
		fprintf(stderr, " < %.0f", lat->total_io - total_counted);
	fprintf(stderr, "\n");
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
void finish_io(struct thread_info *t, struct io_unit *io, long result,
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

int read_some_events(struct thread_info *t)
{
	struct io_unit *event_io;
	struct io_event *event;
	int nr;
	int i;
	int min_nr = io_iter;
	struct timeval stop_time;

	if (t->num_global_pending < io_iter)
		min_nr = t->num_global_pending;

#ifdef NEW_GETEVENTS
	nr = io_getevents(t->io_ctx, min_nr, t->num_global_events, t->events,
			  NULL);
#else
	nr = io_getevents(t->io_ctx, t->num_global_events, t->events, NULL);
#endif
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
		if (grab_iou(event_io, oper)) {
			fprintf(stderr, "io unit on free list but not free\n");
			abort();
		}
		return event_io;
	}
	nr = read_some_events(t);
	if (nr > 0)
		goto retry;
	else
		fprintf(stderr, "no free ious after read_some_events\n");
	return NULL;
}

/*
 * wait for all pending requests for this io operation to finish
 */
static int io_oper_wait(struct thread_info *t, struct io_oper *oper)
{
	struct io_event event;
	struct io_unit *event_io;

	if (oper == NULL) {
		return 0;
	}

	if (oper->num_pending == 0)
		goto done;

	/* this func is not speed sensitive, no need to go wild reading
	 * more than one event at a time
	 */
#ifdef NEW_GETEVENTS
	while (io_getevents(t->io_ctx, 1, 1, &event, NULL) > 0) {
#else
	while (io_getevents(t->io_ctx, 1, &event, NULL) > 0) {
#endif
		struct timeval tv_now;
		event_io = (struct io_unit *)((unsigned long)event.obj);

		gettimeofday(&tv_now, NULL);
		finish_io(t, event_io, event.res, &tv_now);

		if (oper->num_pending == 0)
			break;
	}
done:
	if (oper->num_err) {
		fprintf(stderr, "%u errors on oper, last %u\n",
			oper->num_err, oper->last_err);
	}
	return 0;
}

off_t random_byte_offset(struct io_oper * oper)
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

	if (rand_byte + oper->reclen > oper->end) {
		rand_byte -= oper->reclen;
	}
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
	if (!io) {
		fprintf(stderr, "unable to find io unit\n");
		return NULL;
	}

	switch (oper->rw) {
	case WRITE:
		io_prep_pwrite(&io->iocb, oper->fd, io->buf, oper->reclen,
			       oper->last_offset);
		oper->last_offset += oper->reclen;
		break;
	case READ:
		io_prep_pread(&io->iocb, oper->fd, io->buf, oper->reclen,
			      oper->last_offset);
		oper->last_offset += oper->reclen;
		break;
	case RREAD:
		rand_byte = random_byte_offset(oper);
		oper->last_offset = rand_byte;
		io_prep_pread(&io->iocb, oper->fd, io->buf, oper->reclen,
			      rand_byte);
		break;
	case RWRITE:
		rand_byte = random_byte_offset(oper);
		oper->last_offset = rand_byte;
		io_prep_pwrite(&io->iocb, oper->fd, io->buf, oper->reclen,
			       rand_byte);

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
	if (oper->num_pending > 0) {
		fprintf(stderr, "oper num_pending is %d\n", oper->num_pending);
	}
	close(oper->fd);
	free(oper);
	return last_err;
}

/*
 * allocates an io operation and fills in all the fields.  returns
 * null on error
 */
static struct io_oper *create_oper(int fd, int rw, off_t start, off_t end,
				   int reclen, int depth, int iter,
				   char *file_name)
{
	struct io_oper *oper;

	oper = malloc(sizeof(*oper));
	if (!oper) {
		fprintf(stderr, "unable to allocate io oper\n");
		return NULL;
	}
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
int build_oper(struct thread_info *t, struct io_oper *oper, int num_ios,
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
		if (!io) {
			return -1;
		}
		my_iocbs[i] = &io->iocb;
	}
	return num_ios;
}

/*
 * runs through the iocbs in the array provided and updates
 * counters in the associated oper struct
 */
static void update_iou_counters(struct iocb **my_iocbs, int nr,
				struct timeval *tv_now)
{
	struct io_unit *io;
	int i;
	for (i = 0; i < nr; i++) {
		io = (struct io_unit *)(my_iocbs[i]);
		io->io_oper->num_pending++;
		io->io_oper->started_ios++;
		io->io_start_time = *tv_now;	/* set time of io_submit */
	}
}

/* starts some io for a given file, returns zero if all went well */
int run_built(struct thread_info *t, int num_ios, struct iocb **my_iocbs)
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
			if ((ret = read_some_events(t) > 0)) {
				goto resubmit;
			} else {
				fprintf(stderr, "ret was %d and now is %d\n",
					ret, old_ret);
				abort();
			}
		}

		fprintf(stderr, "ret %d (%s) on io_submit\n", ret,
			strerror(-ret));
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

	/* this switch falls through */
	switch (oper->rw) {
	case WRITE:
		if (stages & (1 << READ))
			new_rw = READ;
	case READ:
		if (!new_rw && stages & (1 << RWRITE))
			new_rw = RWRITE;
	case RWRITE:
		if (!new_rw && stages & (1 << RREAD))
			new_rw = RREAD;
	}

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
	int ret;

	/* first context is always runnable, if started_ios > 0, no need to
	 * redo the calculations
	 */
	if (oper->started_ios || oper->start == 0)
		return 1;
	/*
	 * only the sequential phases force delays in starting */
	if (oper->rw >= RWRITE)
		return 1;
	ret = fstat(oper->fd, &buf);
	if (ret < 0) {
		perror("fstat");
		exit(1);
	}
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
static int run_active_list(struct thread_info *t,
			   int io_iter, int max_io_submit)
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
		if (ret < 0) {
			fprintf(stderr, "error %d on run_built\n", ret);
			exit(1);
		}
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

void drop_shm()
{
	int ret;
	struct shmid_ds ds;
	if (use_shm != USE_SHM)
		return;

	ret = shmctl(shm_id, IPC_RMID, &ds);
	if (ret) {
		perror("shmctl IPC_RMID");
	}
}

void aio_setup(io_context_t * io_ctx, int n)
{
	int res = io_queue_init(n, io_ctx);
	if (res != 0) {
		fprintf(stderr, "io_queue_setup(%d) returned %d (%s)\n",
			n, res, strerror(-res));
		exit(3);
	}
}

/*
 * allocate io operation and event arrays for a given thread
 */
int setup_ious(struct thread_info *t,
	       int num_files, int depth, int reclen, int max_io_submit)
{
	int i;
	size_t bytes = num_files * depth * sizeof(*t->ios);

	t->ios = malloc(bytes);
	if (!t->ios) {
		fprintf(stderr, "unable to allocate io units\n");
		return -1;
	}
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

	t->iocbs = malloc(sizeof(struct iocb *) * max_io_submit);
	if (!t->iocbs) {
		fprintf(stderr, "unable to allocate iocbs\n");
		goto free_buffers;
	}

	memset(t->iocbs, 0, max_io_submit * sizeof(struct iocb *));

	t->events = malloc(sizeof(struct io_event) * depth * num_files);
	if (!t->events) {
		fprintf(stderr, "unable to allocate ram for events\n");
		goto free_buffers;
	}
	memset(t->events, 0, num_files * sizeof(struct io_event) * depth);

	t->num_global_ios = num_files * depth;
	t->num_global_events = t->num_global_ios;
	return 0;

free_buffers:
	free(t->ios);
	free(t->iocbs);
	free(t->events);
	return -1;
}

/*
 * The buffers used for file data are allocated as a single big
 * malloc, and then each thread and operation takes a piece and uses
 * that for file data.  This lets us do a large shm or bigpages alloc
 * and without trying to find a special place in each thread to map the
 * buffers to
 */
int setup_shared_mem(int num_threads, int num_files, int depth,
		     int reclen, int max_io_submit)
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
		p = malloc(total_ram);
	} else if (use_shm == USE_SHM) {
		shm_id = shmget(IPC_PRIVATE, total_ram, IPC_CREAT | 0700);
		if (shm_id < 0) {
			perror("shmget");
			drop_shm();
			goto free_buffers;
		}
		p = shmat(shm_id, (char *)0x50000000, 0);
		if ((long)p == -1) {
			perror("shmat");
			goto free_buffers;
		}
		/* won't really be dropped until we shmdt */
		drop_shm();
	} else if (use_shm == USE_SHMFS) {
		char mmap_name[16];	/* /dev/shm/ + null + XXXXXX */
		int fd;

		strcpy(mmap_name, "/dev/shm/XXXXXX");
		fd = mkstemp(mmap_name);
		if (fd < 0) {
			perror("mkstemp");
			goto free_buffers;
		}
		unlink(mmap_name);
		ftruncate(fd, total_ram);
		shm_id = fd;
		p = mmap((char *)0x50000000, total_ram,
			 PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

		if (p == MAP_FAILED) {
			perror("mmap");
			goto free_buffers;
		}
	}
	if (!p) {
		fprintf(stderr, "unable to allocate buffers\n");
		goto free_buffers;
	}
	unaligned_buffer = p;
	p = (char *)((intptr_t) (p + page_size_mask) & ~page_size_mask);
	aligned_buffer = p;
	return 0;

free_buffers:
	drop_shm();
	if (unaligned_buffer)
		free(unaligned_buffer);
	return -1;
}

/*
 * runs through all the thread_info structs and calculates a combined
 * throughput
 */
void global_thread_throughput(struct thread_info *t, char *this_stage)
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
		fprintf(stderr, "%s throughput (%.2f MB/s) ", this_stage,
			total_mb / runtime);
		fprintf(stderr, "%.2f MB in %.2fs", total_mb, runtime);
		if (stonewall)
			fprintf(stderr, " min transfer %.2fMB", min_trans);
		fprintf(stderr, "\n");
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
int worker(struct thread_info *t)
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
	while (t->active_opers
	       && (cnt < iterations || iterations == RUN_FOREVER)) {
		if (stonewall && threads_ending) {
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
		if (fsync_stages)
			fsync(oper->fd);
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
		fprintf(stderr,
			"thread %td %s totals (%.2f MB/s) %.2f MB in %.2fs\n",
			t - global_thread_info, this_stage,
			t->stage_mb_trans / seconds, t->stage_mb_trans,
			seconds);
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
	if (t->active_opers && (cnt < iterations || iterations == RUN_FOREVER)) {
		iteration++;
		goto restart;
	}

	/* finally, free all the ram */
	while (t->finished_opers) {
		oper = t->finished_opers;
		oper_list_del(oper, &t->finished_opers);
		status = finish_oper(t, oper);
	}

	if (t->num_global_pending) {
		fprintf(stderr, "global num pending is %d\n",
			t->num_global_pending);
	}
	io_queue_release(t->io_ctx);

	return status;
}

typedef void *(*start_routine) (void *);
int run_workers(struct thread_info *t, int num_threads)
{
	int ret;
	int i;

	for (i = 0; i < num_threads; i++) {
		ret =
		    pthread_create(&t[i].tid, NULL, (start_routine) worker,
				   t + i);
		if (ret) {
			perror("pthread_create");
			exit(1);
		}
	}
	for (i = 0; i < num_threads; i++) {
		ret = pthread_join(t[i].tid, NULL);
		if (ret) {
			perror("pthread_join");
			exit(1);
		}
	}
	return 0;
}

off_t parse_size(char *size_arg, off_t mult)
{
	char c;
	int num;
	off_t ret;
	c = size_arg[strlen(size_arg) - 1];
	if (c > '9') {
		size_arg[strlen(size_arg) - 1] = '\0';
	}
	num = atoi(size_arg);
	switch (c) {
	case 'g':
	case 'G':
		mult = 1024 * 1024 * 1024;
		break;
	case 'm':
	case 'M':
		mult = 1024 * 1024;
		break;
	case 'k':
	case 'K':
		mult = 1024;
		break;
	case 'b':
	case 'B':
		mult = 1;
		break;
	}
	ret = mult * num;
	return ret;
}

void print_usage(void)
{
	printf
	    ("usage: aio-stress [-s size] [-r size] [-a size] [-d num] [-b num]\n");
	printf
	    ("                  [-i num] [-t num] [-c num] [-C size] [-nxhOS ]\n");
	printf("                  file1 [file2 ...]\n");
	printf("\t-a size in KB at which to align buffers\n");
	printf("\t-b max number of iocbs to give io_submit at once\n");
	printf("\t-c number of io contexts per file\n");
	printf("\t-C offset between contexts, default 2MB\n");
	printf("\t-s size in MB of the test file(s), default 1024MB\n");
	printf("\t-r record size in KB used for each io, default 64KB\n");
	printf
	    ("\t-d number of pending aio requests for each file, default 64\n");
	printf("\t-i number of I/O per file sent before switching\n"
	       "\t   to the next file, default 8\n");
	printf("\t-I total number of ayncs I/O the program will run, "
	       "default is run until Cntl-C\n");
	printf("\t-O Use O_DIRECT (not available in 2.4 kernels),\n");
	printf("\t-S Use O_SYNC for writes\n");
	printf("\t-o add an operation to the list: write=0, read=1,\n");
	printf("\t   random write=2, random read=3.\n");
	printf("\t   repeat -o to specify multiple ops: -o 0 -o 1 etc.\n");
	printf
	    ("\t-m shm use ipc shared memory for io buffers instead of malloc\n");
	printf("\t-m shmfs mmap a file in /dev/shm for io buffers\n");
	printf("\t-n no fsyncs between write stage and read stage\n");
	printf("\t-l print io_submit latencies after each stage\n");
	printf("\t-L print io completion latencies after each stage\n");
	printf("\t-t number of threads to run\n");
	printf("\t-u unlink files after completion\n");
	printf("\t-v verification of bytes written\n");
	printf("\t-x turn off thread stonewalling\n");
	printf("\t-h this message\n");
	printf
	    ("\n\t   the size options (-a -s and -r) allow modifiers -s 400{k,m,g}\n");
	printf("\t   translate to 400KB, 400MB and 400GB\n");
	printf("version %s\n", PROG_VERSION);
}

int main(int ac, char **av)
{
	int rwfd;
	int i;
	int j;
	int c;

	off_t file_size = 1 * 1024 * 1024 * 1024;
	int first_stage = WRITE;
	struct io_oper *oper;
	int status = 0;
	int num_files = 0;
	int open_fds = 0;
	struct thread_info *t;

	page_size_mask = getpagesize() - 1;

	while (1) {
		c = getopt(ac, av, "a:b:c:C:m:s:r:d:i:I:o:t:lLnhOSxvu");
		if (c < 0)
			break;

		switch (c) {
		case 'a':
			page_size_mask = parse_size(optarg, 1024);
			page_size_mask--;
			break;
		case 'c':
			num_contexts = atoi(optarg);
			break;
		case 'C':
			context_offset = parse_size(optarg, 1024 * 1024);
		case 'b':
			max_io_submit = atoi(optarg);
			break;
		case 's':
			file_size = parse_size(optarg, 1024 * 1024);
			break;
		case 'd':
			depth = atoi(optarg);
			break;
		case 'r':
			rec_len = parse_size(optarg, 1024);
			break;
		case 'i':
			io_iter = atoi(optarg);
			break;
		case 'I':
			iterations = atoi(optarg);
			break;
		case 'n':
			fsync_stages = 0;
			break;
		case 'l':
			latency_stats = 1;
			break;
		case 'L':
			completion_latency_stats = 1;
			break;
		case 'm':
			if (!strcmp(optarg, "shm")) {
				fprintf(stderr, "using ipc shm\n");
				use_shm = USE_SHM;
			} else if (!strcmp(optarg, "shmfs")) {
				fprintf(stderr, "using /dev/shm for buffers\n");
				use_shm = USE_SHMFS;
			}
			break;
		case 'o':
			i = atoi(optarg);
			stages |= 1 << i;
			fprintf(stderr, "adding stage %s\n", stage_name(i));
			break;
		case 'O':
			o_direct = O_DIRECT;
			break;
		case 'S':
			o_sync = O_SYNC;
			break;
		case 't':
			num_threads = atoi(optarg);
			break;
		case 'x':
			stonewall = 0;
			break;
		case 'u':
			unlink_files = 1;
			break;
		case 'v':
			verify = 1;
			break;
		case 'h':
		default:
			print_usage();
			exit(1);
		}
	}

	/*
	 * make sure we don't try to submit more I/O than we have allocated
	 * memory for
	 */
	if (depth < io_iter) {
		io_iter = depth;
		fprintf(stderr, "dropping io_iter to %d\n", io_iter);
	}

	if (optind >= ac) {
		print_usage();
		exit(1);
	}

	num_files = ac - optind;

	if (num_threads > (num_files * num_contexts)) {
		num_threads = num_files * num_contexts;
		fprintf(stderr,
			"dropping thread count to the number of contexts %d\n",
			num_threads);
	}

	t = malloc(num_threads * sizeof(*t));
	if (!t) {
		perror("malloc");
		exit(1);
	}
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
		fprintf(stderr, "dropping io_iter to %d\n", io_iter);
	}

	if (!stages) {
		stages =
		    (1 << WRITE) | (1 << READ) | (1 << RREAD) | (1 << RWRITE);
	} else {
		for (i = 0; i < LAST_STAGE; i++) {
			if (stages & (1 << i)) {
				first_stage = i;
				fprintf(stderr, "starting with %s\n",
					stage_name(i));
				break;
			}
		}
	}

	if (file_size < num_contexts * context_offset) {
		fprintf(stderr, "file size %ld too small for %d contexts\n",
			(long)file_size, num_contexts);
		exit(1);
	}

	fprintf(stderr, "file size %ldMB, record size %ldKB, depth %d, "
		"I/O per iteration %d\n",
		(long)(file_size / (1024 * 1024)),
		rec_len / 1024, depth, io_iter);
	fprintf(stderr, "max io_submit %d, buffer alignment set to %luKB\n",
		max_io_submit, (page_size_mask + 1) / 1024);
	fprintf(stderr, "threads %d files %d contexts %d context offset %ldMB "
		"verification %s\n", num_threads, num_files, num_contexts,
		(long)(context_offset / (1024 * 1024)), verify ? "on" : "off");
	/* open all the files and do any required setup for them */
	for (i = optind; i < ac; i++) {
		int thread_index;
		for (j = 0; j < num_contexts; j++) {
			thread_index = open_fds % num_threads;
			open_fds++;

			rwfd =
			    open(av[i], O_CREAT | O_RDWR | o_direct | o_sync,
				 0600);
			if (rwfd == -1) {
				fprintf(stderr,
					"error while creating file %s: %s",
					av[i], strerror(errno));
				exit(1);
			}

			oper =
			    create_oper(rwfd, first_stage, j * context_offset,
					file_size - j * context_offset, rec_len,
					depth, io_iter, av[i]);
			if (!oper) {
				fprintf(stderr, "error in create_oper\n");
				exit(-1);
			}
			oper_list_add(oper, &t[thread_index].active_opers);
			t[thread_index].num_files++;
		}
	}
	if (setup_shared_mem(num_threads, num_files * num_contexts,
			     depth, rec_len, max_io_submit)) {
		exit(1);
	}
	for (i = 0; i < num_threads; i++) {
		if (setup_ious
		    (&t[i], t[i].num_files, depth, rec_len, max_io_submit))
			exit(1);
	}
	if (num_threads > 1) {
		printf("Running multi thread version num_threads:%d\n",
		       num_threads);
		run_workers(t, num_threads);
	} else {
		printf("Running single thread version \n");
		status = worker(t);
	}
	if (unlink_files) {
		for (i = optind; i < ac; i++) {
			printf("Cleaning up file %s \n", av[i]);
			unlink(av[i]);
		}
	}

	if (status) {
		exit(1);
	}
	return status;
}
#else
int main(void)
{
	fprintf(stderr, "test requires libaio and it's development packages\n");
	return TCONF;
}
#endif
