/* IBM Corporation */
/* 01/02/2003	Port to LTP avenakt@us.ibm.com */
/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */
/*
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
#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
/*****  LTP Port        *****/
#include "test.h"
#define FAILED 0
#define PASSED 1

int local_flag = PASSED;
char *TCID = "mmapstress09";
FILE *temp;
int TST_TOTAL = 1;

int anyfail();
void ok_exit();
/*****  **      **      *****/

/*
 *  This test is mostly duplicated from the tmmap test, but tests
 *  stress tests anonymous maps.  It forks a specified number of children,
 *  who inherit an anonymous shared map, and who, make a given number of
 *  accesses to random pages in the map (reading & writing and comparing data).
 *  Then the child exits and the parent forks another to take its place.
 *  The test fails if a child sees incorrect data.
 *
 *  This program continues to run until it either receives a SIGINT,
 *  or times out (if a timeout value is specified).  When either of
 *  these things happens, it cleans up its kids, then checks
 *  the map to make sure it has the correct data.
 *
 *  usage:
 *	mmapstress09 -p nprocs [-t minutes -s mapsize -m msync -r -d]
 *
 *  where:
 *	-p nprocs	- specifies the number of mapping children
 *			  to create.  (nprocs + 1 children actually
 *			  get created, since one is the writer child)
 *	-t minutes	- specifies minutes to run.  If not specified,
 *			  default is to run forever until a SIGINT
 *			  is received.
 *	-s mapsize	- mapsize (defaults to MAPSIZE)
 *	-m 		- do msyncs
 *	-r		- randomize number of pages map children check.
 *			  (random % MAXLOOPS).  If not specified, each
 *			  child checks MAXLOOPS pages.
 *	-d		- enable debug outputd
 */

#define MAXLOOPS	500	/* max pages for map children to write */
#define	MAPSIZE		(64*1024)	/* default mapsize set up by parent */
#ifdef roundup
#undef roundup
#endif
#define roundup(x, y)	((((x)+((y)-1))/(y))*(y))
#define min(x, y)	(((x) < (y)) ? (x) : (y))

extern time_t time(time_t *);
extern char *ctime(const time_t *);
extern void *malloc(size_t);
extern void exit(int);
extern long lrand48(void);
extern void srand(unsigned);
extern void srand48(long);
extern int rand(void);
extern int atoi(const char *);

typedef unsigned char uchar_t;

char *usage = "-p nprocs [-t minutes -s mapsize -m -r -d]";

unsigned int initrand(void);
void finish(int sig);
void child_mapper(unsigned procno, unsigned nprocs);
int mapokay(uchar_t * expbuf);

int finished = 0;
int debug = 0;
int mapsize = MAPSIZE;
unsigned mappages;
int pagesize;
unsigned randloops = 0;
unsigned dosync = 0;
unsigned pattern = 0;
caddr_t mapaddr;

int main(int argc, char *argv[])
{
	char *progname;
	unsigned c;
	extern char *optarg;
	unsigned nprocs = 0;
	unsigned procno;
	pid_t *pidarray = NULL;
	pid_t pid;
	uchar_t *buf, *ptr;
	unsigned int seed;
	float alarmtime = 0;
	struct sigaction sa;
	unsigned i, j;
	uchar_t data;
	int no_prob = 0;
	time_t t;
	int wait_stat;

	progname = *argv;
	pagesize = sysconf(_SC_PAGE_SIZE);

	if (argc < 2) {
		(void)fprintf(stderr, "usage: %s %s\n", progname, usage);
		anyfail();
	}

	while ((c = getopt(argc, argv, "mdrp:t:s:")) != -1) {
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
		case 'm':
			dosync = 1;
			break;
		case 's':
			mapsize = atoi(optarg);
			if (mapsize < 0) {
				(void)fprintf(stderr, "error: negative "
					      "mapsize\n");
				anyfail();
			}
			break;
		case 'r':
			randloops = 1;
			break;
		default:
			(void)fprintf(stderr, "usage: %s %s\n", progname,
				      usage);
			anyfail();
		}
	}

	/* nprocs is unsigned */
	if (nprocs > 255) {
		(void)fprintf(stderr, "invalid nprocs %d - (range 0-255)\n",
			      nprocs);
		anyfail();
	}
	(void)time(&t);
//      (void)printf("%s: Started %s", argv[0], ctime(&t)); LTP Port

	seed = initrand();
	pattern = seed & 0xff;

	if (debug) {
		(void)printf("%s mapsize %d bytes, pattern %d\n",
			     progname, mapsize, pattern);
		if (alarmtime)
			(void)printf("running for %f minutes\n",
				     alarmtime / 60);
		else
			(void)printf("running with no time limit\n");
	}

	if ((mapaddr = mmap(0, mapsize, PROT_READ | PROT_WRITE,
			    MAP_ANONYMOUS | MAP_SHARED, 0, 0))
	    == (caddr_t) - 1) {
		perror("mmap error");
		anyfail();
	}

	if ((buf = malloc(pagesize)) == NULL
	    || (pidarray = malloc(nprocs * sizeof(pid_t))) == NULL) {
		perror("malloc error");
		anyfail();
	}

	for (i = 0; i < nprocs; i++)
		*(pidarray + i) = 0;

	/*
	 * Initialize page compare buffer, then initialize map.
	 */

	for (i = 0, data = 0; i < pagesize; i++) {
		*(buf + i) = (data + pattern) & 0xff;
		if (++data == nprocs)
			data = 0;
	}

	mappages = roundup(mapsize, pagesize) / pagesize;
	ptr = (uchar_t *) mapaddr;

	for (i = 0; i < mappages; i++) {
		for (j = 0; j < pagesize; j++)
			*ptr++ = *(buf + j);
	}

	/*
	 *  Fork off mmap children.
	 */
	for (procno = 0; procno < nprocs; procno++) {
		switch (pid = fork()) {

		case -1:
			perror("fork error");
			goto cleanup;

		case 0:
			child_mapper(procno, nprocs);
			exit(0);

		default:
			pidarray[procno] = pid;
		}
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
		perror("sigaction error");
		goto cleanup;
	}

	if (alarmtime) {
		if (sigaction(SIGALRM, &sa, 0) == -1) {
			perror("sigaction error");
			goto cleanup;
		}
		(void)alarm(alarmtime);
	}

	/*
	 *  Now wait for children and refork them as needed.
	 */

	while (!finished) {
		do {
			pid = wait(&wait_stat);
		} while (pid == -1 && errno == EINTR);
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
				(void)fprintf(stderr,
					      "unknown child pid %d, <x%x>\n",
					      pid, wait_stat);
				goto cleanup;
			}

			if ((pid = fork()) == -1) {
				perror("fork error");
				pidarray[i] = 0;
				goto cleanup;
			} else if (pid == 0) {	/* child */
				child_mapper(i, nprocs);
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
	 *  Finished!  Check the map for sanity, then kill all
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
		(void)kill(pidarray[i], SIGKILL);	/* failure? oh well. */

	while (wait(&wait_stat) != -1 || errno != ECHILD)
		continue;

	if (no_prob) {		/* only check file if no errors */
		if (!mapokay(buf)) {
			(void)fprintf(stderr, "map data incorrect!\n");
			anyfail();
		} else
			(void)printf("map data okay\n");
	}

	(void)time(&t);
//      (void)printf("%s: Finished %s", argv[0], ctime(&t)); LTP POrt
	ok_exit();
	tst_exit();
}

/*
 *  Child process that reads/writes map.  The child reads/writes
 *  its own locations on random pages of the map (its locations being
 *  determined based on nprocs & procno).  After a specific number of
 *  iterations, it exits.
 */
void child_mapper(unsigned procno, unsigned nprocs)
{
	uchar_t *paddr;
	unsigned randpage;
	unsigned int seed;
	unsigned loopcnt;
	unsigned nloops;
	unsigned i;

	seed = initrand();	/* initialize random seed */

	nloops = (randloops) ? (lrand48() % MAXLOOPS) : MAXLOOPS;

	if (debug)
		(void)printf("child %d (pid %d): seed %d, loop %d\n",
			     procno, getpid(), seed, nloops);

	/*
	 *  Now loop read/writing random pages.
	 */

	for (loopcnt = 0; loopcnt < nloops; loopcnt++) {
		randpage = lrand48() % mappages;
		/* find the page address */
		paddr = (uchar_t *) (mapaddr + (randpage * pagesize));

		for (i = procno; i < pagesize; i += nprocs) {
			if (*((unsigned char *)(paddr + i))
			    != ((procno + pattern) & 0xff)) {
				(void)fprintf(stderr,
					      "child %d: invalid data <x%x>",
					      procno,
					      *((unsigned char *)(paddr + i)));
				(void)fprintf(stderr,
					      " at pg %d off %d, exp <x%x>\n",
					      randpage, i,
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
		randpage = (unsigned)lrand48() % mappages;
		paddr = (uchar_t *) mapaddr + (randpage * pagesize);
		if (msync((caddr_t) paddr, (mappages - randpage) * pagesize,
			  MS_SYNC) == -1) {
			perror("msync error");
			anyfail();
		}
	}

	exit(0);
}

/*
 *  Make sure file has all the correct data.
 */
int mapokay(uchar_t * expbuf)
{
	uchar_t *ptr;
	unsigned i, j;

	ptr = (uchar_t *) mapaddr;
	for (i = 0; i < mappages; i++) {
		/*
		 *  Compare read bytes of data.
		 */
		for (j = 0; j < pagesize; j++) {
			if (*ptr != expbuf[j]) {
				(void)fprintf(stderr,
					      "bad map data: exp %c got %c)",
					      expbuf[j], *ptr);
				(void)fprintf(stderr, ", pg %d off %d\n", i, j);
				anyfail();
			}
			ptr++;
		}
	}

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
	tst_resm(TPASS, "Test passed\n");
	tst_exit();
}

int anyfail(void)
{
	tst_brkm(TFAIL, NULL, "Test failed\n");
}

/*****  **      **      *****/
