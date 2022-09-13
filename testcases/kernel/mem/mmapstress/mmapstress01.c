/* IBM Corporation */
/* 01/02/2003	Port to LTP avenkat@us.ibm.com */
/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */
/*
 *   Copyright (c) International Business Machines  Corp., 2003
 *
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

#define _GNU_SOURCE 1
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <limits.h>
/*****  LTP Port        *****/
#include "test.h"
#define FAILED 0
#define PASSED 1

int local_flag = PASSED;
char *TCID = "mmapstress01";	//tmnoextend
FILE *temp;
int TST_TOTAL = 1;

int anyfail();
void ok_exit();
/*****  **      **      *****/

/*
 *  This test stresses mmaps, without dealing with fragments or anything!
 *  It forks a specified number of children,
 *  all of whom mmap the same file, make a given number of accesses
 *  to random pages in the map (reading & writing and comparing data).
 *  Then the child exits and the parent forks another to take its place.
 *  Each time a child is forked, it stats the file and maps the full
 *  length of the file.
 *
 *  This program continues to run until it either receives a SIGINT,
 *  or times out (if a timeout value is specified).  When either of
 *  these things happens, it cleans up its kids, then checks the
 *  file to make sure it has the correct data.
 *
 *  usage:
 *	tmnoextend -p nprocs [-t minutes -f filesize -S sparseoffset
 *			      -r -o -m -l -d]
 *  where:
 *	-p nprocs	- specifies the number of mapping children
 *			  to create.  (nprocs + 1 children actually
 *			  get created, since one is the writer child)
 *	-t minutes	- specifies minutes to run.  If not specified,
 *			  default is to run forever until a SIGINT
 *			  is received.
 *	-f filesize	- initial filesize (defaults to FILESIZE)
 *	-S sparseoffset - when non-zero, causes a sparse area to
 *			  be left before the data, meaning that the
 *			  actual initial file size is sparseoffset +
 *			  filesize.  Useful for testing large files.
 *			  (default is 0).
 *	-r		- randomize number of pages map children check.
 *			  (random % MAXLOOPS).  If not specified, each
 *			  child checks MAXLOOPS pages.
 *	-o		- randomize offset of file to map. (default is 0)
 *	-m		- do random msync/fsyncs as well
 *	-l		- if set, the output file is not removed on
 *			  program exit.
 *	-d		- enable debug output
 *
 *  Compile with -DLARGE_FILE to enable file sizes > 2 GB.
 */

#define MAXLOOPS	500	/* max pages for map children to write */
#define	FILESIZE	4096	/* initial filesize set up by parent */

#ifdef roundup
#undef roundup
#endif
#define roundup(x, y)	((((x)+((y)-1))/(y))*(y))

extern time_t time(time_t *);
extern char *ctime(const time_t *);
extern void *malloc(size_t);
extern long lrand48(void);
extern void srand(unsigned);
extern void srand48(long);
extern int rand(void);
extern int atoi(const char *);

char *usage =
    "-p nprocs [-t minutes -f filesize -S sparseoffset -r -o -m -l -d]";

typedef unsigned char uchar_t;
#define SIZE_MAX UINT_MAX

unsigned int initrand(void);
void finish(int sig);
void child_mapper(char *file, unsigned procno, unsigned nprocs);
int fileokay(char *file, uchar_t * expbuf);
int finished = 0;
int leavefile = 0;

int debug = 0;
#ifdef LARGE_FILE
off64_t filesize = FILESIZE;
off64_t sparseoffset = 0;
#else /* LARGE_FILE */
off_t filesize = FILESIZE;
off_t sparseoffset = 0;
#endif /* LARGE_FILE */
unsigned randloops = 0;
unsigned dosync = 0;
unsigned do_offset = 0;
unsigned pattern = 0;

int main(int argc, char *argv[])
{
	char *progname;
	int fd;
	int c;
	extern char *optarg;
	unsigned nprocs = 0;
	unsigned procno;
	pid_t *pidarray = NULL;
	pid_t pid;
	uchar_t *buf = NULL;
	unsigned int seed;
	int pagesize = sysconf(_SC_PAGE_SIZE);
	float alarmtime = 0;
	struct sigaction sa;
	unsigned i;
	int write_cnt;
	uchar_t data;
	int no_prob = 0;
	int wait_stat;
	time_t t;
#ifdef LARGE_FILE
	off64_t bytes_left;
#else /* LARGE_FILE */
	off_t bytes_left;
#endif /* LARGE_FILE */
	const char *filename = "mmapstress01.out";

	progname = *argv;
	tst_tmpdir();
	if (argc < 2) {
		tst_brkm(TBROK, NULL, "usage: %s %s", progname, usage);
	}

	while ((c = getopt(argc, argv, "S:omdlrf:p:t:")) != -1) {
		switch (c) {
		case 'd':
			debug = 1;
			break;
		case 't':
			alarmtime = atof(optarg) * 60;
			break;
		case 'p':
			nprocs = atoi(optarg);
			break;
		case 'l':
			leavefile = 1;
			break;
		case 'f':
#ifdef LARGE_FILE
			filesize = atoll(optarg);
#else /* LARGE_FILE */
			filesize = atoi(optarg);
#endif /* LARGE_FILE */
			if (filesize < 0) {
				(void)fprintf(stderr, "error: negative "
					      "filesize\n");
				anyfail();
			}
			break;
		case 'r':
			randloops = 1;
			break;
		case 'm':
			dosync = 1;
			break;
		case 'o':
			do_offset = 1;
			break;
		case 'S':
#ifdef LARGE_FILE
			sparseoffset = atoll(optarg);
#else /* LARGE_FILE */
			sparseoffset = atoi(optarg);
#endif /* LARGE_FILE */
			if (sparseoffset % pagesize != 0) {
				fprintf(stderr,
					"sparseoffset must be pagesize multiple\n");
				anyfail();
			}
			break;
		default:
			(void)fprintf(stderr, "usage: %s %s\n", progname,
				      usage);
			tst_exit();
		}
	}

	/* nprocs is >= 0 since it's unsigned */
	if (nprocs > 255) {
		(void)fprintf(stderr, "invalid nprocs %d - (range 0-255)\n",
			      nprocs);
		anyfail();
	}

	(void)time(&t);

	seed = initrand();
	pattern = seed & 0xff;

	if (debug) {
#ifdef LARGE_FILE
		(void)printf("creating file <%s> with %Ld bytes, pattern %d\n",
			     filename, filesize, pattern);
#else /* LARGE_FILE */
		(void)printf("creating file <%s> with %ld bytes, pattern %d\n",
			     filename, filesize, pattern);
#endif /* LARGE_FILE */
		if (alarmtime)
			(void)printf("running for %f minutes\n",
				     alarmtime / 60);
		else
			(void)printf("running with no time limit\n");
	}

	/*
	 *  Plan for death by signal.  User may have specified
	 *  a time limit, in which set an alarm and catch SIGALRM.
	 *  Also catch and cleanup with SIGINT.
	 */
	sa.sa_handler = finish;
	sa.sa_flags = 0;
	if (sigemptyset(&sa.sa_mask)) {
		perror("sigemptyset error");
		goto cleanup;
	}

	if (sigaction(SIGINT, &sa, 0) == -1) {
		perror("sigaction error SIGINT");
		goto cleanup;
	}
	if (sigaction(SIGQUIT, &sa, 0) == -1) {
		perror("sigaction error SIGQUIT");
		goto cleanup;
	}
	if (sigaction(SIGTERM, &sa, 0) == -1) {
		perror("sigaction error SIGTERM");
		goto cleanup;
	}

	if (alarmtime) {
		if (sigaction(SIGALRM, &sa, 0) == -1) {
			perror("sigaction error");
			goto cleanup;
		}
		(void)alarm(alarmtime);
	}
#ifdef LARGE_FILE
	if ((fd = open64(filename, O_CREAT | O_TRUNC | O_RDWR, 0664)) == -1) {
#else /* LARGE_FILE */
	if ((fd = open(filename, O_CREAT | O_TRUNC | O_RDWR, 0664)) == -1) {
#endif /* LARGE_FILE */
		perror("open error");
		anyfail();
	}

	if ((buf = malloc(pagesize)) == NULL
	    || (pidarray = malloc(nprocs * sizeof(pid_t))) == NULL) {
		perror("malloc error");
		anyfail();
	}

	for (i = 0; i < nprocs; i++)
		*(pidarray + i) = 0;

	for (i = 0, data = 0; i < pagesize; i++) {
		*(buf + i) = (data + pattern) & 0xff;
		if (++data == nprocs)
			data = 0;
	}
#ifdef LARGE_FILE
	if (lseek64(fd, sparseoffset, SEEK_SET) < 0) {
#else /* LARGE_FILE */
	if (lseek(fd, sparseoffset, SEEK_SET) < 0) {
#endif /* LARGE_FILE */
		perror("lseek");
		anyfail();
	}
	for (bytes_left = filesize; bytes_left; bytes_left -= c) {
		write_cnt = MIN(pagesize, (int)bytes_left);
		if ((c = write(fd, buf, write_cnt)) != write_cnt) {
			if (c == -1) {
				perror("write error");
			} else {
				(void)fprintf(stderr, "write: wrote %d of %d "
					      "bytes\n", c, write_cnt);
			}
			(void)close(fd);
			(void)unlink(filename);
			anyfail();
		}
	}

	(void)close(fd);

	/*
	 *  Fork off mmap children.
	 */
	for (procno = 0; procno < nprocs; procno++) {
		switch (pid = fork()) {

		case -1:
			perror("fork error");
			goto cleanup;

		case 0:
			child_mapper(filename, procno, nprocs);
			exit(0);

		default:
			pidarray[procno] = pid;
		}
	}

	/*
	 *  Now wait for children and refork them as needed.
	 */

	while (!finished) {
		pid = wait(&wait_stat);
		/*
		 *  Block signals while processing child exit.
		 */

		if (sighold(SIGALRM) || sighold(SIGINT)) {
			perror("sighold error");
			goto cleanup;
		}

		if (pid != -1) {
			/*
			 *  Check exit status, then refork with the
			 *  appropriate procno.
			 */
			if (!WIFEXITED(wait_stat)
			    || WEXITSTATUS(wait_stat) != 0) {
				(void)fprintf(stderr, "child exit with err "
					      "<x%x>\n", wait_stat);
				goto cleanup;
			}
			for (i = 0; i < nprocs; i++)
				if (pid == pidarray[i])
					break;
			if (i == nprocs) {
				(void)fprintf(stderr, "unknown child pid %d, "
					      "<x%x>\n", pid, wait_stat);
				goto cleanup;
			}

			if ((pid = fork()) == -1) {
				perror("fork error");
				pidarray[i] = 0;
				goto cleanup;
			} else if (pid == 0) {	/* child */
				child_mapper(filename, i, nprocs);
				exit(0);
			} else
				pidarray[i] = pid;
		} else {
			/*
			 *  wait returned an error.  If EINTR, then
			 *  normal finish, else it's an unexpected
			 *  error...
			 */
			if (errno != EINTR || !finished) {
				perror("unexpected wait error");
				goto cleanup;
			}
		}
		if (sigrelse(SIGALRM) || sigrelse(SIGINT)) {
			perror("sigrelse error");
			goto cleanup;
		}
	}

	/*
	 *  Finished!  Check the file for sanity, then kill all
	 *  the children and done!.
	 */

	if (sighold(SIGALRM)) {
		perror("sighold error");
		goto cleanup;
	}
	(void)alarm(0);
	no_prob = 1;

cleanup:
	for (i = 0; i < nprocs; i++)
		(void)kill(pidarray[i], SIGKILL);

	while (wait(&wait_stat) != -1 || errno != ECHILD)
		continue;

	if (no_prob) {		/* only check file if no errors */
		if (!fileokay(filename, buf)) {
			(void)fprintf(stderr, "file data incorrect!\n");
			(void)printf("  leaving file <%s>\n", filename);
			/***** LTP Port *****/
			local_flag = FAILED;
			anyfail();
			/*****	**	*****/
		} else {
			(void)printf("file data okay\n");
			if (!leavefile)
				(void)unlink(filename);
		}
	} else
		(void)printf("  leaving file <%s>\n", filename);

	(void)time(&t);
	//(void)printf("%s: Finished %s", argv[0], ctime(&t)); LTP Port
	ok_exit();
	tst_exit();
}

/*
 *  Child process that reads/writes map.  The child stats the file
 *  to determine the size, maps the size of the file, then reads/writes
 *  its own locations on random pages of the map (its locations being
 *  determined based on nprocs & procno).  After a specific number of
 *  iterations, it exits.
 */
void child_mapper(char *file, unsigned procno, unsigned nprocs)
{
#ifdef LARGE_FILE
	struct stat64 statbuf;
	off64_t filesize;
	off64_t offset;
#else /* LARGE_FILE */
	struct stat statbuf;
	off_t filesize;
	off_t offset;
#endif /* LARGE_FILE */
	size_t validsize;
	size_t mapsize;
	char *maddr = NULL, *paddr;
	int fd;
	size_t pagesize = sysconf(_SC_PAGE_SIZE);
	unsigned randpage;
	unsigned int seed;
	unsigned loopcnt;
	unsigned nloops;
	unsigned mappages;
	unsigned i;

	seed = initrand();	/* initialize random seed */

#ifdef LARGE_FILE
	if (stat64(file, &statbuf) == -1) {
#else /* LARGE_FILE */
	if (stat(file, &statbuf) == -1) {
#endif /* LARGE_FILE */
		perror("stat error");
		anyfail();
	}
	filesize = statbuf.st_size;

#ifdef LARGE_FILE
	if ((fd = open64(file, O_RDWR)) == -1) {
#else /* LARGE_FILE */
	if ((fd = open(file, O_RDWR)) == -1) {
#endif /* LARGE_FILE */
		perror("open error");
		anyfail();
	}

	if (statbuf.st_size - sparseoffset > SIZE_MAX) {
		fprintf(stderr, "size_t overflow when setting up map\n");
		anyfail();
	}
	mapsize = (size_t) (statbuf.st_size - sparseoffset);
	mappages = roundup(mapsize, pagesize) / pagesize;
	offset = sparseoffset;
	if (do_offset) {
		int pageoffset = lrand48() % mappages;
		int byteoffset = pageoffset * pagesize;
		offset += byteoffset;
		mapsize -= byteoffset;
		mappages -= pageoffset;
	}
	nloops = (randloops) ? (lrand48() % MAXLOOPS) : MAXLOOPS;

	if (debug) {
#ifdef LARGE_FILE
		(void)printf("child %d (pid %ld): seed %d, fsize %Ld, "
			     "mapsize %d, off %Ld, loop %d\n",
			     procno, getpid(), seed, filesize, mapsize,
			     offset / pagesize, nloops);
#else /* LARGE_FILE */
		(void)printf("child %d (pid %d): seed %d, fsize %ld, "
			     "mapsize %ld, off %ld, loop %d\n",
			     procno, getpid(), seed, filesize, (long)mapsize,
			     offset / pagesize, nloops);
#endif /* LARGE_FILE */
	}
#ifdef LARGE_FILE
	if ((maddr = mmap64(0, mapsize, PROT_READ | PROT_WRITE, MAP_SHARED,
			    fd, offset)) == (caddr_t) - 1) {
#else /* LARGE_FILE */
	if ((maddr = mmap(0, mapsize, PROT_READ | PROT_WRITE, MAP_SHARED,
			  fd, offset)) == (caddr_t) - 1) {
#endif /* LARGE_FILE */
		perror("mmap error");
		anyfail();
	}

	(void)close(fd);

	/*
	 *  Now loop read/writing random pages.
	 */
	for (loopcnt = 0; loopcnt < nloops; loopcnt++) {
		randpage = lrand48() % mappages;
		paddr = maddr + (randpage * pagesize);	/* page address */

		if (randpage < mappages - 1 || !(mapsize % pagesize))
			validsize = pagesize;
		else
			validsize = mapsize % pagesize;

		for (i = procno; i < validsize; i += nprocs) {
			if (*((unsigned char *)(paddr + i))
			    != ((procno + pattern) & 0xff)) {
				(void)fprintf(stderr, "child %d: invalid data "
					      "<x%x>", procno,
					      *((unsigned char *)(paddr + i)));
				(void)fprintf(stderr, " at pg %d off %d, exp "
					      "<x%x>\n", randpage, i,
					      (procno + pattern) & 0xff);
				anyfail();
			}

			/*
			 *  Now write it.
			 */
			*(paddr + i) = (procno + pattern) & 0xff;
		}
	}
	if (dosync) {
		/*
		 * Exercise msync() as well!
		 */
		randpage = lrand48() % mappages;
		paddr = maddr + (randpage * pagesize);	/* page address */
		if (msync(paddr, (mappages - randpage) * pagesize,
			  MS_SYNC) == -1) {
			anyfail();
		}
	}
	if (munmap(maddr, mapsize) == -1) {
		perror("munmap failed");
		local_flag = FAILED;
		anyfail();
	}
	exit(0);
}

/*
 *  Make sure file has all the correct data.
 */
int fileokay(char *file, uchar_t * expbuf)
{
#ifdef LARGE_FILE
	struct stat64 statbuf;
#else /* LARGE_FILE */
	struct stat statbuf;
#endif /* LARGE_FILE */
	size_t mapsize;
	unsigned mappages;
	unsigned pagesize = sysconf(_SC_PAGE_SIZE);
	uchar_t readbuf[pagesize];
	int fd;
	int cnt;
	unsigned i, j;

#ifdef LARGE_FILE
	if ((fd = open64(file, O_RDONLY)) == -1) {
#else /* LARGE_FILE */
	if ((fd = open(file, O_RDONLY)) == -1) {
#endif /* LARGE_FILE */
		perror("open error");
		/***** LTP Port *****/
		local_flag = FAILED;
		anyfail();
		/*****	**	*****/
		return 0;
	}
#ifdef LARGE_FILE
	if (fstat64(fd, &statbuf) == -1) {
#else /* LARGE_FILE */
	if (fstat(fd, &statbuf) == -1) {
#endif /* LARGE_FILE */
		perror("stat error");
		/***** LTP Port *****/
		local_flag = FAILED;
		anyfail();
		/*****	**	*****/
		return 0;
	}
#ifdef LARGE_FILE
	if (lseek64(fd, sparseoffset, SEEK_SET) < 0) {
#else /* LARGE_FILE */
	if (lseek(fd, sparseoffset, SEEK_SET) < 0) {
#endif /* LARGE_FILE */
		perror("lseek");
		anyfail();
	}

	if (statbuf.st_size - sparseoffset > SIZE_MAX) {
		fprintf(stderr, "size_t overflow when setting up map\n");
		anyfail();
	}
	mapsize = (size_t) (statbuf.st_size - sparseoffset);

	mappages = roundup(mapsize, pagesize) / pagesize;

	for (i = 0; i < mappages; i++) {
		cnt = read(fd, readbuf, pagesize);
		if (cnt == -1) {
			perror("read error");
			/***** LTP Port *****/
			local_flag = FAILED;
			anyfail();
			/*****	**	*****/
			return 0;
		} else if (cnt != pagesize) {
			/*
			 *  Okay if at last page in file...
			 */
			if ((i * pagesize) + cnt != mapsize) {
				(void)fprintf(stderr, "read %d of %ld bytes\n",
					      (i * pagesize) + cnt,
					      (long)mapsize);
				close(fd);
				return 0;
			}
		}
		/*
		 *  Compare read bytes of data.
		 */
		for (j = 0; j < cnt; j++) {
			if (expbuf[j] != readbuf[j]) {
				(void)fprintf(stderr,
					      "read bad data: exp %c got %c)",
					      expbuf[j], readbuf[j]);
#ifdef LARGE_FILE
				(void)fprintf(stderr, ", pg %d off %d, "
					      "(fsize %Ld)\n", i, j,
					      statbuf.st_size);
#else /* LARGE_FILE */
				(void)fprintf(stderr, ", pg %d off %d, "
					      "(fsize %ld)\n", i, j,
					      statbuf.st_size);
#endif /* LARGE_FILE */
				close(fd);
				return 0;
			}
		}
	}
	close(fd);

	return 1;
}

 /*ARGSUSED*/ void finish(int sig)
{
	finished++;
	return;
}

unsigned int initrand(void)
{
	unsigned int seed;

	/*
	 *  Initialize random seed...  Got this from a test written
	 *  by scooter:
	 *      Use srand/rand to diffuse the information from the
	 *      time and pid.  If you start several processes, then
	 *      the time and pid information don't provide much
	 *      variation.
	 */
	srand((unsigned int)getpid());
	seed = rand();
	srand((unsigned int)time(NULL));
	seed = (seed ^ rand()) % 100000;
	srand48((long int)seed);
	return (seed);
}

/*****  LTP Port        *****/
void ok_exit(void)
{
	tst_resm(TPASS, "Test passed");
	tst_rmdir();
	tst_exit();
}

int anyfail(void)
{
	tst_brkm(TFAIL, tst_rmdir, "Test failed");
}

/*****  **      **      *****/
