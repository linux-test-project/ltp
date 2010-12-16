/******************************************************************************/
/* Copyright (c) Tim LaBerge <tim.laberge@quantum.com>, 2009                  */
/*                                                                            */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                  */
/* the GNU General Public License for more details.                           */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program;  if not, write to the Free Software               */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* File:        dma_thread_diotest7.c                                         */
/*                                                                            */
/* Description: The man page for open(2) states the following:                */
/*   O_DIRECT (Since Linux 2.6.10). Try to minimize cache effects of the I/O  */
/*   to and from this file. In general this will degrade performance, but it  */
/*   is useful in special situations, such as when applications do their own  */
/*   caching. File I/O is done directly to/from user space buffers. The I/O is*/
/*   synchronous, that is, at the completion of a read(2) or write(2), data is*/
/*   guranteed to have been transferred. Under Linux 2.4 transfer sizes, and  */
/*   the alignment of user buffer and file  offset  must all be multiples of  */
/*   the logical block size of the file system.  Under Linux 2.6 alignment to */
/*   512-byte bound-aries suffices.                                           */
/*   However, it appears that data corruption may occur when a multithreaded  */
/*   process reads into a non-page size aligned user buffer. A test program   */
/*   which reliably reproduces the problem on ext3 and xfs is attached. The   */
/*   program creates, patterns, reads, and verify a series of files. In the   */
/*   read phase, a file is opened with O_DIRECT n times, where n is the       */
/*   number of cpu's. A single buffer large enough to contain the file is     */
/*   allocated and patterned with data not found in any of the files. The     */
/*   alignment of the buffer is controlled by a command line option. Each file*/
/*   is read in parallel by n threads, where n is the number of cpu's. Thread */
/*   0 reads the first page of data from the file into the first page of the  */
/*   buffer, thread 1 reads the second page of data in to the second page of  */
/*   the buffer, and so on.  Thread n - 1 reads the remainder of the file into*/
/*   the remainder of the buffer.                                             */
/*   After a thread reads data into the buffer, it immediately verifies that  */
/*   the contents of the buffer are correct. If the buffer contains corrupt   */
/*   data, the thread dumps the data surrounding the corruption and calls     */
/*   abort(). Otherwise, the thread exits.                                    */
/*   Crucially, before the reader threads are dispatched, another thread is   */
/*   started which calls fork()/msleep() in a loop until all reads are compl- */
/*   eted. The child created by fork() does nothing but call exit(0). A comm- */
/*   and line option controls whether the buffer is aligned.  In the case wh- */
/*   ere the buffer is aligned on a page boundary, all is well. In the case   */
/*   where the buffer is aligned on a page + 512 byte offset, corruption is   */
/*   seen frequently.                                                         */
/*   I believe that what is happening is that in the direct IO path, because  */
/*   the user's buffer is not aligned, some user pages are being mapped twice.*/
/*   When a fork() happens in between the calls to map the page, the page will*/
/*   be marked as COW. When the second map happens (via get_user_pages()), a  */
/*   new physical page will be allocated and copied. Thus, there is a race    */
/*   between the completion of the first read from disk (and write to the user*/
/*   page) and get_user_pages() mapping the page for the second time. If the  */
/*   write does not complete before the page is copied, the user will see     */
/*   stale data in the first 512 bytes of this page of their buffer. Indeed,  */
/*   this is corruption most frequently seen. (It's also possible for the race*/
/*   to be lost the other way, so that the last 3584 bytes of the page are    */
/*   stale.)                                                                  */
/*   The attached program (which is a heavily modified version of a program   */
/*   provided by a customer seeing this problem) reliably reproduces the pro- */
/*   blem on any multicore linux machine on both ext3 and xfs, although any   */
/*   filesystem using the generic blockdev_direct_IO() routine is probably    */
/*   vulnerable. I've seen a few threads that mention the potential for this  */
/*   kind of problem, but no definitive solution or workaround (other than    */
/*   "Don't do that").                                                        */
/*   http://marc.info/?l=linux-mm&m=122668235304637&w=2                       */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   dma_thread_diotest7                                           */
/*                                                                            */
/* Author:      Tim LaBerge <tim.laberge@quantum.com>                         */
/*                                                                            */
/* History:     Reported - Jan 07 2009 - Li Zefan <lizf@cn.fujitsu.com>       */
/*              Ported   - Jan 23 2009 - Subrata <subrata@linux.vnet.ibm.com> */
/*                                                                            */
/******************************************************************************/

#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <memory.h>
#include <pthread.h>
#include <getopt.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#define FILESIZE (12*1024*1024)
#define READSIZE  (1024*1024)

#define FILENAME    "_dma_thread_test_%.04d.tmp"
#define FILECOUNT   100
#define MIN_WORKERS 2
#define MAX_WORKERS 256
#define PAGE_SIZE getpagesize()

#define true	1
#define false	0

typedef int bool;

bool	done	= false;
int	workers = 2;

#define PATTERN (0xfa)

static void
usage (void)
{
    fprintf(stderr, "\nUsage: dma_thread [-h | -a <alignment> [ -w <workers>]\n"
		    "\nWith no arguments, generate test files and exit.\n"
		    "-h Display this help and exit.\n"
		    "-a align read buffer to offset <alignment>.\n"
		    "-w number of worker threads, 2 (default) to 256,\n"
		    "   defaults to number of cores.\n\n"

		    "Run first with no arguments to generate files.\n"
		    "Then run with -a <alignment> = 512  or 0. \n");
}

typedef struct {
    pthread_t	    tid;
    int		    worker_number;
    int		    fd;
    int		    offset;
    int		    length;
    int		    pattern;
    unsigned char  *buffer;
} worker_t;

void *worker_thread(void * arg)
{
    int		    bytes_read;
    int		    i,k;
    worker_t	   *worker  = (worker_t *) arg;
    int		    offset  = worker->offset;
    int		    fd	    = worker->fd;
    unsigned char  *buffer  = worker->buffer;
    int		    pattern = worker->pattern;
    int		    length  = worker->length;

    if (lseek(fd, offset, SEEK_SET) < 0) {
	fprintf(stderr, "Failed to lseek to %d on fd %d: %s.\n",
			offset, fd, strerror(errno));
	exit(1);
    }

    bytes_read = read(fd, buffer, length);
    if (bytes_read != length) {
	fprintf(stderr, "read failed on fd %d: bytes_read %d, %s\n",
			fd, bytes_read, strerror(errno));
	exit(1);
    }

    /* Corruption check */
    for (i = 0; i < length; i++) {
	if (buffer[i] != pattern) {
	    printf("Bad data at 0x%.06x: %p, \n", i, buffer + i);
	    printf("Data dump starting at 0x%.06x:\n", i - 8);
	    printf("Expect 0x%x followed by 0x%x:\n",
		    pattern, PATTERN);

	    for (k = 0; k < 16; k++) {
		printf("%02x ", buffer[i - 8 + k]);
		if (k == 7) {
		    printf("\n");
		}
	    }

	    printf("\n");
	    abort();
	}
    }

  return 0;
}

void *fork_thread (void *arg)
{
    pid_t pid;

    while (!done) {
	pid = fork();
	if (pid == 0) {
	    exit(0);
	} else if (pid < 0) {
	    fprintf(stderr, "Failed to fork child.\n");
	    exit(1);
	}
	waitpid(pid, NULL, 0 );
	usleep(100);
    }

    return NULL;

}

int main(int argc, char *argv[])
{
    unsigned char  *buffer = NULL;
    char	    filename[1024];
    int		    fd;
    bool	    dowrite = true;
    pthread_t	    fork_tid;
    int		    c, n, j;
    worker_t	   *worker;
    int		    align = 0;
    int		    offset, rc;

    workers = sysconf(_SC_NPROCESSORS_ONLN);

    while ((c = getopt(argc, argv, "a:hw:")) != -1) {
	switch (c) {
	case 'a':
	    align = atoi(optarg);
	    if (align < 0 || align > PAGE_SIZE) {
		printf("Bad alignment %d.\n", align);
		exit(1);
	    }
	    dowrite = false;
	    break;

	case 'h':
	    usage();
	    exit(0);
	    break;

	case 'w':
	    workers = atoi(optarg);
	    if (workers < MIN_WORKERS || workers > MAX_WORKERS) {
		fprintf(stderr, "Worker count %d not between "
				"%d and %d, inclusive.\n",
				workers, MIN_WORKERS, MAX_WORKERS);
		usage();
		exit(1);
	    }
	    dowrite = false;
	    break;

	default:
	    usage();
	    exit(1);
	}
    }

    if (argc > 1 && (optind < argc)) {
	fprintf(stderr, "Bad command line.\n");
	usage();
	exit(1);
    }

    if (dowrite) {

	buffer = malloc(FILESIZE);
	if (buffer == NULL) {
	    fprintf(stderr, "Failed to malloc write buffer.\n");
	    exit(1);
	}

	for (n = 1; n <= FILECOUNT; n++) {
	    sprintf(filename, FILENAME, n);
	    fd = open(filename, O_RDWR|O_CREAT|O_TRUNC, 0666);
	    if (fd < 0) {
		printf("create failed(%s): %s.\n", filename, strerror(errno));
		exit(1);
	    }
	    memset(buffer, n, FILESIZE);
	    printf("Writing file %s.\n", filename);
	    if (write(fd, buffer, FILESIZE) != FILESIZE) {
		printf("write failed (%s)\n", filename);
	    }

	    close(fd);
	    fd = -1;
	}

	free(buffer);
	buffer = NULL;

	printf("done\n");
	exit(0);
    }

    printf("Using %d workers.\n", workers);

    worker = malloc(workers * sizeof(worker_t));
    if (worker == NULL) {
	fprintf(stderr, "Failed to malloc worker array.\n");
	exit(1);
    }

    for (j = 0; j < workers; j++) {
	worker[j].worker_number = j;
    }

    printf("Using alignment %d.\n", align);

    posix_memalign((void *)&buffer, PAGE_SIZE, READSIZE+ align);
    printf("Read buffer: %p.\n", buffer);
    for (n = 1; n <= FILECOUNT; n++) {

	sprintf(filename, FILENAME, n);
	for (j = 0; j < workers; j++) {
	    if ((worker[j].fd = open(filename,  O_RDONLY|O_DIRECT)) < 0) {
		fprintf(stderr, "Failed to open %s: %s.\n",
				filename, strerror(errno));
		exit(1);
	    }

	    worker[j].pattern = n;
	}

	printf("Reading file %d.\n", n);

	for (offset = 0; offset < FILESIZE; offset += READSIZE) {
	    memset(buffer, PATTERN, READSIZE + align);
	    for (j = 0; j < workers; j++) {
		worker[j].offset = offset + j * PAGE_SIZE;
		worker[j].buffer = buffer + align + j * PAGE_SIZE;
		worker[j].length = PAGE_SIZE;
	    }
	    /* The final worker reads whatever is left over. */
	    worker[workers - 1].length = READSIZE - PAGE_SIZE * (workers - 1);

	    done = 0;

	    rc = pthread_create(&fork_tid, NULL, fork_thread, NULL);
	    if (rc != 0) {
		fprintf(stderr, "Can't create fork thread: %s.\n",
				strerror(rc));
		exit(1);
	    }

	    for (j = 0; j < workers; j++) {
		rc = pthread_create(&worker[j].tid,
				    NULL,
				    worker_thread,
				    worker + j);
		if (rc != 0) {
		    fprintf(stderr, "Can't create worker thread %d: %s.\n",
				    j, strerror(rc));
		    exit(1);
		}
	    }

	    for (j = 0; j < workers; j++) {
		rc = pthread_join(worker[j].tid, NULL);
		if (rc != 0) {
		    fprintf(stderr, "Failed to join worker thread %d: %s.\n",
				    j, strerror(rc));
		    exit(1);
		}
	    }

	    /* Let the fork thread know it's ok to exit */
	    done = 1;

	    rc = pthread_join(fork_tid, NULL);
	    if (rc != 0) {
		fprintf(stderr, "Failed to join fork thread: %s.\n",
				strerror(rc));
		exit(1);
	    }
	}

	/* Close the fd's for the next file. */
	for (j = 0; j < workers; j++) {
	    close(worker[j].fd);
	}
    }

  return 0;
}