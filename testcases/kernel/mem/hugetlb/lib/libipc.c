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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * NAME
 *	libipc.c
 *
 * DESCRIPTION
 *	common routines for the IPC system call tests.
 *
 *	The library contains the following routines:
 *
 *	getipckey()
 *	getuserid()
 *	rm_shm()
 *	help()
 */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/timeb.h>
#include <pwd.h>
#include "ipcshm.h"

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
		tst_brkm(TBROK|TERRNO, cleanup, "getcwd(curdir)");

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

	ipc_key = ftok(curdir, ascii_a + random()%26);
	if (ipc_key == -1)
		tst_brkm(TBROK|TERRNO, cleanup, "ftok");

	return ipc_key;
}

/*
 * getuserid() - return the integer value for the "user" id
 */
int getuserid(char *user)
{
	struct passwd *ent;

	ent = (struct passwd *)malloc(sizeof(struct passwd));
	if (ent == NULL)
		tst_brkm(TBROK|TERRNO, cleanup, "malloc ent");

	ent = getpwnam(user);
	if (ent == NULL)
		tst_brkm(TBROK|TERRNO, cleanup, "getpwnam");

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
		tst_resm(TINFO, "WARNING: shared memory deletion failed.");
		tst_resm(TINFO, "This could lead to IPC resource problems.");
		tst_resm(TINFO, "id = %d", shm_id);
	}
}

void help(void)
{
	printf("    -s NUM  Set the number of hugepages to be allocated\n");
}
