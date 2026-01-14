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
/* along with this program;  if not, write to the Free Software Foundation,   */
/* Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA           */
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
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <memory.h>
#include <pthread.h>
#include <getopt.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mount.h>

#include "test.h"
#include "tso_safe_macros.h"

#define FILESIZE	(12*1024*1024)
#define READSIZE	(1024*1024)

#define MNT_POINT	"mntpoint"
#define FILE_BASEPATH   MNT_POINT "/_dma_thread_test_%.04d.tmp"
#define DIR_MODE	(S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP| \
			 S_IXGRP|S_IROTH|S_IXOTH)
#define FILECOUNT	100
#define PATTERN		(0xfa)
#define PAGE_SIZE	getpagesize()
#define MIN_WORKERS	2
#define MAX_WORKERS	(READSIZE/PAGE_SIZE)

char *TCID = "dma_thread_diotest";
int TST_TOTAL = 1;

static void setup(void);
static void dma_thread_diotest_verify(void);
static void cleanup(void);
static void help(void);

static unsigned char *buffer;

static char *align_str;
static int align;
static char *workers_str;
static int workers;
static char *device;
static int mount_flag;
static option_t options[] = {
	{"a:", NULL, &align_str},
	{"w:", NULL, &workers_str},
	{NULL, NULL, NULL}
};

static volatile int done;
static volatile int tst_result;

typedef struct {
	pthread_t tid;
	int worker_number;
	int fd;
	int offset;
	int length;
	int pattern;
	unsigned char *buffer;
} worker_t;
static worker_t *worker;

static void *worker_thread(void *arg)
{
	int i, k;
	int nread;
	worker_t *worker = (worker_t *)arg;
	int offset = worker->offset;
	int fd = worker->fd;
	unsigned char *buffer = worker->buffer;
	int pattern = worker->pattern;
	int length = worker->length;

	if (lseek(fd, offset, SEEK_SET) < 0) {
		fprintf(stderr, "Failed to lseek to %d on fd %d: %s.\n",
			offset, fd, strerror(errno));
		return (void *) 1;
	}

	nread = read(fd, buffer, length);
	if (nread == -1 || nread != length) {
		fprintf(stderr, "read failed in worker thread%d: %s",
			worker->worker_number, strerror(errno));
		return (void *) 1;
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
			tst_result = 1;
			return NULL;
		}
	}

	return NULL;
}

static void *fork_thread(void *arg)
{
	pid_t pid;

	(void) arg;

	while (!done) {
		pid = tst_fork();
		if (pid == 0) {
			exit(0);
		} else if (pid < 0) {
			fprintf(stderr, "Failed to fork child: %s.\n",
				strerror(errno));
			return (void *) 1;
		}
		waitpid(pid, NULL, 0);
		usleep(100);
	}

	return NULL;
}

int main(int argc, char *argv[])
{
	int i, lc;

	workers = sysconf(_SC_NPROCESSORS_ONLN);
	if (workers > MAX_WORKERS)
		workers = MAX_WORKERS;
	tst_parse_opts(argc, argv, options, help);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++)
			dma_thread_diotest_verify();
	}

	cleanup();
	tst_exit();
}

static void dma_thread_diotest_verify(void)
{
	int n, j, offset, rc;
	void *retval;
	char filename[PATH_MAX];
	pthread_t fork_tid;

	tst_result = 0;

	for (n = 1; n <= FILECOUNT; n++) {
		snprintf(filename, sizeof(filename), FILE_BASEPATH, n);
		for (j = 0; j < workers; j++) {
			worker[j].fd = SAFE_OPEN(cleanup, filename,
						 O_RDONLY | O_DIRECT);
			worker[j].pattern = n;
		}

		tst_resm(TINFO, "Reading file %d.", n);

		for (offset = 0; offset < FILESIZE; offset += READSIZE) {
			memset(buffer, PATTERN, READSIZE + align);
			for (j = 0; j < workers; j++) {
				worker[j].offset = offset + j * PAGE_SIZE;
				worker[j].buffer =
				    buffer + align + j * PAGE_SIZE;
				worker[j].length = PAGE_SIZE;
			}
			/* The final worker reads whatever is left over. */
			worker[workers - 1].length =
			    READSIZE - PAGE_SIZE * (workers - 1);

			done = 0;

			rc = pthread_create(&fork_tid, NULL, fork_thread, NULL);
			if (rc != 0) {
				tst_brkm(TBROK, cleanup, "pthread_create "
					 "failed: %s", strerror(rc));
			}

			for (j = 0; j < workers; j++) {
				rc = pthread_create(&worker[j].tid, NULL,
						    worker_thread, worker + j);
				if (rc != 0) {
					tst_brkm(TBROK, cleanup, "Can't create "
						 "worker thread %d: %s",
						 j, strerror(rc));
				}
			}

			for (j = 0; j < workers; j++) {
				rc = pthread_join(worker[j].tid, &retval);
				if (rc != 0) {
					tst_brkm(TBROK, cleanup, "Failed to "
						 "join worker thread %d: %s.",
						 j, strerror(rc));
				}
				if ((intptr_t)retval != 0) {
					tst_brkm(TBROK, cleanup, "there is"
						 "some errors in worker[%d], "
						 "return value: %ld",
						 j, (intptr_t)retval);
				}
			}

			/* Let the fork thread know it's ok to exit */
			done = 1;

			rc = pthread_join(fork_tid, &retval);
			if (rc != 0) {
				tst_brkm(TBROK, cleanup,
					 "Failed to join fork thread: %s.",
					 strerror(rc));
			}
			if ((intptr_t)retval != 0) {
				tst_brkm(TBROK, cleanup,
					 "fork() failed in fork thread:"
					 "return value: %ld", (intptr_t)retval);
			}
		}

		/* Close the fd's for the next file. */
		for (j = 0; j < workers; j++)
			SAFE_CLOSE(cleanup, worker[j].fd);
		if (tst_result)
			break;
	}

	if (tst_result)
		tst_resm(TFAIL, "data corruption is detected");
	else
		tst_resm(TPASS, "data corruption is not detected");
}

static void setup(void)
{
	char filename[PATH_MAX];
	int n, j, fd, directflag = 1;
	long type;

	if (align_str) {
		align = atoi(align_str);
		if (align < 0 || align > PAGE_SIZE)
			tst_brkm(TCONF, NULL, "Bad alignment %d.", align);
	}
	tst_resm(TINFO, "using alignment %d", align);

	if (workers_str) {
		workers = atoi(workers_str);
		if (workers < MIN_WORKERS || workers > MAX_WORKERS) {
			tst_brkm(TCONF, NULL, "Worker count %d not between "
				 "%d and %d, inclusive",
				 workers, MIN_WORKERS, MAX_WORKERS);
		}
	}
	tst_resm(TINFO, "using %d workers.", workers);

	tst_sig(FORK, DEF_HANDLER, NULL);
	tst_require_root();

	TEST_PAUSE;

	tst_tmpdir();

	/*
	 * Some file systems may not implement the O_DIRECT flag and open() will
	 * fail with EINVAL if it is used. So add this check for current
	 * filesystem current directory is in, if not supported, we choose to
	 * have this test in LTP_BIG_DEV and mkfs it as ext3.
	 */
	fd = open("testfile", O_CREAT | O_DIRECT, 0644);
	if (fd < 0 && errno == EINVAL) {
		type = tst_fs_type(NULL, ".");
		tst_resm(TINFO, "O_DIRECT flag is not supported on %s "
			 "filesystem", tst_fs_type_name(type));
		directflag = 0;
	} else if (fd > 0) {
		SAFE_CLOSE(NULL, fd);
	}

	SAFE_MKDIR(cleanup, MNT_POINT, DIR_MODE);

	/*
	 * verify whether the current directory has enough free space,
	 * if it is not satisfied, we will use the LTP_BIG_DEV, which
	 * will be exported by runltp with "-z" option.
	 */
	if (!directflag || !tst_fs_has_free(NULL, ".", 1300, TST_MB)) {
		device = getenv("LTP_BIG_DEV");
		if (device == NULL) {
			tst_brkm(TCONF, NULL,
				 "you must specify a big blockdevice(>1.3G)");
		} else {
			tst_mkfs(NULL, device, "ext3", NULL, NULL);
		}

		SAFE_MOUNT(NULL, device, MNT_POINT, "ext3", 0, NULL);
		mount_flag = 1;
	}

	worker = SAFE_MALLOC(cleanup, workers * sizeof(worker_t));

	for (j = 0; j < workers; j++)
		worker[j].worker_number = j;

	for (n = 1; n <= FILECOUNT; n++) {
		snprintf(filename, sizeof(filename), FILE_BASEPATH, n);

		if (tst_fill_file(filename, n, FILESIZE, 1)) {
			tst_brkm(TBROK, cleanup, "failed to create file: %s",
				 filename);
		}
	}

	if (posix_memalign((void **)&buffer, PAGE_SIZE, READSIZE + align) != 0)
		tst_brkm(TBROK, cleanup, "call posix_memalign failed");
}

static void cleanup(void)
{
	free(buffer);

	if (mount_flag && tst_umount(MNT_POINT) < 0)
		tst_resm(TWARN | TERRNO, "umount device:%s failed", device);

	free(worker);

	tst_rmdir();
}

static void help(void)
{
	printf("-a align read buffer to offset <alignment>.\n");
	printf("-w number of worker threads, 2 (default) to %d,"
	       " defaults to number of cores.\n", MAX_WORKERS);
}
