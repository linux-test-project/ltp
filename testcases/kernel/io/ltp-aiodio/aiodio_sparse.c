
/*
 * Copyright (c) 2004 Daniel McNeil <daniel@osdl.org>
 *               2004 Open Source Development Lab
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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Module: .c
 */

/*
 * Change History:
 *
 * 2/2004  Marty Ridgeway (mridge@us.ibm.com) Changes to adapt to LTP
 *
*/
#define _GNU_SOURCE

#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <limits.h>

#include <libaio.h>

#include "test.h"

#define NUM_CHILDREN 1000

int debug;
const char* filename1=NULL;
const char* filename2=NULL;

static void setup(void);
static void cleanup(void);

char *TCID="aiodio_sparse";	/* Test program identifier.    */
int TST_TOTAL=1;	/* Total number of test cases. */

#define barrier() __asm__ __volatile__("": : :"memory")
#define WITH_SIGNALS_BLOCKED(code) {											\
		sigset_t held_sigs_;																	\
		sigfillset(&held_sigs_);															\
		sigprocmask(SIG_SETMASK, &held_sigs_, &held_sigs_);	\
		barrier(); \
		code;																									\
		barrier(); \
		sigprocmask(SIG_SETMASK, &held_sigs_, NULL);					\
	}

/*
 * aiodio_sparse - issue async O_DIRECT writes to holes is a file while
 *	concurrently reading the file and checking that the read never reads
 *	uninitailized data.
 */

char *check_zero(unsigned char *buf, int size)
{
	unsigned char *p;

	p = buf;

	while (size > 0) {
		if (*buf != 0) {
			fprintf(stderr, "non zero buffer at buf[%ld] => 0x%02x,%02x,%02x,%02x\n",
				buf - p, (unsigned int)buf[0],
				size > 1 ? (unsigned int)buf[1] : 0,
				size > 2 ? (unsigned int)buf[2] : 0,
				size > 3 ? (unsigned int)buf[3] : 0);
			if (debug)
				fprintf(stderr, "buf %p, p %p\n", buf, p);
			return buf;
		}
		buf++;
		size--;
	}
	return 0;	/* all zeros */
}

int read_sparse(char *filename, int filesize)
{
	int fd;
	int i;
	int j;
	int r;
	char buf[4096];

	while ((fd = open(filename, O_RDONLY)) < 0) {
		sleep(1);	/* wait for file to be created */
	}

	for (i = 0 ; i < 100000000; i++) {
		off_t offset = 0;
		char *badbuf;

		if (debug > 1 && (i % 10) == 0) {
			fprintf(stderr, "child %d, read loop count %d\n",
				getpid(), i);
		}
		lseek(fd, SEEK_SET, 0);
		for (j = 0; j < filesize+1; j += sizeof(buf)) {
			r = read(fd, buf, sizeof(buf));
			if (r > 0) {
				if ((badbuf = check_zero(buf, r))) {
					fprintf(stderr, "non-zero read at offset %ld\n",
						offset + badbuf - buf);
					kill(getppid(), SIGTERM);
					exit(10);
				}
			}
			offset += r;
		}
	}
    return 0;
}

volatile int got_signal;

void
sig_term_func(int i, siginfo_t *si, void *p)
{
	if (debug)
		fprintf(stderr, "sig(%d, %p, %p)\n", i, si, p);
	got_signal++;
}

/*
 * do async DIO writes to a sparse file
 */
void aiodio_sparse(char *filename, int align, int writesize, int filesize, int num_aio)
{
	int fd;
	int i;
	int w;
	static struct sigaction s;
	struct iocb **iocbs;
	off_t offset;
	io_context_t myctx;
	struct io_event event;
	int aio_inflight;

	s.sa_sigaction = sig_term_func;
	s.sa_flags = SA_SIGINFO;
	sigaction(SIGTERM, &s, 0);

	if ((num_aio * writesize) > filesize) {
		num_aio = filesize / writesize;
	}
	memset(&myctx, 0, sizeof(myctx));
	io_queue_init(num_aio, &myctx);

	iocbs = (struct iocb **)malloc(sizeof(struct iocb *) * num_aio);
	for (i = 0; i < num_aio; i++) {
		if ((iocbs[i] = (struct iocb *)malloc(sizeof(struct iocb))) == 0) {
			perror("cannot malloc iocb");
			return;
		}
	}

	WITH_SIGNALS_BLOCKED(
		fd = open(filename, O_DIRECT|O_WRONLY|O_CREAT|O_EXCL, 0600);
		if(fd > 0)
			filename1=filename;
	);

	if (fd < 0) {
		perror("cannot create file");
		return;
	}

	ftruncate(fd, filesize);

	/*
	 * allocate the iocbs array and iocbs with buffers
	 */
	offset = 0;
	for (i = 0; i < num_aio; i++) {
		void *bufptr;

		if (posix_memalign(&bufptr, align, writesize)) {
			perror("cannot malloc aligned memory");
			close(fd);

			WITH_SIGNALS_BLOCKED(
				filename1=NULL;
				unlink(filename);
			);
			return;
		}
		memset(bufptr, 0, writesize);
		io_prep_pwrite(iocbs[i], fd, bufptr, writesize, offset);
		offset += writesize;
	}

	/*
	 * start the 1st num_aio write requests
	 */
	if ((w = io_submit(myctx, num_aio, iocbs)) < 0) {
		printf("io_submit failed error=%d\n", w);
		close(fd);

		WITH_SIGNALS_BLOCKED(
			filename1=NULL;
			unlink(filename);
		);
		return;
	}
	if (debug)
		fprintf(stderr, "io_submit() return %d\n", w);

	/*
	 * As AIO requests finish, keep issuing more AIO until done.
	 */
	aio_inflight = num_aio;
	if (debug)
		fprintf(stderr, "aiodio_sparse: %d i/o in flight\n", aio_inflight);
	while (offset < filesize)  {
		int n;
		struct iocb *iocbp;

		if (debug)
			fprintf(stderr, "aiodio_sparse: offset %p filesize %d inflight %d\n",
				&offset, filesize, aio_inflight);

		if ((n = io_getevents(myctx, 1, 1, &event, 0)) != 1) {
			if (-n != EINTR)
				fprintf(stderr, "io_getevents() returned %d\n", n);
			break;
		}
		if (debug)
			fprintf(stderr, "aiodio_sparse: io_getevent() returned %d\n", n);
		aio_inflight--;
		if (got_signal)
			break;		/* told to stop */
		/*
		 * check if write succeeded.
		 */
		iocbp = (struct iocb *)event.obj;
		if (event.res2 != 0 || event.res != iocbp->u.c.nbytes) {
			fprintf(stderr,
				"AIO write offset %lld expected %ld got %ld\n",
				iocbp->u.c.offset, iocbp->u.c.nbytes,
				event.res);
			break;
		}
		if (debug)
			fprintf(stderr, "aiodio_sparse: io_getevent() res %ld res2 %ld\n",
				event.res, event.res2);
	
		/* start next write */
		io_prep_pwrite(iocbp, fd, iocbp->u.c.buf, writesize, offset);
		offset += writesize;
		if ((w = io_submit(myctx, 1, &iocbp)) < 0) {
			fprintf(stderr, "io_submit failed at offset %ld\n",
				offset);
			perror("");
			break;
		}
		if (debug)
			fprintf(stderr, "io_submit() return %d\n", w);
		aio_inflight++;
	}

	/*
	 * wait for AIO requests in flight.
	 */
	while (aio_inflight > 0) {
		int n;
		struct iocb *iocbp;

		if ((n = io_getevents(myctx, 1, 1, &event, 0)) != 1) {
			perror("io_getevents failed");
			break;
		}
		aio_inflight--;
		/*
		 * check if write succeeded.
		 */
		iocbp = (struct iocb *)event.obj;
		if (event.res2 != 0 || event.res != iocbp->u.c.nbytes) {
			fprintf(stderr,
				"AIO write offset %lld expected %ld got %ld\n",
				iocbp->u.c.offset, iocbp->u.c.nbytes,
				event.res);
		}
	}
	if (debug)
		fprintf(stderr, "AIO DIO write done unlinking file\n");
	close(fd);

	WITH_SIGNALS_BLOCKED(
		filename1=NULL;
		unlink(filename);
	);
}


void dirty_freeblocks(int size)
{
	int fd;
	void *p;
	int pg;
	char filename[1024];

	pg = getpagesize();
	size = ((size + pg - 1) / pg) * pg;
	sprintf(filename, "file.xx.%d", getpid());

	WITH_SIGNALS_BLOCKED(
		fd = open(filename, O_CREAT|O_RDWR|O_EXCL, 0600);
		if(fd != -1)
			filename2 = filename;
	);

	if (fd < 0) {
		perror("cannot open file");
		exit(2);
	}
	ftruncate(fd, size);
	p = mmap(0, size, PROT_WRITE|PROT_READ, MAP_SHARED|MAP_FILE, fd, 0);
	if (p == MAP_FAILED) {
		perror("cannot mmap");
		close(fd);

		WITH_SIGNALS_BLOCKED(
			filename2=NULL;
			unlink(filename);
		);
		exit(2);
	}
	memset(p, 0xaa, size);
	msync(p, size, MS_SYNC);
	munmap(p, size);
	close(fd);
	WITH_SIGNALS_BLOCKED(
		filename2=NULL;
		unlink(filename);
	);
}

int usage()
{
	fprintf(stderr, "usage: dio_sparse [-n children] [-s filesize]"
		" [-w writesize] [-r readsize] \n");
	exit(1);
}

/*
 * Scale value by kilo, mega, or giga.
 */
long long scale_by_kmg(long long value, char scale)
{
	switch (scale) {
	case 'g':
	case 'G':
		value *= 1024;
	case 'm':
	case 'M':
		value *= 1024;
	case 'k':
	case 'K':
		value *= 1024;
		break;
	case '\0':
		break;
	default:
		usage();
		break;
	}
	return value;
}

/*
 *	usage:
 * aiodio_sparse [-r readsize] [-w writesize] [-n chilren] [-a align] [-i num_aio]
 */

int main(int argc, char **argv)
{
	char filename[PATH_MAX];
	int pid[NUM_CHILDREN];
	int num_children = 1;
	int i;
	long alignment = 512;
	int readsize = 65536;
	int writesize = 65536;
	int filesize = 100*1024*1024;
	int num_aio = 16;
	int children_errors = 0;
	int c;
	extern char *optarg;
	extern int optind, optopt, opterr;

	printf("Begin aiodio_sparse tests...\n");

	snprintf(filename, sizeof(filename), "%s/aiodio/file",
		getenv("TMP") ? getenv("TMP") : "/tmp");

	while ((c = getopt(argc, argv, "dr:w:n:a:s:i:")) != -1) {
		char *endp;
		switch (c) {
		case 'd':
			debug++;
			break;
		case 'i':
			num_aio = atoi(optarg);
			break;
		case 'a':
			alignment = strtol(optarg, &endp, 0);
			alignment = (int)scale_by_kmg((long long)alignment,
                                                        *endp);
			break;
		case 'r':
			readsize = strtol(optarg, &endp, 0);
			readsize = (int)scale_by_kmg((long long)readsize, *endp);
			break;
		case 'w':
			writesize = strtol(optarg, &endp, 0);
			writesize = (int)scale_by_kmg((long long)writesize, *endp);
			break;
		case 's':
			filesize = strtol(optarg, &endp, 0);
			filesize = (int)scale_by_kmg((long long)filesize, *endp);
			break;
		case 'n':
			num_children = atoi(optarg);
			if (num_children > NUM_CHILDREN) {
				fprintf(stderr,
					"number of children limited to %d\n",
					NUM_CHILDREN);
				num_children = NUM_CHILDREN;
			}
			break;
		case '?':
			usage();
			break;
		}
	}

	setup();

	/*
	 * Create some dirty free blocks by allocating, writing, syncing,
	 * and then unlinking and freeing.
	 */
	dirty_freeblocks(filesize);

	for (i = 0; i < num_children; i++) {
		if ((pid[i] = fork()) == 0) {
			/* child */
			return read_sparse(filename, filesize);
		} else if (pid[i] < 0) {
			/* error */
			perror("fork error");
			break;
		} else {
			/* Parent */
			continue;
		}
	}

	/*
	 * Parent write to a hole in a file using async direct i/o
	 */

	aiodio_sparse(filename, alignment, writesize, filesize, num_aio);

	if (debug)
		fprintf(stderr, "dio_sparse done writing, kill children\n");

	for (i = 0; i < num_children; i++) {
		kill(pid[i], SIGTERM);
	}

	for (i = 0; i < num_children; i++) {
		int status;
		pid_t p;

		p = waitpid(pid[i], &status, 0);
		if (p < 0) {
			perror("waitpid");
		} else {
			if (WIFEXITED(status) && WEXITSTATUS(status) == 10) {
				children_errors++;
				if (debug) {
					fprintf(stderr, "child %d bad exit\n", p);
				}
			}
		}
	}
	if (debug)
		fprintf(stderr, "aiodio_sparse %d children had errors\n",
			children_errors);
	if (children_errors)
		exit(10);
	return 0;
}

static void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);
	signal(SIGTERM, cleanup);
}

static void cleanup(void)
{
	if(filename1)
		unlink(filename1);
	if(filename2)
		unlink(filename2);

	tst_exit();
}
