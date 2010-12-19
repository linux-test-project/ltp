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
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
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

char *TCID="pipeio"; 		/* Test program identifier.    */
int TST_TOTAL=1;    		/* Total number of test cases. */

#define SAFE_FREE(p) { if (p) { free(p); (p)=NULL; } }

/* To avoid extensive modifications to the code, use this bodge */
#define exit(x) myexit(x)
void
myexit (int x)
{
  if (x)
    tst_resm (TFAIL, "Test failed");
  else
    tst_resm (TPASS, "Test passed");
 tst_exit();
}

#if defined(__linux__)
# define NBPW sizeof(int)
#endif

#define PPATH "tpipe"
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

void sig_handler(), help(), usage(), prt_buf(), prt_examples();
void sig_child();

int count = 0;
int Nchildcomplete = 0;

/*
 * Ensure PATH_MAX is define
 */
#ifndef PATH_MAX
#ifdef MAXPATHLEN
#define PATH_MAX MAXPATHLEN
#else
#define PATH_MAX 1024
#endif /* ! MAXPATHLEN */
#endif /* PATH_MAX */

int
main(ac,av)
int ac;
char *av[];
{
	int i,j,c,error = 0;
	int n;
	int nb;			/* number of bytes read */
	int num_wrters=1;	/* number of writers */
	int num_writes=1;	/* number of writes per child */
	int loop=0;		/* loop indefinitely */
	int exit_error = 1;	/* exit on error #, zero means no exit */
	int size=327;		/* default size */
	int unpipe = 0;		/* un-named pipe if non-zero */
	int verbose=0;		/* verbose mode if set */
	int quiet=0;		/* quiet mode if set */
	int num_rpt=0;		/* ping number, how often to print message */
	int chld_wait = 0;	/* max time to wait between writes, 1 == no wait */
	int parent_wait = 0;	/* max time to wait between reads, 1 == no wait*/
	int ndelay = O_NDELAY;	/* additional flag to open */
	long clock;
	char *writebuf = NULL;
	char *readbuf = NULL;
	double d;
	char   *cp;
        extern char *optarg;
	char pname[PATH_MAX];	/* contains the name of the unamed pipe */
	char dir[PATH_MAX];	/* directory to create pipe in		*/
	char *blk_type;		/* blocking i/o or not */
	char *pipe_type;	/* type of pipe under test */
	int fds[2];		/* un-named pipe fds */
	int read_fd = 0;
	int write_fd = 0;
	int empty_read = 0;
	time_t start_time, current_time, diff_time;	/* start time, current time, diff of times */
	int *count_word;	/* holds address where to write writers count */
	int *pid_word;		/* holds address where to write writers pid */
	int format;
	int format_size = -1;
	int background = 0;	/* if set, put process in background */
	struct stat stbuf;
        int iotype = 0;		/* sync io */
	char *toutput;		/* for getenv() */
	int sem_id;
	struct sembuf sem_op;
	union semun {		/* for semctl() */
	  int val;
	  struct semid_ds *buf;
	  unsigned short int *array;
	} u;

	u.val = 0;
	format = HEX;
	blk_type = NON_BLOCKING_IO;
	dir[0] = '\0';
	sprintf(pname,"%s.%d",PPATH,getpid());

	if ((toutput = getenv("TOUTPUT")) != NULL) {
	    if (strcmp(toutput, "NOPASS") == 0) {
		quiet=1;
	    }
	}

        while ((c=getopt (ac, av, "T:BbCc:D:d:he:Ef:i:I:ln:p:qs:uvW:w:P:")) != EOF) {
	    switch (c) {
		case 'T':
			TCID = optarg;
			break;
		case 'h':
			help();
			exit(0);
			break;
		case 'd':	/* dir name */
			strcpy(dir, optarg);
			break;
		case 'D':	/* pipe name */
			strcpy(pname, optarg);
			break;
		case 'B':	/* background */
			background=1;
			break;
		case 'b':	/* blocked */
			ndelay=0;
			blk_type = BLOCKING_IO;
			break;
		case 'C':
			fprintf(stderr,
			    "%s: --C option not supported on this architecture\n",
			    TCID);
			exit(1);
			break;
		case 'c':	/* number childern */
			if (sscanf(optarg, "%d", &num_wrters) != 1) {
				fprintf(stderr,"%s: --c option invalid arg '%s'.\n",
					TCID,optarg);
				usage();
				exit(1);
			}
			else if (num_wrters <= 0) {
				fprintf(stderr,
				    "%s: --c option must be greater than zero.\n",
				    TCID);
				usage();
				exit(1);
			}
			break;
		case 'e':	/* exit on error # */
			if (sscanf(optarg, "%d", &exit_error) != 1) {
				fprintf(stderr,"%s: --e option invalid arg '%s'.\n",
					TCID,optarg);
				usage();
				exit(1);
			}
			else if (exit_error < 0) {
				fprintf(stderr,
				    "%s: --e option must be greater than zero.\n",
				    TCID);
				usage();
				exit(1);
			}
			break;

		case 'E':
			prt_examples();
			exit(0);
			break;
		case 'f':	/* format of buffer on error */
			switch(optarg[0]) {
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
			case 'n':		/* not output */
			case 'N':
				format = NO_OUT;
				break;

			default :
				fprintf(stderr,"%s: --f option invalid arg '%s'.\n",
					TCID,optarg);
				fprintf(stderr,
				    "\tIt must be x(hex), o(octal), d(decimal), a(ascii) or n(none) with opt sz\n");
				exit(1);
				break;
			}
			cp = optarg;
			cp++;
			if (*cp) {
			    if (sscanf(cp, "%i", &format_size) != 1) {
				fprintf(stderr,"%s: --f option invalid arg '%s'.\n",
					TCID,optarg);
				fprintf(stderr,
				    "\tIt must be x(hex), o(octal), d(decimal), a(ascii) or n(none) with opt sz\n");
				exit(1);
				break;
			    }
			}
			break;

    		case 'I':
     			if ((iotype=lio_parse_io_arg1(optarg)) == -1) {
         		    fprintf(stderr,
             			"%s: --I arg is invalid, must be s, p, f, a, l, L or r.\n",
             			TCID);
         		    exit(1);
     			}
			break;

		case 'l':	/* loop forever */
			++loop;
			break;

		case 'i':
		case 'n':	/* number writes per child */
			if (sscanf(optarg, "%d", &num_writes) != 1) {
				fprintf(stderr,"%s: --i/n option invalid arg '%s'.\n",
					TCID,optarg);
				usage();
				exit(1);
			}
			else if (num_writes < 0) {
				fprintf(stderr,
				    "%s: --i/n option must be greater than equal to zero.\n",
				    TCID);
				usage();
				exit(1);
			}

			if (num_writes == 0)	/* loop forever */
				++loop;

			break;
		case 'P':	/* panic flag */
			fprintf(stderr, "%s: --P not supported on this architecture\n",
			    TCID);
			exit(1);
			break;
		case 'p':	/* ping */
			if (sscanf(optarg, "%d", &num_rpt) != 1) {
				fprintf(stderr,"%s: --p option invalid arg '%s'.\n",
					TCID,optarg);
				usage();
				exit(1);
			}
			else if (num_rpt < 0) {
				fprintf(stderr,
					"%s: --p option must be greater than equal to zero.\n",
					TCID);
				usage();
				exit(1);
			}
			break;
	        case 'q':	/* Quiet - NOPASS */
			quiet=1;
			break;
		case 's':	/* size */
			if (sscanf(optarg, "%d", &size) != 1) {
				fprintf(stderr,"%s: --s option invalid arg '%s'.\n",
					TCID,optarg);
				usage();
				exit(1);
			}
			else if (size <= 0) {
				fprintf(stderr,
				    "%s: --s option must be greater than zero.\n",
				    TCID);
				usage();
				exit(1);
			}
			break;
		case 'u':
			unpipe=1;	/* un-named pipe */
			break;
		case 'v':	/* verbose */
			verbose=1;
			break;
		case 'W':	/* max wait time between writes */
			d = strtod(optarg, &cp);
			if (*cp != '\0') {
				fprintf(stderr,"%s: --w option invalid arg '%s'.\n",
					TCID,optarg);
				usage();
				exit(1);
			}
			else if (d < 0) {
				fprintf(stderr,
				    "%s: --w option must be greater than zero.\n",
				    TCID);
				usage();
				exit(1);
			}
			parent_wait = (int)(d * 1000000.0);
			break;
		case 'w':	/* max wait time between writes */
			d = strtod(optarg, &cp);
			if (*cp != '\0') {
				fprintf(stderr,"%s: --w option invalid arg '%s'.\n",
					TCID,optarg);
				usage();
				exit(1);
			}
			else if (d < 0) {
				fprintf(stderr,
				    "%s: --w option must be greater than zero.\n",
				    TCID);
				usage();
				exit(1);
			}
			chld_wait = (int)(d * 1000000.0);
			break;
		case '?':
			usage();
			exit (1);
			break;
	    }
	}

	if (format_size == -1)
		format_size = size;

	/*
	 *
	 * If there is more than one writer, all writes and reads
	 * must be the same size.  Only writes of a size <= PIPE_BUF
	 * are atomic.  T
	 * Therefore, if size is greater than PIPE_BUF, we will break
	 * the writes into PIPE_BUF chunks.  We will also increase the
	 * number of writes to ensure the same (or more) amount of
	 * data is written.  This is the same as erroring and telling
	 * the user the new cmd line to do the same thing.
	 * Example:
	 *	pipeio -s 5000 -n 10 -c 5
	 *	(each child will write at least 50000 bytes, since all
	 *	writes have to be in 4096 chuncks or 13*4096 (53248)
	 *	bytes will be written.)  This is the same as:
	 *      pipeio -s 4096 -n 13 -c 5
	 */
	if (size > PIPE_BUF && num_wrters > 1) {
	    if (! loop) {
	      /* we must set num_writes s.t. num_writes*num_wrters doesn't overflow later */
		num_writes=MIN(((long long)num_writes*size+PIPE_BUF-1)/PIPE_BUF, INT_MAX/num_wrters);
	        tst_resm (TINFO, "adjusting i/o size to %d, and # of writes to %d",
		    PIPE_BUF, num_writes);
	    }
	    else {
		tst_resm (TINFO, "adjusting i/o size to %d", PIPE_BUF);
	    }
	    size=PIPE_BUF;

	}

	if ((writebuf = (char *) malloc(size)) == NULL ||
	    (readbuf = (char *) malloc(size)) == NULL) {
		tst_resm (TFAIL, "malloc() failed: %s", strerror(errno));
  		SAFE_FREE(writebuf);
		SAFE_FREE(readbuf);
		exit(1);
	}

	memset(writebuf,'Z',size);

	writebuf[size-1] = 'A';	/* to detect partial read/write problem */

	if ((sem_id = semget(IPC_PRIVATE, 1, IPC_CREAT|S_IRWXU)) == -1) {
		tst_brkm(TBROK, NULL, "Couldn't allocate semaphore: %s", strerror(errno));
	}

	if (semctl(sem_id, 0, SETVAL, u) == -1)
		tst_brkm(TBROK, NULL, "Couldn't initialize semaphore value: %s", strerror(errno));

	if (background) {
	    if ((n=fork()) == -1) {
		tst_resm (TFAIL, "fork() failed: %s", strerror(errno));
		exit(1);
	    }
	    else if (n != 0) /* parent */
		exit(0);
	}

	if (unpipe) {
		if (pipe(fds) == -1) {
			tst_resm (TFAIL, "pipe() failed to create un-named pipe: %s", strerror(errno));
			exit(1);
		}
		read_fd = fds[0];
		write_fd = fds[1];
		pipe_type = PIPE_UNNAMED;
		blk_type = UNNAMED_IO;
	} else {
		if (strlen(dir) && chdir(dir) == -1) {
			tst_resm (TFAIL, "chdir(%s) failed: %s", dir, strerror(errno));
			exit(1);
		}

		if (stat(pname, &stbuf) == -1) {

		    if (mkfifo(pname,0777) == -1) {
			tst_resm (TFAIL, "mkfifo(%s,0777) failed: %s", pname, strerror(errno));
			exit(1);
		    }
		}
		pipe_type = PIPE_NAMED;
	}

	start_time=time(0);

#if DEBUG
printf("num_wrters = %d\n", num_wrters);
#endif

#ifdef linux
		signal(SIGCHLD, sig_child);
		signal(SIGHUP, sig_handler);
		signal(SIGINT, sig_handler);
		signal(SIGQUIT, sig_handler);
#ifdef SIGRECOVERY
		signal(SIGRECOVERY, sig_handler);
#endif /* SIGRECOVERY */
#else
		sigset(SIGCHLD, sig_child);
		sigset(SIGHUP, sig_handler);
		sigset(SIGINT, sig_handler);
		sigset(SIGQUIT, sig_handler);
#ifdef SIGRECOVERY
		sigset(SIGRECOVERY, sig_handler);
#endif /* SIGRECOVERY */
#endif /* linux */

	for (i=num_wrters; i > 0; --i) {
		if ((c=fork()) < 0) {
			tst_resm (TFAIL, "fork() failed: %s", strerror(errno));
			exit(1);
		}
		if (c == 0) break;	/* stop child from forking */
	}
	if (c == 0) {	/***** if child *****/
#if DEBUG
printf("child after fork pid = %d\n", getpid());
#endif
		if (! unpipe) {
			if ((write_fd = open(pname,O_WRONLY)) == -1) {
				tst_resm (TFAIL, "child pipe open(%s, %#o) failed: %s", pname, O_WRONLY|ndelay, strerror(errno));
				exit(1);
			}
			if (ndelay && fcntl(write_fd, F_SETFL, O_NONBLOCK) == -1) {
				tst_brkm(TBROK, NULL, "Failed setting the pipe to nonblocking mode: %s", strerror(errno));
			}
		}
		else {
			close(read_fd);
		}

		sem_op = (struct sembuf) {
			.sem_num = 0,
			.sem_op = 1,
			.sem_flg = 0
		};

		if (semop(sem_id, &sem_op, 1) == -1)
			tst_brkm(TBROK, NULL, "Couldn't raise the semaphore: %s", strerror(errno));

		pid_word = (int *)&writebuf[0];
		count_word = (int *)&writebuf[NBPW];

		for (j=0; j < num_writes || loop; ++j) {

			/* writes are only in one unit when the size of the write
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
			*pid_word = getpid();

                        if ((nb = lio_write_buffer(write_fd, iotype, writebuf, size,
                                                        SIGUSR1, &cp, 0)) < 0 ) {
			/*
			 * If lio_write_buffer returns a negative number,
			 * the return will be -errno.
			 */
													tst_resm (TFAIL, "pass %d: lio_write_buffer(%s) failed; it returned %d: %s", j, cp, nb, strerror(-nb));
				exit(1);
			}
			else if (nb != size) {
				tst_resm (TFAIL, "pass %d: lio_write_buffer(%s) failed, write count %d, but expected to write %d",
				          j, cp, nb, size);
			}
		        if (verbose)
				tst_resm (TINFO, "pass %d: pid %d: wrote %d bytes, expected %d bytes",
				          j, getpid(), nb, size);

			if (chld_wait) {
                		clock=time(0);
               			srand48(clock);
                		n = lrand48() % chld_wait;
				usleep(n);
			}
			fflush(stderr);
		}
	}
	if (c > 0) {	/***** if parent *****/

		if (! unpipe) {
			if ((read_fd = open(pname,O_RDONLY)) == -1) {
				tst_resm (TFAIL, "parent pipe open(%s, %#o) failed: %s", pname, O_RDONLY, strerror(errno));
				exit(1);
			}
			if (ndelay && fcntl(read_fd, F_SETFL, O_NONBLOCK) == -1) {
				tst_brkm(TBROK, NULL, "Failed setting the pipe to nonblocking mode: %s", strerror(errno));
			}
		}
		else {
			close(write_fd);
		}

		sem_op = (struct sembuf) {
			.sem_num = 0,
			.sem_op = -num_wrters,
			.sem_flg = 0
		};

		while (Nchildcomplete < num_wrters && semop(sem_id, &sem_op, 1) == -1) {
			if (errno == EINTR) {
				continue;
			}
			tst_brkm(TBROK, NULL, "Couldn't wait on semaphore: %s", strerror(errno));
		}

		for (i=num_wrters*num_writes; i > 0 || loop; --i) {
			if (error >= MAX_ERRS || empty_read >= MAX_EMPTY)
			  break;
			if (parent_wait) {
                		clock=time(0);
                		srand48(clock);
                		n = lrand48() % parent_wait;
				usleep(n);
			}
			++count;
			if ((nb = lio_read_buffer(read_fd, iotype, readbuf, size,
                   					SIGUSR1, &cp, 0)) < 0 ) {
                        /*
                         * If lio_read_buffer returns a negative number,
                         * the return will be -errno.
                         */
				tst_resm (TFAIL, "pass %d: lio_read_buffer(%s) failed; it returned %d: %s", i, cp, nb, strerror(-nb));
			    ++i;
			    count--;
			    error++;
			    continue;

 			} else {
				if (nb == 0) {
					if (Nchildcomplete >= num_wrters) {
						if (!loop)
							tst_resm(TWARN, "The children have died prematurely");
						break; /* All children have died */
					}
					empty_read++;
/*
					fprintf(stdout,
						"%s: Nothing on the pipe (%d),read count %d (read not counted)\n",
						TCID,empty_read,count);
					fflush(stdout);
 */
					++i;
					count--;
					continue;
				} else if (nb < size && size <= PIPE_BUF) {
					tst_resm (TFAIL, "pass %d: partial read from the pipe: read %d bytes, expected %d, read count %d",
					          i, nb, size, count);
					++error;
				} else if (nb == size) {
					for (j=2*NBPW;j < size; ++j) {
						if (writebuf[j] != readbuf[j]) {
							++error;
							tst_resm (TFAIL, "1 FAIL data error on byte %d; rd# %d, sz= %d, %s %s empty_reads= %d, err= %d",
							          j, count, size, pipe_type, blk_type, empty_read, error);
							prt_buf(&readbuf,readbuf,format_size,format);
							fflush(stdout);
							if (exit_error && exit_error == error)
								goto output;

							else
								break;
						}
					}
				}
				if (verbose || (num_rpt && !(count % num_rpt))) {
					current_time = time(0);
					diff_time = current_time - start_time;	/* elapsed time */
					tst_resm (TFAIL, "(%d) rd# %d, sz= %d, %s %s empty_reads= %d, err= %d\n",
						(int)diff_time,count,size,pipe_type,blk_type,empty_read,error);
					fflush(stdout);
				}
			}
		}
    if (empty_read)
      tst_resm(TWARN, "%d empty reads", empty_read);
output:
		if (error)
			tst_resm(TFAIL, "1 FAIL %d data errors on pipe, read size = %d, %s %s",
			         error,size,pipe_type,blk_type);
		else
			if (!quiet)
				tst_resm(TPASS, "1 PASS %d pipe reads complete, read size = %d, %s %s",
				          count+1,size,pipe_type,blk_type);

		semctl(sem_id, 0, IPC_RMID);

		if (!unpipe)
			unlink(pname);
	}

 	SAFE_FREE(writebuf);
	SAFE_FREE(readbuf);
	return (error);
}

void
usage ()
{
	fprintf(stderr, "Usage: %s [-BbCEv][-c #writers][-D pname][-d dir][-h][-e exit_num][-f fmt][-l][-i #writes][-n #writes][-p num_rpt]\n\t[-s size][-W max_wait][-w max_wait][-u]\n",TCID);
	fflush(stderr);

}

void
help()
{
        usage();

    printf("  -B           - execute actions in background\n\
  -b           - blocking reads and writes. default non-block\n\
  -c #writers  - number of writers (childern)\n\
  -D pname     - name of fifo (def tpipe<pid>)\n\
  -d dir       - cd to dir before creating named pipe\n\
               - (silently ignored if used with -u)\n\
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

void
prt_buf(long addr, char * buf, int length, int format)
{

	int i;
	int num_words = length/NBPW;  /* given length in bytes, get length in words */
	int width;		/* number of columns */
	int extra_words = 0;	/* odd or even number of words */
	char *a = buf;
	char b[NBPW];
	char c[NBPW*2];
	char *p;
	long *word;

	if (format == NO_OUT)		/* if no output wanted, return */
		return;

	if (length % NBPW) ++num_words; /* is length in full words? */
	if (format == ASCII) {
	 	width = 3;
	} else {
		width = 2;
	    /* do we have an odd number of words? */
		extra_words = num_words%width;
	}
	for (i=0; i < num_words; ++i, a += NBPW, addr++) {
		word = (long *) a;
		if (!(i%width)) {
			if (i > 0 && format != ASCII) {
		   	  /*
		    	   * print the ascii equivalent of the data
			   * before beginning the next line of output.
			   */
				memset(c,0x00,width*NBPW);
			   /*
			    * get the last 2 words printed
			    */
				memcpy(c,a-(width*NBPW),width*NBPW);
				for (p = c; (p-c) < width*NBPW; ++p) {
					if (*p < '!' || *p > '~')
						*p = '.';
				}
				printf("\t%16.16s",c);
			}
			printf("\n%7lo: ",addr);
			/***printf("\n%7o (%d): ",addr,i);***/
		}

		switch (format) {
			case HEX:
				printf("%16.16lx ",*word);
				break;
			case DECIMAL:
				printf("%10.10ld ",*word);
				break;
			case ASCII:
				memcpy(b,a,NBPW);
				for (p = b; (p-b) < NBPW; ++p) {
					if (*p < '!' || *p > '~')
						*p = '.';
				}
				printf("%8.8s ",b);
				break;
			default:
				printf("%22.22lo ",*word);
				break;
		}
	}
	if (format != ASCII) {
   	  /*
    	   * print the ascii equivalent of the last words in the buffer
	   * before returning.
	   */
		memset(c,0x00,width*NBPW);
		if (extra_words) width = extra_words; /* odd number of words */
		memcpy(c,a-(width*NBPW),width*NBPW);
		for (p = c; (p-c) < width*NBPW; ++p) {
			if (*p < '!' || *p > '~')
				*p = '.';
		}
		if (width == 2)
			printf("\t%16.16s",c);
		else
			printf("\t\t%16.8s",c);
	}
	printf("\n");
	fflush(stdout);
}

void
prt_examples()
{
    printf("%s -c 5 -i 0 -s 4090 -b\n", TCID);
    printf("%s -c 5 -i 0 -s 4090 -b -u \n", TCID);
    printf("%s -c 5 -i 0 -s 4090 -b -W 3 -w 3 \n", TCID);

}

void
sig_child(int sig)
{
    int status;

    Nchildcomplete++;
#if DEBUG
printf("parent: received SIGCHLD\n");
#endif
    waitpid(-1, &status, WNOHANG);
#if linux
    signal(SIGCHLD, sig_child);
#else
    sigset(SIGCHLD, sig_child);
#endif
}

void
sig_handler(int sig)
{
#ifdef SIGRECOVERY
    if (sig == SIGRECOVERY) {
	printf("%s: received SIGRECOVERY, count = %d\n", TCID, count);
        fflush(stdout);
#ifdef linux
	signal(sig, sig_handler);
#else
	sigset(sig, sig_handler);
#endif
	return;
    }
#endif
    printf("%s: received unexpected signal: %d\n", TCID, sig);
    fflush(stdout);
    exit(3);

}