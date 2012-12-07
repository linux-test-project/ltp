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
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/select.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <pthread.h>

#include "config.h"
#include "fh.h"
#include "util.h"

uint64_t ffsb_get_filesize(char *name)
{
#ifndef HAVE_STAT64
#define STAT(a, b) do { stat((a), (b)); } while (0)
	struct stat filestat;
#else
#define STAT(a, b) do { stat64((a), (b)); } while (0)
	struct stat64 filestat;
#endif

	STAT(name, &filestat);
	return (uint64_t) filestat.st_size;
#undef STAT
}

void *ffsb_malloc(size_t size)
{
	void *ptr = malloc((size));
	assert(ptr != NULL);
	memset(ptr, 0, size);
	return ptr;
}

void *ffsb_realloc(void *ptr, size_t size)
{
	void *tmp;
	/* printf("ffsb_realloc: ptr = %p  size = %ld\n",ptr,size); */

	if (ptr == NULL)
		return ffsb_malloc(size);

	tmp = realloc(ptr, size);
	assert(ptr != NULL);
	ptr = tmp;
	return ptr;
}

void *ffsb_align_4k(void *ptr)
{
	unsigned long mask = ~(0xfff);	/* 12 zeros at the end */
	void *ret = (void *)((unsigned long)ptr & mask);
	/* printf("align_4k got %p returning %p\n",ptr,ret); */
	return ret;
}

char *ffsb_strdup(const char *str)
{
	int len = strlen(str);
	char *dup = ffsb_malloc(len + 1);
	/* !!! am I off by one here ?? */
	strncpy(dup, str, len + 1);
	return dup;
}

size_t ffsb_strnlen(const char *str, size_t maxlen)
{
	size_t index = 0;

	while (index < maxlen) {
		if (str[index] == '\0')
			break;
		index++;
	}
	return index;
}

/* not perfect, in case we are somehow interrupted it's borked */
void ffsb_sleep(unsigned secs)
{
	struct timeval tv = { 0, 0 };
	tv.tv_sec = secs;
	select(0, NULL, NULL, NULL, &tv);
}

char *ffsb_printsize(char *buf, double size, int bufsize)
{
	if (size >= 1024 * 1024 * 1024)
		snprintf(buf, bufsize, "%.3gGB", size / (1024 * 1024 * 1024));
	else if (size >= 1024 * 1024)
		snprintf(buf, bufsize, "%.3gMB", size / (1024 * 1024));
	else if (size >= 1024)
		snprintf(buf, bufsize, "%.3gKB", size / 1024);
	else
		snprintf(buf, bufsize, "%.3gB", size);

	return buf;
}

void ffsb_mkdir(char *dirname)
{
	if (mkdir(dirname, S_IRWXU) < 0) {
		fprintf(stderr, "Error creating %s\n", dirname);
		perror("mkdir");
		exit(1);
	}
}

struct timeval tvsub(struct timeval t1, struct timeval t0)
{
	struct timeval tdiff;
	tdiff.tv_sec = t1.tv_sec - t0.tv_sec;
	tdiff.tv_usec = t1.tv_usec - t0.tv_usec;
	if (tdiff.tv_usec < 0)
		tdiff.tv_sec--, tdiff.tv_usec += 1000000;
	return tdiff;
}

struct timeval tvadd(struct timeval t1, struct timeval t0)
{
	struct timeval tdiff;
	tdiff.tv_sec = t1.tv_sec + t0.tv_sec;
	tdiff.tv_usec = t1.tv_usec + t0.tv_usec;
	if (tdiff.tv_usec > 1000000)
		tdiff.tv_sec++, tdiff.tv_usec -= 1000000;
	return tdiff;
}

double tvtodouble(struct timeval *t)
{
	return ((double)t->tv_sec * (1000000.0f) + (double)t->tv_usec) /
	    1000000.0f;
}

double cpu_so_far(void)
{
	struct rusage rusage;

	getrusage(RUSAGE_SELF, &rusage);

	return
	    ((double)rusage.ru_utime.tv_sec) +
	    (((double)rusage.ru_utime.tv_usec) / 1000000.0) +
	    ((double)rusage.ru_stime.tv_sec) +
	    (((double)rusage.ru_stime.tv_usec) / 1000000.0);
}

double cpu_so_far_children(void)
{
	struct rusage rusage;

	getrusage(RUSAGE_CHILDREN, &rusage);

	return
	    ((double)rusage.ru_utime.tv_sec) +
	    (((double)rusage.ru_utime.tv_usec) / 1000000.0) +
	    ((double)rusage.ru_stime.tv_sec) +
	    (((double)rusage.ru_stime.tv_usec) / 1000000.0);
}

/* !!!! check portability */
float getfsutil(char *dirname)
{
	struct statvfs64 fsdata;

	statvfs64(dirname, &fsdata);

/* 	return (float)(fsdata.f_blocks-fsdata.f_bfree)/ */
/* 		(float)(fsdata.f_blocks-fsdata.f_bfree+fsdata.f_bavail); */
	return (float)(((float)(fsdata.f_blocks - fsdata.f_bfree)) /
		       ((float)fsdata.f_blocks));
}

uint64_t getfsutil_size(char *dirname)
{
	struct statvfs64 fsdata;
	statvfs64(dirname, &fsdata);

	return (fsdata.f_blocks - fsdata.f_bfree) * fsdata.f_bsize;
}

int ffsb_system(char *command)
{
	int pid = 0, status;
	extern char **environ;

	if (command == NULL)
		return 1;
	pid = fork();
	if (pid == -1)
		return -1;
	if (pid == 0) {
		char *argv[4];
		argv[0] = "sh";
		argv[1] = "-c";
		argv[2] = command;
		argv[3] = 0;
		execve("/bin/sh", argv, environ);
		exit(127);
	}
	do {
		if (waitpid(pid, &status, 0) == -1) {
			if (errno != EINTR)
				return -1;
		} else
			return status;
	} while (1);
}

void ffsb_sync()
{
	struct timeval starttime, endtime, difftime;
	printf("Syncing()...");
	fflush(stdout);
	gettimeofday(&starttime, NULL);
	sync();
	gettimeofday(&endtime, NULL);
	timersub(&endtime, &starttime, &difftime);
	printf("%ld sec\n", difftime.tv_sec);
}

void ffsb_getrusage(struct rusage *ru_self, struct rusage *ru_children)
{
	int ret = 0;
/* 	printf("cpu_so_far is %lf\n",cpu_so_far()); */
/* 	printf("cpu_so_far_children is %lf\n",cpu_so_far_children()); */
	ret = getrusage(RUSAGE_SELF, ru_self);
	if (ret < 0)
		perror("getrusage self");

/* 	printf("self returned %d\n",ret); */
	ret = getrusage(RUSAGE_CHILDREN, ru_children);
	if (ret < 0)
		perror("getrusage children");
/* 	printf("children returned %d\n",ret); */
}

void ffsb_milli_sleep(unsigned time)
{
	struct timeval tv = { 0, 0 };
	if (!time)
		return;
	tv.tv_usec = time * 1000;
	select(0, NULL, NULL, NULL, &tv);
}

void ffsb_micro_sleep(unsigned time)
{
	struct timeval tv = { 0, 0 };
	if (!time)
		return;
	tv.tv_usec = time;
	select(0, NULL, NULL, NULL, &tv);
}

void ffsb_barrier_init(ffsb_barrier_t * fb, unsigned count)
{
	memset(fb, 0, sizeof(*fb));
	pthread_mutex_init(&fb->plock, NULL);
	pthread_cond_init(&fb->pcond, NULL);
	fb->required_count = count;
}

void ffsb_barrier_wait(ffsb_barrier_t * fb)
{
	pthread_mutex_lock(&fb->plock);

	fb->current_count++;

	if (fb->current_count == fb->required_count)
		pthread_cond_broadcast(&fb->pcond);
	else
		while (fb->current_count != fb->required_count)
			pthread_cond_wait(&fb->pcond, &fb->plock);

	pthread_mutex_unlock(&fb->plock);
}

void ffsb_unbuffer_stdout(void)
{
#ifndef SETVBUF_REVERSED
	setvbuf(stdout, NULL, _IONBF, 0);
#else
	setvbuf(stdout, _IONBF, NULL, 0);
#endif
}

void ffsb_bench_gettimeofday(void)
{
	unsigned long i = 0;
	uint64_t total_usec;
	uint64_t average = 0;
	struct timeval starttime, endtime, junk, difftime;
	gettimeofday(&starttime, NULL);
	for (i = 0; i < 1000000; i++)
		gettimeofday(&junk, NULL);
	gettimeofday(&endtime, NULL);
	timersub(&endtime, &starttime, &difftime);
	total_usec = difftime.tv_sec * 1000000;
	total_usec += difftime.tv_usec;
	average = total_usec / 1000ull;
	printf("average time for gettimeofday(): %llu nsec\n", average);
}

void ffsb_bench_getpid(void)
{
	unsigned long i = 0;
	uint64_t total_usec;
	uint64_t average = 0;
	struct timeval starttime, endtime, difftime;
	gettimeofday(&starttime, NULL);
	for (i = 0; i < 1000000; i++)
		getpid();
	gettimeofday(&endtime, NULL);
	timersub(&endtime, &starttime, &difftime);
	total_usec = difftime.tv_sec * 1000000;
	total_usec += difftime.tv_usec;
	average = total_usec / 1000ull;
	printf("average time for getpid(): %llu nsec\n", average);
}
