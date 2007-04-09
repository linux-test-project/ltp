/*
 * Copyright (c) Itsuro ODA 2004 all rights reserved
 *
 * DTT helper application: issue setuid system-call.
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
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <err.h>

#define USERID 500 
#define LOOP 100

int main(int argc, char *argv[])
{
	int ret;
	int i;

	for (i = 0; i < LOOP; i++) {
		ret = setuid(USERID);
		if (ret < 0) {
			err(1, NULL); /* exit */
		}
	}

	return 0;
}
