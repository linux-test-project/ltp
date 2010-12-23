/*
 * a race in pid generation that causes pids to be reused immediately
 *
 * From the mainline commit 5fdee8c4a5e1800489ce61963208f8cc55e42ea1:
 * 
 * A program that repeatedly forks and waits is susceptible to having
 * the same pid repeated, especially when it competes with another
 * instance of the same program.  This is really bad for bash
 * implementation.  Furthermore, many shell scripts assume that pid
 * numbers will not be used for some length of time.
 * 
 * Race Description:
 *
 * A                                B
 *
 * // pid == offset == n            // pid == offset == n + 1
 * test_and_set_bit(offset, map->page)
 *                                  test_and_set_bit(offset, map->page);
 *                                  pid_ns->last_pid = pid;
 * pid_ns->last_pid = pid;
 *                                  // pid == n + 1 is freed (wait())
 *
 *                                  // Next fork()...
 *                                  last = pid_ns->last_pid; // == n
 *                                  pid = last + 1;
 *
 * Copyright (C) 2010  Red Hat, Inc.
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it
 * is free of the rightful claim of any third person regarding
 * infringement or the like.  Any license provided herein, whether
 * implied or otherwise, applies only to this software file.  Patent
 * licenses, if any, provided herein do not apply to combinations of
 * this program with other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "test.h"
#include "usctest.h"

char *TCID = "fork13";
int TST_TOTAL = 1;

static char pid_max[BUFSIZ];

#define PATH	"/proc/sys/kernel/pid_max"
#define PIDMAX	32768
#define RETURN	256

static void setup(void);
static int pid_distance(pid_t first, pid_t second);
static void cleanup(void);
static void check(void);

int main(int argc, char* argv[])
{
	/* message returned from parse_opts */
	char *msg;

	msg = parse_opts(argc, argv, NULL, NULL);
	if (msg != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	setup();
	check();
	cleanup();
	tst_exit();
}

void check(void)
{
	long lc;
	pid_t last_pid = 0;
	pid_t pid;
	int child_exit_code, distance, reaped, status;

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		Tst_count = 0;
		child_exit_code = lc % RETURN;
		switch (pid = fork()) {
		case -1:
			tst_brkm(TBROK|TERRNO, cleanup, "fork");
		case 0:
			exit(child_exit_code);
		default:
			if (lc > 0) {
				distance = pid_distance(last_pid, pid);
				if (distance == 0) {
					tst_resm(TFAIL,
						"Unexpected pid sequence: "
						"previous fork: pid=%d, "
						"current fork: pid=%d for "
						"iteration=%ld.", last_pid, pid,
						lc);
					return;
				}
			}
			last_pid = pid;

			reaped = wait(&status);
			if (reaped != pid) {
				tst_resm(TFAIL,
					"Wait return value: expected pid=%d, "
					"got %d, iteration %ld.", pid, reaped,
					lc);
				return;
			}
			else if (WEXITSTATUS(status) != child_exit_code) {
				tst_resm(TFAIL, "Unexpected exit status %x, "
					"iteration %ld.", WEXITSTATUS(status),
					lc);
				return;
			}
		}
	}
	tst_resm(TPASS, "%ld pids forked, all passed", lc);
}

void setup(void)
{
	FILE *fp;
	int fd;
	char buf[BUFSIZ];

	tst_require_root(NULL);

	tst_sig(FORK, DEF_HANDLER, cleanup);
	TEST_PAUSE;
	/* Backup pid_max value. */
	fp = fopen(PATH, "r+");
	if (fp == NULL)
		tst_brkm(TBROK|TERRNO, cleanup, "fopen");
	if (fgets(pid_max, BUFSIZ, fp) == NULL)
		tst_brkm(TBROK|TERRNO, cleanup, "fgets");
	fclose(fp);

	fd = open(PATH, O_WRONLY);
	if (fd == -1)
		tst_resm(TBROK|TERRNO, "open");
	sprintf(buf, "%d", PIDMAX);
	if (write(fd, buf, strlen(buf)) != strlen(buf))
		tst_resm(TBROK|TERRNO, "write");
	close(fd);
}

void cleanup(void)
{
	int fd;

	fd = open(PATH, O_WRONLY);
	if (fd == -1)
		tst_resm(TWARN|TERRNO, "open");
	if (write(fd, pid_max, strlen(pid_max)) != strlen(pid_max))
		tst_resm(TWARN|TERRNO, "write");
	close(fd);

	TEST_CLEANUP;
}

/* The distance mod PIDMAX between two pids, where the first pid is
   expected to be smaller than the second. */
int pid_distance(pid_t first, pid_t second)
{
	return (second + PIDMAX - first) % PIDMAX;
}
