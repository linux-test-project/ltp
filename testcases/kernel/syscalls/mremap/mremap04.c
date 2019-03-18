/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Test Name: mremap04
 *
 * Test Description:
 *  Verify that,
 *   mremap() fails when used to expand the existing virtual memory mapped
 *   region to the requested size, if the memory area cannot be expanded at
 *   the current virtual address and MREMAP_MAYMOVE flag not set.
 *
 * Expected Result:
 *  mremap() should return -1 and set errno to ENOMEM.
 *
 * Algorithm:
 *  Setup:
 *   Setup signal handling.
 *   Create temporary directory.
 *   Pause for SIGUSR1 if option specified.
 *
 *  Test:
 *   Loop if the proper options are given.
 *   Execute system call
 *   Check return code, if system call failed (return=-1)
 *	if errno set == expected errno
 *		Issue sys call failed with expected return value and errno.
 *	Otherwise,
 *		Issue sys call failed with unexpected errno.
 *   Otherwise,
 *	Issue sys call returns unexpected value.
 *
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *   Delete the temporary directory(s)/file(s) created.
 *
 * Usage:  <for command-line>
 *  mremap04 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -p x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 *      11/09/2001 Manoj Iyer (manjo@austin.ibm.com)
 *      Modified.
 *      - #include <linux/mman.h> should not be included as per man page for
 *        mremap, #include <sys/mman.h> alone should do the job. But inorder
 *        to include definition of MREMAP_MAYMOVE defined in bits/mman.h
 *        (included by sys/mman.h) __USE_GNU needs to be defined.
 *        There may be a more elegant way of doing this...
 *
 *      26/02/2008 Renaud Lottiaux (Renaud.Lottiaux@kerlabs.com)
 *      - Fix concurrency issue. Use a shm key from getipckey instead of
 *        a fixed hard-coded value.
 *
 * RESTRICTIONS:
 *  None.
 */
#define _GNU_SOURCE
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "test.h"

#define SHM_MODE	(SHM_R | SHM_W)	/* mode permissions of shared memory */

char *TCID = "mremap04";
int TST_TOTAL = 1;
char *addr;			/* addr of memory mapped region */
char *shmaddr;			/* pointer to shared memory segment */
int shmid;			/* shared memory identifier. */
int memsize;			/* memory mapped size */
int newsize;			/* new size of virtual memory block */

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */

extern int getipckey();

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		/*
		 * Attempt to expand the existing shared
		 * memory region of newsize by newsize limits
		 * using mremap() should fail as specified
		 * memory area already locked and MREMAP_MAYMOVE
		 * flag unset.
		 */
		errno = 0;
		addr = mremap(shmaddr, memsize, newsize, 0);
		TEST_ERRNO = errno;

		/* Check for the return value of mremap() */
		if (addr != MAP_FAILED) {
			tst_resm(TFAIL,
				 "mremap returned invalid value, expected: -1");

			/* Unmap the mapped memory region */
			if (munmap(addr, newsize) != 0) {
				tst_brkm(TFAIL, cleanup, "munmap failed to "
					 "unmap the expanded memory region, "
					 "error=%d", errno);
			}
			continue;
		}

		if (TEST_ERRNO == ENOMEM) {
			tst_resm(TPASS, "mremap() failed, "
				 "'MREMAP_MAYMOVE flag unset', "
				 "errno %d", TEST_ERRNO);
		} else {
			tst_resm(TFAIL, "mremap() failed, "
				 "Unexpected errno %d", TEST_ERRNO);
		}
	}

	cleanup();
	tst_exit();

}

/*
 * setup() - performs all ONE TIME setup for this test.
 *
 * Get system page size, Set the size of virtual memory area and the
 * newsize after resize,
 * Create a named shared memory segment SHMKEY of newsize and mode SHM_MODE
 * by using shmget() which returns a shared memory identifier associated
 * with the created shared memory segment.
 * Call shmat() to attach the shared memory segment to the data segment of the
 * calling process. The segment is attached at the first available address as
 * selected by the system.
 */
void setup(void)
{
	key_t shmkey;

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	/* Get the system page size */
	if ((memsize = getpagesize()) < 0) {
		tst_brkm(TBROK, NULL,
			 "getpagesize() failed to get system page size");
	}

	/* Get the New size of virtual memory block after resize */
	newsize = (memsize * 2);

	/* get an IPC resource key */
	shmkey = getipckey();

	/*
	 * Create a shared memory segment represented by SHMKEY of
	 * specified size 'newsize' and mode permissions 'SHM_MODE'.
	 */
	shmid = shmget(shmkey, newsize, IPC_CREAT | SHM_MODE);
	if (shmid == -1) {
		tst_brkm(TBROK, NULL, "shmget() Failed to create a shared "
			 "memory, error:%d", errno);
	}

	/*
	 * Attach  the shared memory segment associated with the shared
	 * memory identifier specified by "shmid" to the data segment of
	 * the calling process at the first available address as selected
	 * by the system.
	 */
	shmaddr = shmat(shmid, NULL, 0);
	if (shmaddr == (void *)-1) {
		tst_brkm(TBROK, cleanup, "shmat() Failed to attach shared "
			 "memory, error:%d", errno);
	}
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *	       Detach the shared memory segment and remove the shared memory
 *	       identifier associated with the shared memory.
 */
void cleanup(void)
{

	/*
	 * Detach the shared memory segment attached to
	 * the calling process's data segment
	 */
	if (shmdt(shmaddr) < 0) {
		tst_brkm(TFAIL, NULL, "shmdt() Failed to detach shared "
			 "memory, error:%d", errno);
	}

	/*
	 * Remove the shared memory identifier associated with
	 * the shared memory segment and destroy the shared memory
	 * segment.
	 */
	if (shmctl(shmid, IPC_RMID, 0) < 0) {
		tst_brkm(TFAIL, NULL, "shmctl() Failed to remove shared "
			 "memory, error:%d", errno);
	}

	tst_rmdir();

	/* Exit the program */

}
