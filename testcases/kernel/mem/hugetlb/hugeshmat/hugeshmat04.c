/*
 *   Copyright (c) Linux Test Project, 2014
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *   02110-1301 USA
 */

/*
 * DESCRIPTION
 *	hugeshmat04 - test for hugepage leak inspection.
 *
 *	It is a regression test for shared hugepage leak, when over 1GB
 *	shered memory was alocated in hugepage, the hugepage is not released
 *	though process finished.
 *
 *	You need more than 2GB memory in test job
 *
 * HISTORY
 * 	05/2014 - Written by Fujistu Corp.
 *	12/2014 - Port to LTP by Li Wang.
 *
 * RESTRICTIONS
 * 	test must be run at root
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/wait.h>

#include "test.h"
#include "mem.h"
#include "hugetlb.h"

#define SIZE	(1024 * 1024 * 1024)
#define BOUNDARY (1024 * 1024 * 1024)

char *TCID = "hugeshmat04";
int TST_TOTAL = 3;

static long huge_free;
static long huge_free2;
static long hugepages;
static long orig_shmmax, new_shmmax;

static void shared_hugepage(void);

int main(int ac, char **av)
{
	int lc, i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {

			huge_free = read_meminfo("HugePages_Free:");
			shared_hugepage();
			huge_free2 = read_meminfo("HugePages_Free:");

			if (huge_free2 != huge_free)
				tst_brkm(TFAIL, cleanup,
				"Test failed. Hugepage leak inspection.");
			else
				tst_resm(TPASS, "No regression found.");
		}
	}

	cleanup();
	tst_exit();
}

void shared_hugepage(void)
{
	pid_t pid;
	int status, shmid;
	size_t size = (size_t)SIZE;
	void *buf;

	shmid = shmget(IPC_PRIVATE, size, SHM_HUGETLB | IPC_CREAT | 0777);
	if (shmid < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "shmget");

	buf = shmat(shmid, (void *)BOUNDARY, SHM_RND | 0777);
	if (buf == (void *)-1) {
		shmctl(shmid, IPC_RMID, NULL);
		tst_brkm(TBROK | TERRNO, cleanup, "shmat");
	}

	memset(buf, 2, size);
	pid = fork();

	if (pid == 0)
		exit(1);
	else if (pid < 0)
		tst_brkm(TBROK | TERRNO, cleanup, "fork");

	wait(&status);
	shmdt(buf);
	shmctl(shmid, IPC_RMID, NULL);
}

void setup(void)
{
	long mem_total, hpage_size;

	tst_require_root();
	check_hugepage();

	mem_total = read_meminfo("MemTotal:");
	SAFE_FILE_SCANF(NULL, PATH_SHMMAX, "%ld", &orig_shmmax);
	SAFE_FILE_PRINTF(NULL, PATH_SHMMAX, "%ld", (long)SIZE);
	SAFE_FILE_SCANF(NULL, PATH_SHMMAX, "%ld", &new_shmmax);

	if (mem_total < 2L*1024*1024)
		tst_brkm(TCONF,	NULL, "Needed > 2GB RAM, have: %ld", mem_total);

	if (new_shmmax < SIZE)
		tst_brkm(TCONF,	NULL, "shmmax too low, have: %ld", new_shmmax);

	orig_hugepages = get_sys_tune("nr_hugepages");
	hpage_size = read_meminfo("Hugepagesize:") * 1024;

	hugepages = (orig_hugepages * hpage_size + SIZE) / hpage_size;
	set_sys_tune("nr_hugepages", hugepages, 1);

	TEST_PAUSE;
}

void cleanup(void)
{
	set_sys_tune("nr_hugepages", orig_hugepages, 0);
	SAFE_FILE_PRINTF(NULL, PATH_SHMMAX, "%ld", orig_shmmax);
}
