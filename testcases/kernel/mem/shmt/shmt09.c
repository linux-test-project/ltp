/*
 *
 *   Copyright (c) International Business Machines  Corp., 2002
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

/* 12/20/2002   Port to LTP     robbiew@us.ibm.com */
/* 06/30/2001   Port to Linux   nsharoff@us.ibm.com */

/*
 * NAME
 *	shmt09
 *
 * CALLS
 *	sbrk(2) shmctl(2) shmget(2)
 *
 * ALGORITHM
 * Create a shared memory segment and attach at the default address.
 * Increase the size of the data segment.
 * Attach at an address that is less than the break value: should FAIL.
 * decrease the size of the data segment.
 * Attach at 4K past the break value: should SUCCEED.
 * Sbrk() past the attached segment: should FAIL.
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <unistd.h>

#define K_1  1024

#define SHMBYTES    (SHMLBA - 1)
#define SHMALIGN(p) (((unsigned long)(p) + SHMBYTES) & ~SHMBYTES)

/** LTP Port **/
#include "test.h"

char *TCID = "shmt09";		/* Test program identifier.    */
int TST_TOTAL = 4;		/* Total number of test cases. */
/**************/

#ifdef __ia64__
#define INCREMENT 		8388608	/* 8Mb */
#elif defined (__mips__)  ||  defined (__hppa__) || defined (__sparc__)
#define INCREMENT		262144	/* 256Kb */
#elif defined __sh__ || defined (__arm__) || defined(__aarch64__)
#define INCREMENT 		16384	/* 16kb */
#else
#define INCREMENT 		SHMLBA
#endif

static int rm_shm(int);

int main(void)
{
	char *c1 = NULL, *c2 = NULL, *c3 = NULL;
	void *vp;
	int shmid;
	key_t key;

	key = (key_t) getpid();

/*-----------------------------------------------------------*/

	if ((unsigned long)sbrk(16384) >= (-4095UL)) {
		perror("sbrk");
		tst_brkm(TFAIL, NULL, "Error: sbrk failed, errno = %d\n",
			 errno);
	}

	if ((unsigned long)sbrk(-4097) >= (-4095UL)) {
		perror("sbrk");
		tst_brkm(TFAIL, NULL, "Error: sbrk failed, errno = %d\n",
			 errno);
	}

	if ((shmid = shmget(key, 10 * K_1, IPC_CREAT | 0666)) < 0) {
		perror("shmget");
		tst_brkm(TFAIL,
			 NULL,
			 "Error: shmget Failed, shmid = %d, errno = %d\n",
			 shmid, errno);
	}

	c1 = shmat(shmid, NULL, 0);
	if (c1 == (char *)-1) {
		perror("shmat");
		tst_resm(TFAIL,
			 "Error: shmat Failed, shmid = %d, errno = %d\n",
			 shmid, errno);
		rm_shm(shmid);
		tst_exit();
	}

	tst_resm(TPASS, "sbrk, sbrk, shmget, shmat");

/*--------------------------------------------------------*/

	if ((unsigned long)sbrk(32 * K_1) >= (-4095UL)) {
		perror("sbrk");
		tst_resm(TFAIL, "Error: sbrk failed, errno = %d\n", errno);
		rm_shm(shmid);
		tst_exit();
	}
	vp = (void *)((char *)sbrk(0) - 2 * K_1);
	c2 = shmat(shmid, vp, 0);
	if (c2 != (char *)-1) {
		tst_resm(TFAIL,
			 "ERROR: shmat: succeeded!: shmid = %d, shmaddr = %p, "
			 "att_addr = %p", shmid, c2, vp);
		rm_shm(shmid);
		tst_exit();
	}

	tst_resm(TPASS, "sbrk, shmat");

/*---------------------------------------------------------*/

	if ((unsigned long)sbrk(-16000) >= (-4095UL)) {
		perror("sbrk");
		tst_resm(TFAIL, "Error: sbrk failed, errno = %d\n", errno);
		rm_shm(shmid);
		tst_exit();
	}
#ifdef __mips__
	vp = (void *)((char *)sbrk(0) + 256 * K_1);
#elif  defined(__powerpc__) || defined(__powerpc64__)
	vp = (void *)((char *)sbrk(0) + getpagesize());
#else
	/* SHM_RND rounds vp on the nearest multiple of SHMLBA */
	vp = (void *)SHMALIGN((char *)sbrk(0) + 1);
#endif

	c3 = shmat(shmid, vp, SHM_RND);
	if (c3 == (char *)-1) {
		perror("shmat1");
		tst_resm(TFAIL,
			 "Error: shmat Failed, shmid = %d, errno = %d\n",
			 shmid, errno);
		rm_shm(shmid);
		tst_exit();
	}

	tst_resm(TPASS, "sbrk, shmat");

/*--------------------------------------------------------*/
#if defined (__ia64__) || defined(__mips__) || defined(__hppa__) || defined(__arm__) || defined(__aarch64__)
	while ((vp = sbrk(INCREMENT)) != (void *)-1) ;
	if (errno != ENOMEM) {
		tst_resm(TFAIL, "Error: sbrk failed, errno = %d\n", errno);
		rm_shm(shmid);
		tst_exit();
	}
#else
	if ((vp = sbrk(INCREMENT)) != (void *)-1) {
		tst_resm(TFAIL,
			 "Error: sbrk succeeded!  ret = %p, curbrk = %p, ",
			 vp, sbrk(0));
		rm_shm(shmid);
		tst_exit();
	}
#endif

	tst_resm(TPASS, "sbrk");

/*------------------------------------------------------*/

	rm_shm(shmid);
	tst_exit();
}

static int rm_shm(int shmid)
{
	if (shmctl(shmid, IPC_RMID, NULL) == -1) {
		perror("shmctl");
		tst_brkm(TFAIL,
			 NULL,
			 "shmctl Failed to remove: shmid = %d, errno = %d\n",
			 shmid, errno);
	}
	return (0);
}
