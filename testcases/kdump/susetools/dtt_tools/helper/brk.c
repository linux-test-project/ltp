/*
 * Copyright (c) Itsuro ODA 2004 all rights reserved
 *
 * DTT helper application: issue brk system-call.
 *
 * This program assumes "data segment size" and "max memory size"
 * of process resource are set "unlimited".
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include <unistd.h>
#include <stdio.h>
#include <err.h>

#define BSIZE 4096 

int main(int argc, char *argv[])
{
	int i;
	char *p;

	for (i = 0; ; i++) {
		p = (char *)sbrk(BSIZE);
		if ((int)p == -1) {
			warn("(%d pages allocated)", i);
			(void)pause();	/* wait until kill */
		}
		*p = 1;	/* force a real page allocated and make dirty */
	}
}
