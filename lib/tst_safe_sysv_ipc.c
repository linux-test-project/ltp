/*
 * Copyright (c) 2017 Xiao yang <yangx.jy@cn.fujitsu.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tst_safe_sysv_ipc.h"

int safe_msgget(const char *file, const int lineno, key_t key, int msgflg)
{
	int rval;

	rval = msgget(key, msgflg);
	if (rval == -1) {
		tst_brk(TBROK | TERRNO, "%s:%d: msgget(%i, %x) failed",
			file, lineno, (int)key, msgflg);
	}

	return rval;
}

int safe_msgsnd(const char *file, const int lineno, int msqid, const void *msgp,
		size_t msgsz, int msgflg)
{
	int rval;

	rval = msgsnd(msqid, msgp, msgsz, msgflg);
	if (rval == -1) {
		tst_brk(TBROK | TERRNO,
			"%s:%d: msgsnd(%i, %p, %zu, %x) failed",
			file, lineno, msqid, msgp, msgsz, msgflg);
	}

	return rval;
}

ssize_t safe_msgrcv(const char *file, const int lineno, int msqid, void *msgp,
		size_t msgsz, long msgtyp, int msgflg)
{
	ssize_t rval;

	rval = msgrcv(msqid, msgp, msgsz, msgtyp, msgflg);
	if (rval == -1) {
		tst_brk(TBROK | TERRNO,
			"%s:%d: msgrcv(%i, %p, %zu, %li, %x) failed",
			file, lineno, msqid, msgp, msgsz, msgtyp, msgflg);
	}

	return rval;
}

int safe_msgctl(const char *file, const int lineno, int msqid, int cmd,
		struct msqid_ds *buf)
{
	int  rval;

	rval = msgctl(msqid, cmd, buf);
	if (rval == -1) {
		tst_brk(TBROK | TERRNO, "%s:%d: msgctl(%i, %i, %p) failed",
			file, lineno, msqid, cmd, buf);
	}

	return rval;
}
