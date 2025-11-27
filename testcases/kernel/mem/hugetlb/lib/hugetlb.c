// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2006-2025
 * Copyright (c) International Business Machines Corp., 2001
 */

#define TST_NO_DEFAULT_MAIN
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <pwd.h>
#include "hugetlb.h"

key_t shmkey;

/*
 * getipckey() - generates and returns a message key used by the "get"
 *		 calls to create an IPC resource.
 */
int getipckey(void)
{
	const char a = 'a';
	int ascii_a = (int)a;
	char *curdir = NULL;
	size_t size = 0;
	key_t ipc_key;
	struct timeval time_info;

	curdir = SAFE_GETCWD(curdir, size);

	/*
	 * Get a Sys V IPC key
	 *
	 * ftok() requires a character as a second argument.  This is
	 * refered to as a "project identifier" in the man page.  In
	 * order to maximize the chance of getting a unique key, the
	 * project identifier is a "random character" produced by
	 * generating a random number between 0 and 25 and then adding
	 * that to the ascii value of 'a'.  The "seed" for the random
	 * number is the microseconds value that is set in the timeval
	 * structure after calling gettimeofday().
	 */
	gettimeofday(&time_info, NULL);
	srandom((unsigned int)time_info.tv_usec);

	ipc_key = ftok(curdir, ascii_a + random() % 26);
	if (ipc_key == -1)
		tst_brk(TBROK | TERRNO, __func__);

	return ipc_key;
}

/*
 * getuserid() - return the integer value for the "user" id
 */
int getuserid(char *user)
{
	struct passwd *ent;

	ent = SAFE_GETPWNAM(user);
	return ent->pw_uid;
}

/*
 * rm_shm() - removes a shared memory segment.
 */
void rm_shm(int shm_id)
{
	if (shm_id == -1)
		return;

	/*
	 * check for # of attaches ?
	 */
	if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
		tst_res(TINFO, "WARNING: shared memory deletion failed.");
		tst_res(TINFO, "This could lead to IPC resource problems.");
		tst_res(TINFO, "id = %d", shm_id);
	}
}

#define RANDOM_CONSTANT 0x1234ABCD
int do_readback(void *p, size_t size, char *desc)
{
	unsigned int *q = p;
	size_t i;

	for (i = 0; i < (size / sizeof(*q)); i++)
		q[i] = RANDOM_CONSTANT ^ i;

	for (i = 0; i < (size / sizeof(*q)); i++) {
		if (q[i] != (RANDOM_CONSTANT ^ i)) {
			tst_res(TFAIL, "At \"%s\": Mismatch at offset 0x%lx: 0x%x "
					"instead of 0x%lx", desc, i, q[i], RANDOM_CONSTANT ^ i);
			return -1;
		}
	}
	return 0;
}

void update_shm_size(size_t * shm_size)
{
	size_t shmmax;

	SAFE_FILE_SCANF(PATH_SHMMAX, "%zu", &shmmax);
	if (*shm_size > shmmax) {
		tst_res(TINFO, "Set shm_size to shmmax: %zu", shmmax);
		*shm_size = shmmax;
	}
}
