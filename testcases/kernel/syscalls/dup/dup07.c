/*
 *
 *   Copyright (c) International Business Machines  Corp., 2002
 *    ported from SPIE, section2/iosuite/dup3.c, by Airong Zhang
 *   Copyright (c) 2013 Cyril Hrubis <chrubis@suse.cz>
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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
  WHAT:  Is the access mode the same for both file descriptors?
          0: read only?
          1: write only?
          2: read/write?
  HOW:   Creat a file with each access mode; dup each file descriptor;
         stat each file descriptor and compare mode of each pair
*/

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "test.h"

char *TCID = "dup07";
int TST_TOTAL = 3;

static const char *testfile = "dup07";

static void setup(void);
static void cleanup(void);

int main(int ac, char **av)
{
	struct stat retbuf;
	struct stat dupbuf;
	int rdoret, wroret, rdwret;
	int duprdo, dupwro, duprdwr;

	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		if ((rdoret = creat(testfile, 0444)) == -1) {
			tst_resm(TFAIL, "Unable to creat file '%s'", testfile);
		} else {
			if ((duprdo = dup(rdoret)) == -1) {
				tst_resm(TFAIL, "Unable to dup '%s'", testfile);
			} else {
				fstat(rdoret, &retbuf);
				fstat(duprdo, &dupbuf);
				if (retbuf.st_mode != dupbuf.st_mode) {
					tst_resm(TFAIL,
						 "rdonly and dup do not match");
				} else {
					tst_resm(TPASS,
					         "Passed in read mode.");
				}
				close(duprdo);
			}
			close(rdoret);
		}

		unlink(testfile);
		
		if ((wroret = creat(testfile, 0222)) == -1) {
			tst_resm(TFAIL, "Unable to creat file '%s'", testfile);
		} else {
			if ((dupwro = dup(wroret)) == -1) {
				tst_resm(TFAIL, "Unable to dup '%s'", testfile);
			} else {
				fstat(wroret, &retbuf);
				fstat(dupwro, &dupbuf);
				if (retbuf.st_mode != dupbuf.st_mode) {
					tst_resm(TFAIL,
						 "wronly and dup do not match");
				} else {
					tst_resm(TPASS,
					         "Passed in write mode.");
				}
				close(dupwro);
			}
			close(wroret);

		}

		unlink(testfile);

		if ((rdwret = creat(testfile, 0666)) == -1) {
			tst_resm(TFAIL, "Unable to creat file '%s'", testfile);
		} else {
			if ((duprdwr = dup(rdwret)) == -1) {
				tst_resm(TFAIL, "Unable to dup '%s'", testfile);
			} else {
				fstat(rdwret, &retbuf);
				fstat(duprdwr, &dupbuf);
				if (retbuf.st_mode != dupbuf.st_mode) {
					tst_resm(TFAIL,
						 "rdwr and dup do not match");
				} else {
					tst_resm(TPASS,
					         "Passed in read/write mode.");
				}
				close(duprdwr);
			}
			close(rdwret);
		}
		
		unlink(testfile);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_tmpdir();
}

static void cleanup(void)
{
	tst_rmdir();
}
