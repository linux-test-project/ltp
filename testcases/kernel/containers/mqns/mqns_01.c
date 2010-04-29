/*
* Copyright (c) International Business Machines Corp., 2009
* Copyright (c) Nadia Derbey, 2009
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
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
* Author: Nadia Derbey <Nadia.Derbey@bull.net>
*
* Check mqns isolation: father mqns cannot be accessed from newinstance
*
* Mount mqueue fs
* Create a posix mq -->mq1
* unshare
* In unshared process:
*    Mount newinstance mqueuefs
*    Check that mq1 is not readable from new ns

***************************************************************************/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sys/wait.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "mqns.h"

char *TCID = "posixmq_namespace_01";
int TST_TOTAL=1;

int p1[2];
int p2[2];

int check_mqueue(void *vtest)
{
	char buf[30];
	mqd_t mqd;

	close(p1[1]);
	close(p2[0]);

	if (read(p1[0], buf, strlen("go") + 1) < 0)
		tst_resm(TBROK | TERRNO, "read(p1[0], ...) failed");
	mqd = syscall(__NR_mq_open, NOSLASH_MQ1, O_RDONLY);
	if (mqd == -1) {
		if (write(p2[1], "notfnd", strlen("notfnd") + 1) < 0)
			tst_resm(TBROK | TERRNO, "write(p2[1], ...) failed");
	} else {
		if (write(p2[1], "exists", strlen("exists") + 1) < 0)
			tst_resm(TBROK | TERRNO, "write(p2[1], \"exists\", 7) failed");
		else if (mq_close(mqd) < 0)
			tst_resm(TBROK | TERRNO, "mq_close(mqd) failed");
	}

	tst_exit();
}

int
main(int argc, char *argv[])
{
	int r;
	mqd_t mqd;
	char buf[30];
	int use_clone = T_UNSHARE;

	if (argc == 2 && strcmp(argv[1], "-clone") == 0) {
		tst_resm(TINFO, "Testing posix mq namespaces through clone(2).\n");
		use_clone = T_CLONE;
	} else
		tst_resm(TINFO, "Testing posix mq namespaces through unshare(2).\n");

	if (pipe(p1) == -1) { perror("pipe"); exit(EXIT_FAILURE); }
	if (pipe(p2) == -1) { perror("pipe"); exit(EXIT_FAILURE); }

	mqd = syscall(__NR_mq_open, NOSLASH_MQ1, O_RDWR|O_CREAT|O_EXCL, 0777,
			NULL);
	if (mqd == -1) {
		perror("mq_open");
		tst_resm(TFAIL, "mq_open failed\n");
		tst_exit();
	}

	tst_resm(TINFO, "Checking namespaces isolation from parent to child\n");
	/* fire off the test */
	r = do_clone_unshare_test(use_clone, CLONE_NEWIPC, check_mqueue, NULL);
	if (r < 0) {
		tst_resm(TFAIL, "failed clone/unshare\n");
		mq_close(mqd);
		syscall(__NR_mq_unlink, NOSLASH_MQ1);
		tst_exit();
	}

	close(p1[0]);
	close(p2[1]);
	if (write(p1[1], "go", strlen("go") + 1) < 0)
		tst_resm(TBROK | TERRNO, "write(p1[1], \"go\", ...) failed");
	else if (read(p2[0], buf, 7) < 0)
		tst_resm(TBROK | TERRNO, "read(p2[0], buf, ...) failed");
	else {
		if (!strcmp(buf, "exists")) {
			tst_resm(TFAIL, "child process found mqueue\n");
		} else if (!strcmp(buf, "notfnd")) {
			tst_resm(TPASS, "child process didn't find mqueue\n");
		} else {
			tst_resm(TFAIL, "UNKNOWN RESULT\n");
		}
	}

	/* destroy the mqueue */
	mq_close(mqd);
	syscall(__NR_mq_unlink, NOSLASH_MQ1);

	tst_exit();
}
