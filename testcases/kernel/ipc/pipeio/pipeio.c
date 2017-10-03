/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
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
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 *
 */
/* $Header: /cvsroot/ltp/ltp/testcases/kernel/ipc/pipeio/pipeio.c,v 1.18 2009/03/19 07:10:02 subrata_modak Exp $ */
/*
 *  This tool can be used to beat on system or named pipes.
 *  See the help() function below for user information.
 */
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/sem.h>

#include "tlibio.h"

#include "test.h"
#include "safe_macros.h"
#include "lapi/semun.h"

char *TCID = "pipeio";
int TST_TOTAL = 1;

#define SAFE_FREE(p) { if (p) { free(p); (p)=NULL; } }

#if defined(__linux__)
#define NBPW sizeof(int)
#endif

#define OCTAL	'o'
#define HEX	'x'
#define DECIMAL	'd'
#define ASCII	'a'
#define NO_OUT	'n'

#define PIPE_NAMED	"named pipe,"
#define PIPE_UNNAMED	"sys pipe,"

#define BLOCKING_IO	"blking,"
#define NON_BLOCKING_IO	"non-blking,"
#define UNNAMED_IO	""

#define MAX_ERRS 16
#define MAX_EMPTY 256

static int parse_options(int argc, char *argv[]);
static void setup(int argc, char *argv[]);
static void cleanup(void);

static void do_child(void);
static void do_parent(void);

static void help(void), usage(void), prt_examples(void);
static void prt_buf(char **addr, char *buf, int length, int format);
static void sig_child(int sig);
static int check_rw_buf(void);

static volatile sig_atomic_t nchildcompleted;

/* variables may be modified in setup() */
static int num_writers = 1;	/* number of writers */
static int num_writes = 1;	/* number of writes per child */
static int loop;		/* loop indefinitely */
static int exit_error = 1;	/* exit on error #, zero means no exit */
static int size = 327;		/* default size */
static int unpipe;		/* un-named pipe if non-zero */
static int verbose;		/* verbose mode if set */
static int quiet;		/* quiet mode if set */
static int num_rpt;		/* ping number, how often to print message */
static int chld_wait;	/* max time to wait between writes, 1 == no wait */
static int parent_wait;	/* max time to wait between reads, 1 == no wait */
static int ndelay = O_NDELAY;	/* additional flag to open */
static char *writebuf;
static char *readbuf;
static char pname[PATH_MAX];	/* contains the name of the named pipe */
static char *blk_type = NON_BLOCKING_IO; /* blocking i/o or not */
static char *pipe_type;		/* type of pipe under test */
static int format = HEX;
static int format_size = -1;
static int iotype;		/* sync io */

/* variables will be modified in running */
static int error;
static int count;
static int read_fd;
static int write_fd;
static int empty_read;
static int sem_id;

static union semun u;

int main(int ac, char *av[])
{
	int i;
	unsigned int j;
	unsigned int uwait_iter = 1000, uwait_total = 5000000;
	pid_t child;

	setup(ac, av);

	for (i = num_writers; i > 0; --i) {

		child = tst_fork();
		switch (child) {
		case -1:
			tst_brkm(TBROK | TERRNO, cleanup, "fork() failed");
		case 0:
			do_child();
			exit(0);
		default:
			break;
		}
	}

	do_parent();

	if (empty_read)
		tst_resm(TWARN, "%d empty reads", empty_read);

	if (error) {
		tst_resm(TFAIL, "%d data errors on pipe, read size = %d, %s %s",
			 error, size, pipe_type, blk_type);
	} else if (!quiet) {
		tst_resm(TPASS, "%d pipe reads complete, read size = %d, %s %s",
			 count + 1, size, pipe_type, blk_type);
	}

	/*
	 * wait for all children to finish, timeout after uwait_total
	 * semtimedop might not be available everywhere
	 */
	for (j = 0; j < uwait_total; j += uwait_iter) {
		if (semctl(sem_id, 1, GETVAL) == 0)
			break;
		usleep(uwait_iter);
	}

	if (j >= uwait_total) {
		tst_resm(TWARN,
			 "Timed out waiting for child processes to exit");
	}

	cleanup();
	tst_exit();
}

static int parse_options(int argc, char *argv[])
{
	char *cp;
	int c;
	int ret = 0;
	static double d;

	while ((c = getopt(argc, argv, "T:bc:D:he:Ef:i:I:ln:p:qs:uvW:w:"))
	       != -1) {
		switch (c) {
		case 'T':
			TCID = optarg;
			break;
		case 'h':
			help();
			ret = 1;
			break;
		case 'D':	/* pipe name */
			strcpy(pname, optarg);
			break;
		case 'b':	/* blocked */
			ndelay = 0;
			blk_type = BLOCKING_IO;
			break;
		case 'c':	/* number childern */
			if (sscanf(optarg, "%d", &num_writers) != 1) {
				fprintf(stderr,
					"%s: --c option invalid arg '%s'.\n",
					TCID, optarg);
				ret = 1;
			} else if (num_writers <= 0) {
				fprintf(stderr, "%s: --c option must be "
					"greater than zero.\n", TCID);
				ret = 1;
			}
			break;
		case 'e':	/* exit on error # */
			if (sscanf(optarg, "%d", &exit_error) != 1) {
				fprintf(stderr,
					"%s: --e option invalid arg '%s'.\n",
					TCID, optarg);
				ret = 1;
			} else if (exit_error < 0) {
				fprintf(stderr, "%s: --e option must be "
					"greater than zero.\n", TCID);
				ret = 1;
			}
			break;
		case 'E':
			prt_examples();
			ret = 1;
			break;
		case 'f':	/* format of buffer on error */
			switch (optarg[0]) {
			case 'x':
			case 'X':
				format = HEX;
				break;
			case 'o':
			case 'O':
				format = OCTAL;
				break;
			case 'd':
			case 'D':
				format = DECIMAL;
				break;
			case 'a':
			case 'A':
				format = ASCII;
				break;
			case 'n':	/* not output */
			case 'N':
				format = NO_OUT;
				break;

			default:
				fprintf(stderr,
					"%s: --f option invalid arg '%s'.\n",
					TCID, optarg);
				fprintf(stderr, "\tIt must be x(hex), o(octal),"
					"d(decimal), a(ascii) or n(none) with "
					"opt sz\n");
				ret = 1;
				break;
			}
			cp = optarg;
			cp++;
			if (*cp) {
				if (sscanf(cp, "%i", &format_size) != 1) {
					fprintf(stderr, "%s: --f option invalid"
						"arg '%s'.\n", TCID, optarg);
					fprintf(stderr, "\tIt must be x(hex),"
						"o(octal), d(decimal), a(ascii)"
						" or n(none) with opt sz\n");
					ret = 1;
					break;
				}
			}
			break;

		case 'I':
			iotype = lio_parse_io_arg1(optarg);
			if (iotype == -1) {
				fprintf(stderr, "%s: --I arg is invalid, "
					"must be s, p, f, a, l, L or r.\n",
					TCID);
				ret = 1;
			}
			break;

		case 'l':	/* loop forever */
			++loop;
			break;

		case 'i':
		case 'n':	/* number writes per child */
			if (sscanf(optarg, "%d", &num_writes) != 1) {
				fprintf(stderr, "%s: --i/n option invalid "
					"arg '%s'.\n", TCID, optarg);
				ret = 1;
			} else if (num_writes < 0) {
				fprintf(stderr, "%s: --i/n option must be "
					"greater than equal to zero.\n",
					TCID);
				ret = 1;
			}

			if (num_writes == 0)	/* loop forever */
				++loop;
			break;
		case 'p':	/* ping */
			if (sscanf(optarg, "%d", &num_rpt) != 1) {
				fprintf(stderr,
					"%s: --p option invalid arg '%s'.\n",
					TCID, optarg);
				ret = 1;
			} else if (num_rpt < 0) {
				fprintf(stderr, "%s: --p option must be greater"
					" than equal to zero.\n", TCID);
				ret = 1;
			}
			break;
		case 'q':	/* Quiet - NOPASS */
			quiet = 1;
			break;
		case 's':	/* size */
			if (sscanf(optarg, "%d", &size) != 1) {
				fprintf(stderr,
					"%s: --s option invalid arg '%s'.\n",
					TCID, optarg);
				ret = 1;
			} else if (size <= 0) {
				fprintf(stderr, "%s: --s option must be greater"
					" than zero.\n", TCID);
				ret = 1;
			}
			break;
		case 'u':
			unpipe = 1;	/* un-named pipe */
			break;
		case 'v':	/* verbose */
			verbose = 1;
			break;
		case 'W':	/* max wait time between reads */
			d = strtod(optarg, &cp);
			if (*cp != '\0') {
				fprintf(stderr,
					"%s: --w option invalid arg '%s'.\n",
					TCID, optarg);
				ret = 1;
			} else if (d < 0) {
				fprintf(stderr, "%s: --w option must be greater"
					" than zero.\n", TCID);
				ret = 1;
			}
			parent_wait = (int)(d * 1000000.0);
			break;
		case 'w':	/* max wait time between writes */
			d = strtod(optarg, &cp);
			if (*cp != '\0') {
				fprintf(stderr,
					"%s: --w option invalid arg '%s'.\n",
					TCID, optarg);
				ret = 1;
			} else if (d < 0) {
				fprintf(stderr, "%s: --w option must be greater"
					" than zero.\n", TCID);
				ret = 1;
			}
			chld_wait = (int)(d * 1000000.0);
			break;
		case '?':
			ret = 1;
			break;
		}

		if (ret == 1) {
			usage();
			return ret;
		}
	}

	return ret;
}

static void setup(int argc, char *argv[])
{
	int ret;
	char *toutput;
	int fds[2];

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	if (signal(SIGCHLD, sig_child) == SIG_ERR) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "set signal handler for SIGCHLD failed");
	}

	toutput = getenv("TOUTPUT");
	if (toutput != NULL && strcmp(toutput, "NOPASS") == 0)
		quiet = 1;

	sprintf(pname, "%s", "tpipe");

	ret = parse_options(argc, argv);
	if (ret == 1)
		tst_brkm(TBROK, cleanup, "options parse error");

	if (format_size == -1)
		format_size = size;

	/*
	 * If there is more than one writer, all writes and reads
	 * must be the same size.  Only writes of a size <= PIPE_BUF
	 * are atomic.  T
	 * Therefore, if size is greater than PIPE_BUF, we will break
	 * the writes into PIPE_BUF chunks.  We will also increase the
	 * number of writes to ensure the same (or more) amount of
	 * data is written.  This is the same as erroring and telling
	 * the user the new cmd line to do the same thing.
	 * Example:
	 *      pipeio -s 5000 -n 10 -c 5
	 *      (each child will write at least 50000 bytes, since all
	 *      writes have to be in 4096 chuncks or 13*4096 (53248)
	 *      bytes will be written.)  This is the same as:
	 *      pipeio -s 4096 -n 13 -c 5
	 */
	if (size > PIPE_BUF && num_writers > 1) {
		if (!loop) {
			/*
			 * we must set num_writes*num_writers
			 * doesn't overflow later
			 */
			num_writes = MIN(((long long)num_writes * size +
					 PIPE_BUF - 1) / PIPE_BUF,
					 INT_MAX / num_writers);
			tst_resm(TINFO, "adjusting i/o size to %d, and # of "
				 "writes to %d", PIPE_BUF, num_writes);
		} else {
			tst_resm(TINFO, "adjusting i/o size to %d", PIPE_BUF);
		}
		size = PIPE_BUF;
	}

	writebuf = SAFE_MALLOC(cleanup, size);
	readbuf = SAFE_MALLOC(cleanup, size);

	memset(writebuf, 'Z', size);
	writebuf[size - 1] = 'A';

	sem_id = semget(IPC_PRIVATE, 2, IPC_CREAT | S_IRWXU);
	if (sem_id == -1) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "Couldn't allocate semaphore");
	}

	if (semctl(sem_id, 0, SETVAL, u) == -1) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "Couldn't initialize semaphore 0 value");
	}

	if (semctl(sem_id, 1, SETVAL, u) == -1) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "Couldn't initialize semaphore 1 value");
	}

	if (unpipe) {
		SAFE_PIPE(cleanup, fds);
		read_fd = fds[0];
		write_fd = fds[1];
		pipe_type = PIPE_UNNAMED;
		blk_type = UNNAMED_IO;
	} else {
		SAFE_MKFIFO(cleanup, pname, 0777);
		pipe_type = PIPE_NAMED;
	}
}

static void cleanup(void)
{
	SAFE_FREE(writebuf);
	SAFE_FREE(readbuf);

	semctl(sem_id, 0, IPC_RMID);

	if (!unpipe)
		unlink(pname);

	tst_rmdir();
}

static void do_child(void)
{
	int *count_word;        /* holds address where to write writers count */
	int *pid_word;          /* holds address where to write writers pid */
	int nb, j;
	long clock;
	char *cp;
	long int n;
	struct sembuf sem_op;
	pid_t self_pid =  getpid();

	if (!unpipe) {
		write_fd = open(pname, O_WRONLY);
		if (write_fd == -1) {
			fprintf(stderr, "child pipe open(%s, %#o) failed",
				pname, O_WRONLY | ndelay);
			exit(1);
		}
		if (ndelay && fcntl(write_fd, F_SETFL, O_NONBLOCK) == -1) {
			fprintf(stderr, "Failed setting the pipe to "
				"nonblocking mode");
			exit(1);
		}
	} else {
		close(read_fd);
	}

	sem_op = (struct sembuf) {
		 .sem_num = 0, .sem_op = 1, .sem_flg = 0};

	if (semop(sem_id, &sem_op, 1) == -1) {
		fprintf(stderr, "child: %d couldn't raise the semaphore 0",
			self_pid);
		exit(1);
	}

	pid_word = (int *)&writebuf[0];
	count_word = (int *)&writebuf[NBPW];

	for (j = 0; j < num_writes || loop; ++j) {
		/*
		 * writes are only in one unit when the size of the write
		 * is <= PIPE_BUF.
		 * Therefore, if size is greater than PIPE_BUF, we will break
		 * the writes into PIPE_BUF chunks.
		 * All writes and read need to be same size.
		 */

		/*
		 * write pid and count in first two
		 * words of buffer
		 */
		*count_word = j;
		*pid_word = self_pid;

		nb = lio_write_buffer(write_fd, iotype, writebuf, size,
				      SIGUSR1, &cp, 0);
		if (nb < 0) {
			/*
			 * If lio_write_buffer returns a negative number,
			 * the return will be -errno.
			 */
			fprintf(stderr, "pass %d: lio_write_buffer(%s) failed;"
				" it returned %d: %s",
				j, cp, nb, strerror(-nb));
				exit(1);
		} else if (nb != size) {
			fprintf(stderr, "pass %d: lio_write_buffer(%s) failed,"
				" write count %d, but expected to write %d",
				j, cp, nb, size);
		}
		if (verbose) {
			fprintf(stderr, "pass %d: pid %d: wrote %d bytes,"
				"expected %d bytes",
				j, self_pid, nb, size);
		}

		if (chld_wait) {
			clock = time(0);
			srand48(clock);
			n = lrand48() % chld_wait;
			usleep(n);
		}
		fflush(stderr);
	}

	/* child waits until parent completes open() */
	sem_op = (struct sembuf) {
		  .sem_num = 1, .sem_op = -1, .sem_flg = 0};
	if (semop(sem_id, &sem_op, 1) == -1)
		fprintf(stderr, "Couldn't lower the semaphore 1");

	exit(0);
}

static int check_rw_buf(void)
{
	int i;

	for (i = 2 * NBPW; i < size; ++i) {
		if (writebuf[i] != readbuf[i]) {
			++error;
			tst_resm(TFAIL,
				 "FAIL data error on byte %d; rd# %d, sz= %d, "
				 "%s %s empty_reads= %d, err= %d",
				 i, count, size, pipe_type, blk_type,
				 empty_read, error);
			prt_buf(&readbuf, readbuf, format_size, format);
			fflush(stdout);
			return 1;
		}
	}

	return 0;
}

static void do_parent(void)
{
	int i, nb;
	long clock;
	time_t start_time, current_time, diff_time;
	char *cp;
	long int n;
	struct sembuf sem_op;

	start_time = time(0);
	if (!unpipe) {
		read_fd = SAFE_OPEN(cleanup, pname, O_RDONLY);
		if (ndelay && fcntl(read_fd, F_SETFL, O_NONBLOCK) == -1) {
			tst_brkm(TBROK | TERRNO, cleanup,
				 "Failed setting the pipe to nonblocking mode");
		}
	} else {
		SAFE_CLOSE(cleanup, write_fd);
	}

	/* raise semaphore so children can exit */
	sem_op = (struct sembuf) {
		  .sem_num = 1, .sem_op = num_writers, .sem_flg = 0};
	if (semop(sem_id, &sem_op, 1) == -1) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "Couldn't raise the semaphore 1");
	}

	sem_op = (struct sembuf) {
		  .sem_num = 0, .sem_op = -num_writers, .sem_flg = 0};

	while (nchildcompleted < num_writers
	       && semop(sem_id, &sem_op, 1) == -1) {
		if (errno == EINTR)
			continue;
		tst_brkm(TBROK | TERRNO, cleanup,
			 "Couldn't wait on semaphore 0");
	}

	/* parent start to read pipe */
	for (i = num_writers * num_writes; i > 0 || loop; --i) {
		if (error >= MAX_ERRS || empty_read >= MAX_EMPTY)
			break;
		if (parent_wait) {
			clock = time(0);
			srand48(clock);
			n = lrand48() % parent_wait;
			usleep(n);
		}
		++count;
		nb = lio_read_buffer(read_fd, iotype, readbuf, size,
				     SIGUSR1, &cp, 0);
		if (nb < 0) {
			/*
			 * If lio_read_buffer returns a negative number,
			 * the return will be -errno.
			 */
			tst_resm(TFAIL, "pass %d: lio_read_buffer(%s) failed; "
				 "returned %d: %s", i, cp, nb, strerror(-nb));
			++i;
			count--;
			error++;
			continue;
		} else {
			if (nb == 0) {
				if (nchildcompleted >= num_writers && !loop) {
					tst_resm(TWARN, "The children have "
						 "died prematurely");
					break;	/* All children have died */
				}
				empty_read++;
				++i;
				count--;
				continue;
			} else if (nb < size && size <= PIPE_BUF) {
				tst_resm(TFAIL, "pass %d: partial read from the"
					" pipe: read %d bytes, expected %d, "
					"read count %d", i, nb, size, count);
				++error;
			} else if (nb == size) {
				check_rw_buf();
				if (exit_error && exit_error == error)
					return;
			}

			if (verbose || (num_rpt && !(count % num_rpt))) {
				current_time = time(0);
				diff_time = current_time - start_time;
				tst_resm(TFAIL,
					 "(%d) rd# %d, sz= %d, %s %s "
					 "empty_reads= %d, err= %d\n",
					 (int)diff_time, count, size,
					 pipe_type, blk_type,
					 empty_read, error);
				fflush(stdout);
			}
		}
	}

	SAFE_CLOSE(cleanup, read_fd);
}

static void usage(void)
{
	fprintf(stderr, "Usage: %s [-bEv][-c #writers][-D pname][-h]"
		"[-e exit_num][-f fmt][-l][-i #writes][-n #writes][-p num_rpt]"
		"\n\t[-s size][-W max_wait][-w max_wait][-u]\n", TCID);
	fflush(stderr);
}

static void help(void)
{
	usage();

	printf(" -b    - blocking reads and writes. default non-block\n\
  -c #writers  - number of writers (childern)\n\
  -D pname     - name of fifo (def tpipe<pid>)\n\
  -h           - print this help message\n\
  -e exit_num  - exit on error exit_num, 0 is ignore errors, 1 is default.\n\
  -E           - print cmd line examples and exit\n\
  -f format    - define format of bad buffer: h(hex), o(octal)\n\
                 d(decimal), a(ascii), n (none). hex is default\n\
	         option size can be added to control output\n\
  -i #writes   - number write per child, zero means forever.\n\
  -I io_type   - Specifies io type: s - sync, p - polled async, a - async (def s)\n\
                 l - listio sync, L - listio async, r - random\n\
  -l           - loop forever (implied by -n 0).\n\
  -n #writes   - same as -i (for compatability).\n\
  -p num_rpt   - number of reads before a report\n\
  -q           - quiet mode, no PASS results are printed\n\
  -s size      - size of read and write (def 327)\n\
                 if size >= 4096, i/o will be in 4096 chuncks\n\
  -w max_wait  - max time (seconds) for sleep between writes.\n\
                 max_wait is interpreted as a double with ms accuracy.\n\
  -W max_wait  - max time (seconds) for sleep between reads\n\
                 max_wait is interpreted as a double with ms accuracy.\n\
  -u           - un-named pipe instead of named pipe\n\
  -v           - verbose mode, all writes/reads resutlts printed\n");

	fflush(stdout);
}

static void prt_buf(char **addr, char *buf, int length, int format)
{
	int i;
	int num_words = length / NBPW;	/* given length in bytes, get length in words */
	int width;		/* number of columns */
	int extra_words = 0;	/* odd or even number of words */
	char *a = buf;
	char b[NBPW];
	char c[NBPW * 2];
	char *p;
	long *word;

	if (format == NO_OUT)	/* if no output wanted, return */
		return;

	if (length % NBPW)
		++num_words;	/* is length in full words? */
	if (format == ASCII) {
		width = 3;
	} else {
		width = 2;
		/* do we have an odd number of words? */
		extra_words = num_words % width;
	}
	for (i = 0; i < num_words; ++i, a += NBPW, addr++) {
		word = (long *)a;
		if (!(i % width)) {
			if (i > 0 && format != ASCII) {
				/*
				 * print the ascii equivalent of the data
				 * before beginning the next line of output.
				 */
				memset(c, 0x00, width * NBPW);
				/*
				 * get the last 2 words printed
				 */
				memcpy(c, a - (width * NBPW), width * NBPW);
				for (p = c; (p - c) < (int)(width*NBPW); ++p) {
					if (*p < '!' || *p > '~')
						*p = '.';
				}
				printf("\t%16.16s", c);
			}
			printf("\n%p: ", addr);
			/***printf("\n%7o (%d): ",addr,i);***/
		}

		switch (format) {
		case HEX:
			printf("%16.16lx ", *word);
			break;
		case DECIMAL:
			printf("%10.10ld ", *word);
			break;
		case ASCII:
			memcpy(b, a, NBPW);
			for (p = b; (p - b) < (int)NBPW; ++p) {
				if (*p < '!' || *p > '~')
					*p = '.';
			}
			printf("%8.8s ", b);
			break;
		default:
			printf("%22.22lo ", *word);
			break;
		}
	}
	if (format != ASCII) {
		/*
		 * print the ascii equivalent of the last words in the buffer
		 * before returning.
		 */
		memset(c, 0x00, width * NBPW);
		if (extra_words)
			width = extra_words;	/* odd number of words */
		memcpy(c, a - (width * NBPW), width * NBPW);
		for (p = c; (p - c) < (int)(width * NBPW); ++p) {
			if (*p < '!' || *p > '~')
				*p = '.';
		}
		if (width == 2)
			printf("\t%16.16s", c);
		else
			printf("\t\t%16.8s", c);
	}
	printf("\n");
	fflush(stdout);
}

static void prt_examples(void)
{
	printf("%s -c 5 -i 0 -s 4090 -b\n", TCID);
	printf("%s -c 5 -i 0 -s 4090 -b -u \n", TCID);
	printf("%s -c 5 -i 0 -s 4090 -b -W 3 -w 3 \n", TCID);
}

static void sig_child(int sig)
{
	int status;

	nchildcompleted++;
#if DEBUG
	#define STR	"parent: received SIGCHLD\n"
	write(STDOUT_FILENO, str, strlen(STR));
#endif
	waitpid(-1, &status, WNOHANG);
}
