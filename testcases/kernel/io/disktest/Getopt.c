/*
* Disktest
* Copyright (c) International Business Machines Corp., 2001
*
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*
*  Please send e-mail to yardleyb@us.ibm.com if you have
*  questions or comments.
*
*  Project Website:  TBD
*
*
* $Id: Getopt.c,v 1.5 2008/02/14 08:22:22 subrata_modak Exp $
* $Log: Getopt.c,v $
* Revision 1.5  2008/02/14 08:22:22  subrata_modak
* Disktest application update to version 1.4.2, by, Brent Yardley <yardleyb@us.ibm.com>
*
* Revision 1.4  2002/03/30 01:32:14  yardleyb
* Major Changes:
*
* Added Dumping routines for
* data miscompares,
*
* Updated performance output
* based on command line.  Gave
* one decimal in MB/s output.
*
* Rewrote -pL IO routine to show
* correct stats.  Now show pass count
* when using -C.
*
* Minor Changes:
*
* Code cleanup to remove the plethera
* if #ifdef for windows/unix functional
* differences.
*
* Revision 1.3  2002/02/21 21:32:19  yardleyb
* Added more unix compatability
* ifdef'd function out when
* compiling for unix env. that
* have getopt
*
* Revision 1.2  2002/02/04 20:35:38  yardleyb
* Changed max. number of threads to 64k.
* Check for max threads in parsing.
* Fixed windows getopt to return correctly
* when a bad option is given.
* Update time output to be in the format:
*   YEAR/MONTH/DAY-HOUR:MIN:SEC
* instead of epoch time.
*
* Revision 1.1  2001/12/04 18:57:36  yardleyb
* This source add for windows compatability only.
*
*/

#ifdef WINDOWS

#include <stdio.h>
#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include "getopt.h"

/*
 *
 *  FUNCTION: getopt()
 *
 *	  Get next command line option and parameter
 *	  source gathered from MS examples and modified
 *	  to conform with unix like getopt.
 *
 */

/* Global Exportable */
int optind;
char *optarg;

int getopt(int argc, char **argv, char *pszValidOpts)
{
	char chOpt;
	char *psz = NULL;
	char *pszParam = NULL;
	static int iArg = 1;

	if (iArg < argc) {
		psz = &(argv[iArg][0]);
		if (*psz == '-' || *psz == '/') {
			/* we have an option specifier */
			chOpt = argv[iArg][1];
			if (isalnum(chOpt) || ispunct(chOpt)) {
				/* we have an option character */
				psz = strchr(pszValidOpts, chOpt);
				if (psz != NULL) {
					/* option is valid, we want to return chOpt */
					if (psz[1] == ':') {
						/* option can have a parameter */
						psz = &(argv[iArg][2]);
						if (*psz == '\0') {
							/* must look at next argv for param */
							if (iArg + 1 < argc) {
								psz =
								    &(argv
								      [iArg +
								       1][0]);
								if (*psz == '-'
								    || *psz ==
								    '/') {
									/* next argv is a new option, so param
									 * not given for current option
									 */
									fprintf
									    (stderr,
									     "-%c option requires an argument.\n",
									     chOpt);
									chOpt =
									    '?';
									pszParam
									    =
									    NULL;
								} else {
									/* next argv is the param */
									iArg++;
									pszParam
									    =
									    psz;
								}
							} else {
								/* reached end of args looking for param */
							}
						} else {
							/* param is attached to option */
							pszParam = psz;
						}
					} else {
						/* option is alone, has no parameter */
					}
				} else {
					/* option specified is not in list of valid options */
					fprintf(stderr,
						"Invalid option -- %c\n",
						chOpt);
					chOpt = '?';
					pszParam = NULL;
				}
			} else {
				/* though option specifier was given, option character
				 * is not alpha or was was not specified
				 */
				chOpt = 0;
				pszParam = &(argv[iArg][0]);
			}
		} else {
			/* standalone arg given with no option specifier */
			chOpt = -1;
			pszParam = &(argv[iArg][0]);
		}
	} else {
		/* end of argument list */
		chOpt = -1;
	}

	optind = iArg++;
	optarg = pszParam;
	return (chOpt);
}

#endif /* defined WINDOWS */
