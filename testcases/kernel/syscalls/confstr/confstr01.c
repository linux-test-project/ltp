/*
 *
 *   Copyright (c) International Business Machines  Corp., 2002
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* 11/20/2002	Port to LTP	robbiew@us.ibm.com */
/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */

/*
 * NAME
 *	confstr1.c - test for confstr(3C) - Get configuration-defined string
 *	values.
 *
 * CALLS
 *	confstr(3C)
 *
 * RESTRICTIONS
 * MUST RUN AS ROOT
 *
 */

#define _XOPEN_SOURCE	500
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>

#define INVAL_FLAG	-1

/** LTP Port **/
#include "test.h"
#include "usctest.h"

#define FAILED 0
#define PASSED 1

int local_flag = PASSED;

char *TCID = "confstr01";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
/**************/

int confstr_var_vals[] = { _CS_PATH, _CS_XBS5_ILP32_OFF32_CFLAGS,
	_CS_XBS5_ILP32_OFF32_LDFLAGS,
	_CS_XBS5_ILP32_OFF32_LIBS,
	_CS_XBS5_ILP32_OFF32_LINTFLAGS,
	_CS_XBS5_ILP32_OFFBIG_CFLAGS,
	_CS_XBS5_ILP32_OFFBIG_LDFLAGS,
	_CS_XBS5_ILP32_OFFBIG_LIBS,
	_CS_XBS5_ILP32_OFFBIG_LINTFLAGS,
	_CS_XBS5_LP64_OFF64_CFLAGS,
	_CS_XBS5_LP64_OFF64_LDFLAGS,
	_CS_XBS5_LP64_OFF64_LIBS,
	_CS_XBS5_LP64_OFF64_LINTFLAGS,
	_CS_XBS5_LPBIG_OFFBIG_CFLAGS,
	_CS_XBS5_LPBIG_OFFBIG_LDFLAGS,
	_CS_XBS5_LPBIG_OFFBIG_LIBS,
	_CS_XBS5_LPBIG_OFFBIG_LINTFLAGS,
	0
};

char *confstr_vars[] = { "PATH", "XBS5_ILP32_OFF32_CFLAGS",
	"XBS5_ILP32_OFF32_LDFLAGS", "XBS5_ILP32_OFF32_LIBS",
	"XBS5_ILP32_OFF32_LINTFLAGS",
	"XBS5_ILP32_OFFBIG_CFLAGS",
	"XBS5_ILP32_OFFBIG_LDFLAGS",
	"XBS5_ILP32_OFFBIG_LIBS",
	"XBS5_ILP32_OFFBIG_LINTFLAGS",
	"XBS5_LP64_OFF64_CFLAGS",
	"XBS5_LP64_OFF64_LDFLAGS",
	"XBS5_LP64_OFF64_LIBS",
	"XBS5_LP64_OFF64_LINTFLAGS",
	"XBS5_LPBIG_OFFBIG_CFLAGS",
	"XBS5_LPBIG_OFFBIG_LDFLAGS",
	"XBS5_LPBIG_OFFBIG_LIBS",
	"XBS5_LPBIG_OFFBIG_LINTFLAGS",
	"XXX5_MYBIG_VERBIG_MYFLAGS",
	0
};

int main()
{
	size_t len = 0, retval;	/* return values for confstr(3C) */
	int i;
	char *buf = NULL;

	tst_tmpdir();		/* Now temp file is open */

/*--------------------------------------------------------------------------*/

	errno = 0;
	for (i = 0; confstr_vars[i]; i++) {
		len = confstr(confstr_var_vals[i], NULL, (size_t) 0);
		if (len != 0) {
			/* Allocate space for the buffer with size len */
			if ((buf = (char *)malloc(len)) == NULL) {
				tst_resm(TFAIL,
					 "\tmalloc() fails, error= %d\n",
					 errno);
				local_flag = FAILED;
				break;
			}
			/* Better to reset memory space of buffer */
			memset(buf, 0, len);

			/* Get the value of config. variables thur. confstr */
			retval = confstr(confstr_var_vals[i], buf, len);
			if (retval != len) {
				tst_resm(TFAIL,
					 "\tconfstr returns invalid value :%s for variable: %s\n",
					 buf, confstr_vars[i]);
				local_flag = FAILED;

				/* Free the memory and exit from loop */
				free(buf);
				break;
			}

			/* Free the memory allocated for config. name */
			free(buf);
			/* Reset the buffer contents to NULL */
			buf = '\0';
		} else {
			if (!buf) {
				if ((!strcmp("XXX5_MYBIG_VERBIG_MYFLAGS",
					     confstr_vars[i])) &&
				    (errno != EINVAL)) {
					tst_resm(TFAIL,
						 "\tconfstr returns invalid error %d\n",
						 errno);
					local_flag = FAILED;
					break;
				}
			} else {
				tst_resm(TFAIL,
					 "\tconfstr returns string value: %s for variable %s\n",
					 buf, confstr_vars[i]);
				local_flag = FAILED;
				break;
			}
		}
	}
	(local_flag == PASSED) ? tst_resm(TPASS, "Test Passed")
	    : tst_resm(TFAIL, "Test Failed");

	tst_rmdir();
/*--------------------------------------------------------------*/
	tst_exit();		/* THIS CALL DOES NOT RETURN - EXITS!!  */
/*--------------------------------------------------------------*/
	tst_exit();

}