/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
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

/*
 * runcc.a - common functions for lib6 testing
 *
 * HISTORY
 *	05/2005 written by David L Stevens
 *
 * RESTRICTIONS:
 *  None.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>

#include <sys/wait.h>

#include "test.h"
#include "usctest.h"

char fieldref[1024];
char program[8192];

char *filetmpl = "/tmp/%.*s_XXXXXX";

char cmd[1024];

/*
 * like strspn, with ASCII, numbers and underscore only
 */
int
strfpn(char *name)
{
	int i;

	for (i=0; *name; ++name, ++i)
		if (!(isalnum(*name) || *name == '_'))
			break;
	return i;
}

int
runcc(char *tname, char *filename0, char *program)
{
	static char filename[1024];
	int fd, es, saved_errno;
	char * cflags = "";

	fd = mkstemp(filename0);
	if (fd < 0) {
		perror("mkstemp");
		return -1;
	}
	strncpy(filename, filename0, sizeof(filename)-1);
	filename[sizeof(filename)-1] = '\0';
	strncat(filename, ".c", sizeof(filename)-strlen(filename)-1);
	if (rename(filename0, filename) < 0) {
		perror("rename");
		unlink(filename0);
		return -1;
	}
	if (write(fd, program, strlen(program)) < 0) {
		perror("write");
		unlink(filename);
		return -1;
	}
	(void) close(fd);

	cflags = getenv("CFLAGS");
	if (cflags == NULL) {
		tst_resm(TINFO, "CFLAGS not found, using default cc arch.");
		cflags = "";
	}

	snprintf(cmd, sizeof(cmd), "%s %s -o %s %s > /tmp/test 2>&1", "cc",
		cflags, filename0, filename);
	es = system(cmd);
	if (WEXITSTATUS(es) == 127) {
		tst_resm(TBROK, "can't run C compiler: \"%s\"", cmd);
		if (unlink(filename) < 0)
			tst_resm(TWARN, "%s; unlink \"%s\" failed: %s", tname,
				filename, strerror(errno));
		return -1;
	}
	if (unlink(filename) < 0)
		tst_resm(TWARN, "%s: unlink \"%s\" failed: %s", tname,
			filename, strerror(errno));

	if (WIFSIGNALED(es) &&
	    (WTERMSIG(es) == SIGINT || WTERMSIG(es) == SIGQUIT))
		exit(1);

	if (WEXITSTATUS(es)) {
		tst_resm(TFAIL, "%s: not present", tname);
		return -1;
	}
	/* run the test */

	es = system(filename0);
	saved_errno = errno;
	if (unlink(filename0) < 0)
		tst_resm(TWARN, "%s: unlink \"%s\" failed: %s", tname,
			filename0, strerror(errno));

	if (WIFSIGNALED(es) &&
	    (WTERMSIG(es) == SIGINT || WTERMSIG(es) == SIGQUIT))
		exit(1);

	if (WEXITSTATUS(es) == 127)
		tst_resm(TBROK, "%s: can't run \"%s\": %s", tname, filename0,
			strerror(saved_errno));
	if (WEXITSTATUS(es))
		tst_resm(TFAIL, "%s: present, but incorrect", tname);
	else
		tst_resm(TPASS, "%s present and correct", tname);
	return 0;
}

char *field_fmt = "\n\texit((offsetof(struct %s, %s) != %s) || "
	"sizeof(tst.%s) != (%s));\n";
/* no offset check */
char *field_fmt2 = "\n\texit(sizeof(tst.%s) != (%s));\n";

const char *stmpl =
"%s\n#ifndef offsetof\n"\
"#define offsetof(dtype, dfield) ((int)&((dtype *)0)->dfield)\n"\
"#endif\n\nstruct %s tst;\n\nmain(int argc, char *argv[])\n{\n\t%s\n}\n";

int
structcheck(char *tname, char *incl, char *structure, char *field,
	char *offset, char *size)
{
	int rv;
	static char filename[1024];

	if (offset)
		sprintf(fieldref, field_fmt, structure, field, offset, field,
			size);
	else
		sprintf(fieldref, field_fmt2, field, size);
	sprintf(program, stmpl, incl, structure, fieldref);
	snprintf(filename, sizeof(filename), filetmpl, strfpn(structure),
		structure);
	rv = runcc(tname, filename, program);
	return rv;
}

char *aliasfmt = "exit(&tst.%s != &tst.%s || sizeof(tst.%s) != sizeof(tst.%s));";

int
aliascheck(char *tname, char *incl, char *structure, char *field, char *dname)
{
	int rv;
	static char filename[1024];

	sprintf(fieldref, aliasfmt, field, dname, field, dname);
	sprintf(program, stmpl, incl, structure, fieldref);
	snprintf(filename, sizeof(filename), filetmpl, strfpn(structure),
		structure);
	rv = runcc(tname, filename, program);
	return rv;
}

const char *dtmpl =
"%s\n\nmain(int argc, char *argv[])\n{\n\texit((%s) != (%s));\n}\n";

int
valuecheck(char *tname, char *incl, char *dname, char *dval)
{
	int rv;
	static char filename[1024];

	sprintf(program, dtmpl, incl, dname, dval);
	snprintf(filename, sizeof(filename), filetmpl, strfpn(dname), dname);
	rv = runcc(tname, filename, program);
	return rv;
}

const char *ftmpl =
"%s\n\nmain(int argc, char *argv[])\n{\n#ifdef %s\n\texit(0);\n#else\n"
	"\tsyntax error;\n#endif\n}\n";

int
funccheck(char *tname, char *incl, char *fname)
{
	int rv;
	static char filename[1024];

	sprintf(program, ftmpl, incl, fname);
	snprintf(filename, sizeof(filename), filetmpl, strfpn(fname), fname);
	rv = runcc(tname, filename, program);
	return rv;
}