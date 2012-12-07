/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 *
 */
/* $Id: debug.c,v 1.1 2000/09/21 21:35:06 alaffin Exp $ */
#include <stdio.h>
#include <string.h>
#include "reporter.h"

#ifdef DEBUGGING
int Debug[MAXDEBUG];		/* Debug level in their areas */
#endif

/*
 *	set debug areas & levels
 *
 * Syntax:   area[,area]:level[,area[,area]:level]...
 */
int set_debug(char *optarg)
{
#ifdef DEBUGGING
	/* pointers to the debug area and level in the option's arguments */
	char *d_area, *d_level;
	/* debug area and level after converted to integers */
	int db_area, db_level;

	d_area = optarg;

	while (*d_area) {
		d_level = strchr(d_area, ':');
		*d_level++ = '\0';
		db_level = atoi(d_level);
		db_area = atoi(d_area);

		if (db_area > MAXDEBUG) {
			printf("Error - Debug area %s > maximum of %d\n",
			       d_area, MAXDEBUG);
			exit(-1);
		}

		while (d_area != NULL) {
			db_area = atoi(d_area);
			printf("Debug area %d set to %d\n", db_area, db_level);
			Debug[db_area] = db_level;
			if ((d_area = strchr(d_area, ',')) != NULL)
				d_area++;
		}
		if ((d_area = strchr(d_level, ',')) == NULL)
			break;
	}
#else
	printf("Debugging is not enabled.  -D has been ignored\n");
#endif

	return 0;
}
