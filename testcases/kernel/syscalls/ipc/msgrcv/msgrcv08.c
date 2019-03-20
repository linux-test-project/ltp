/*
 * Copyright (c) 2015   Author: Gabriellla Schmidt <gsc@bruker.de>
 *                      Modify: Li Wang <liwang@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * you should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/*
 * Description:
 *
 * A regression test for:
 *      commit e7ca2552369c1dfe0216c626baf82c3d83ec36bb
 *      Author: Mateusz Guzik <mguzik@redhat.com>
 *      Date:   Mon Jan 27 17:07:11 2014 -0800
 *
 *           ipc: fix compat msgrcv with negative msgtyp
 *
 * Reproduce:
 *
 *      32-bit application using the msgrcv() system call
 *      gives the error message:
 *
 *           msgrcv: No message of desired type
 *
 *      If this progarm is compiled as 64-bit application it works.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "test.h"
#include "lapi/abisize.h"

const char *TCID = "msgrcv08";
const int TST_TOTAL = 1;

#ifdef TST_ABI32

struct mbuf {
	long mtype;     /* message type, must be > 0 */
	char mtext[16]; /* message data */
};

static void msr(int msqid)
{
	struct mbuf msbs;
	struct mbuf msbr;
	ssize_t sret;
	long mtype = 121;

	memset(&msbs, 0, sizeof(msbs));
	msbs.mtype = mtype;

	if (msgsnd(msqid, &msbs, sizeof(msbs.mtext), IPC_NOWAIT))
		tst_brkm(TBROK | TERRNO, NULL, "msgsnd error");

	sret = msgrcv(msqid, &msbr, sizeof(msbr.mtext), -mtype, IPC_NOWAIT | MSG_NOERROR);

	if (sret < 0) {
		tst_resm(TFAIL, "Bug: No message of desired type.");
		return;
	}

	if (msbr.mtype != mtype)
		tst_brkm(TBROK, NULL,
			"found mtype %ld, expected %ld\n", msbr.mtype, mtype);

	if ((size_t)sret != sizeof(msbs.mtext))
		tst_brkm(TBROK, NULL, "received %zi, expected %zu\n",
			 sret, sizeof(msbs.mtext));

	tst_resm(TPASS, "No regression found.");
}

static void msgrcv_test(void)
{
	int msqid = msgget(IPC_PRIVATE, IPC_CREAT | IPC_EXCL | 0666);

	if (msqid < 0)
		tst_brkm(TBROK | TERRNO, NULL, "msgget error");

	msr(msqid);

	if (msgctl(msqid, IPC_RMID, 0))
		tst_brkm(TBROK | TERRNO, NULL, "msgctl error");
}

int main(int argc, char *argv[])
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	for (lc = 0; TEST_LOOPING(lc); lc++)
		msgrcv_test();

	tst_exit();
}

#else /* no 64-bit */
int main(void)
{
	tst_brkm(TCONF, NULL, "not works when compiled as 64-bit application.");
}
#endif
