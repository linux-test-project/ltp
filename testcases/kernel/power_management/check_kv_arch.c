/************************************************************************
* Copyright (c) International Business Machines Corp., 2008
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
* the GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
***************************************************************************/
#include <stdio.h>
#include <test.h>

int kernel_is_too_old(void) {
	if (tst_kvercmp(2,6,21) < 0)
		return 1;
	return 0;
}

/*
 * yeah, to make the makefile coding easier, do_check returns 
 * 1 if unshare is not supported, 0 if it is
 */
#ifdef __i386__
int do_check(void) { return kernel_is_too_old(); }
#elif __x86_64__
int do_check(void) { return kernel_is_too_old(); }
#else
int do_check(void) { return 1; }
#endif

int main() {
	return do_check();
}

