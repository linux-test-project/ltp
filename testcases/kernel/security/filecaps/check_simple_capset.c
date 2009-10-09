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
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    */
/*                                                                            */
/******************************************************************************/


#include <stdio.h>
#include "config.h"
#if HAVE_SYS_CAPABILITY_H
#include <sys/capability.h>
#endif

int main()
{
#if HAVE_SYS_CAPABILITY_H
	cap_t caps, caps2;
	int ret;

#if HAVE_DECL_CAP_FROM_TEXT && HAVE_DECL_CAP_SET_PROC && HAVE_DECL_CAP_COMPARE
	caps = cap_from_text("cap_setpcap+ep");
	caps2 = cap_from_text("cap_setpcap+ep");
	ret = cap_set_proc(caps);
	ret = cap_compare(caps, caps2);
#else
	printf("System doesn't support full POSIX capabilities.\n");
	return 1;
#endif
	printf("Caps were %sthe same\n", ret ? "not " : "");

#if HAVE_DECL_CAP_FREE
	cap_free(caps);
	cap_free(caps2);
#endif
	return ret;
#else
	printf("System doesn't support POSIX capabilities.\n");
	return 1;
#endif
}
