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
 * Changelog:
 *
 *	Added timer options: William Jay Huie, IBM
 *	01/27/03 - Added: Manoj Iyer, manjo@mail.utexas.edu
 *			   - option '-p' (pretty printing)i to enabled formatted printing
 *			     of results.
 *
 *	01/27/03 - Added: Manoj Iyer, manjo@mail.utexas.edu
 *			   - added code to print system information
 *
 *	01/28/03 - Added: Manoj Iyer, manjo@mail.utexas.edu
 *			   - added code to print test exit value.
 *
 *	01/29/03 - Added: Manoj Iyer, manjo@mail.utexas.edu
 *			   - added code supresses test start and test end tags.
 *
 * 	07/22/07 - Added: Ricardo Salveti de Araujo, rsalveti@linux.vnet.ibm.com
 *			   - added option to create a command file with all failed tests.
 *
 */
/* $Id: ltp-pan.c,v 1.4 2009/10/15 18:45:55 yaberauneya Exp $ */

#include <sys/param.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <errno.h>
#include <err.h>
#include <limits.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include "splitstr.h"
#include "zoolib.h"
#include "tst_res_flags.h"

/* One entry in the command line collection.  */
struct coll_entry {
	char *name;		/* tag name */
	char *cmdline;		/* command line */
	char *pcnt_f;		/* location of %f in the command line args, flag */
	struct coll_entry *next;
};

struct collection {
	int cnt;
	struct coll_entry **ary;
};

struct tag_pgrp {
	int pgrp;
	int stopping;
	time_t mystime;
	struct coll_entry *cmd;
	char output[PATH_MAX];
};

struct orphan_pgrp {
	int pgrp;
	struct orphan_pgrp *next;
};

static pid_t run_child(struct coll_entry *colle, struct tag_pgrp *active,
		       int quiet_mode, int *failcnt, int fmt_print,
		       FILE * logfile, int no_kmsg);
static char *slurp(char *file);
static struct collection *get_collection(char *file, int optind, int argc,
					 char **argv);
static void pids_running(struct tag_pgrp *running, int keep_active);
static int check_pids(struct tag_pgrp *running, int *num_active,
		      int keep_active, FILE * logfile, FILE * failcmdfile,
		      FILE *tconfcmdfile, struct orphan_pgrp *orphans,
		      int fmt_print, int *failcnt, int *tconfcnt,
		      int quiet_mode, int no_kmsg);
static void propagate_signal(struct tag_pgrp *running, int keep_active,
			     struct orphan_pgrp *orphans);
static void dump_coll(struct collection *coll);
static char *subst_pcnt_f(struct coll_entry *colle);
static void mark_orphan(struct orphan_pgrp *orphans, pid_t cpid);
static void orphans_running(struct orphan_pgrp *orphans);
static void check_orphans(struct orphan_pgrp *orphans, int sig);

static void copy_buffered_output(struct tag_pgrp *running);
static void write_test_start(struct tag_pgrp *running, int no_kmsg);
static void write_test_end(struct tag_pgrp *running, const char *init_status,
			   time_t exit_time, char *term_type, int stat_loc,
			   int term_id, struct tms *tms1, struct tms *tms2);

//wjh
static char PAN_STOP_FILE[] = "PAN_STOP_FILE";

static char *panname = NULL;
static char *test_out_dir = NULL;	/* dir to buffer output to */
zoo_t zoofile;
static char *reporttype = NULL;

/* Common format string for ltp-pan results */
#define ResultFmt	"%-50s %-10.10s"

/* zoolib */
int rec_signal;			/* received signal */
int send_signal;		/* signal to send */

/* Debug Bits */
int Debug = 0;
#define Dbuffile	0x000400	/* buffer file use */
#define	Dsetup		0x000200	/* one-time set-up */
#define	Dshutdown	0x000100	/* killed by signal */
#define	Dexit		0x000020	/* exit status */
#define	Drunning	0x000010	/* current pids running */
#define	Dstartup	0x000004	/* started command */
#define	Dstart		0x000002	/* started command */
#define Dwait		0x000001	/* wait interrupted */

int main(int argc, char **argv)
{
	extern char *optarg;
	extern int optind;
	char *zooname = NULL;	/* name of the zoo file to use */
	char *filename = "/dev/null";	/* filename to read test tags from */
	char *logfilename = NULL;
	char *failcmdfilename = NULL;
	char *tconfcmdfilename = NULL;
	char *outputfilename = NULL;
	struct collection *coll = NULL;
	struct tag_pgrp *running;
	struct orphan_pgrp *orphans, *orph;
	struct utsname unamebuf;
	FILE *logfile = NULL;
	FILE *failcmdfile = NULL;
	FILE *tconfcmdfile = NULL;
	int keep_active = 1;
	int num_active = 0;
	int failcnt = 0;  /* count of total testcases that failed. */
	int tconfcnt = 0; /* count of total testcases that return TCONF */
	int err, i;
	int starts = -1;
	int timed = 0;
	int run_time = -1;
	char modifier = 'm';
	int ret = 0;
	int stop;
	int go_idle;
	int has_brakes = 0;	/* stop everything if a test case fails */
	int sequential = 0;	/* run tests sequentially */
	int fork_in_road = 0;
	int exit_stat;
	int track_exit_stats = 0;	/* exit non-zero if any test exits non-zero */
	int fmt_print = 0;	/* enables formatted printing of logfiles. */
	int quiet_mode = 0;	/* supresses test start and test end tags. */
	int no_kmsg = 0;	/* don't log into /dev/kmsg */
	int c;
	pid_t cpid;
	struct sigaction sa;

	while ((c =
		getopt(argc, argv, "AO:Sa:C:QT:d:ef:hl:n:o:pqr:s:t:x:y"))
		       != -1) {
		switch (c) {
		case 'A':	/* all-stop flag */
			has_brakes = 1;
			track_exit_stats = 1;
			break;
		case 'O':	/* output buffering directory */
			test_out_dir = strdup(optarg);
			break;
		case 'S':	/* run tests sequentially */
			sequential = 1;
			break;
		case 'a':	/* name of the zoo file to use */
			zooname = strdup(optarg);
			break;
		case 'C':	/* name of the file where all failed commands will be */
			failcmdfilename = strdup(optarg);
			break;
		case 'Q':
			no_kmsg = 1;
			break;
		case 'T':
			/*
			 * test cases that are not fully tested will be recorded
			 * in this file
			 */
			tconfcmdfilename = strdup(optarg);
			break;
		case 'd':	/* debug options */
			sscanf(optarg, "%i", &Debug);
			break;
		case 'e':	/* exit non-zero if any test exists non-zero */
			track_exit_stats = 1;
			break;
		case 'f':	/* filename to read test tags from */
			filename = strdup(optarg);
			break;
		case 'h':	/* help */
			fprintf(stdout,
				"Usage: pan -n name [ -SyAehpqQ ] [ -s starts ]"
				" [-t time[s|m|h|d] [ -x nactive ] [ -l logfile ]\n\t"
				"[ -a active-file ] [ -f command-file ] "
				"[ -C fail-command-file ] "
				"[ -d debug-level ]\n\t[-o output-file] "
				"[-O output-buffer-directory] [cmd]\n");
			exit(0);
		case 'l':	/* log file */
			logfilename = strdup(optarg);
			break;
		case 'n':	/* tag given to pan */
			panname = strdup(optarg);
			break;
		case 'o':	/* send test output here */
			outputfilename = strdup(optarg);
			break;
		case 'p':	/* formatted printing. */
			fmt_print = 1;
			break;
		case 'q':	/* supress test start and test end messages */
			quiet_mode = 1;
			break;
		case 'r':	/* reporting type: none, rts */
			reporttype = strdup(optarg);
			break;
		case 's':	/* number of tags to run */
			starts = atoi(optarg);
			break;
		case 't':	/* run_time to run */
			ret = sscanf(optarg, "%d%c", &run_time, &modifier);
			if (ret == 0) {
				fprintf(stderr,
					"Need proper time input: ####x where"
					"x is one of s,m,h,d\n");
				break;
			} else if (ret == 1) {
				fprintf(stderr, "Only got a time value of %d "
					"modifiers need to come immediately after #"
					" assuming %c\n", run_time, modifier);
			} else {
				switch (modifier) {
				case 's':
					run_time = run_time;
					break;
				case 'm':
					run_time = run_time * 60;
					break;
				case 'h':
					run_time = run_time * 60 * 60;
					break;
				case 'd':
					run_time = run_time * 60 * 60 * 24;
					break;
				default:
					fprintf(stderr,
						"Invalid time modifier, try: s|h|m|d\n");
					exit(-1);
				}
				if (!quiet_mode)
					printf("PAN will run for %d seconds\n",
					       run_time);
			}
			timed = 1;	//-t implies run as many starts as possible, by default
			break;
		case 'x':	/* number of tags to keep running */
			keep_active = atoi(optarg);
			break;
		case 'y':	/* restart on failure or signal */
			fork_in_road = 1;
			break;
		}
	}

	if (panname == NULL) {
		fprintf(stderr, "pan: Must supply -n\n");
		exit(1);
	}
	if (zooname == NULL) {
		zooname = zoo_getname();
		if (zooname == NULL) {
			fprintf(stderr,
				"pan(%s): Must supply -a or set ZOO env variable\n",
				panname);
			exit(1);
		}
	}
	if (reporttype) {
		/* make sure we understand the report type */
		if (strcasecmp(reporttype, "rts")
		    && strcasecmp(reporttype, "none")
		    /* && strcasecmp(reporttype, "xml") */
		    )
			reporttype = "rts";
	} else {
		/* set the default */
		reporttype = "rts";
	}

	if (logfilename != NULL) {
		time_t startup;
		char *s;

		if (!strcmp(logfilename, "-")) {
			logfile = stdout;
		} else {
			if ((logfile = fopen(logfilename, "a+")) == NULL) {
				fprintf(stderr,
					"pan(%s): Error %s (%d) opening log file '%s'\n",
					panname, strerror(errno), errno,
					logfilename);
				exit(1);
			}
		}

		time(&startup);
		s = ctime(&startup);
		*(s + strlen(s) - 1) = '\0';
		if (!fmt_print)
			fprintf(logfile, "startup='%s'\n", s);
		else {
			fprintf(logfile, "Test Start Time: %s\n", s);
			fprintf(logfile,
				"-----------------------------------------\n");
			fprintf(logfile, ResultFmt" %-10.10s\n",
				"Testcase", "Result", "Exit Value");
			fprintf(logfile, ResultFmt" %-10.10s\n",
				"--------", "------", "------------");
		}
		fflush(logfile);
	}

	coll = get_collection(filename, optind, argc, argv);
	if (!coll)
		exit(1);
	if (coll->cnt == 0) {
		fprintf(stderr,
			"pan(%s): Must supply a file collection or a command\n",
			panname);
		exit(1);
	}

	if (Debug & Dsetup)
		dump_coll(coll);

	/* a place to store the pgrps we're watching */
	running =
		malloc((keep_active + 1) *
			sizeof(struct tag_pgrp));
	if (running == NULL) {
		fprintf(stderr, "pan(%s): Failed to allocate memory: %s\n",
			panname, strerror(errno));
		exit(2);
	}
	memset(running, 0, keep_active * sizeof(struct tag_pgrp));
	running[keep_active].pgrp = -1;	/* end sentinel */

	/* a head to the orphaned pgrp list */
	orphans = malloc(sizeof(struct orphan_pgrp));
	memset(orphans, 0, sizeof(struct orphan_pgrp));

	srand48(time(NULL) ^ (getpid() + (getpid() << 15)));

	/* Supply a default for starts.  If we are in sequential mode, use
	 * the number of commands available; otherwise 1.
	 */
	if (timed == 1 && starts == -1) {	/* timed, infinite by default */
		starts = -1;
	} else if (starts == -1) {
		if (sequential) {
			starts = coll->cnt;
		} else {
			starts = 1;
		}
	} else if (starts == 0) {	/* if the user specified infinite, set it */
		starts = -1;
	} else {		/* else, make sure we are starting at least keep_active processes */
		if (starts < keep_active)
			starts = keep_active;
	}

	/* if we're buffering output, but we're only running on process at a time,
	 * then essentially "turn off buffering"
	 */
	if (test_out_dir && (keep_active == 1)) {
		free(test_out_dir);
		test_out_dir = NULL;
	}

	if (test_out_dir) {
		struct stat sbuf;

		if (stat(test_out_dir, &sbuf) < 0) {
			fprintf(stderr,
				"pan(%s): stat of -O arg '%s' failed.  errno: %d  %s\n",
				panname, test_out_dir, errno, strerror(errno));
			exit(1);
		}
		if (!S_ISDIR(sbuf.st_mode)) {
			fprintf(stderr,
				"pan(%s): -O arg '%s' must be a directory.\n",
				panname, test_out_dir);
			exit(1);
		}
		if (access(test_out_dir, W_OK | R_OK | X_OK) < 0) {
			fprintf(stderr,
				"pan(%s): permission denied on -O arg '%s'.  errno: %d  %s\n",
				panname, test_out_dir, errno, strerror(errno));
			exit(1);
		}
	}

	if (outputfilename) {
		if (!freopen(outputfilename, "a+", stdout)) {
			fprintf(stderr,
				"pan(%s): Error %s (%d) opening output file '%s'\n",
				panname, strerror(errno), errno,
				outputfilename);
			exit(1);
		}
	}

	if (failcmdfilename) {
		if (!(failcmdfile = fopen(failcmdfilename, "a+"))) {
			fprintf(stderr,
				"pan(%s): Error %s (%d) opening fail cmd file '%s'\n",
				panname, strerror(errno), errno,
				failcmdfilename);
			exit(1);
		}
	}

	if (tconfcmdfilename) {
		tconfcmdfile = fopen(tconfcmdfilename, "a+");
		if (!tconfcmdfile) {
			fprintf(stderr, "pan(%s): Error %s (%d) opening "
				"tconf cmd file '%s'\n", panname,
				strerror(errno), errno, tconfcmdfilename);
			exit(1);
		}
	}

	if ((zoofile = zoo_open(zooname)) == NULL) {
		fprintf(stderr, "pan(%s): %s\n", panname, zoo_error);
		exit(1);
	}
	if (zoo_mark_args(zoofile, getpid(), panname, argc, argv)) {
		fprintf(stderr, "pan(%s): %s\n", panname, zoo_error);
		exit(1);
	}

	/* Allocate N spaces for max-arg commands.
	 * this is an "active file cleanliness" thing
	 */
	{
		for (c = 0; c < keep_active; c++) {
			if (zoo_mark_cmdline(zoofile, c, panname, "")) {
				fprintf(stderr, "pan(%s): %s\n", panname,
					zoo_error);
				exit(1);
			}
		}
		for (c = 0; c < keep_active; c++) {
			if (zoo_clear(zoofile, c)) {
				fprintf(stderr, "pan(%s): %s\n", panname,
					zoo_error);
				exit(1);
			}
		}
	}

	rec_signal = send_signal = 0;
	if (run_time != -1) {
		alarm(run_time);
	}

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = wait_handler;

	sigaction(SIGALRM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGHUP, &sa, NULL);
	sigaction(SIGUSR1, &sa, NULL);	/* ignore fork_in_road */
	sigaction(SIGUSR2, &sa, NULL);	/* stop the scheduler */

	c = 0;			/* in this loop, c is the command index */
	stop = 0;
	exit_stat = 0;
	go_idle = 0;
	while (1) {

		while ((num_active < keep_active) && (starts != 0)) {
			if (stop || rec_signal || go_idle)
				break;

			if (!sequential)
				c = lrand48() % coll->cnt;

			/* find a slot for the child */
			for (i = 0; i < keep_active; ++i) {
				if (running[i].pgrp == 0)
					break;
			}
			if (i == keep_active) {
				fprintf(stderr,
					"pan(%s): Aborting: i == keep_active = %d\n",
					panname, i);
				wait_handler(SIGINT);
				exit_stat++;
				break;
			}

			cpid =
			    run_child(coll->ary[c], running + i, quiet_mode,
				      &failcnt, fmt_print, logfile, no_kmsg);
			if (cpid != -1)
				++num_active;
			if ((cpid != -1 || sequential) && starts > 0)
				--starts;

			if (sequential)
				if (++c >= coll->cnt)
					c = 0;

		}		/* while ((num_active < keep_active) && (starts != 0)) */

		if (starts == 0) {
			if (!quiet_mode)
				printf("incrementing stop\n");
			++stop;
		} else if (starts == -1)	//wjh
		{
			FILE *f = (FILE *) - 1;
			if ((f = fopen(PAN_STOP_FILE, "r")) != 0) {
				printf("Got %s Stopping!\n", PAN_STOP_FILE);
				fclose(f);
				unlink(PAN_STOP_FILE);
				stop++;
			}
		}

		if (rec_signal) {
			/* propagate everything except sigusr2 */

			if (rec_signal == SIGUSR2) {
				if (fork_in_road)
					++go_idle;
				else
					++stop;
				rec_signal = send_signal = 0;
			} else {
				if (rec_signal == SIGUSR1)
					fork_in_road = 0;
				propagate_signal(running, keep_active, orphans);
				if (fork_in_road)
					++go_idle;
				else
					++stop;
			}
		}

		err = check_pids(running, &num_active, keep_active, logfile,
				 failcmdfile, tconfcmdfile, orphans, fmt_print,
				 &failcnt, &tconfcnt, quiet_mode, no_kmsg);
		if (Debug & Drunning) {
			pids_running(running, keep_active);
			orphans_running(orphans);
		}
		if (err) {
			if (fork_in_road)
				++go_idle;
			if (track_exit_stats)
				exit_stat++;
			if (has_brakes) {
				fprintf(stderr, "pan(%s): All stop!%s\n",
					panname, go_idle ? " (idling)" : "");
				wait_handler(SIGINT);
			}
		}

		if (stop && (num_active == 0))
			break;

		if (go_idle && (num_active == 0)) {
			go_idle = 0;	/* It is idle, now resume scheduling. */
			wait_handler(0);	/* Reset the signal ratchet. */
		}
	}

	/* Wait for orphaned pgrps */
	while (1) {
		for (orph = orphans; orph != NULL; orph = orph->next) {
			if (orph->pgrp == 0)
				continue;
			/* Yes, we have orphaned pgrps */
			sleep(5);
			if (!rec_signal) {
				/* force an artificial signal, move us
				 * through the signal ratchet.
				 */
				wait_handler(SIGINT);
			}
			propagate_signal(running, keep_active, orphans);
			if (Debug & Drunning)
				orphans_running(orphans);
			break;
		}
		if (orph == NULL)
			break;
	}

	if (zoo_clear(zoofile, getpid())) {
		fprintf(stderr, "pan(%s): %s\n", panname, zoo_error);
		++exit_stat;
	}
	fclose(zoofile);
	if (logfile && fmt_print) {
		if (uname(&unamebuf) == -1)
			fprintf(stderr, "ERROR: uname(): %s\n",
				strerror(errno));
		fprintf(logfile,
			"\n-----------------------------------------------\n");
		fprintf(logfile, "Total Tests: %d\n", coll->cnt);
		fprintf(logfile, "Total Skipped Tests: %d\n", tconfcnt);
		fprintf(logfile, "Total Failures: %d\n", failcnt);
		fprintf(logfile, "Kernel Version: %s\n", unamebuf.release);
		fprintf(logfile, "Machine Architecture: %s\n",
			unamebuf.machine);
		fprintf(logfile, "Hostname: %s\n\n", unamebuf.nodename);
	}
	if (logfile && (logfile != stdout))
		fclose(logfile);

	if (failcmdfile)
		fclose(failcmdfile);

	if (tconfcmdfile)
		fclose(tconfcmdfile);
	exit(exit_stat);
}

static void
propagate_signal(struct tag_pgrp *running, int keep_active,
		 struct orphan_pgrp *orphans)
{
	int i;

	if (Debug & Dshutdown)
		fprintf(stderr, "pan was signaled with sig %d...\n",
			rec_signal);

	if (rec_signal == SIGALRM) {
		printf("PAN stop Alarm was received\n");
		rec_signal = SIGTERM;
	}

	for (i = 0; i < keep_active; ++i) {
		if (running[i].pgrp == 0)
			continue;

		if (Debug & Dshutdown)
			fprintf(stderr, "  propagating sig %d to %d\n",
				send_signal, -running[i].pgrp);
		if (kill(-running[i].pgrp, send_signal) != 0) {
			fprintf(stderr,
				"pan(%s): kill(%d,%d) failed on tag (%s).  errno:%d  %s\n",
				panname, -running[i].pgrp, send_signal,
				running[i].cmd->name, errno, strerror(errno));
		}
		running[i].stopping = 1;
	}

	check_orphans(orphans, send_signal);

	rec_signal = send_signal = 0;
}

static int
check_pids(struct tag_pgrp *running, int *num_active, int keep_active,
	   FILE *logfile, FILE *failcmdfile, FILE *tconfcmdfile,
	   struct orphan_pgrp *orphans, int fmt_print, int *failcnt,
	   int *tconfcnt, int quiet_mode, int no_kmsg)
{
	int w;
	pid_t cpid;
	int stat_loc;
	int ret = 0;
	int i;
	time_t t;
	char *status;
	char *result_str;
	int signaled = 0;
	struct tms tms1, tms2;
	clock_t tck;

	check_orphans(orphans, 0);

	tck = times(&tms1);
	if (tck == -1) {
		fprintf(stderr, "pan(%s): times(&tms1) failed.  errno:%d  %s\n",
			panname, errno, strerror(errno));
	}
	cpid = wait(&stat_loc);
	tck = times(&tms2);
	if (tck == -1) {
		fprintf(stderr, "pan(%s): times(&tms2) failed.  errno:%d  %s\n",
			panname, errno, strerror(errno));
	}

	if (cpid < 0) {
		if (errno == EINTR) {
			if (Debug)
				fprintf(stderr, "pan(%s): wait() interrupted\n",
					panname);
		} else if (errno != ECHILD) {
			fprintf(stderr,
				"pan(%s): wait() failed.  errno:%d  %s\n",
				panname, errno, strerror(errno));
		}
	} else if (cpid > 0) {

		if (WIFSIGNALED(stat_loc)) {
			w = WTERMSIG(stat_loc);
			status = "signaled";
			if (Debug & Dexit)
				fprintf(stderr,
					"child %d terminated with signal %d\n",
					cpid, w);
			--*num_active;
			signaled = 1;
		} else if (WIFEXITED(stat_loc)) {
			w = WEXITSTATUS(stat_loc);
			status = "exited";
			if (Debug & Dexit)
				fprintf(stderr,
					"child %d exited with status %d\n",
					cpid, w);
			--*num_active;
			if (w != 0 && w != TCONF)
				ret++;
		} else if (WIFSTOPPED(stat_loc)) {	/* should never happen */
			w = WSTOPSIG(stat_loc);
			status = "stopped";
			ret++;
		} else {	/* should never happen */
			w = 0;
			status = "unknown";
			ret++;
		}

		for (i = 0; i < keep_active; ++i) {
			if (running[i].pgrp == cpid) {
				if ((w == 130) && running[i].stopping &&
				    (strcmp(status, "exited") == 0)) {
					/* The child received sigint, but
					 * did not trap for it?  Compensate
					 * for it here.
					 */
					w = 0;
					ret--;	/* undo */
					if (Debug & Drunning)
						fprintf(stderr,
							"pan(%s): tag=%s exited 130, known to be signaled; will give it an exit 0.\n",
							panname,
							running[i].cmd->name);
				}
				time(&t);
				if (logfile != NULL) {
					if (!fmt_print)
						fprintf(logfile,
							"tag=%s stime=%d dur=%d exit=%s stat=%d core=%s cu=%d cs=%d\n",
							running[i].cmd->name,
							(int)(running[i].
							      mystime),
							(int)(t -
							      running[i].
							      mystime), status,
							w,
							(stat_loc & 0200) ?
							"yes" : "no",
							(int)(tms2.tms_cutime -
							      tms1.tms_cutime),
							(int)(tms2.tms_cstime -
							      tms1.tms_cstime));
					else {
						if (strcmp(status, "exited") ==
						    0 && w == TCONF) {
							++*tconfcnt;
							result_str = "CONF";
						} else if (w != 0) {
							++*failcnt;
							result_str = "FAIL";
						} else {
							result_str = "PASS";
						}

						fprintf(logfile,
							ResultFmt" %-5d\n",
							running[i].cmd->name,
							result_str,
							w);
					}

					fflush(logfile);
				}

				if (w != 0) {
					if (tconfcmdfile != NULL &&
					    w == TCONF) {
						fprintf(tconfcmdfile, "%s %s\n",
						running[i].cmd->name,
						running[i].cmd->cmdline);
					} else if (failcmdfile != NULL) {
						fprintf(failcmdfile, "%s %s\n",
						running[i].cmd->name,
						running[i].cmd->cmdline);
					}
				}

				if (running[i].stopping)
					status = "driver_interrupt";

				if (test_out_dir) {
					if (!quiet_mode)
						write_test_start(running + i, no_kmsg);
					copy_buffered_output(running + i);
					unlink(running[i].output);
				}
				if (!quiet_mode)
					write_test_end(running + i, "ok", t,
						       status, stat_loc, w,
						       &tms1, &tms2);

				/* If signaled and we weren't expecting
				 * this to be stopped then the proc
				 * had a problem.
				 */
				if (signaled && !running[i].stopping)
					ret++;

				running[i].pgrp = 0;
				if (zoo_clear(zoofile, cpid)) {
					fprintf(stderr, "pan(%s): %s\n",
						panname, zoo_error);
					exit(1);
				}

				/* Check for orphaned pgrps */
				if ((kill(-cpid, 0) == 0) || (errno == EPERM)) {
					if (zoo_mark_cmdline
					    (zoofile, cpid, "panorphan",
					     running[i].cmd->cmdline)) {
						fprintf(stderr, "pan(%s): %s\n",
							panname, zoo_error);
						exit(1);
					}
					mark_orphan(orphans, cpid);
					/* status of kill doesn't matter */
					kill(-cpid, SIGTERM);
				}

				break;
			}
		}
	}
	return ret;
}

static pid_t
run_child(struct coll_entry *colle, struct tag_pgrp *active, int quiet_mode,
	  int *failcnt, int fmt_print, FILE * logfile, int no_kmsg)
{
	ssize_t errlen;
	int cpid;
	int c_stdout = -1;	/* child's stdout, stderr */
	int capturing = 0;	/* output is going to a file instead of stdout */
	char *c_cmdline;
	static long cmdno = 0;
	int errpipe[2];		/* way to communicate to parent that the tag  */
	char errbuf[1024];	/* didn't actually start */

	/* Try to open the file that will be stdout for the test */
	if (test_out_dir) {
		capturing = 1;
		do {
			sprintf(active->output, "%s/%s.%ld",
				test_out_dir, colle->name, cmdno++);
			c_stdout =
			    open(active->output,
				 O_CREAT | O_RDWR | O_EXCL | O_SYNC, 0666);
		} while (c_stdout < 0 && errno == EEXIST);
		if (c_stdout < 0) {
			fprintf(stderr,
				"pan(%s): open of stdout file failed (tag %s).  errno: %d  %s\n  file: %s\n",
				panname, colle->name, errno, strerror(errno),
				active->output);
			return -1;
		}
	}

	/* get the tag's command line arguments ready.  subst_pcnt_f() uses a
	 * static counter, that's why we do it here instead of after we fork.
	 */
	if (colle->pcnt_f) {
		c_cmdline = subst_pcnt_f(colle);
	} else {
		c_cmdline = colle->cmdline;
	}

	if (pipe(errpipe) < 0) {
		fprintf(stderr, "pan(%s): pipe() failed. errno:%d %s\n",
			panname, errno, strerror(errno));
		if (capturing) {
			close(c_stdout);
			unlink(active->output);
		}
		return -1;
	}

	time(&active->mystime);
	active->cmd = colle;

	if (!test_out_dir && !quiet_mode)
		write_test_start(active, no_kmsg);

	fflush(NULL);

	if ((cpid = fork()) == -1) {
		fprintf(stderr,
			"pan(%s): fork failed (tag %s).  errno:%d  %s\n",
			panname, colle->name, errno, strerror(errno));
		if (capturing) {
			unlink(active->output);
			close(c_stdout);
		}
		close(errpipe[0]);
		close(errpipe[1]);
		return -1;
	} else if (cpid == 0) {
		/* child */

		fclose(zoofile);
		close(errpipe[0]);
		fcntl(errpipe[1], F_SETFD, 1);	/* close the pipe if we succeed */
		setpgrp();

		umask(0);

#define WRITE_OR_DIE(fd, buf, buflen) do {				\
	if (write((fd), (buf), (buflen)) != (buflen)) {			\
		err(1, "failed to write out %zd bytes at line %d",	\
		    buflen, __LINE__);					\
	}								\
} while(0)

		/* if we're putting output into a buffer file, we need to do the
		 * redirection now.  If we fail
		 */
		if (capturing) {
			if (dup2(c_stdout, fileno(stdout)) == -1) {
				errlen =
				    sprintf(errbuf,
					    "pan(%s): couldn't redirect stdout for tag %s.  errno:%d  %s",
					    panname, colle->name, errno,
					    strerror(errno));
				WRITE_OR_DIE(errpipe[1], &errlen,
					     sizeof(errlen));
				WRITE_OR_DIE(errpipe[1], errbuf, errlen);
				exit(2);
			}
			if (dup2(c_stdout, fileno(stderr)) == -1) {
				errlen =
				    sprintf(errbuf,
					    "pan(%s): couldn't redirect stderr for tag %s.  errno:%d  %s",
					    panname, colle->name, errno,
					    strerror(errno));
				WRITE_OR_DIE(errpipe[1], &errlen,
					     sizeof(errlen));
				WRITE_OR_DIE(errpipe[1], errbuf, errlen);
				exit(2);
			}
		} else {	/* stderr still needs to be redirected */
			if (dup2(fileno(stdout), fileno(stderr)) == -1) {
				errlen =
				    sprintf(errbuf,
					    "pan(%s): couldn't redirect stderr for tag %s.  errno:%d  %s",
					    panname, colle->name, errno,
					    strerror(errno));
				WRITE_OR_DIE(errpipe[1], &errlen,
					     sizeof(errlen));
				WRITE_OR_DIE(errpipe[1], errbuf, errlen);
				exit(2);
			}
		}
		/* If there are any shell-type characters in the cmdline
		 * such as '>', '<', '$', '|', etc, then we exec a shell and
		 * run the cmd under a shell.
		 *
		 * Otherwise, break the cmdline at white space and exec the
		 * cmd directly.
		 */
		if (strpbrk(c_cmdline, "\"';|<>$\\")) {
			execlp("sh", "sh", "-c", c_cmdline, NULL);
			errlen = sprintf(errbuf,
					 "pan(%s): execlp of '%s' (tag %s) failed.  errno:%d %s",
					 panname, c_cmdline, colle->name, errno,
					 strerror(errno));
		} else {
			char **arg_v;

			arg_v = (char **)splitstr(c_cmdline, NULL, NULL);

			execvp(arg_v[0], arg_v);
			errlen = sprintf(errbuf,
					 "pan(%s): execvp of '%s' (tag %s) failed.  errno:%d  %s",
					 panname, arg_v[0], colle->name, errno,
					 strerror(errno));
		}
		WRITE_OR_DIE(errpipe[1], &errlen, sizeof(errlen));
		WRITE_OR_DIE(errpipe[1], errbuf, errlen);
		exit(errno);
	}

	/* parent */

	/* subst_pcnt_f() allocates the command line dynamically
	 * free the malloc to prevent a memory leak
	 */
	if (colle->pcnt_f)
		free(c_cmdline);

	close(errpipe[1]);

	/* if the child couldn't go through with the exec,
	 * clean up the mess, note it, and move on
	 */
	if (read(errpipe[0], &errlen, sizeof(errlen))) {
		int status;
		time_t end_time;
		int termid;
		char *termtype;
		struct tms notime = { 0, 0, 0, 0 };

		if (read(errpipe[0], errbuf, errlen) < 0)
			fprintf(stderr, "Failed to read from errpipe[0]\n");
		close(errpipe[0]);
		errbuf[errlen] = '\0';
		/* fprintf(stderr, "%s", errbuf); */
		waitpid(cpid, &status, 0);
		if (WIFSIGNALED(status)) {
			termid = WTERMSIG(status);
			termtype = "signaled";
		} else if (WIFEXITED(status)) {
			termid = WEXITSTATUS(status);
			termtype = "exited";
		} else if (WIFSTOPPED(status)) {
			termid = WSTOPSIG(status);
			termtype = "stopped";
		} else {
			termid = 0;
			termtype = "unknown";
		}
		time(&end_time);
		if (logfile != NULL) {
			if (!fmt_print) {
				fprintf(logfile,
					"tag=%s stime=%d dur=%d exit=%s "
					"stat=%d core=%s cu=%d cs=%d\n",
					colle->name, (int)(active->mystime),
					(int)(end_time - active->mystime),
					termtype, termid,
					(status & 0200) ? "yes" : "no", 0, 0);
			} else {
				if (termid != 0)
					++ * failcnt;

				fprintf(logfile, ResultFmt" %-5d\n",
					colle->name,
					((termid != 0) ? "FAIL" : "PASS"),
					termid);
			}
			fflush(logfile);
		}

		if (!quiet_mode) {
			write_test_end(active, errbuf, end_time, termtype,
				       status, termid, &notime, &notime);
		}
		if (capturing) {
			close(c_stdout);
			unlink(active->output);
		}
		return -1;
	}

	close(errpipe[0]);
	if (capturing)
		close(c_stdout);

	active->pgrp = cpid;
	active->stopping = 0;

	if (zoo_mark_cmdline(zoofile, cpid, colle->name, colle->cmdline)) {
		fprintf(stderr, "pan(%s): %s\n", panname, zoo_error);
		exit(1);
	}

	if (Debug & Dstartup)
		fprintf(stderr, "started %s cpid=%d at %s",
			colle->name, cpid, ctime(&active->mystime));

	if (Debug & Dstart) {
		fprintf(stderr, "Executing test = %s as %s", colle->name,
			colle->cmdline);
		if (capturing)
			fprintf(stderr, "with output file = %s\n",
				active->output);
		else
			fprintf(stderr, "\n");
	}

	return cpid;
}

static char *subst_pcnt_f(struct coll_entry *colle)
{
	static int counter = 1;
	char pid_and_counter[20];
	char new_cmdline[1024];

	/* if we get called falsely, do the right thing anyway */
	if (!colle->pcnt_f)
		return colle->cmdline;

	snprintf(pid_and_counter, 20, "%d_%d", getpid(), counter++);
	snprintf(new_cmdline, 1024, colle->cmdline, pid_and_counter);
	return strdup(new_cmdline);
}

static struct collection *get_collection(char *file, int optind, int argc,
					 char **argv)
{
	char *buf, *a, *b;
	struct coll_entry *head, *p, *n;
	struct collection *coll;
	int i;

	buf = slurp(file);
	if (!buf)
		return NULL;

	coll = malloc(sizeof(struct collection));
	coll->cnt = 0;

	head = p = n = NULL;
	a = b = buf;
	while (a) {
		/* set b to the start of the next line and add a NULL character
		 * to separate the two lines */
		if ((b = strchr(a, '\n')) != NULL)
			*b++ = '\0';

		/* If this is line isn't a comment */
		if ((*a != '#') && (*a != '\0') && (*a != ' ')) {
			n = malloc(sizeof(struct coll_entry));
			if ((n->pcnt_f = strstr(a, "%f"))) {
				n->pcnt_f[1] = 's';
			}
			n->name = strdup(strsep(&a, " \t"));
			while (a != NULL && isspace(*a))
				a++;
			if (a == NULL || a[0] == 0) {
				fprintf(stderr,
					"pan(%s): Testcase '%s' requires a command to execute.\n",
					panname, n->name);
				return NULL;
			}
			n->cmdline = strdup(a);
			n->next = NULL;

			if (p) {
				p->next = n;
			}
			if (head == NULL) {
				head = n;
			}
			p = n;
			coll->cnt++;
		}
		a = b;
	}
	free(buf);

	/* is there something on the commandline to be counted? */
	if (optind < argc) {
		char workstr[1024] = "";
		int workstr_left = 1023;

		/* fill arg list */
		for (i = 0; optind < argc; ++optind, ++i) {
			strncat(workstr, argv[optind], workstr_left);
			workstr_left = workstr_left - strlen(argv[optind]);
			strncat(workstr, " ", workstr_left);
			workstr_left--;
		}

		n = malloc(sizeof(struct coll_entry));
		if ((n->pcnt_f = strstr(workstr, "%f"))) {
			n->pcnt_f[1] = 's';
		}
		n->cmdline = strdup(workstr);
		n->name = "cmdln";
		n->next = NULL;
		if (p) {
			p->next = n;
		}
		if (head == NULL) {
			head = n;
		}
		coll->cnt++;
	}

	/* get an array */
	coll->ary = malloc(coll->cnt * sizeof(struct coll_entry *));

	/* fill the array */
	i = 0;
	n = head;
	while (n != NULL) {
		coll->ary[i] = n;
		n = n->next;
		++i;
	}
	if (i != coll->cnt)
		fprintf(stderr, "pan(%s): i doesn't match cnt\n", panname);

	return coll;
}

static char *slurp(char *file)
{
	char *buf;
	int fd;
	struct stat sbuf;

	if ((fd = open(file, O_RDONLY)) < 0) {
		fprintf(stderr,
			"pan(%s): open(%s,O_RDONLY) failed.  errno:%d  %s\n",
			panname, file, errno, strerror(errno));
		return NULL;
	}

	if (fstat(fd, &sbuf) < 0) {
		fprintf(stderr, "pan(%s): fstat(%s) failed.  errno:%d  %s\n",
			panname, file, errno, strerror(errno));
		return NULL;
	}

	buf = malloc(sbuf.st_size + 1);
	if (read(fd, buf, sbuf.st_size) != sbuf.st_size) {
		fprintf(stderr, "pan(%s): slurp failed.  errno:%d  %s\n",
			panname, errno, strerror(errno));
		free(buf);
		return NULL;
	}
	buf[sbuf.st_size] = '\0';

	close(fd);
	return buf;
}

static void check_orphans(struct orphan_pgrp *orphans, int sig)
{
	struct orphan_pgrp *orph;

	for (orph = orphans; orph != NULL; orph = orph->next) {
		if (orph->pgrp == 0)
			continue;

		if (Debug & Dshutdown)
			fprintf(stderr,
				"  propagating sig %d to orphaned pgrp %d\n",
				sig, -(orph->pgrp));
		if (kill(-(orph->pgrp), sig) != 0) {
			if (errno == ESRCH) {
				/* This pgrp is now empty */
				if (zoo_clear(zoofile, orph->pgrp)) {
					fprintf(stderr, "pan(%s): %s\n",
						panname, zoo_error);
				}
				orph->pgrp = 0;
			} else {
				fprintf(stderr,
					"pan(%s): kill(%d,%d) on orphaned pgrp failed.  errno:%d  %s\n",
					panname, -(orph->pgrp), sig, errno,
					strerror(errno));
			}
		}
	}
}

static void mark_orphan(struct orphan_pgrp *orphans, pid_t cpid)
{
	struct orphan_pgrp *orph;

	for (orph = orphans; orph != NULL; orph = orph->next) {
		if (orph->pgrp == 0)
			break;
	}
	if (orph == NULL) {
		/* make a new struct */
		orph = malloc(sizeof(struct orphan_pgrp));

		/* plug in the new struct just after the head */
		orph->next = orphans->next;
		orphans->next = orph;
	}
	orph->pgrp = cpid;
}

static void copy_buffered_output(struct tag_pgrp *running)
{
	char *tag_output;

	tag_output = slurp(running->output);
	if (tag_output) {
		printf("%s", tag_output);
		/* make sure the output ends with a newline */
		if (tag_output[strlen(tag_output) - 1] != '\n')
			printf("\n");
		fflush(stdout);
		free(tag_output);
	}
}

static void write_kmsg(const char *fmt, ...)
{
	FILE *kmsg;
	va_list ap;

	if ((kmsg = fopen("/dev/kmsg", "r+")) == NULL) {
		fprintf(stderr, "Error %s: (%d) opening /dev/kmsg\n",
				strerror(errno), errno);
		exit(1);
	}

	va_start(ap, fmt);
	vfprintf(kmsg, fmt, ap);
	va_end(ap);
	fclose(kmsg);
}

static void write_test_start(struct tag_pgrp *running, int no_kmsg)
{
	if (!strcmp(reporttype, "rts")) {

		printf
		    ("%s\ntag=%s stime=%ld\ncmdline=\"%s\"\ncontacts=\"%s\"\nanalysis=%s\n%s\n",
		     "<<<test_start>>>", running->cmd->name, running->mystime,
		     running->cmd->cmdline, "", "exit", "<<<test_output>>>");
	}
	fflush(stdout);
	if (no_kmsg)
		return;

	if (strcmp(running->cmd->name, running->cmd->cmdline))
		write_kmsg("LTP: starting %s (%s)\n", running->cmd->name,
			   running->cmd->cmdline);
	else
		write_kmsg("LTP: starting %s\n", running->cmd->name);
}

static void
write_test_end(struct tag_pgrp *running, const char *init_status,
	       time_t exit_time, char *term_type, int stat_loc,
	       int term_id, struct tms *tms1, struct tms *tms2)
{
	if (!strcmp(reporttype, "rts")) {
		printf
		    ("%s\ninitiation_status=\"%s\"\nduration=%ld termination_type=%s "
		     "termination_id=%d corefile=%s\ncutime=%d cstime=%d\n%s\n",
		     "<<<execution_status>>>", init_status,
		     (long)(exit_time - running->mystime), term_type, term_id,
		     (stat_loc & 0200) ? "yes" : "no",
		     (int)(tms2->tms_cutime - tms1->tms_cutime),
		     (int)(tms2->tms_cstime - tms1->tms_cstime),
		     "<<<test_end>>>");
	}
	fflush(stdout);
}

/* The functions below are all debugging related */

static void pids_running(struct tag_pgrp *running, int keep_active)
{
	int i;

	fprintf(stderr, "pids still running: ");
	for (i = 0; i < keep_active; ++i) {
		if (running[i].pgrp != 0)
			fprintf(stderr, "%d ", running[i].pgrp);
	}
	fprintf(stderr, "\n");
}

static void orphans_running(struct orphan_pgrp *orphans)
{
	struct orphan_pgrp *orph;

	fprintf(stderr, "orphans still running: ");
	for (orph = orphans; orph != NULL; orph = orph->next) {
		if (orph->pgrp != 0)
			fprintf(stderr, "%d ", -(orph->pgrp));
	}
	fprintf(stderr, "\n");
}

static void dump_coll(struct collection *coll)
{
	int i;

	for (i = 0; i < coll->cnt; ++i) {
		fprintf(stderr, "coll %d\n", i);
		fprintf(stderr, "  name=%s cmdline=%s\n", coll->ary[i]->name,
			coll->ary[i]->cmdline);
	}
}

void wait_handler(int sig)
{
	static int lastsent = 0;

	if (sig == 0) {
		lastsent = 0;
	} else {
		rec_signal = sig;
		if (sig == SIGUSR2)
			return;
		if (lastsent == 0)
			send_signal = sig;
		else if (lastsent == SIGUSR1)
			send_signal = SIGINT;
		else if (lastsent == sig)
			send_signal = SIGTERM;
		else if (lastsent == SIGTERM)
			send_signal = SIGHUP;
		else if (lastsent == SIGHUP)
			send_signal = SIGKILL;
		lastsent = send_signal;
	}
}
