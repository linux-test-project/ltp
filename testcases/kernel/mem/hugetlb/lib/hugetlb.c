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
 * NAME
 *	hugetlb.c
 *
 * DESCRIPTION
 *	common routines for the hugepage tests.
 *
 *	The library contains the following routines:
 *
 *	getipckey()
 *	getuserid()
 *	rm_shm()
 */

#define TST_NO_DEFAULT_MAIN
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/timeb.h>
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
	struct timeb time_info;

	curdir = getcwd(curdir, size);
	if (curdir == NULL)
		tst_brk(TBROK | TERRNO, "getcwd(curdir)");

	/*
	 * Get a Sys V IPC key
	 *
	 * ftok() requires a character as a second argument.  This is
	 * refered to as a "project identifier" in the man page.  In
	 * order to maximize the chance of getting a unique key, the
	 * project identifier is a "random character" produced by
	 * generating a random number between 0 and 25 and then adding
	 * that to the ascii value of 'a'.  The "seed" for the random
	 * number is the millisecond value that is set in the timeb
	 * structure after calling ftime().
	 */
	ftime(&time_info);
	srandom((unsigned int)time_info.millitm);

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

	ent = getpwnam(user);
	if (ent == NULL)
		tst_brk(TBROK | TERRNO, "getpwnam");

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
