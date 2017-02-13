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
 */
/**************************************************************
 *
 *    OS Testing - Silicon Graphics, Inc.
 *
 *    FUNCTION NAME     : parse_open_flags
 *			  openflags2symbols
 *
 *    FUNCTION TITLE    : converts open flag symbols into bitmask
 *			  converts open flag bitmask into symbols
 *
 *    SYNOPSIS:
 *      int parse_open_flags(symbols, badname)
 *	char *symbols;
 *	char **badname;
 *
 *      char *openflags2symbols(openflags, sep, mode)
 *	int openflags;
 *	char *sep;
 *	int mode;
 *
 *    AUTHOR            : Richard Logan
 *
 *    CO-PILOT(s)       : Dean Roehrich
 *
 *    INITIAL RELEASE   : UNICOS 8.0
 *
 *    DESIGN DESCRIPTION
 *	The parse_open_flags function can be used to convert
 *	a list of comma separated open(2) flag symbols (i.e. O_TRUNC)
 *	into the bitmask that can be used by open(2).
 *	If a symbol is unknown and <badname> is not NULL, <badname>
 *	will updated to point that symbol in <string>.
 *	Parse_open_flags will return -1 on this error.
 *      Otherwise parse_open_flags will return the open flag bitmask.
 *	If parse_open_flags returns, <string> will left unchanged.
 *
 * 	The openflags2symbols function attempts to convert open flag
 *	bits into human readable  symbols (i.e. O_TRUNC).  If there
 *	are more than one symbol, the <sep> string will be placed as
 *	a separator between symbols.  Commonly used separators would
 *	be a comma "," or pipe "|".  If <mode> is one and not all
 *	<openflags> bits can be converted to symbols, the "UNKNOWN"
 *	symbol will be added to return string.
 * 	Openflags2symbols will return the indentified symbols.
 * 	If no symbols are recognized the return value will be a empty
 * 	string or the "UNKNOWN" symbol.
 *
 *    SPECIAL REQUIREMENTS
 *	None.
 *
 *    UPDATE HISTORY
 *      This should contain the description, author, and date of any
 *      "interesting" modifications (i.e. info should helpful in
 *      maintaining/enhancing this module).
 *      username     description
 *      ----------------------------------------------------------------
 *	rrl    This code was first created during the beginning
 *		of the SFS testing days.  I think that was in 1993.
 *	       This code was updated in 05/96.
 *		(05/96)  openflags2symbols was written.
 *
 *    BUGS/LIMITATIONS
 * 	Currently (05/96) all known symbols are coded into openflags2symbols.
 * 	If new open flags are added this code will have to updated
 * 	to know about them or they will not be recognized.
 *
 **************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/param.h>
#include <string.h>		/* strcat */
#include "open_flags.h"

#define UNKNOWN_SYMBOL	"UNKNOWN"

static char Open_symbols[512];	/* space for openflags2symbols return value */

struct open_flag_t {
	char *symbol;
	int flag;
};

static struct open_flag_t Open_flags[] = {
	{"O_RDONLY", O_RDONLY},
	{"O_WRONLY", O_WRONLY},
	{"O_RDWR", O_RDWR},
	{"O_SYNC", O_SYNC},
	{"O_CREAT", O_CREAT},
	{"O_TRUNC", O_TRUNC},
	{"O_EXCL", O_EXCL},
	{"O_APPEND", O_APPEND},
	{"O_NONBLOCK", O_NONBLOCK},
#if O_NOCTTY
	{"O_NOCTTY", O_NOCTTY},
#endif
#if O_DSYNC
	{"O_DSYNC", O_DSYNC},
#endif
#if O_RSYNC
	{"O_RSYNC", O_RSYNC},
#endif
#if O_ASYNC
	{"O_ASYNC", O_ASYNC},
#endif
#if O_PTYIGN
	{"O_PTYIGN", O_PTYIGN},
#endif
#if O_NDELAY
	{"O_NDELAY", O_NDELAY},
#endif
#if O_RAW
	{"O_RAW", O_RAW},
#endif
#ifdef O_SSD
	{"O_SSD", O_SSD},
#endif
#if O_BIG
	{"O_BIG", O_BIG},
#endif
#if O_PLACE
	{"O_PLACE", O_PLACE},
#endif
#if O_RESTART
	{"O_RESTART", O_RESTART},
#endif
#if O_SFSXOP
	{"O_SFSXOP", O_SFSXOP},
#endif
#if O_SFS_DEFER_TM
	{"O_SFS_DEFER_TM", O_SFS_DEFER_TM},
#endif
#if O_WELLFORMED
	{"O_WELLFORMED", O_WELLFORMED},
#endif
#if O_LDRAW
	{"O_LDRAW", O_LDRAW},
#endif
#if O_T3D
	{"O_T3D", O_T3D},
#endif /* O_T3D */
#if O_PARALLEL
	{"O_PARALLEL", O_PARALLEL},
	{"O_FSA", O_PARALLEL | O_WELLFORMED | O_RAW},	/* short cut */
#endif /* O_PARALLEL */
#ifdef O_LARGEFILE
	{"O_LARGEFILE", O_LARGEFILE},
#endif
#ifdef O_DIRECT
	{"O_DIRECT", O_DIRECT},
#endif
#ifdef O_PRIV
	{"O_PRIV", O_PRIV},
#endif

};

int parse_open_flags(char *string, char **badname)
{
	int bits = 0;
	char *name;
	char *cc;
	char savecc;
	int found;
	unsigned int ind;

	name = string;
	cc = name;

	while (1) {

		for (; ((*cc != ',') && (*cc != '\0')); cc++) ;
		savecc = *cc;
		*cc = '\0';

		found = 0;

		for (ind = 0;
		     ind < sizeof(Open_flags) / sizeof(struct open_flag_t);
		     ind++) {
			if (strcmp(name, Open_flags[ind].symbol) == 0) {
				bits |= Open_flags[ind].flag;
				found = 1;
				break;
			}
		}

		*cc = savecc;	/* restore string */

		if (found == 0) {	/* invalid name */
			if (badname != NULL)
				*badname = name;
			return -1;
		}

		if (savecc == '\0')
			break;

		name = ++cc;

	}			/* end while */

	return bits;

}				/* end of parse_open_flags */

char *openflags2symbols(int openflags, char *sep, int mode)
{
	int ind;
	int size;
	int bits = openflags;
	int havesome = 0;

	Open_symbols[0] = '\0';

	size = sizeof(Open_flags) / sizeof(struct open_flag_t);

	/*
	 * Deal with special case of O_RDONLY.  If O_WRONLY nor O_RDWR
	 * bits are not set, assume O_RDONLY.
	 */

	if ((bits & (O_WRONLY | O_RDWR)) == 0) {
		strcat(Open_symbols, "O_RDONLY");
		havesome = 1;
	}

	/*
	 *  Loop through all but O_RDONLY elments of Open_flags
	 */
	for (ind = 1; ind < size; ind++) {

		if ((bits & Open_flags[ind].flag) == Open_flags[ind].flag) {
			if (havesome)
				strcat(Open_symbols, sep);

			strcat(Open_symbols, Open_flags[ind].symbol);
			havesome++;

			/* remove flag bits from bits */
			bits = bits & (~Open_flags[ind].flag);
		}
	}

	/*
	 * If not all bits were identified and mode was equal to 1,
	 * added UNKNOWN_SYMBOL to return string
	 */
	if (bits && mode == 1) {	/* not all bits were identified */
		if (havesome)
			strcat(Open_symbols, sep);
		strcat(Open_symbols, UNKNOWN_SYMBOL);
	}

	return Open_symbols;

}				/* end of openflags2symbols */

#ifdef UNIT_TEST

/*
 * The following code provides a UNIT test main for
 * parse_open_flags and openflags2symbols functions.
 */

int main(argc, argv)
int argc;
char **argv;
{
	int bits;
	int ret;
	char *err;

	if (argc == 1) {
		printf("Usage: %s openflagsbits\n\t%s symbols\n", argv[0],
		       argv[0]);
		exit(1);
	}

	if (sscanf(argv[1], "%i", &bits) == 1) {
		printf("openflags2symbols(%#o, \",\", 1) returned %s\n",
		       bits, openflags2symbols(bits, ",", 1));

	} else {
		ret = parse_open_flags(argv[1], &err);
		if (ret == -1)
			printf
			    ("parse_open_flags(%s, &err) returned -1, err = %s\n",
			     argv[0], err);
		else
			printf("parse_open_flags(%s, &err) returned %#o\n",
			       argv[0], ret);
	}

	exit(0);
}

#endif /* end of UNIT_TEST */
