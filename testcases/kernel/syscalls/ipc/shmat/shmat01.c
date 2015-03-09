/*
 * Copyright (c) International Business Machines  Corp., 2001
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * NAME
 *	shmat01.c
 *
 * DESCRIPTION
 *	shmat01 - test that shmat() works correctly
 *
 * ALGORITHM
 *	create a shared memory resouce with read/write permissions
 *	loop if that option was specified
 *	call shmat() with the TEST() macro using three valid conditions
 *	check the return code
 *	  if failure, issue a FAIL message.
 *	otherwise,
 *	  if doing functionality testing
 *		check for the correct conditions after the call
 *		if correct,
 *			issue a PASS message
 *		otherwise
 *			issue a FAIL message
 *	call cleanup
 */

#include "ipcshm.h"
#include "shmat_common.h"

#define CASE0		10
#define CASE1		20

char *TCID = "shmat01";
int TST_TOTAL = 4;

int shm_id_1 = -1;

/*
 * By probing this address first, we can make
 * non-aligned addresses from it for different
 * architectures without explicitly code it.
 */
void *base_addr;
void *addr;

static struct test_case_t {
	int *shmid;
	int offset;
	int flags;
	int getbase;
} *TC;

static void check_functionality(int);

int main(int argc, char *argv[])
{
	int lc, i;
	void *attchaddr;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {

			if (TC[i].getbase) {
				base_addr = probe_free_addr();
				attchaddr = base_addr + TC[i].offset;
			} else {
				attchaddr = NULL;
			}

			addr = shmat(*(TC[i].shmid), attchaddr, TC[i].flags);

			TEST_ERRNO = errno;
			if (addr == (void *)-1) {
				tst_brkm(TFAIL | TTERRNO, cleanup,
					 "shmat call failed");
			} else {
				check_functionality(i);
			}

			if (shmdt(addr) == -1)
				tst_brkm(TBROK, cleanup,
					 "Couldn't detach shared memory");
		}
	}

	cleanup();

	tst_exit();
}

/*
 * check_functionality - check various conditions to make sure they
 *			 are correct.
 */
static void check_functionality(int i)
{
	void *orig_add;
	int *shared;
	int fail = 0;
	struct shmid_ds buf;

	shared = (int *)addr;

	/* stat the shared memory ID */
	if (shmctl(shm_id_1, IPC_STAT, &buf) == -1)
		tst_brkm(TBROK, cleanup, "couldn't stat shared memory");

	/* check the number of attaches */
	if (buf.shm_nattch != 1) {
		tst_resm(TFAIL, "# of attaches is incorrect");
		return;
	}

	/* check the size of the segment */
	if (buf.shm_segsz != INT_SIZE) {
		tst_resm(TFAIL, "segment size is incorrect");
		return;
	}

	/* check for specific conditions depending on the type of attach */
	switch (i) {
	case 0:
	case 1:
		/*
		 * Check the functionality of shmat by simply "writing"
		 * a value to the shared memory space.
		 * If this fails the program will get a SIGSEGV, dump
		 * core and exit.
		 */

		*shared = CASE0;
		break;
	case 2:
		/*
		 * Check the functionality of shmat by writing a value
		 * to the shared memory space and then checking that
		 * the original address given was rounded down as
		 * specified in the man page.
		 */

		*shared = CASE1;
		orig_add = addr + ((unsigned long)TC[2].offset % SHMLBA);
		if (orig_add != base_addr + TC[2].offset) {
			tst_resm(TFAIL, "shared memory address is not "
				 "correct");
			fail = 1;
		}
		break;
	case 3:
		/*
		 * This time the shared memory is read only.  Read the value
		 * and check that it is equal to the value set in last case,
		 * because shared memory is persistent.
		 */

		if (*shared != CASE1) {
			tst_resm(TFAIL, "shared memory value isn't correct");
			fail = 1;
		}
		break;
	}

	if (!fail)
		tst_resm(TPASS, "conditions and functionality are correct");
}

void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	TC = malloc(TST_TOTAL * sizeof(struct test_case_t));
	if (TC == NULL)
		tst_brkm(TFAIL | TERRNO, cleanup, "failed to allocate memory");

	/* set NULL as attaching address*/
	TC[0].shmid = &shm_id_1;
	TC[0].offset = 0;
	TC[0].flags = 0;
	TC[0].getbase = 0;

	/* a straight forward read/write attach */
	TC[1].shmid = &shm_id_1;
	TC[1].offset = 0;
	TC[1].flags = 0;
	TC[1].getbase = 1;

	/* an attach using unaligned memory */
	TC[2].shmid = &shm_id_1;
	TC[2].offset = SHMLBA - 1;
	TC[2].flags = SHM_RND;
	TC[2].getbase = 1;

	/* a read only attach */
	TC[3].shmid = &shm_id_1;
	TC[3].offset = 0;
	TC[3].flags = SHM_RDONLY;
	TC[3].getbase = 1;

	tst_tmpdir();

	shmkey = getipckey();

	shm_id_1 = shmget(shmkey++, INT_SIZE, SHM_RW | IPC_CREAT | IPC_EXCL);
	if (shm_id_1 == -1)
		tst_brkm(TBROK, cleanup, "Failed to create shared memory "
			 "resource 1 in setup()");
}

void cleanup(void)
{
	rm_shm(shm_id_1);

	if (TC != NULL)
		free(TC);

	tst_rmdir();
}
