/******************************************************************************/
/*                                                                            */
/* Copyright (c) International Business Machines  Corp., 2008                 */
/*                                                                            */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                  */
/* the GNU General Public License for more details.                           */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program;  if not, write to the Free Software               */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    */
/*                                                                            */
/******************************************************************************/

#include <stdio.h>
#include "config.h"
#if HAVE_SYS_CAPABILITY_H
#include <linux/types.h>
#include <sys/capability.h>
#endif

int main(void)
{
#ifdef HAVE_LIBCAP
	cap_t caps, caps2;
	int ret;

	caps = cap_from_text("cap_setpcap+ep");
	caps2 = cap_from_text("cap_setpcap+ep");
	ret = cap_set_proc(caps);
	ret = cap_compare(caps, caps2);
	printf("Caps were %sthe same\n", ret ? "not " : "");

	cap_free(caps);
	cap_free(caps2);
	return ret;
#else
	printf("System doesn't support full POSIX capabilities.\n");
	return 1;
#endif
}
