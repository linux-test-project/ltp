/*
* Copyright (c) International Business Machines Corp., 2009
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
* Author: Serge Hallyn <serue@us.ibm.com>
*
* Check mqueuefs lifetime
* . parent creates /dev/mqueue2
* . child mounts mqueue there
* . child does mq_open("/ab")
* . parent checks for /dev/mqueue2
* . child exits
* . parent checks for /dev/mqueue2
* . parent tries 'touch /dev/mqueue2/dd' -> should fail
* . parent umounts /dev/mqueue2

***************************************************************************/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "mqns.h"
#include "mqns_helper.h"

char *TCID = "posixmq_namespace_04";
int TST_TOTAL = 1;

int p1[2];
int p2[2];

#define FNAM1 DEV_MQUEUE2 SLASH_MQ1
#define FNAM2 DEV_MQUEUE2 SLASH_MQ2

int check_mqueue(void *vtest)
{
	char buf[30];
	mqd_t mqd;
	int rc;

	(void) vtest;

	close(p1[1]);
	close(p2[0]);

	read(p1[0], buf, 3);	/* go */

	mqd = ltp_syscall(__NR_mq_open, NOSLASH_MQ1, O_RDWR | O_CREAT | O_EXCL,
		0755, NULL);
	if (mqd == -1) {
		write(p2[1], "mqfail", 7);
		tst_exit();
	}

	mq_close(mqd);

	rc = mount("mqueue", DEV_MQUEUE2, "mqueue", 0, NULL);
	if (rc == -1) {
		perror("mount");
		write(p2[1], "mount", 6);
		tst_exit();
	}

	write(p2[1], "go", 3);
	read(p1[0], buf, 3);

	tst_exit();
}

static void setup(void)
{
	tst_require_root();
	check_mqns();
}

int main(int argc, char *argv[])
{
	int rc;
	int status;
	char buf[30];
	struct stat statbuf;
	int use_clone = T_UNSHARE;

	setup();

	if (argc == 2 && strcmp(argv[1], "-clone") == 0) {
		tst_resm(TINFO,
			 "Testing posix mq namespaces through clone(2).");
		use_clone = T_CLONE;
	} else
		tst_resm(TINFO,
			 "Testing posix mq namespaces through unshare(2).");

	if (pipe(p1) == -1) {
		perror("pipe");
		exit(EXIT_FAILURE);
	}
	if (pipe(p2) == -1) {
		perror("pipe");
		exit(EXIT_FAILURE);
	}

	mkdir(DEV_MQUEUE2, 0755);

	tst_resm(TINFO, "Checking mqueue filesystem lifetime");

	/* fire off the test */
	rc = do_clone_unshare_test(use_clone, CLONE_NEWIPC, check_mqueue, NULL);
	if (rc < 0) {
		tst_resm(TFAIL, "failed clone/unshare");
		goto fail;
	}

	close(p1[0]);
	close(p2[1]);
	write(p1[1], "go", 3);

	read(p2[0], buf, 7);
	if (!strcmp(buf, "mqfail")) {
		tst_resm(TFAIL, "child process could not create mqueue");
		goto fail;
	} else if (!strcmp(buf, "mount")) {
		tst_resm(TFAIL, "child process could not mount mqueue");
		goto fail;
	}

	rc = stat(FNAM1, &statbuf);
	if (rc == -1) {
		perror("stat");
		write(p1[1], "go", 3);
		tst_resm(TFAIL, "parent could not see child's created mq");
		goto fail;
	}
	write(p1[1], "go", 3);

	rc = wait(&status);
	if (rc == -1) {
		perror("wait");
		tst_resm(TFAIL, "error while parent waited on child to exit");
		goto fail;
	}
	if (!WIFEXITED(status)) {
		tst_resm(TFAIL, "Child did not exit normally (status %d)",
			 status);
		goto fail;
	}
	rc = stat(FNAM1, &statbuf);
	if (rc == -1) {
		tst_resm(TFAIL,
			 "parent's view of child's mq died with child");
		goto fail;
	}

	rc = creat(FNAM2, 0755);
	if (rc != -1) {
		tst_resm(TFAIL,
			 "parent was able to create a file in dead child's mqfs");
		goto fail;
	}

	tst_resm(TPASS, "Child mqueue fs still visible for parent");

fail:
	umount(DEV_MQUEUE2);
	rmdir(DEV_MQUEUE2);

	tst_exit();
}
