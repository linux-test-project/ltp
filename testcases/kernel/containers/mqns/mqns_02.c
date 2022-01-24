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
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*
* Author: Nadia Derbey <Nadia.Derbey@bull.net>
*
* Check mqns isolation: child mqns cannot be accessed from father
*
* Mount mqueue fs
* unshare
* In unshared process:
*    Mount newinstance mqueuefs
*    Create a posix mq -->mq1
* Check that mq1 is not readable from father
*
* Changelog:
*	Dec 16: accomodate new mqns semantics (Serge Hallyn)

***************************************************************************/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mqns.h"
#include "mqns_helper.h"

char *TCID = "posixmq_namespace_02";
int TST_TOTAL = 1;

int p1[2];
int p2[2];

int check_mqueue(void *vtest)
{
	char buf[30];
	mqd_t mqd;

	(void) vtest;

	close(p1[1]);
	close(p2[0]);

	if (read(p1[0], buf, 3) < 0) {
		perror("read(p1[0], ..) failed");
		exit(1);
	} else {

		mqd =
		    tst_syscall(__NR_mq_open, NOSLASH_MQ1,
			    O_RDWR | O_CREAT | O_EXCL, 0777, NULL);
		if (mqd == -1) {
			if (write(p2[1], "mqfail", strlen("mqfail") + 1) < 0) {
				perror("write(p2[1], \"mqfail\", ..) failed");
				exit(1);
			}
		} else {

			if (write(p2[1], "mqopen", strlen("mqopen") + 1) < 0) {
				perror("write(p2[1], \"mqopen\", ..) failed");
				exit(1);
			} else {

				if (read(p1[0], buf, 5) < 0) {
					perror("read(p1[0], ..) failed");
					exit(1);
				} else {

					/* destroy the mqueue */
					if (mq_close(mqd) < 0) {
						perror("mq_close(mqd) failed");
						exit(1);
					} else if (tst_syscall(__NR_mq_unlink,
							   NOSLASH_MQ1) < 0) {
						perror("mq_unlink(" NOSLASH_MQ1
						       ") failed");
						exit(1);
					} else if (write(p2[1], "done",
							 strlen("done") + 1)
						   < 0) {
						perror("write(p2[1], "
						       "\"done\", ..) failed");
						exit(1);
					}

				}

			}

		}

	}
	exit(0);

}

static void setup(void)
{
	tst_require_root();
	check_mqns();
}

int main(int argc, char *argv[])
{
	int r;
	mqd_t mqd;
	char buf[30];
	int use_clone = T_UNSHARE;

	setup();

	if (argc == 2 && strcmp(argv[1], "-clone") == 0) {
		tst_resm(TINFO,
			 "Testing posix mq namespaces through clone(2).");
		use_clone = T_CLONE;
	} else
		tst_resm(TINFO,
			 "Testing posix mq namespaces through unshare(2).");

	if (pipe(p1) == -1 || pipe(p2) == -1) {
		tst_brkm(TBROK | TERRNO, NULL, "pipe");
	}

	/* fire off the test */
	r = do_clone_unshare_test(use_clone, CLONE_NEWIPC, check_mqueue, NULL);
	if (r < 0) {
		tst_brkm(TFAIL, NULL, "failed clone/unshare");
	}

	tst_resm(TINFO, "Checking namespaces isolation (child to parent)");

	close(p1[0]);
	close(p2[1]);
	if (write(p1[1], "go", strlen("go") + 1) < 0) {
		tst_brkm(TBROK, NULL, "write(p1[1], \"go\", ..) failed");
	}

	if (read(p2[0], buf, 7) < 0) {
		tst_resm(TBROK | TERRNO, "read(p2[0], ..) failed");
	} else if (!strcmp(buf, "mqfail")) {
		tst_resm(TFAIL, "child process could not create mqueue");
		umount(DEV_MQUEUE);
	} else if (strcmp(buf, "mqopen")) {
		tst_resm(TFAIL, "child process could not create mqueue");
		umount(DEV_MQUEUE);
	} else {
		mqd = tst_syscall(__NR_mq_open, NOSLASH_MQ1, O_RDONLY);
		if (mqd == -1) {
			tst_resm(TPASS,
				 "Parent process can't see the mqueue");
		} else {
			tst_resm(TFAIL | TERRNO,
				 "Parent process found mqueue");
			mq_close(mqd);
		}
		if (write(p1[1], "cont", 5) < 0) {
			tst_resm(TBROK | TERRNO, "write(p1[1], ..) failed");
		}
		read(p2[0], buf, 7);
	}

	tst_exit();
}
