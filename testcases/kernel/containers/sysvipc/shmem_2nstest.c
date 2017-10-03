/* *************************************************************************
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
* Author: Veerendra C <vechandr@in.ibm.com>
*
* Test Assertion:
* This testcase verifies the Shared Memory isoloation in 2 containers.
* It tries to create/access a Shared Memory created with the same KEY.
*
* Description:
* Create 2 'containers' with the below flag value
*   Flag = clone, clone(CLONE_NEWIPC), or unshare(CLONE_NEWIPC)
* In Cont1, create Shared Memory segment with key 124426L
* In Cont2, try to access the MQ created in Cont1.
* PASS :
* 		If flag = None and the shmem seg is accessible in Cont2.
*		If flag = unshare/clone and the shmem seg is not accessible in Cont2.
* 		If shmem seg is not accessible in Cont2,
*		creates new shmem with same key to double check isloation in IPCNS.
*
* FAIL :
* 		If flag = none and the shmem seg is not accessible.
* 		If flag = unshare/clone and shmem seg is accessible in Cont2.
*		If the new shmem seg creation Fails.
***************************************************************************/

#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <libclone.h>
#include "test.h"
#include "safe_macros.h"
#include "ipcns_helper.h"

#define TESTKEY    124426L
#define UNSHARESTR "unshare"
#define CLONESTR   "clone"
#define NONESTR    "none"

char *TCID = "shmem_2nstest";
int TST_TOTAL = 1;
int p2[2];
int p1[2];

/*
 * check_shmem1() does not read -- it writes to check_shmem2() when it's done.
 */
int check_shmem1(void *vtest)
{
	int id1;

	(void) vtest;

	close(p1[0]);

	/* first create the key */
	id1 = shmget(TESTKEY, 100, IPC_CREAT);
	if (id1 == -1)
		tst_brkm(TFAIL | TERRNO, NULL, "shmget failed");

	tst_resm(TINFO, "Cont1: Able to create shared mem segment");
	write(p1[1], "done", 5);
	tst_exit();
}

/*
 * check_shmem2() reads from check_shmem1() and writes to main() when it's done.
 */
int check_shmem2(void *vtest)
{
	char buf[3];
	int id2;

	(void) vtest;

	close(p1[1]);
	close(p2[0]);

	read(p1[0], buf, 3);
	/* Trying to access shmem, if not existing create new shmem */
	id2 = shmget(TESTKEY, 100, 0);
	if (id2 == -1) {
		id2 = shmget(TESTKEY, 100, IPC_CREAT);
		if (id2 == -1)
			tst_resm(TFAIL | TERRNO, "shmget failed");
		else
			tst_resm(TINFO,
				 "Cont2: Able to allocate shmem seg with "
				 "the same key");
		write(p2[1], "notfnd", 7);
	} else
		write(p2[1], "exists", 7);

	tst_exit();
}

static void setup(void)
{
	tst_require_root();
	check_newipc();
}

int main(int argc, char *argv[])
{
	int ret, use_clone = T_NONE;
	char *tsttype = NONESTR;
	char buf[7];
	int id;

	setup();

	if (argc != 2) {
		tst_resm(TINFO, "Usage: %s <clone| unshare| none>", argv[0]);
		tst_resm(TINFO, " where clone, unshare, or fork specifies"
			 " unshare method.");
		tst_exit();
	}

	/* Using PIPE's to sync between containers and Parent */
	SAFE_PIPE(NULL, p1);
	SAFE_PIPE(NULL, p2);

	if (strcmp(argv[1], "clone") == 0) {
		use_clone = T_CLONE;
		tsttype = CLONESTR;
	} else if (strcmp(argv[1], "unshare") == 0) {
		use_clone = T_UNSHARE;
		tsttype = UNSHARESTR;
	}

	tst_resm(TINFO, "Shared Memory namespace test : %s", tsttype);

	/* Create 2 containers */
	ret =
	    do_clone_unshare_test(use_clone, CLONE_NEWIPC, check_shmem1, NULL);
	if (ret < 0)
		tst_brkm(TFAIL, NULL, "clone/unshare failed");

	ret =
	    do_clone_unshare_test(use_clone, CLONE_NEWIPC, check_shmem2, NULL);
	if (ret < 0)
		tst_brkm(TFAIL, NULL, "clone/unshare failed");

	close(p2[1]);
	read(p2[0], buf, 7);

	if (strcmp(buf, "exists") == 0) {
		if (use_clone == T_NONE)
			tst_resm(TPASS,
				 "Plain cloned process able to access shmem "
				 "segment created");
		else
			tst_resm(TFAIL,
				 "%s : In namespace2 found the shmem segment "
				 "created in Namespace1", tsttype);
	} else {
		if (use_clone == T_NONE)
			tst_resm(TFAIL,
				 "Plain cloned process didn't find shmem seg");
		else
			tst_resm(TPASS,
				 "%s : In namespace2 unable to access the shmem seg "
				 "created in Namespace1", tsttype);
	}
	/* destroy the key */

	id = shmget(TESTKEY, 100, 0);
	shmctl(id, IPC_RMID, NULL);

	tst_exit();
}
