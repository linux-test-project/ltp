/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the header declares struct shmid_ds with the members below, at a
 * minimum:
 *	struct ipc_perm	shm_perm
 *	size_t		shm_segsz
 *	pid_t		shm_lpid
 *	pid_t		shm_cpid
 *	shmatt_t	shm_nattch
 *	time_t		shm_atime
 *	time_t		shm_dtime
 *	time_t		shm_ctime
 */

#include <sys/shm.h>

struct shmid_ds this_type_should_exist, t;

int dummyfcn(void)
{
	struct ipc_perm perm = { 0 };
	size_t sz = 0;
	pid_t lpid = 0, cpid = 0;
	shmatt_t nattch = 0;
	time_t atime = 0, dtime = 0, ctime = 0;

	t.shm_perm = perm;
	t.shm_segsz = sz;
	t.shm_lpid = lpid;
	t.shm_cpid = cpid;
	t.shm_nattch = nattch;
	t.shm_atime = atime;
	t.shm_dtime = dtime;
	t.shm_ctime = ctime;

	return 0;
}
