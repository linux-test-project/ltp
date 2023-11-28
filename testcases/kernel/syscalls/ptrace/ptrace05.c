/*
 ******************************************************************************
 *
 *   ptrace05 - an app which ptraces itself as per arbitrarily specified signals,
 *   over a user specified range.
 *
 *   Copyright (C) 2009, Ngie Cooper
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 ******************************************************************************
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <libgen.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ptrace.h>

#include "test.h"
#include "lapi/signal.h"

char *TCID = "ptrace05";
int TST_TOTAL = 0;

int usage(const char *);

int usage(const char *argv0)
{
	fprintf(stderr, "usage: %s [start-signum] [end-signum]\n", argv0);
	return 1;
}

int main(int argc, char **argv)
{

	int end_signum = -1;
	int signum;
	int start_signum = -1;
	int status;

	pid_t child;

	tst_parse_opts(argc, argv, NULL, NULL);

	if (start_signum == -1) {
		start_signum = 0;
	}
	if (end_signum == -1) {
		end_signum = SIGRTMAX;
	}

	for (signum = start_signum; signum <= end_signum; signum++) {

		if (signum >= __SIGRTMIN && signum < SIGRTMIN)
			continue;

		switch (child = fork()) {
		case -1:
			tst_brkm(TBROK | TERRNO, NULL, "fork() failed");
		case 0:

			if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) != -1) {
				tst_resm(TINFO, "[child] Sending kill(.., %d)",
					 signum);
				if (kill(getpid(), signum) < 0) {
					tst_resm(TINFO | TERRNO,
						 "[child] kill(.., %d) failed.",
						 signum);
				}
			} else {

				/*
				 * This won't increment the TST_COUNT var.
				 * properly, but it'll show up as a failure
				 * nonetheless.
				 */
				tst_resm(TFAIL | TERRNO,
					 "Failed to ptrace(PTRACE_TRACEME, ...) "
					 "properly");

			}
			/* Shouldn't get here if signum == 0. */
			exit((signum == 0 ? 0 : 2));
			break;

		default:

			waitpid(child, &status, 0);

			switch (signum) {
			case 0:
				if (WIFEXITED(status)
				    && WEXITSTATUS(status) == 0) {
					tst_resm(TPASS,
						 "kill(.., 0) exited "
						 "with 0, as expected.");
				} else {
					tst_resm(TFAIL,
						 "kill(.., 0) didn't exit "
						 "with 0.");
				}
				break;
			case SIGKILL:
				if (WIFSIGNALED(status)) {
					/* SIGKILL must be uncatchable. */
					if (WTERMSIG(status) == SIGKILL) {
						tst_resm(TPASS,
							 "Killed with SIGKILL, "
							 "as expected.");
					} else {
						tst_resm(TPASS,
							 "Didn't die with "
							 "SIGKILL (?!) ");
					}
				} else if (WIFEXITED(status)) {
					tst_resm(TFAIL,
						 "Exited unexpectedly instead "
						 "of dying with SIGKILL.");
				} else if (WIFSTOPPED(status)) {
					tst_resm(TFAIL,
						 "Stopped instead of dying "
						 "with SIGKILL.");
				}
				break;
				/* All other processes should be stopped. */
			default:
				if (WIFSTOPPED(status)) {
					tst_resm(TPASS, "Stopped as expected");
				} else {
					tst_resm(TFAIL, "Didn't stop as "
						 "expected.");
					if (kill(child, 0)) {
						tst_resm(TINFO,
							 "Is still alive!?");
					} else if (WIFEXITED(status)) {
						tst_resm(TINFO,
							 "Exited normally");
					} else if (WIFSIGNALED(status)) {
						tst_resm(TINFO,
							 "Was signaled with "
							 "signum=%d",
							 WTERMSIG(status));
					}

				}

				break;

			}

		}
		/* Make sure the child dies a quick and painless death ... */
		kill(child, 9);

	}

	tst_exit();

}
