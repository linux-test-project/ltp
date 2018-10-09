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
/* $Id: kill10.c,v 1.7 2009/03/23 13:35:53 subrata_modak Exp $ */
/**********************************************************
 *
 *    OS Test - Silicon Graphics, Inc.
 *
 *    TEST IDENTIFIER	: kill10
 *
 *    EXECUTED BY	: anyone
 *
 *    TEST TITLE	: signal flooding test
 *
 *    TEST CASE TOTAL	: 1
 *
 *    WALL CLOCK TIME	:
 *
 *    CPU TYPES		: ALL
 *
 *    AUTHOR		: Nate Straz
 *
 *    DATE STARTED	: 04/09/2001
 *
 *    INITIAL RELEASE	: Linux 2.4.x
 *
 *    TEST CASES
 *
 * 	1.) Create a large number of processes and signal between them.
 *
 *    INPUT SPECIFICATIONS
 * 	The standard options for system call tests are accepted.
 *	(See the parse_opts(3) man page).
 *
 *    OUTPUT SPECIFICATIONS
 *$
 *    DURATION
 * 	Terminates - with frequency and infinite modes.
 *
 *    SIGNALS
 * 	Uses SIGUSR1 to pause before test if option set.
 * 	(See the parse_opts(3) man page).
 *
 *    RESOURCES
 * 	None
 *
 *    ENVIRONMENTAL NEEDS
 *      No run-time environmental needs.
 *
 *    SPECIAL PROCEDURAL REQUIREMENTS
 * 	None
 *
 *    INTERCASE DEPENDENCIES
 * 	None
 *
 *    DETAILED DESCRIPTION
 *  This test creates -g groups of -n processes each and prepares them to send
 *  large numbers of signals.  All process fall into three levels.
 *    * Level 1 - Master
 *                This is the parent of all processes.  It handles test looping
 *                and making sure that all level 2 Managers report in.
 *                SIGUSR1 -> ack Manager is ready
 *                SIGUSR2 -> ack Manager is done and sends reset
 *    * Level 2 - Managers
 *                There are -g (default 2) of these processes.  They handle
 *                forking off -n procs and setting up their signal handling.
 *                Managers are in a pgid with their Children.
 *                SIGALRM -> Process making your children
 *                SIGUSR1 ->
 *                SIGUSR2 -> Reply to Child to stop
 *                SIGHUP  -> Reset child signal counter
 *                SIGQUIT -> Exit gracefully
 *    * Level 3 - Child
 *                There are -n (default 10) of these process per Manager.  Their
 *                only job is to send signals to their Managers when told to by
 *                the Master.
 *                SIGUSR1 -> Start signaling Manager
 *                SIGUSR2 -> Stop signaling Manager
 *                SIGHUP  -> IGNORE
 *                SIGQUIT -> Exit gracefully
 *
 *  During each test loop, Master sends SIGUSR1 to the pgid of each Manager.
 *  This tells the Children to start signalling their manager.  They do this
 *  until the manager signals them to stop.  Once the manager finds that all
 *  children have been signaled (by checking them off in the checklist), the
 *  Manager signals the Master.  Once the Master acknowledges that all Managers
 *  have talked to all their Children, the test iteration is over.
 *
 * 	Setup:
 *	  Pause for SIGUSR1 if option specified.
 *	  Fork -g Managers
 *	    Set up signal handling for Children
 *	    Fork -n Children for each manager
 *	    Set up signal handling for Managers
 *	  Set up signal handling for Master
 *
 * 	Test:
 *	 Loop if the proper options are given.
 *	   Send SIGUSR1 to all Managers and their Children
 *	   Wait for Managers to send SIGUSR2
 *
 * 	Cleanup:
 * 	  Send SIGQUIT to all Manager process groups and wait for Manager to quit.
 * 	  Print errno log and/or timing stats if options given
 *
 *  Debugging:
 *    0 - normal operations
 *    1 - Master setup
 *    2 - Master processing
 *    3 - Master - Manager interaction
 *    4 - Manager setup
 *    5 - Manager processing
 *    6 - Manager - Child interaction
 *    7 - Child setup
 *    8 - Child processing
 *
 *
 *#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#**/

#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include "test.h"

void setup();
void help();
void cleanup();
void fork_pgrps(int pgrps_left);
void manager(int num_procs);
void fork_procs(int procs_left);

/* signal handlers */
void ack_ready(int sig, siginfo_t * si, void *data);
void ack_done(int sig, siginfo_t * si, void *data);
void set_create_procs(int sig);
void graceful_exit(int sig);
void set_signal_parents(int sig);
void clear_signal_parents(int sig);
void set_confirmed_ready(int sig);
void reset_counter(int sig);
void reply_to_child(int sig, siginfo_t * si, void *data);
void wakeup(int sig);

/* pid checklist management */
struct pid_list_item {
	pid_t pid;
	short flag;
} *child_checklist = NULL;
int child_checklist_total = 0;
int checklist_cmp(const void *a, const void *b);
void checklist_reset(int bit);

static inline int k_sigaction(int sig, struct sigaction *sa, struct sigaction *osa);

char *TCID = "kill10";
int TST_TOTAL = 1;

int num_procs = 10;
int num_pgrps = 2;
int pgrps_ready = 0;
int child_signal_counter = 0;

int create_procs_flag = 0;
int signal_parents_flag = 0;
int confirmed_ready_flag = 0;
int debug_flag = 0;
pid_t mypid = 0;

char *narg, *garg, *darg;
int nflag = 0, gflag = 0, dflag = 0;

option_t options[] = {
	{"n:", &nflag, &narg},	/* -n #procs */
	{"g:", &gflag, &garg},	/* -g #pgrps */
	{"d:", &dflag, &darg},	/* -d <debug level>  */
	{NULL, NULL, NULL}
};

int main(int ac, char **av)
{
	int lc;
	int cnt;

	tst_parse_opts(ac, av, options, &help);

	if (nflag) {
		if (sscanf(narg, "%i", &num_procs) != 1) {
			tst_brkm(TBROK, NULL, "-n option arg is not a number");
		}
	}
	if (gflag) {
		if (sscanf(garg, "%i", &num_pgrps) != 1) {
			tst_brkm(TBROK, NULL, "-g option arg is not a number");
		}
	}

	if (dflag) {
		if (sscanf(darg, "%i", &debug_flag) != 1) {
			tst_brkm(TBROK, NULL, "-d option arg is not a number");
		}
	}

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;
		child_signal_counter = 0;
		pgrps_ready = 0;
		checklist_reset(0x03);

		/* send SIGUSR1 to each pgroup */
		for (cnt = 0; cnt < child_checklist_total; ++cnt) {
			if (debug_flag >= 2)
				printf("%d: test_loop, SIGUSR1 -> %d\n",
				       mypid, -child_checklist[cnt].pid);
			kill(-child_checklist[cnt].pid, SIGUSR1);
		}

		/* wait for the managers to signal they are done */
		while (child_signal_counter < num_pgrps) {
			alarm(1);
			if (debug_flag >= 2)
				printf("%d: Master pausing for done (%d/%d)\n",
				       mypid, child_signal_counter, num_pgrps);
			pause();
		}
		tst_resm(TPASS, "All %d pgrps received their signals",
			 child_signal_counter);

	}

	cleanup();

	tst_exit();
}

void help(void)
{
	printf("  -g n    Create n process groups (default: %d)\n", num_pgrps);
	printf
	    ("  -n n    Create n children in each process group (default: %d)\n",
	     num_procs);
	printf("  -d n    Set debug level to n (default: %d)\n", debug_flag);
}

void setup(void)
{
	struct sigaction sa;
	int i;

	/* You will want to enable some signal handling so you can capture
	 * unexpected signals like SIGSEGV.
	 */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* One cavet that hasn't been fixed yet.  TEST_PAUSE contains the code to
	 * fork the test with the -c option.  You want to make sure you do this
	 * before you create your temporary directory.
	 */
	TEST_PAUSE;

	mypid = getpid();
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (debug_flag >= 1)
		printf("%d: setting SIGTRAP -> SIG_DFL\n", mypid);
	k_sigaction(SIGTRAP, &sa, NULL);
	if (debug_flag >= 1)
		printf("%d: setting SIGCONT -> SIG_DFL\n", mypid);
	k_sigaction(SIGCONT, &sa, NULL);

	sa.sa_handler = set_create_procs;
	if (debug_flag >= 4)
		printf("%d: setting SIGALRM -> set_create_procs\n", mypid);
	k_sigaction(SIGALRM, &sa, NULL);

	sa.sa_handler = NULL;
	sa.sa_sigaction = ack_ready;
	sa.sa_flags = SA_SIGINFO;
	if (debug_flag >= 1)
		printf("%d: setting SIGUSR1 -> ack_ready\n", mypid);
	k_sigaction(SIGUSR1, &sa, NULL);

	fork_pgrps(num_pgrps);

	/* wait for all pgrps to report in */
	if (debug_flag)
		printf("Master: %d\n", mypid);
	while (pgrps_ready < num_pgrps) {
		if (debug_flag >= 3)
			printf
			    ("%d: Master pausing for Managers to check in (%d/%d)\n",
			     mypid, pgrps_ready, num_pgrps);
		/*
		 * We might receive the signal from the (last manager) before
		 * we issue a pause. In that case we might hang even if we have
		 * all the managers reported in. So set an alarm so that we can
		 * wake up.
		 */
		alarm(1);

		pause();
	}
	checklist_reset(0x03);
	if (debug_flag) {
		printf("Managers: \n");
		for (i = 0; i < num_pgrps; i++) {
			printf("%d ", child_checklist[i].pid);
		}
		printf("\n");
	}

	/* set up my signal processing */
	/* continue on ALRM */
	sa.sa_handler = wakeup;
	if (debug_flag >= 4)
		printf("%d: setting SIGALRM -> wakeup\n", mypid);
	k_sigaction(SIGALRM, &sa, NULL);
	/* reply to child on USR2 */
	sa.sa_handler = NULL;
	sa.sa_sigaction = ack_done;
	sa.sa_flags = SA_SIGINFO;
	if (debug_flag >= 1)
		printf("%d: setting SIGUSR2 -> ack_done\n", mypid);
	k_sigaction(SIGUSR2, &sa, NULL);
}

void ack_ready(int sig, siginfo_t * si, void *data)
{
	struct pid_list_item findit, *result;

	findit.pid = si->si_pid;

	result = bsearch(&findit, child_checklist, child_checklist_total,
			 sizeof(*child_checklist), checklist_cmp);
	if (result) {
		if (!(result->flag & 0x01)) {
			if (debug_flag >= 3)
				printf("%d: ack_ready, SIGUSR1 -> %d\n", mypid,
				       si->si_pid);
			kill(si->si_pid, SIGUSR1);
			result->flag = result->flag | 0x01;
			++pgrps_ready;
		} else {
			if (debug_flag >= 3)
				printf("%d: ack_ready, already acked %d\n",
				       mypid, si->si_pid);
		}
	} else {
		printf("received unexpected signal %d from %d",
		       sig, si->si_pid);
	}
}

void ack_done(int sig, siginfo_t * si, void *data)
{
	struct pid_list_item findit, *result;

	findit.pid = si->si_pid;

	result = bsearch(&findit, child_checklist, child_checklist_total,
			 sizeof(*child_checklist), checklist_cmp);
	if (result) {
		if (!(result->flag & 0x02)) {
			if (debug_flag >= 3)
				printf("%d: ack_done, SIGHUP -> %d\n", mypid,
				       si->si_pid);
			kill(si->si_pid, SIGHUP);
			++child_signal_counter;
			result->flag = result->flag | 0x02;
		} else {
			if (debug_flag >= 3)
				printf("%d: ack_done, already told %d\n", mypid,
				       si->si_pid);
		}
	} else {
		printf("received unexpected signal %d from %d",
		       sig, si->si_pid);
	}
}

/***************************************************************
 * cleanup() - performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 ***************************************************************/
void cleanup(void)
{
	int i;
	/* send SIGHUP to all pgroups */
	for (i = 0; i < num_pgrps; ++i) {
		/* try to do this as nicely as possible */
		kill(-child_checklist[i].pid, SIGQUIT);
		waitpid(child_checklist[i].pid, NULL, 0);
	}
	free(child_checklist);

}

/*********************************************************************
 * fork_pgrps() forks off a child, changes it's pgrp, then continues
 ********************************************************************/
void fork_pgrps(int pgrps_left)
{
	pid_t child;

	if (!(child_checklist = calloc(pgrps_left, sizeof(*child_checklist)))) {
		tst_brkm(TBROK, cleanup,
			 "%d: couldn't calloc child_checklist, errno=%d : %s",
			 mypid, errno, strerror(errno));
	}
	child_checklist_total = 0;
	while (pgrps_left) {
		if (debug_flag >= 1)
			printf("%d: forking new Manager\n", mypid);
		switch (child = fork()) {
		case -1:
			tst_brkm(TBROK | TERRNO, cleanup,
				 "fork() failed in fork_pgrps(%d)", pgrps_left);
			break;
		case 0:
			mypid = getpid();
			free(child_checklist);
			child_checklist = NULL;
			manager(num_procs);
			break;
		default:
			child_checklist[child_checklist_total++].pid = child;
			setpgid(child, child);
			if (debug_flag >= 3)
				printf("%d: fork_pgrps, SIGALRM -> %d\n", mypid,
				       child);
			kill(child, SIGALRM);
		}
		--pgrps_left;
	}
	qsort(child_checklist, child_checklist_total, sizeof(*child_checklist),
	      checklist_cmp);
}

void set_create_procs(int sig)
{
	if (debug_flag >= 3)
		printf("%d: Manager cleared to fork\n", getpid());
	create_procs_flag++;
	return;
}

/*********************************************************************
 * new_pgrg() - handle the creation of the pgrp managers and their
 *              children
 ********************************************************************/
void manager(int num_procs)
{
	struct sigaction sa;

	/* Wait for the parent to change our pgid before we start forking */
	while (!create_procs_flag) {
		alarm(1);
		if (debug_flag >= 3)
			printf("%d: Manager pausing, not cleared to fork\n",
			       mypid);
		pause();
	}

	/* set up the signal handling the children will use */

	/* ignore HUP */
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (debug_flag >= 4)
		printf("%d: setting SIGHUP -> SIG_IGN\n", mypid);
	k_sigaction(SIGHUP, &sa, NULL);

	/* We use ALRM to make sure that we don't miss the signal effects ! */
	sa.sa_handler = wakeup;
	if (debug_flag >= 4)
		printf("%d: setting SIGALRM -> wakeup\n", mypid);
	k_sigaction(SIGALRM, &sa, NULL);

	/* exit on QUIT */
	sa.sa_handler = graceful_exit;
	if (debug_flag >= 4)
		printf("%d: setting SIGQUIT -> graceful_exit\n", mypid);
	k_sigaction(SIGQUIT, &sa, NULL);

	/* start signaling on USR1 */
	sa.sa_handler = set_signal_parents;
	sigfillset(&sa.sa_mask);
	if (debug_flag >= 7)
		printf("%d: setting SIGUSR1 -> set_signal_parents\n", mypid);
	k_sigaction(SIGUSR1, &sa, NULL);
	/* stop signaling on USR2 */
	sa.sa_handler = clear_signal_parents;
	if (debug_flag >= 7)
		printf("%d: setting SIGUSR2 -> clear_signal_parents\n", mypid);
	k_sigaction(SIGUSR2, &sa, NULL);

	fork_procs(num_procs);
	sleep(1);		/* wait a sec to let all the children pause */

	/* now set up my signal handling */

	/* continue on ALRM */
	sa.sa_handler = wakeup;
	if (debug_flag >= 4)
		printf("%d: setting SIGALRM -> wakeup\n", mypid);
	k_sigaction(SIGALRM, &sa, NULL);
	/* mark ready confirmation on USR1 */
	sa.sa_handler = set_confirmed_ready;
	if (debug_flag >= 4)
		printf("%d: setting SIGUSR1 -> set_confirmed_ready\n", mypid);
	k_sigaction(SIGUSR1, &sa, NULL);
	/* reset our counter on HUP */
	sa.sa_handler = reset_counter;
	if (debug_flag >= 4)
		printf("%d: setting SIGHUP -> reset_counter\n", mypid);
	k_sigaction(SIGHUP, &sa, NULL);

	/* reply to child on USR2 */
	sa.sa_handler = NULL;
	sa.sa_sigaction = reply_to_child;
	sa.sa_flags = SA_SIGINFO;
	if (debug_flag >= 4)
		printf("%d: setting SIGUSR2 -> reply_to_child\n", mypid);
	k_sigaction(SIGUSR2, &sa, NULL);

	/* tell our parent that we are ready to rock */
	while (!confirmed_ready_flag) {
		if (debug_flag >= 3)
			printf("%d: Manager, SIGUSR1 -> %d\n", mypid,
			       getppid());
		if (kill(getppid(), SIGUSR1) == -1) {
			printf("%d: Couldn't signal master (%d) that we're "
			       "ready. %d: %s",
			       mypid, getppid(), errno, strerror(errno));
			exit(errno);
		}
		usleep(100);
	}

	/* handle pgroup management while the tests are running */
	while (1) {
		alarm(1);
		if (debug_flag >= 5)
			printf("%d: Manager pausing (%d/%d)\n",
			       mypid, child_signal_counter, num_procs);
		pause();
		if (child_signal_counter >= num_procs) {
			confirmed_ready_flag = 0;
			printf("%d: All %d children reported in\n",
			       mypid, child_signal_counter);
			while (child_signal_counter) {
				if (debug_flag >= 3)
					printf("%d: Manager, SIGUSR2 -> %d\n",
					       mypid, getppid());
				if (kill(getppid(), SIGUSR2) == -1) {
					printf("%d: Couldn't signal master "
					       "(%d) that we're ready. %d: %s\n",
					       mypid, getppid(), errno,
					       strerror(errno));
					exit(errno);
				}
				usleep(100);
			}
		}
	}
}

/* some simple signal handlers for the kids */
void graceful_exit(int sig)
{
	exit(0);
}

void set_signal_parents(int sig)
{
	if (debug_flag >= 8)
		printf("%d: Child start signaling\n", mypid);
	signal_parents_flag = 1;
}

void clear_signal_parents(int sig)
{
	if (debug_flag >= 8)
		printf("%d: Child stop signaling\n", mypid);
	signal_parents_flag = 0;
}

void set_confirmed_ready(int sig)
{

	if (debug_flag >= 3)
		printf("%d: Manager confirmed ready\n", mypid);
	confirmed_ready_flag = 1;
}

void reset_counter(int sig)
{
	checklist_reset(0xFF);
	child_signal_counter = 0;
	if (debug_flag >= 3)
		printf("%d: reset_counter\n", mypid);
}

void reply_to_child(int sig, siginfo_t * si, void *data)
{
	struct pid_list_item findit, *result;

	findit.pid = si->si_pid;

	result = bsearch(&findit, child_checklist, child_checklist_total,
			 sizeof(*child_checklist), checklist_cmp);
	if (result) {
		if (!result->flag) {
			if (debug_flag >= 6)
				printf("%d: reply_to_child, SIGUSR1 -> %d\n",
				       mypid, si->si_pid);
			kill(si->si_pid, SIGUSR2);
			++child_signal_counter;
			result->flag = 1;
		} else {
			if (debug_flag >= 6)
				printf("%d: reply_to_child, already told %d\n",
				       mypid, si->si_pid);
		}
	} else {
		tst_brkm(TBROK, cleanup,
			 "received unexpected signal from %d", si->si_pid);
	}
}

void wakeup(int sig)
{
	return;
}

/*************************************************
 * fork_procs() - create all the children
 ************************************************/
void fork_procs(int procs_left)
{
	pid_t child;

	if (!(child_checklist = calloc(procs_left, sizeof(*child_checklist)))) {
		tst_brkm(TBROK, cleanup,
			 "%d: couldn't calloc child_checklist, errno=%d : %s",
			 mypid, errno, strerror(errno));
	}
	child_checklist_total = 0;

	/* We are setting the flag for children, to avoid missing any signals */
	signal_parents_flag = 0;

	while (procs_left) {
		if (debug_flag >= 4)
			printf("%d: forking new child\n", mypid);
		switch (child = fork()) {
		case -1:
			tst_brkm(TBROK | TERRNO, cleanup,
				 "fork() failed in fork_procs(%d)", procs_left);
			break;
		case 0:
			mypid = getpid();
			while (1) {
				/* wait to start */
				if (debug_flag >= 8)
					printf("%d: child pausing\n", mypid);
				/*
				 * If we have already received the signal, we dont
				 * want to pause for it !
				 */
				while (!signal_parents_flag) {
					alarm(2);
					pause();
				}

				/* if we started, call mama */
				while (signal_parents_flag) {
					if (debug_flag >= 6)
						printf("%d: child, SIGUSR2 "
						       "-> %d\n",
						       mypid, getppid());
					if (kill(getppid(), SIGUSR2) == -1) {
						/* something went wrong */
						printf("%d: kill(ppid:%d, "
						       "SIGUSR2) failed. %d: %s",
						       mypid, getppid(), errno,
						       strerror(errno));
						exit(errno);
					}
					usleep(100);
				}
			}
			break;
		default:
			child_checklist[child_checklist_total++].pid = child;
		}
		procs_left--;
	}
	qsort(child_checklist, child_checklist_total, sizeof(*child_checklist),
	      checklist_cmp);
}

int checklist_cmp(const void *a, const void *b)
{
	const struct pid_list_item *pa = (const struct pid_list_item *)a;
	const struct pid_list_item *pb = (const struct pid_list_item *)b;

	return (pa->pid > pb->pid) - (pa->pid < pb->pid);
}

void checklist_reset(int bit)
{
	int i;
	for (i = 0; i < child_checklist_total; i++) {
		child_checklist[i].flag = child_checklist[i].flag & (~bit);
	}

}

static inline int k_sigaction(int sig, struct sigaction *sa, struct sigaction *osa)
{
	int ret;
	if ((ret = sigaction(sig, sa, osa)) == -1) {
		tst_brkm(TBROK | TERRNO, cleanup, "sigaction(%d, ...) failed",
			 sig);
	}
	return ret;
}
