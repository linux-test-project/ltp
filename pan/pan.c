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
/* $Id: pan.c,v 1.2 2000/09/21 20:42:31 nstraz Exp $ */

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/times.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <time.h>
#include <math.h>		/* log10() for subst_pcnt_f */
#include <stdlib.h>
#include <limits.h>

#include "splitstr.h"
#include "zoolib.h"

struct coll_entry
{
    char *name;
    char **argv;
    int argc;
    char *pcnt_f;
    struct coll_entry *next;
};

struct collection
{
    int cnt;
    struct coll_entry **ary;
};

struct tag_pgrp
{
    int pgrp;
    int stopping;
    time_t stime;
    struct coll_entry *cmd;
    char output[PATH_MAX];
};

struct orphan_pgrp
{
    int pgrp;
    struct orphan_pgrp *next;
};

static pid_t run_child(struct coll_entry *colle, struct tag_pgrp *active);
static char *slurp(char *file);
static struct collection *get_collection(char *file, int optind, int argc,
					 char **argv);
static void pids_running(struct tag_pgrp *running, int keep_active);
static int check_pids(struct tag_pgrp *running, int *num_active,
		      int keep_active, FILE * logfile, struct orphan_pgrp *orphans);
static void propagate_signal(struct tag_pgrp *running, int keep_active,
			     struct orphan_pgrp *orphans);
static void dump_coll(struct collection *coll);
static char **subst_pcnt_f(struct coll_entry *colle);
static void mark_orphan(struct orphan_pgrp *orphans, pid_t cpid);
static void orphans_running(struct orphan_pgrp *orphans);
static void check_orphans(struct orphan_pgrp *orphans, int sig);

static void copy_buffered_output(struct tag_pgrp *running);
static void write_test_start(struct tag_pgrp *running, const char *init_status);
static void write_test_end(struct tag_pgrp *running,
			   time_t exit_time, char *term_type, int stat_loc,
			   int term_id, struct tms *tms1, struct tms *tms2);

static char *panname = NULL;
static char *test_out_dir = NULL;
FILE *zoofile;
static char *reporttype = NULL;
static char *errmsg;

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

int
main(int argc, char **argv)
{
    extern char *optarg;
    extern int optind;
    int c;
    char *zooname = NULL;
    char *filename = "/dev/null";
    char *logfilename = NULL;
    FILE *logfile = NULL;
    char *outputfilename = NULL;
    struct collection *coll = NULL;
    pid_t cpid;
    struct tag_pgrp *running;
    struct orphan_pgrp *orphans, *orph;
    int keep_active = 1;
    int num_active = 0;
    int err, i;
    int starts = -1;
    int stop;
    int go_idle;
    int has_brakes = 0;
    int sequential = 0;
    int fork_in_road = 0;
    int exit_stat;
    int track_exit_stats = 0;

    while ((c = getopt(argc, argv, "AO:Sa:d:ef:hl:n:o:r:s:x:y")) != -1) {
	switch (c) {
	case 'A':
	    has_brakes = 1;
	    track_exit_stats = 1;
	    break;
	case 'O':
	    test_out_dir = strdup(optarg);
	    break;
	case 'S':
	    sequential = 1;
	    break;
	case 'a':
	    zooname = strdup(optarg);
	    break;
	case 'd':
	    sscanf(optarg, "%i", &Debug);
	    break;
	case 'e':
	    track_exit_stats = 1;
	    break;
	case 'f':
	    filename = strdup(optarg);
	    break;
	case 'h':
	    printf
		("Usage: pan -n name [ -SyAeh ] [ -s starts ] [ -x nactive ] [ -l logfile ]\n\t[ -a active-file ] [ -f command-file ] [ -d debug-level ] [cmd]\n");
	    exit(0);
	case 'l':
	    logfilename = strdup(optarg);
	    break;
	case 'n':
	    panname = strdup(optarg);
	    break;
	case 'o':
	    outputfilename = strdup(optarg);
	    break;
	case 'r':
	    reporttype = strdup(optarg);
	    break;
	case 's':
	    starts = atoi(optarg);
	    break;
	case 'x':
	    keep_active = atoi(optarg);
	    break;
	case 'y':
	    fork_in_road = 1;
	    break;
	}
    }

    if (panname == NULL) {
	fprintf(stderr, "pan: Must supply -n\n");
	exit(1);
    }
    if (zooname == NULL) {
	zooname = zoo_active();
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
			/* && strcasecmp(reporttype, "xml")*/)
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
			panname, strerror(errno), errno, logfilename);
		exit(1);
	    }
	}

	time(&startup);
	s = ctime(&startup);
	*(s + strlen(s) - 1) = '\0';
	fprintf(logfile, "startup='%s'\n", s);
    }

    coll = get_collection(filename, optind, argc, argv);
    if (coll->cnt == 0) {
	fprintf(stderr,
		"pan(%s): Must supply a file collection or a command\n",
		panname);
	exit(1);
    }

    if (Debug & Dsetup)
	dump_coll(coll);

    /* a place to store the pgrps we're watching */
    running = (struct tag_pgrp *)malloc((keep_active + 1) * sizeof(struct tag_pgrp));
    memset(running, 0, keep_active * sizeof(struct tag_pgrp));
    running[keep_active].pgrp = -1;	/* end sentinel */

    /* a head to the orphaned pgrp list */
    orphans = (struct orphan_pgrp *) malloc(sizeof(struct orphan_pgrp));
    memset(orphans, 0, sizeof(struct orphan_pgrp));

    srand48(time(NULL) ^ (getpid() + (getpid() << 15)));

    /* Supply a default for starts.  If we are in sequential mode, use
     * the number of commands available; otherwise 1.
     */
    if (starts == -1) {
	if (sequential) {
	    starts = coll->cnt;
	} else {
	    starts = 1;
	}
    } else if (starts == 0) {	/* if the user specified infinite, set it */
	starts = -1;
    } else {			/* else, make sure we are starting at least keep_active processes */
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
	    fprintf(stderr, "pan(%s): -O arg '%s' must be a directory.\n",
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
		    "pan(%s): Error %s (%d) openning output file '%s'\n",
		    panname, strerror(errno), errno, outputfilename);
	    exit(1);
	}
    }

    if ((zoofile = open_file(zooname, "r+", &errmsg)) == NULL) {
	fprintf(stderr, "pan(%s): %s\n", panname, errmsg);
	exit(1);
    }
    if (write_active_args(zoofile, getpid(), panname, argc, argv, &errmsg) ==
	-1) {
	fprintf(stderr, "pan(%s): %s\n", panname, errmsg);
	exit(1);
    }

    /* Allocate N spaces for max-arg commands.
     * this is an "active file cleanliness" thing
     */
    {
	char *av[2], bigarg[82];
	int t;

	t = 1;
	memset(bigarg, '.', 81);
	bigarg[81] = '\0';
	av[0] = bigarg;
	av[1] = NULL;

	for (c = 0; c < keep_active; c++) {
	    if (write_active_args(zoofile, t, panname, 1, av, &errmsg) == -1) {
		fprintf(stderr, "pan(%s): %s\n", panname, errmsg);
		exit(1);
	    }
	}
	for (c = 0; c < keep_active; c++) {
	    if (clear_active(zoofile, t, &errmsg) != 1) {
		fprintf(stderr, "pan(%s): %s\n", panname, errmsg);
		exit(1);
	    }
	}
    }

    rec_signal = send_signal = 0;
    signal(SIGINT, wait_handler);
    signal(SIGTERM, wait_handler);
    signal(SIGHUP, wait_handler);
    signal(SIGUSR1, wait_handler);	/* ignore fork_in_road */
    signal(SIGUSR2, wait_handler);	/* stop the scheduler */

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
		fprintf(stderr, "pan(%s): Aborting: i == keep_active = %d\n",
			panname, i);
		wait_handler(SIGINT);
		exit_stat++;
		break;
	    }

	    cpid = run_child(coll->ary[c], running + i);
	    if (cpid != -1) {
		++num_active;
		if (starts > 0)
		    --starts;
	    }

	    if (sequential)
		if (++c >= coll->cnt)
		    c = 0;

	}			/* while( (num_active < keep_active) && (starts != 0) ) */

	if (starts == 0)
	    ++stop;

	if (rec_signal) {
	    /* propagate everything except sigusr2 */

	    if (rec_signal == SIGUSR2) {
		if (fork_in_road)
		    ++go_idle;
		else
		    ++stop;
		signal(rec_signal, wait_handler);
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

	err = check_pids(running, &num_active, keep_active,
			 logfile, orphans);
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
		fprintf(stderr, "pan(%s): All stop!%s\n", panname,
			go_idle ? " (idling)" : "");
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

    signal(SIGINT, SIG_DFL);
    if (clear_active(zoofile, getpid(), &errmsg) != 1) {
	fprintf(stderr, "pan(%s): %s\n", panname, errmsg);
	++exit_stat;
    }
    fclose(zoofile);

    if (logfile && (logfile != stdout))
	fclose(logfile);

    exit(exit_stat);
}



static void
propagate_signal(struct tag_pgrp *running, int keep_active,
		 struct orphan_pgrp *orphans)
{
    int i;

    if (Debug & Dshutdown)
	fprintf(stderr, "pan was signaled with sig %d...\n", rec_signal);

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

    signal(rec_signal, wait_handler);
    rec_signal = send_signal = 0;
}


static int
check_pids(struct tag_pgrp *running, int *num_active, int keep_active,
	   FILE * logfile, struct orphan_pgrp *orphans)
{
    int w;
    pid_t cpid;
    int stat_loc;
    int ret = 0;
    int i;
    time_t t;
    char *status;
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
		fprintf(stderr, "pan(%s): wait() interrupted\n", panname);
	} else if (errno != ECHILD) {
	    fprintf(stderr, "pan(%s): wait() failed.  errno:%d  %s\n",
		    panname, errno, strerror(errno));
	}
    } else if (cpid > 0) {

	if (WIFSIGNALED(stat_loc)) {
	    w = WTERMSIG(stat_loc);
	    status = "signaled";
	    if (Debug & Dexit)
		fprintf(stderr, "child %d terminated with signal %d\n", cpid,
			w);
	    --*num_active;
	    signaled = 1;
	} else if (WIFEXITED(stat_loc)) {
	    w = WEXITSTATUS(stat_loc);
	    status = "exited";
	    if (Debug & Dexit)
		fprintf(stderr, "child %d exited with status %d\n", cpid, w);
	    --*num_active;
	    if (w != 0)
		ret++;
	} else if (WIFSTOPPED(stat_loc)) {	/* should never happen */
	    w = WSTOPSIG(stat_loc);
	    status = "stopped";
	    ret++;
	} else {		/* should never happen */
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
				panname, running[i].cmd->name);
		}
		if (logfile != NULL) {
		    time(&t);
		    fprintf(logfile,
			    "tag=%s stime=%d dur=%d exit=%s stat=%d core=%s cu=%d cs=%d\n",
			    running[i].cmd->name, (int) (running[i].stime),
			    (int) (t - running[i].stime), status, w,
			    (stat_loc & 0200) ? "yes" : "no",
			    (int) (tms2.tms_cutime - tms1.tms_cutime),
			    (int) (tms2.tms_cstime - tms1.tms_cstime));
		    fflush(logfile);
		}


		if (running[i].stopping)
		    status = "driver_interrupt";

		if (test_out_dir) {
		    write_test_start(running+i, "ok");
	    	    copy_buffered_output(running + i);
    		    unlink(running[i].output);
		}
		write_test_end(running+i, t, status,
		   stat_loc, w, &tms1, &tms2);

		/* If signaled and we weren't expecting
		 * this to be stopped then the proc
		 * had a problem.
		 */
		if (signaled && !running[i].stopping)
		    ret++;

		running[i].pgrp = 0;
		if (clear_active(zoofile, cpid, &errmsg) == -1) {
		    fprintf(stderr, "pan(%s): %s\n", panname, errmsg);
		    exit(1);
		}

		/* Check for orphaned pgrps */
		if ((kill(-cpid, 0) == 0) || (errno == EPERM)) {
		    if (write_active_args(zoofile, cpid, "panorphan",
					  running[i].cmd->argc,
					  running[i].cmd->argv,
					  &errmsg) == -1) {
			fprintf(stderr, "pan(%s): %s\n", panname, errmsg);
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
run_child(struct coll_entry *colle, struct tag_pgrp *active)
{
    int cpid;
    int c_stdout;		/* child's stdout, stderr */
    int capturing = 0;		/* output is going to a file instead of stdout */
    char **eargv;
    static long cmdno = 0;
    int i;
    int errpipe[2];		/* way to communicate to parent that the tag  */
    char errbuf[1024];		/* didn't actually start */
    int errlen;

    /* Try to open the file that will be stdout for the test */
    if (test_out_dir) {
	capturing = 1;
	do {
	    sprintf(active->output, "%s/%s.%ld",
		    test_out_dir, colle->name, cmdno++);
	    c_stdout = open(active->output, O_CREAT | O_RDWR | O_EXCL, 0666);
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
	eargv = subst_pcnt_f(colle);
    } else {
	eargv = colle->argv;
    }
    
    if (pipe(errpipe) < 0) {
	fprintf(stderr, "pan(%s): pipe() failed. errno:%d %s\n",
			panname, errno, strerror(errno));
	if (capturing)
	    close(c_stdout);
	return -1;
    }

    if ((cpid = fork()) < 0) {
	fprintf(stderr, "pan(%s): fork failed (tag %s).  errno:%d  %s\n",
		panname, colle->name, errno, strerror(errno));
	if (capturing)
	    close(c_stdout);
	close(errpipe[0]);
	close(errpipe[1]);
	return -1;
    } else if (cpid == 0) {
	/* child */

	fclose(zoofile);
	close(errpipe[0]);
	fcntl(errpipe[1], F_SETFD, 1);  /* close the pipe if we succeed */
	setpgrp();

	umask(0);

	/* if we're putting output into a buffer file, we need to do the
	 * redirection now.  If we fail
	 */
	if (capturing) {
	    if (dup2(c_stdout, fileno(stdout)) == -1) {
		errlen = sprintf(errbuf, "pan(%s): couldn't redirect stdout for tag %s.  errno:%d  %s",
				panname, colle->name, errno, strerror(errno));
		write(errpipe[1], &errlen, sizeof(errlen));
		write(errpipe[1], errbuf, errlen);
		exit(2);
	    }
	    if (dup2(c_stdout, fileno(stderr)) == -1) {
		errlen = sprintf(errbuf, "pan(%s): couldn't redirect stderr for tag %s.  errno:%d  %s",
				panname, colle->name, errno, strerror(errno));
		write(errpipe[1], &errlen, sizeof(errlen));
		write(errpipe[1], errbuf, errlen);
		exit(2);
	    }
	} else { /* stderr still needs to be redirected */
	    if (dup2(fileno(stdout), fileno(stderr)) == -1) {
		errlen = sprintf(errbuf, "pan(%s): couldn't redirect stderr for tag %s.  errno:%d  %s",
				panname, colle->name, errno, strerror(errno));
		write(errpipe[1], &errlen, sizeof(errlen));
		write(errpipe[1], errbuf, errlen);
		exit(2);
	    }
	}
	/* execute command */
	execvp(eargv[0], eargv);
	errlen = sprintf(errbuf,
		"pan(%s): execvp of '%s' (tag %s) failed.  errno:%d  %s",
		panname, eargv[0], colle->name, errno, strerror(errno));
	write(errpipe[1], &errlen, sizeof(errlen));
	write(errpipe[1], errbuf, errlen);
	exit(errno);
    }

    /* parent */

    /* subst_pcnt_f() allocates dynamically any arguments with %f in it.
     * free the mallocs now to prevent memory leak
     */
    if (colle->pcnt_f) {
	for (i = 0; i < colle->argc; ++i)
	    if (strstr(colle->argv[i], "%f"))
		free(eargv[i]);
	free(eargv);
    }
    close(errpipe[1]);
    time(&active->stime);
    active->cmd = colle;

    /* if the child couldn't go through with the exec, 
     * clean up the mess, note it, and move on
     */
    if(read(errpipe[0], &errlen, sizeof(errlen))) {
	int status;
	time_t end_time;
	int termid;
	char *termtype;
	struct tms notime = {0, 0, 0, 0};

	read(errpipe[0], errbuf, errlen);
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
	write_test_start(active, errbuf);
	write_test_end(active, end_time, termtype, status, 
			termid, &notime, &notime);
	return -1;
    }

    close(errpipe[0]);
	
    if (!test_out_dir) 
	write_test_start(active, "ok");

    active->pgrp = cpid;
    active->stopping = 0;

    if (write_active_args
	(zoofile, cpid, colle->name, colle->argc, colle->argv, &errmsg) == -1) {
	fprintf(stderr, "pan(%s): %s\n", panname, errmsg);
	exit(1);
    }

    if (Debug & Dstartup)
	fprintf(stderr, "started %s cpid=%d at %s",
		colle->name, cpid, ctime(&active->stime));

    if (Debug & Dstart) {
	int ac;
	fprintf(stderr, "Executing test = %s as ", colle->name);
	for (ac = 0; ac < colle->argc; ac++) {
	    fprintf(stderr, "%s ", colle->argv[ac]);
	}
	if (capturing)
	    fprintf(stderr, "with output file = %s\n", active->output);
	else
    	    fprintf(stderr, "\n");
    }

    return cpid;
}


static char **
subst_pcnt_f(struct coll_entry *colle)
{
    char **eargv;
    char *p;
    int i;

    eargv = (char **) malloc((colle->argc + 1) * sizeof(char *));

    for (i = 0; i < colle->argc; ++i) {
	if ((p = strstr(colle->argv[i], "%f")) != NULL) {
	    /* simple, for now */
	    static int counter = 1;
	    char *b, *p2;
	    int pidlen, counterlen;

	    *p = '\0';		/* cut off at % */
	    p2 = p + 2;		/* stuff that follows %f */

	    pidlen = 1 + (int) log10((double) getpid());
	    counterlen = 1 + (int) log10((double) counter);

	    b = (char *) malloc(strlen(colle->argv[i]) +
				pidlen + 1 + counterlen + strlen(p2) + 1);
	    sprintf(b, "%s%d_%d%s", colle->argv[i], getpid(), counter++, p2);
	    *p = '%';		/* restore % */
	    eargv[i] = b;
	} else {
	    eargv[i] = colle->argv[i];
	}
    }
    eargv[i] = NULL;
    return eargv;
}

static struct collection *
get_collection(char *file, int optind, int argc, char **argv)
{
    char *buf, *a, *b;
    struct coll_entry *head, *p, *n;
    struct collection *coll;
    int i;

    buf = slurp(file);

    coll = (struct collection *) malloc(sizeof(struct collection));
    coll->cnt = 0;

    head = p = n = NULL;
    a = b = buf;
    while (*b != '\0') {
	if ((b = strchr(a, '\n')) != NULL)
	    *b = '\0';

	if ((*a != '#') && (*a != '\0') && (*a != ' ')) {
	    if (head == NULL) {
		head =
		    (struct coll_entry *) malloc(sizeof(struct coll_entry));
		head->pcnt_f = strstr(a, "%f");
		head->argv = (char **) splitstr(a, NULL, &head->argc);
		head->name = head->argv[0];
		head->argv++;	/* remove name from command */
		head->argc--;
		head->next = NULL;
		p = head;
	    } else {
		n = (struct coll_entry *) malloc(sizeof(struct coll_entry));
		p->next = n;
		n->pcnt_f = strstr(a, "%f");
		n->argv = (char **) splitstr(a, NULL, &n->argc);
		n->name = n->argv[0];
		n->argv++;	/* remove name from command */
		n->argc--;
		n->next = NULL;
		p = n;
	    }
	    coll->cnt++;
	}
	a += strlen(a) + 1;
	b = a;
    }
    free(buf);

    /* is there something on the commandline to be counted? */
    if (optind < argc) {
	char **args;
	char *pcnt_f = NULL;

	args = (char **) malloc((argc - optind + 1) * sizeof(char *));
	/* fill arg list */
	for (i = 0; optind < argc; ++optind, ++i) {
	    args[i] = argv[optind];
	    if ((pcnt_f == NULL) && ((strstr(args[i], "%f")) != NULL)) {
		pcnt_f = args[i];
	    }
	}
	args[i] = NULL;

	if (head == NULL) {
	    head = (struct coll_entry *) malloc(sizeof(struct coll_entry));
	    head->pcnt_f = pcnt_f;
	    head->argv = args;
	    head->name = "cmdln";
	    head->argc = i;
	    head->next = NULL;
	} else {
	    n = (struct coll_entry *) malloc(sizeof(struct coll_entry));
	    p->next = n;
	    n->pcnt_f = pcnt_f;
	    n->argv = args;
	    n->name = "cmdln";
	    n->argc = i;
	    n->next = NULL;
	}
	coll->cnt++;
    }

    /* get an array */
    coll->ary = (struct coll_entry **) malloc(coll->cnt *
					      sizeof(struct coll_entry *));

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


static char *
slurp(char *file)
{
    char *buf;
    int fd;
    struct stat sbuf;

    if ((fd = open(file, O_RDONLY)) < 0) {
	fprintf(stderr, "pan(%s): open(%s,O_RDONLY) failed.  errno:%d  %s\n",
		panname, file, errno, strerror(errno));
	return NULL;
    }

    if (fstat(fd, &sbuf) < 0) {
	fprintf(stderr, "pan(%s): fstat(%s) failed.  errno:%d  %s\n",
		panname, file, errno, strerror(errno));
	return NULL;
    }

    buf = (char *) malloc(sbuf.st_size + 1);
    if (read(fd, buf, sbuf.st_size) != sbuf.st_size) {
	fprintf(stderr, "pan(%s): slurp failed.  errno:%d  %s\n",
		panname, errno, strerror(errno));
	return NULL;
    }
    buf[sbuf.st_size] = '\0';

    close(fd);
    return buf;
}

static void
check_orphans(struct orphan_pgrp *orphans, int sig)
{
    struct orphan_pgrp *orph;

    for (orph = orphans; orph != NULL; orph = orph->next) {
	if (orph->pgrp == 0)
	    continue;

	if (Debug & Dshutdown)
	    fprintf(stderr, "  propagating sig %d to orphaned pgrp %d\n",
		    sig, -(orph->pgrp));
	if (kill(-(orph->pgrp), sig) != 0) {
	    if (errno == ESRCH) {
		/* This pgrp is now empty */
		if (clear_active(zoofile, orph->pgrp, &errmsg) == -1) {
		    fprintf(stderr, "pan(%s): %s\n", panname, errmsg);
		}
		orph->pgrp = 0;
	    } else {
		fprintf(stderr,
			"pan(%s): kill(%d,%d) on orphaned pgrp failed.  errno:%d  %s\n",
			panname, -(orph->pgrp), sig, errno, strerror(errno));
	    }
	}
    }
}


static void
mark_orphan(struct orphan_pgrp *orphans, pid_t cpid)
{
    struct orphan_pgrp *orph;

    for (orph = orphans; orph != NULL; orph = orph->next) {
	if (orph->pgrp == 0)
	    break;
    }
    if (orph == NULL) {
	/* make a new struct */
	orph = (struct orphan_pgrp *) malloc(sizeof(struct orphan_pgrp));

	/* plug in the new struct just after the head */
	orph->next = orphans->next;
	orphans->next = orph;
    }
    orph->pgrp = cpid;
}



static void
copy_buffered_output(struct tag_pgrp *running)
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


static void
write_test_start(struct tag_pgrp *running, const char *init_status)
{
    if (!strcmp(reporttype, "rts")) {
	char *args = NULL;

	args = cat_args(running->cmd->argc, running->cmd->argv, &errmsg);
	if (!args) args = "Unable_to_malloc_cmdline";

	printf("%s\ntag=%s stime=%ld\ncmdline=\"%s\"\ncontacts=\"%s\"\nanalysis=%s\ninitiation_status=\"%s\"\n%s\n",
			"<<<test_start>>>",
			running->cmd->name, running->stime, args, "",
			"exit", init_status,
			"<<<test_output>>>");
	if (args) free(args);
    }
    fflush(stdout);
}


static void
write_test_end(struct tag_pgrp *running, time_t exit_time,
	       char *term_type, int stat_loc, int term_id,
	       struct tms *tms1, struct tms *tms2)
{
    if (!strcmp(reporttype, "rts")) {
	printf("%s\nduration=%ld termination_type=%s termination_id=%d corefile=%s\ncutime=%d cstime=%d\n%s\n",
		  	"<<<execution_status>>>", 
			(long) (exit_time - running->stime),
			term_type, term_id, (stat_loc & 0200) ? "yes" : "no",
			(int) (tms2->tms_cutime - tms1->tms_cutime),
			(int) (tms2->tms_cstime - tms1->tms_cstime),
			"<<<test_end>>>");
    }
    fflush(stdout);
}

/* The functions below are all debugging related */

static void
pids_running(struct tag_pgrp *running, int keep_active)
{
    int i;

    fprintf(stderr, "pids still running: ");
    for (i = 0; i < keep_active; ++i) {
	if (running[i].pgrp != 0)
	    fprintf(stderr, "%d ", running[i].pgrp);
    }
    fprintf(stderr, "\n");
}

static void
orphans_running(struct orphan_pgrp *orphans)
{
    struct orphan_pgrp *orph;

    fprintf(stderr, "orphans still running: ");
    for (orph = orphans; orph != NULL; orph = orph->next) {
	if (orph->pgrp != 0)
	    fprintf(stderr, "%d ", -(orph->pgrp));
    }
    fprintf(stderr, "\n");
}

static void
dump_coll(struct collection *coll)
{
    int x, i;

    for (i = 0; i < coll->cnt; ++i) {
	fprintf(stderr, "coll %d\n", i);
	fprintf(stderr, "  name=%s #args=%d\n", coll->ary[i]->name,
		coll->ary[i]->argc);
	for (x = 0; coll->ary[i]->argv[x]; ++x) {
	    fprintf(stderr, "  argv[%d] = (%s)\n", x, coll->ary[i]->argv[x]);
	}
    }
}
