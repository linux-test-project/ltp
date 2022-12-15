/*
* Copyright (c) International Business Machines Corp., 2007
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
* the GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*
***************************************************************************
 * Copyright 2007 IBM
 * Author: Serge Hallyn <serue@us.ibm.com>
 *
 * test1:
	P1: A=gethostname
	P2: B=gethostname
	Ensure(A==B)

 * test2:
	P1: sethostname(A);
	P2: (wait); B=gethostname
	Ensure (A==B)

 * test3:
	P1: A=gethostname; unshare(utsname); sethostname(newname); C=gethostname
	P2: B=gethostname; (wait); (wait); D=gethostname
	Ensure (A==B && A==D && C!=D)

 * test4:
	P1: A=gethostname; unshare(utsname); (wait); C=gethostname
	P2: B=gethostname; (wait); sethostname(newname); D=gethostname
	Ensure (A==B && A==C && C!=D)

 * test5:
	P1: drop_privs(); unshare(utsname); (wait); C=gethostname
	P2: (wait); sethostname(B); D=gethostname
	Ensure (B==C==D) and state is ok.
 *
 */

#define _GNU_SOURCE 1
#include <sys/wait.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "libclone.h"
#include "test.h"
#include "safe_macros.h"

char *TCID = "uts_namespace";
int TST_TOTAL = 1;

static int dummy_child(void *v)
{
	(void) v;
	return 0;
}

static void check_newuts(void)
{
	int pid, status;

	pid = do_clone_unshare_test(T_CLONE, CLONE_NEWUTS, dummy_child, NULL);
	if (pid == -1)
		tst_brkm(TCONF | TERRNO, NULL, "CLONE_NEWUTS not supported");

	SAFE_WAIT(NULL, &status);
}

int drop_root(void)
{
	int ret;
	ret = setresuid(1000, 1000, 1000);
	if (ret) {
		perror("setresuid");
		exit(4);
	}
	return 1;
}

#define HLEN 100
#define NAME1 "serge1"
#define NAME2 "serge2"

int p1fd[2], p2fd[2];
static char oldhost[HLEN];
pid_t cpid;

void picknewhostname(char *orig, char *new)
{
	memset(new, 0, HLEN);
	if (strcmp(orig, NAME1) == 0)
		strcpy(new, NAME2);
	else
		strcpy(new, NAME1);
}

void zeroize(char *s)
{
	memset(s, 0, HLEN);
}

char *tsttype;
int P1(void *vtest)
{
	char hostname[HLEN], newhostname[HLEN], rhostname[HLEN];
	int err;
	int len;
	int testnum;

	testnum = atoi((char *)vtest);

	close(p1fd[1]);
	close(p2fd[0]);

	switch (testnum) {
	case 1:
		gethostname(hostname, HLEN);
		zeroize(rhostname);
		len = read(p1fd[0], rhostname, HLEN);
		if (strcmp(hostname, rhostname) == 0) {
			tst_resm(TPASS, "test 1 (%s): success", tsttype);
			tst_exit();
		}
		tst_brkm(TFAIL, NULL,
			 "test 1 (%s): hostname 1 %s, hostname 2 %s",
			 tsttype, hostname, rhostname);
	case 2:
		gethostname(hostname, HLEN);
		picknewhostname(hostname, newhostname);
		err = sethostname(newhostname, strlen(newhostname));
		write(p2fd[1], "1", 1);
		if (err == -1) {
			tst_brkm(TFAIL, NULL,
				 "test 2 (%s): failed to sethostname",
				 tsttype);
		}
		zeroize(rhostname);
		len = read(p1fd[0], rhostname, HLEN);
		if (strcmp(newhostname, rhostname) == 0) {
			tst_resm(TPASS, "test 2 (%s): success", tsttype);
			tst_exit();
		}
		tst_brkm(TFAIL, NULL,
			 "test 2 (%s) hostname 1 %s, hostname 2 %s",
			 tsttype, newhostname, rhostname);
	case 3:
		gethostname(hostname, HLEN);
		picknewhostname(hostname, newhostname);
		err = sethostname(newhostname, strlen(newhostname));
		write(p2fd[1], "1", 1);
		if (err == -1) {
			tst_brkm(TFAIL, NULL,
				 "test 3 (%s): failed to sethostname",
				 tsttype);
		}

		zeroize(rhostname);
		len = read(p1fd[0], rhostname, HLEN);
		if (strcmp(newhostname, rhostname) == 0) {
			tst_brkm(TFAIL,
				 NULL,
				 "test 3 (%s): hostname 1 %s, hostname 2 %s, these should have been different",
				 tsttype, newhostname, rhostname);
		}
		if (strcmp(hostname, rhostname) == 0) {
			tst_resm(TPASS, "test 3 (%s): success", tsttype);
			tst_exit();
		}
		tst_brkm(TFAIL,
			 NULL,
			 "test 3 (%s): hostname 1 %s, hostname 2 %s, should have been same",
			 tsttype, hostname, rhostname);

	case 4:
		gethostname(hostname, HLEN);
		write(p2fd[1], "1", 1);	/* tell p2 to go ahead and sethostname */
		zeroize(rhostname);
		len = read(p1fd[0], rhostname, HLEN);
		gethostname(newhostname, HLEN);
		if (strcmp(hostname, newhostname) != 0) {
			tst_brkm(TFAIL,
				 NULL,
				 "test 4 (%s): hostname 1 %s, hostname 2 %s, should be same",
				 tsttype, hostname, newhostname);
		}
		if (strcmp(hostname, rhostname) == 0) {
			tst_brkm(TFAIL,
				 NULL,
				 "test 4 (%s): hostname 1 %s, hostname 2 %s, should be different",
				 tsttype, hostname, rhostname);
		}
		tst_resm(TPASS, "test 4 (%s): successful", tsttype);
		tst_exit();
	case 5:
		write(p2fd[1], "1", 1);	/* tell p2 to go ahead and sethostname */
		zeroize(rhostname);
		len = read(p1fd[0], rhostname, HLEN);
		gethostname(newhostname, HLEN);
		if (strcmp(rhostname, newhostname) != 0) {
			tst_brkm(TFAIL,
				 NULL,
				 "test 5 (%s): hostnames %s and %s should be same",
				 tsttype, rhostname, newhostname);
		}
		tst_resm(TPASS, "test 5 (%s): successful", tsttype);
		tst_exit();
	default:
		break;
	}
	tst_exit();
}

int P2(void *vtest)
{
	char hostname[HLEN], newhostname[HLEN];
	int len;
	int testnum;

	testnum = atoi((char *)vtest);

	close(p1fd[0]);
	close(p2fd[1]);

	switch (testnum) {
	case 1:
		gethostname(hostname, HLEN);
		write(p1fd[1], hostname, strlen(hostname));
		break;
	case 2:
	case 3:
		len = 0;
		while (!len) {
			len = read(p2fd[0], hostname, 1);
		}
		gethostname(hostname, HLEN);
		write(p1fd[1], hostname, strlen(hostname));
		break;
	case 4:
	case 5:
		len = 0;
		while (!len) {
			len = read(p2fd[0], hostname, 1);
		}
		if (hostname[0] == '0') {
			tst_resm(TPASS, "P2: P1 claims error");
			return 0;
		}
		gethostname(hostname, HLEN);
		picknewhostname(hostname, newhostname);
		sethostname(newhostname, strlen(newhostname));
		write(p1fd[1], newhostname, strlen(newhostname));
		break;
	default:
		tst_resm(TFAIL, "undefined test: %d", testnum);
		break;
	}
	return 0;
}

static void setup(void)
{
	gethostname(oldhost, HLEN);
	tst_require_root();
	check_newuts();
}

static void cleanup(void)
{
	sethostname(oldhost, strlen(oldhost));
}

#define UNSHARESTR "unshare"
#define CLONESTR "clone"
int main(int argc, char *argv[])
{
	int r, pid, use_clone = T_UNSHARE;
	int testnum;
	void *vtest;

	setup();
	if (argc != 3) {
		tst_resm(TFAIL, "Usage: %s <clone|unshare> <testnum>",
			 argv[0]);
		tst_resm(TFAIL,
			 " where clone or unshare specifies unshare method,");
		tst_resm(TFAIL, " and testnum is between 1 and 5 inclusive");
		exit(2);
	}
	if (pipe(p1fd) == -1) {
		perror("pipe");
		exit(EXIT_FAILURE);
	}
	if (pipe(p2fd) == -1) {
		perror("pipe");
		exit(EXIT_FAILURE);
	}

	tsttype = UNSHARESTR;
	if (strcmp(argv[1], "clone") == 0) {
		use_clone = T_CLONE;
		tsttype = CLONESTR;
	}

	testnum = atoi(argv[2]);

	vtest = (void *)argv[2];
	switch (testnum) {
	case 1:
	case 2:
		r = do_clone_unshare_tests(T_NONE, 0, P1, vtest, P2, vtest);
		break;
	case 3:
	case 4:
		r = do_clone_unshare_tests(use_clone, CLONE_NEWUTS,
					   P1, vtest, P2, vtest);
		break;
	case 5:
		pid = fork();
		if (pid == -1) {
			perror("fork");
			exit(2);
		}
		if (pid == 0) {
			if (!drop_root()) {
				tst_brkm(TFAIL, NULL, "failed to drop root.");
			}
			r = do_clone_unshare_test(use_clone, CLONE_NEWUTS,
						  P1, vtest);
			write(p2fd[1], "0", 1);	/* don't let p2 hang */
			exit(0);
		} else {
			P2(vtest);
		}
		break;
	default:
		tst_resm(TFAIL,
			 "testnum should be between 1 and 5 inclusive.");
		break;
	}

	cleanup();
	tst_exit();
}
