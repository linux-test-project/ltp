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
* $Id: Getopt.c,v 1.1 2002/02/19 21:29:26 robbiew Exp $
* $Log: Getopt.c,v $
* Revision 1.1  2002/02/19 21:29:26  robbiew
* Added disktest.
*
* Revision 1.1  2001/12/04 18:57:36  yardleyb
* This source add for windows compatability only.
*
*/

#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include "getopt.h"

/*
 *
 *  FUNCTION: getopt()
 *
 *      Get next command line option and parameter
 *      source gathered from MS examples and modified
 *      to conform with unix like getopt.
 *
 */

/* Global Exportable */
int optind;
char *optarg;

int getopt (int argc, char** argv, char* pszValidOpts) {
    char chOpt;
    char* psz = NULL;
    char* pszParam = NULL;
	static int iArg = 1;

    if (iArg < argc)
    {
        psz = &(argv[iArg][0]);
        if (*psz == '-' || *psz == '/')
        {
            /* we have an option specifier */
            chOpt = argv[iArg][1];
            if (isalnum(chOpt) || ispunct(chOpt))
            {
                /* we have an option character */
                psz = strchr(pszValidOpts, chOpt);
                if (psz != NULL)
                {
                    /* option is valid, we want to return chOpt */
                    if (psz[1] == ':')
                    {
                        /* option can have a parameter */
                        psz = &(argv[iArg][2]);
                        if (*psz == '\0')
                        {
                            /* must look at next argv for param */
                            if (iArg+1 < argc)
                            {
                                psz = &(argv[iArg+1][0]);
                                if (*psz == '-' || *psz == '/')
                                {
                                    /* next argv is a new option, so param
                                     * not given for current option
									 */
                                }
                                else
                                {
                                    /* next argv is the param */
                                    iArg++;
                                    pszParam = psz;
                                }
                            }
                            else
                            {
                                /* reached end of args looking for param */
                            }

                        }
                        else
                        {
                            /* param is attached to option */
                            pszParam = psz;
                        }
                    }
                    else
                    {
                        /* option is alone, has no parameter */
                    }
                }
                else
                {
                    /* option specified is not in list of valid options */
                    chOpt = -1;
                    pszParam = &(argv[iArg][0]);
                }
            }
            else
            {
                /* though option specifier was given, option character
                 * is not alpha or was was not specified
				 */
                chOpt = -1;
                pszParam = &(argv[iArg][0]);
            }
        }
        else
        {
            /* standalone arg given with no option specifier */
            chOpt = -1;
            pszParam = &(argv[iArg][0]);
        }
    }
    else
    {
        /* end of argument list */
        chOpt = -1;
    }

    optind = iArg++;
    optarg = pszParam;
    return (chOpt);
}
