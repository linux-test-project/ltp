/*
 *   Copyright (c) International Business Machines Corp., 2001-2006
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
#ifndef _UTIL_H_
#define _UTIL_H_

#include "config.h"

#include <sys/time.h>
#include <sys/resource.h>

#ifdef HAVE_SYS_VFS_H
#include <sys/vfs.h>
#endif
#include <sys/statvfs.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>


void ffsb_sleep(unsigned secs);
void *ffsb_malloc(size_t size);
void *ffsb_realloc(void *ptr, size_t size);
char *ffsb_strdup(const char *str);
size_t ffsb_strnlen(const char *str, size_t maxlen);

void ffsb_mkdir(char *dirname);
void ffsb_getrusage(struct rusage *ru_self, struct rusage *ru_children);
void ffsb_sync(void);
void *ffsb_align_4k(void *ptr);
char *ffsb_printsize(char *buf, double size, int bufsize);

int ffsb_system(char *command);
void ffsb_milli_sleep(unsigned time);
void ffsb_micro_sleep(unsigned time);
void ffsb_unbuffer_stdout(void);
void ffsb_bench_gettimeofday(void);
void ffsb_bench_getpid(void);

uint64_t ffsb_get_filesize(char *name);

typedef struct {
	unsigned required_count;
	unsigned current_count;
	pthread_mutex_t plock;
	pthread_cond_t pcond;
} ffsb_barrier_t ;

void ffsb_barrier_init(ffsb_barrier_t *fb, unsigned count);
void ffsb_barrier_wait(ffsb_barrier_t *fb);

double cpu_so_far(void);
double time_so_far(void);
double cpu_so_far_children(void);
float getfsutil(char *dirname);
uint64_t getfsutil_size(char *dirname);

struct timeval tvsub(struct timeval t1, struct timeval t0);
struct timeval tvadd(struct timeval t1, struct timeval t0);
double tvtodouble(struct timeval *t);


#define max(a, b) (((a) > (b)) ? (a) : (b))

#ifndef timersub
#define timersub(a, b, result)                                          \
	do {           	                                                \
		(result)->tv_sec = (a)->tv_sec - (b)->tv_sec;		\
		(result)->tv_usec = (a)->tv_usec - (b)->tv_usec;	\
									\
		if ((result)->tv_usec < 0) {				\
			(result)->tv_sec--;				\
			(result)->tv_usec += 1000000;			\
		}							\
	} while (0)
#endif /* timersub */

#endif /* _UTIL_H_ */
