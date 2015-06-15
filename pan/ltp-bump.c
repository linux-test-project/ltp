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
/* $Id: ltp-bump.c,v 1.1 2009/05/19 09:39:11 subrata_modak Exp $ */
#include <stdio.h>
#include <errno.h>
#include <sys/signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "zoolib.h"

pid_t read_active(FILE * fp, char *name);

int main(int argc, char **argv)
{
	int c;
	char *active = NULL;
	pid_t nanny;
	zoo_t zoo;
	int sig = SIGINT;

	while ((c = getopt(argc, argv, "a:s:12")) != -1) {
		switch (c) {
		case 'a':
			active = malloc(strlen(optarg) + 1);
			strcpy(active, optarg);
			break;
		case 's':
			sig = atoi(optarg);
			break;
		case '1':
			sig = SIGUSR1;
			break;
		case '2':
			sig = SIGUSR2;
			break;
		}
	}

	if (active == NULL) {
		active = zoo_getname();
		if (active == NULL) {
			fprintf(stderr,
				"ltp-bump: Must supply -a or set ZOO env variable\n");
			exit(1);
		}
	}

	if (optind == argc) {
		fprintf(stderr, "ltp-bump: Must supply names\n");
		exit(1);
	}

	/* need r+ here because we're using write-locks */
	if ((zoo = zoo_open(active)) == NULL) {
		fprintf(stderr, "ltp-bump: %s\n", zoo_error);
		exit(1);
	}

	while (optind < argc) {
		/*printf("argv[%d] = (%s)\n", optind, argv[optind] ); */
		nanny = zoo_getpid(zoo, argv[optind]);
		if (nanny == -1) {
			fprintf(stderr, "ltp-bump: Did not find tag '%s'\n",
				argv[optind]);
		} else {
			if (kill(nanny, sig) == -1) {
				if (errno == ESRCH) {
					fprintf(stderr,
						"ltp-bump: Tag %s (pid %d) seems to be dead already.\n",
						argv[optind], nanny);
					if (zoo_clear(zoo, nanny))
						fprintf(stderr,
							"ltp-bump: %s\n",
							zoo_error);
				}
			}
		}
		++optind;
	}
	zoo_close(zoo);

	exit(0);
}
