// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2002
 *	12/20/2002	Port to LTP	robbiew@us.ibm.com
 *	06/30/2001	Port to Linux	nsharoff@us.ibm.com
 * Copyright (c) 2025 SUSE LLC Ricardo B. Marlière <rbm@suse.com>
 */

/*\
 * Create a shared memory segment and attach at the default address.
 * Increase the size of the data segment.
 * Attach at an address that is less than the break value: should FAIL.
 * decrease the size of the data segment.
 * Attach at 4K past the break value: should SUCCEED.
 * Sbrk() past the attached segment: should FAIL.
 */

#include <sys/shm.h>

#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"
#include "tse_newipc.h"
#include "tst_test_macros.h"

#define SHMSIZE 16
#define SHMBYTES (SHMLBA - 1)
#define SHMALIGN(p) (((unsigned long)(p) + SHMBYTES) & ~SHMBYTES)

#ifdef __ia64__
#define INCREMENT 8388608 /* 8Mb */
#elif defined(__mips__) || defined(__hppa__) || defined(__sparc__)
#define INCREMENT 262144 /* 256Kb */
#elif defined __sh__ || defined(__arm__) || defined(__aarch64__)
#define INCREMENT 16384 /* 16kb */
#else
#define INCREMENT SHMLBA
#endif

static void run(void)
{
	void *vp;
	int shmid;
	key_t key;

	key = GETIPCKEY();

	if ((unsigned long)sbrk(16384) >= (-4095UL))
		tst_brk(TFAIL, "Error: sbrk failed, errno = %d", errno);

	if ((unsigned long)sbrk(-4097) >= (-4095UL))
		tst_brk(TFAIL, "Error: sbrk failed, errno = %d", errno);

	shmid = SAFE_SHMGET(key, 10 * SHMSIZE, IPC_CREAT | 0666);
	SAFE_SHMAT(shmid, NULL, 0);

	if ((unsigned long)sbrk(32 * SHMSIZE) >= (-4095UL))
		tst_brk(TFAIL, "Error: sbrk failed, errno = %d", errno);

	vp = (void *)((char *)sbrk(0) - 2 * SHMSIZE);
	TST_EXP_FAIL((long)shmat(shmid, vp, 0), EINVAL);

	if ((unsigned long)sbrk(-16000) >= (-4095UL))
		tst_brk(TFAIL, "Error: sbrk failed, errno = %d", errno);

#ifdef __mips__
	vp = (void *)((char *)sbrk(0) + 256 * SHMSIZE);
#elif defined(__powerpc__) || defined(__powerpc64__)
	vp = (void *)((char *)sbrk(0) + getpagesize());
#else
	/* SHM_RND rounds vp on the nearest multiple of SHMLBA */
	vp = (void *)SHMALIGN((char *)sbrk(0) + 1);
#endif

	SAFE_SHMAT(shmid, vp, SHM_RND);

	TST_EXP_FAIL((long)sbrk(INCREMENT), ENOMEM);

	SAFE_SHMCTL(shmid, IPC_RMID, NULL);
}

static struct tst_test test = {
	.test_all = run,
};
