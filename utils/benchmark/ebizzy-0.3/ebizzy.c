/*
 * Ebizzy - replicate a large ebusiness type of workload.
 *
 * Written by Valerie Henson <val@nmt.edu>
 *
 * Copyright 2006 - 2007 Intel Corporation
 * Copyright 2007 Valerie Henson <val@nmt.edu>
 *
 * Rodrigo Rubira Branco <rrbranco@br.ibm.com> - HP/BSD/Solaris port and some
 *  						 new features
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 * USA
 *
 */

/*
 * This program is designed to replicate a common web search app
 * workload.  A lot of search applications have the basic pattern: Get
 * a request to find a certain record, index into the chunk of memory
 * that contains it, copy it into another chunk, then look it up via
 * binary search.  The interesting parts of this workload are:
 *
 * Large working set
 * Data alloc/copy/free cycle
 * Unpredictable data access patterns
 *
 * Fiddle with the command line options until you get something
 * resembling the kind of workload you want to investigate.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "ebizzy.h"

/*
 * Command line options
 */

static unsigned int always_mmap;
static unsigned int never_mmap;
static unsigned int chunks;
static unsigned int use_permissions;
static unsigned int use_holes;
static unsigned int random_size;
static unsigned int chunk_size;
static unsigned int seconds;
static unsigned int threads;
static unsigned int verbose;
static unsigned int linear;
static unsigned int touch_pages;
static unsigned int no_lib_memcpy;

/*
 * Other global variables
 */

typedef size_t record_t;
static unsigned int record_size = sizeof(record_t);
static char *cmd;
static record_t **mem;
static char **hole_mem;
static unsigned int page_size;
static time_t start_time;
static volatile int threads_go;
static unsigned int records_read;

static void usage(void)
{
	fprintf(stderr, "Usage: %s [options]\n"
		"-T\t\t Just 'touch' the allocated pages\n"
		"-l\t\t Don't use library memcpy\n"
		"-m\t\t Always use mmap instead of malloc\n"
		"-M\t\t Never use mmap\n"
		"-n <num>\t Number of memory chunks to allocate\n"
		"-p \t\t Prevent mmap coalescing using permissions\n"
		"-P \t\t Prevent mmap coalescing using holes\n"
		"-R\t\t Randomize size of memory to copy and search\n"
		"-s <size>\t Size of memory chunks, in bytes\n"
		"-S <seconds>\t Number of seconds to run\n"
		"-t <num>\t Number of threads (2 * number cpus by default)\n"
		"-v[v[v]]\t Be verbose (more v's for more verbose)\n"
		"-z\t\t Linear search instead of binary search\n", cmd);
	exit(1);
}

/*
 * Read options, check them, and set some defaults.
 */

static void read_options(int argc, char *argv[])
{
	int c;

	page_size = getpagesize();

	/*
	 * Set some defaults.  These are currently tuned to run in a
	 * reasonable amount of time on my laptop.
	 *
	 * We could set the static defaults in the declarations, but
	 * then the defaults would be split between here and the top
	 * of the file, which is annoying.
	 */
	threads = 2 * sysconf(_SC_NPROCESSORS_ONLN);
	chunks = 10;
	chunk_size = record_size * 64 * 1024;
	seconds = 10;

	/* On to option processing */

	cmd = argv[0];
	opterr = 1;

	while ((c = getopt(argc, argv, "lmMn:pPRs:S:t:vzT")) != -1) {
		switch (c) {
		case 'l':
			no_lib_memcpy = 1;
			break;
		case 'm':
			always_mmap = 1;
			break;
		case 'M':
			never_mmap = 1;
			break;
		case 'n':
			chunks = atoi(optarg);
			if (chunks == 0)
				usage();
			break;
		case 'p':
			use_permissions = 1;
			break;
		case 'P':
			use_holes = 1;
			break;
		case 'R':
			random_size = 1;
			break;
		case 's':
			chunk_size = atoi(optarg);
			if (chunk_size == 0)
				usage();
			break;
		case 'S':
			seconds = atoi(optarg);
			if (seconds == 0)
				usage();
			break;
		case 't':
			threads = atoi(optarg);
			if (threads == 0)
				usage();
			break;
		case 'T':
			touch_pages = 1;
			break;
		case 'v':
			verbose++;
			break;
		case 'z':
			linear = 1;
			break;
		default:
			usage();
		}
	}

	if (verbose)
		printf("ebizzy 0.2\n"
		       "(C) 2006-7 Intel Corporation\n"
		       "(C) 2007 Valerie Henson <val@nmt.edu>\n");

	if (verbose) {
		printf("always_mmap %u\n", always_mmap);
		printf("never_mmap %u\n", never_mmap);
		printf("chunks %u\n", chunks);
		printf("prevent coalescing using permissions %u\n",
		       use_permissions);
		printf("prevent coalescing using holes %u\n", use_holes);
		printf("random_size %u\n", random_size);
		printf("chunk_size %u\n", chunk_size);
		printf("seconds %d\n", seconds);
		printf("threads %u\n", threads);
		printf("verbose %u\n", verbose);
		printf("linear %u\n", linear);
		printf("touch_pages %u\n", touch_pages);
		printf("page size %d\n", page_size);
	}

	/* Check for incompatible options */

	if (always_mmap && never_mmap) {
		fprintf(stderr, "Both -m \"always mmap\" and -M "
			"\"never mmap\" option specified\n");
		usage();
	}

	if (never_mmap)
		mallopt(M_MMAP_MAX, 0);

	if (chunk_size < record_size) {
		fprintf(stderr, "Chunk size %u smaller than record size %u\n",
			chunk_size, record_size);
		usage();
	}
}

static void touch_mem(char *dest, size_t size)
{
	int i;
	if (touch_pages) {
		for (i = 0; i < size; i += page_size)
			*(dest + i) = 0xff;
	}
}

static void *alloc_mem(size_t size)
{
	char *p;
	int err = 0;

	if (always_mmap) {
		p = mmap(NULL, size, (PROT_READ | PROT_WRITE),
			 (MAP_PRIVATE | MAP_ANONYMOUS), -1, 0);
		if (p == MAP_FAILED)
			err = 1;
	} else {
		p = malloc(size);
		if (p == NULL)
			err = 1;
	}

	if (err) {
		fprintf(stderr, "Couldn't allocate %zu bytes, try smaller "
			"chunks or size options\n"
			"Using -n %u chunks and -s %u size\n",
			size, chunks, chunk_size);
		exit(1);
	}

	return (p);
}

static void free_mem(void *p, size_t size)
{
	if (always_mmap)
		munmap(p, size);
	else
		free(p);
}

/*
 * Factor out differences in memcpy implementation by optionally using
 * our own simple memcpy implementation.
 */

static void my_memcpy(void *dest, void *src, size_t len)
{
	char *d = (char *)dest;
	char *s = (char *)src;
	int i;

	for (i = 0; i < len; i++)
		d[i] = s[i];
	return;
}

static void allocate(void)
{
	int i;

	mem = alloc_mem(chunks * sizeof(record_t *));

	if (use_holes)
		hole_mem = alloc_mem(chunks * sizeof(record_t *));

	for (i = 0; i < chunks; i++) {
		mem[i] = (record_t *) alloc_mem(chunk_size);
		/* Prevent coalescing using holes */
		if (use_holes)
			hole_mem[i] = alloc_mem(page_size);
	}

	/* Free hole memory */
	if (use_holes)
		for (i = 0; i < chunks; i++)
			free_mem(hole_mem[i], page_size);

	if (verbose)
		printf("Allocated memory\n");
}

static void write_pattern(void)
{
	int i, j;

	for (i = 0; i < chunks; i++) {
		for (j = 0; j < chunk_size / record_size; j++)
			mem[i][j] = (record_t) j;
		/* Prevent coalescing by alternating permissions */
		if (use_permissions && (i % 2) == 0)
			mprotect((void *)mem[i], chunk_size, PROT_READ);
	}
	if (verbose)
		printf("Wrote memory\n");
}

static void *linear_search(record_t key, record_t * base, size_t size)
{
	record_t *p;
	record_t *end = base + (size / record_size);

	for (p = base; p < end; p++)
		if (*p == key)
			return p;
	return NULL;
}

static int compare(const void *p1, const void *p2)
{
	return (*(record_t *) p1 - *(record_t *) p2);
}

/*
 * Stupid ranged random number function.  We don't care about quality.
 *
 * Inline because it's starting to be a scaling issue.
 */

static inline unsigned int rand_num(unsigned int max, unsigned int *state)
{
	*state *= 1103515245 + 12345;
	return ((*state / 65536) % max);
}

/*
 * This function is the meat of the program; the rest is just support.
 *
 * In this function, we randomly select a memory chunk, copy it into a
 * newly allocated buffer, randomly select a search key, look it up,
 * then free the memory.  An option tells us to allocate and copy a
 * randomly sized chunk of the memory instead of the whole thing.
 *
 * Linear search provided for sanity checking.
 *
 */

static unsigned int search_mem(void)
{
	record_t key, *found;
	record_t *src, *copy;
	unsigned int chunk;
	size_t copy_size = chunk_size;
	unsigned int i;
	unsigned int state = 0;

	for (i = 0; threads_go == 1; i++) {
		chunk = rand_num(chunks, &state);
		src = mem[chunk];
		/*
		 * If we're doing random sizes, we need a non-zero
		 * multiple of record size.
		 */
		if (random_size)
			copy_size = (rand_num(chunk_size / record_size, &state)
				     + 1) * record_size;
		copy = alloc_mem(copy_size);

		if (touch_pages) {
			touch_mem((char *)copy, copy_size);
		} else {

			if (no_lib_memcpy)
				my_memcpy(copy, src, copy_size);
			else
				memcpy(copy, src, copy_size);

			key = rand_num(copy_size / record_size, &state);

			if (verbose > 2)
				printf("Search key %zu, copy size %zu\n", key,
				       copy_size);
			if (linear)
				found = linear_search(key, copy, copy_size);
			else
				found =
				    bsearch(&key, copy, copy_size / record_size,
					    record_size, compare);

			/* Below check is mainly for memory corruption or other bug */
			if (found == NULL) {
				fprintf(stderr, "Couldn't find key %zd\n", key);
				exit(1);
			}
		}		/* end if ! touch_pages */

		free_mem(copy, copy_size);
	}

	return (i);
}

static void *thread_run(void *arg)
{

	if (verbose > 1)
		printf("Thread started\n");

	/* Wait for the start signal */

	while (threads_go == 0) ;

	records_read += search_mem();

	if (verbose > 1)
		printf("Thread finished, %f seconds\n",
		       difftime(time(NULL), start_time));

	return NULL;
}

static struct timeval difftimeval(struct timeval *end, struct timeval *start)
{
	struct timeval diff;
	diff.tv_sec = end->tv_sec - start->tv_sec;
	diff.tv_usec = end->tv_usec - start->tv_usec;
	return diff;
}

static void start_threads(void)
{
	pthread_t thread_array[threads];
	double elapsed;
	unsigned int i;
	struct rusage start_ru, end_ru;
	struct timeval usr_time, sys_time;
	int err;

	if (verbose)
		printf("Threads starting\n");

	for (i = 0; i < threads; i++) {
		err = pthread_create(&thread_array[i], NULL, thread_run, NULL);
		if (err) {
			fprintf(stderr, "Error creating thread %d\n", i);
			exit(1);
		}
	}

	/*
	 * Begin accounting - this is when we actually do the things
	 * we want to measure. */

	getrusage(RUSAGE_SELF, &start_ru);
	start_time = time(NULL);
	threads_go = 1;
	sleep(seconds);
	threads_go = 0;
	elapsed = difftime(time(NULL), start_time);
	getrusage(RUSAGE_SELF, &end_ru);

	/*
	 * The rest is just clean up.
	 */

	for (i = 0; i < threads; i++) {
		err = pthread_join(thread_array[i], NULL);
		if (err) {
			fprintf(stderr, "Error joining thread %d\n", i);
			exit(1);
		}
	}

	if (verbose)
		printf("Threads finished\n");

	printf("%u records/s\n",
	       (unsigned int)(((double)records_read) / elapsed));

	usr_time = difftimeval(&end_ru.ru_utime, &start_ru.ru_utime);
	sys_time = difftimeval(&end_ru.ru_stime, &start_ru.ru_stime);

	printf("real %5.2f s\n", elapsed);
	printf("user %5.2f s\n", usr_time.tv_sec + usr_time.tv_usec / 1e6);
	printf("sys  %5.2f s\n", sys_time.tv_sec + sys_time.tv_usec / 1e6);
}

int main(int argc, char *argv[])
{
	read_options(argc, argv);

	allocate();

	write_pattern();

	start_threads();

	return 0;
}
