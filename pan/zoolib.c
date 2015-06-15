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
/* $Id: zoolib.c,v 1.8 2009/06/09 17:59:46 subrata_modak Exp $ */
/*
 * ZooLib
 *
 * A Zoo is a file used to record what test tags are running at the moment.
 * If the system crashes, we should be able to look at the zoo file to find out
 * what was currently running.  This is especially helpful when running multiple
 * tests at the same time.
 *
 * The zoo file is meant to be a text file that fits on a standard console.
 * You should be able to watch it with `cat zoofile`
 *
 * zoo file format:
 * 	80 characters per line, ending with a \n
 * 	available lines start with '#'
 * 	expected line fromat: pid_t,tag,cmdline
 *
 */

#include <signal.h>
#include <stdlib.h>		/* for getenv */
#include <string.h>
#include "zoolib.h"

char zoo_error[ZELEN];

#ifdef __linux__
/* glibc2.2 definition needs -D_XOPEN_SOURCE, which breaks other things. */
extern int sighold(int __sig);
extern int sigrelse(int __sig);
#endif

/* zoo_mark(): private function to make an entry to the zoo
 * 	returns 0 on success, -1 on error */
static int zoo_mark(zoo_t z, char *entry);
static int zoo_lock(zoo_t z);
static int zoo_unlock(zoo_t z);
/* cat_args(): helper function to make cmdline from argc, argv */
char *cat_args(int argc, char **argv);

/* zoo_getname(): create a filename to use for the zoo */
char *zoo_getname(void)
{
	char buf[1024];
	char *zoo;

	zoo = getenv("ZOO");
	if (zoo) {
		snprintf(buf, 1024, "%s/%s", zoo, "active");
		return strdup(buf);
	} else {
		/* if there is no environment variable, we don't know where to put it */
		return NULL;
	}
}

/* zoo_open(): open a zoo for use */
zoo_t zoo_open(char *zooname)
{
	zoo_t new_zoo;

	new_zoo = (zoo_t) fopen(zooname, "r+");
	if (!new_zoo) {
		if (errno == ENOENT) {
			/* file doesn't exist, try fopen(xxx, "a+") */
			new_zoo = (zoo_t) fopen(zooname, "a+");
			if (!new_zoo) {
				/* total failure */
				snprintf(zoo_error, ZELEN,
					 "Could not open zoo as \"%s\", errno:%d %s",
					 zooname, errno, strerror(errno));
				return 0;
			}
			fclose(new_zoo);
			new_zoo = fopen(zooname, "r+");
		} else {
			snprintf(zoo_error, ZELEN,
				 "Could not open zoo as \"%s\", errno:%d %s",
				 zooname, errno, strerror(errno));
		}
	}
	return new_zoo;
}

int zoo_close(zoo_t z)
{
	int ret;

	ret = fclose(z);
	if (ret) {
		snprintf(zoo_error, ZELEN,
			 "closing zoo caused error, errno:%d %s",
			 errno, strerror(errno));
	}
	return ret;
}

static int zoo_mark(zoo_t z, char *entry)
{
	FILE *fp = (FILE *) z;
	int found = 0;
	long pos;
	char buf[BUFLEN];

	if (fp == NULL)
		return -1;

	if (zoo_lock(z))
		return -1;

	/* first fit */
	rewind(fp);

	do {
		pos = ftell(fp);

		if (fgets(buf, BUFLEN, fp) == NULL)
			break;

		if (buf[0] == '#') {
			rewind(fp);
			if (fseek(fp, pos, SEEK_SET)) {
				/* error */
				snprintf(zoo_error, ZELEN,
					 "seek error while writing to zoo file, errno:%d %s",
					 errno, strerror(errno));
				return -1;
			}
			/* write the entry, left justified, and padded/truncated to the
			 * same size as the previous entry */
			fprintf(fp, "%-*.*s\n", (int)strlen(buf) - 1,
				(int)strlen(buf) - 1, entry);
			found = 1;
			break;
		}
	} while (1);

	if (!found) {
		if (fseek(fp, 0, SEEK_END)) {
			snprintf(zoo_error, ZELEN,
				 "error seeking to end of zoo file, errno:%d %s",
				 errno, strerror(errno));
			return -1;
		}
		fprintf(fp, "%-*.*s\n", 79, 79, entry);
	}
	fflush(fp);

	if (zoo_unlock(z))
		return -1;
	return 0;
}

int zoo_mark_cmdline(zoo_t z, pid_t p, char *tag, char *cmdline)
{
	char new_entry[BUFLEN];

	snprintf(new_entry, 80, "%d,%s,%s", p, tag, cmdline);
	return zoo_mark(z, new_entry);
}

int zoo_mark_args(zoo_t z, pid_t p, char *tag, int ac, char **av)
{
	char *cmdline;
	int ret;

	cmdline = cat_args(ac, av);
	ret = zoo_mark_cmdline(z, p, tag, cmdline);

	free(cmdline);
	return ret;
}

int zoo_clear(zoo_t z, pid_t p)
{
	FILE *fp = (FILE *) z;
	long pos;
	char buf[BUFLEN];
	pid_t that_pid;
	int found = 0;

	if (fp == NULL)
		return -1;

	if (zoo_lock(z))
		return -1;
	rewind(fp);

	do {
		pos = ftell(fp);

		if (fgets(buf, BUFLEN, fp) == NULL)
			break;

		if (buf[0] == '#')
			continue;

		that_pid = atoi(buf);
		if (that_pid == p) {
			if (fseek(fp, pos, SEEK_SET)) {
				/* error */
				snprintf(zoo_error, ZELEN,
					 "seek error while writing to zoo file, errno:%d %s",
					 errno, strerror(errno));
				return -1;
			}
			if (ftell(fp) != pos) {
				printf("fseek failed\n");
			}
			fputs("#", fp);
			found = 1;
			break;
		}
	} while (1);

	fflush(fp);

	/* FIXME: unlock zoo file */
	if (zoo_unlock(z))
		return -1;

	if (!found) {
		snprintf(zoo_error, ZELEN,
			 "zoo_clear() did not find pid(%d)", p);
		return 1;
	}
	return 0;

}

pid_t zoo_getpid(zoo_t z, char *tag)
{
	FILE *fp = (FILE *) z;
	char buf[BUFLEN], *s;
	pid_t this_pid = -1;

	if (fp == NULL)
		return -1;

	if (zoo_lock(z))
		return -1;

	rewind(fp);
	do {
		if (fgets(buf, BUFLEN, fp) == NULL)
			break;

		if (buf[0] == '#')
			continue;	/* recycled line */

		if ((s = strchr(buf, ',')) == NULL)
			continue;	/* line was not expected format */

		if (strncmp(s + 1, tag, strlen(tag)))
			continue;	/* tag does not match */

		this_pid = atoi(buf);
		break;
	} while (1);

	if (zoo_unlock(z))
		return -1;
	return this_pid;
}

int zoo_lock(zoo_t z)
{
	FILE *fp = (FILE *) z;
	struct flock zlock;
	sigset_t block_these;
	int ret;

	if (fp == NULL)
		return -1;

	zlock.l_whence = zlock.l_start = zlock.l_len = 0;
	zlock.l_type = F_WRLCK;

	sigemptyset(&block_these);
	sigaddset(&block_these, SIGINT);
	sigaddset(&block_these, SIGTERM);
	sigaddset(&block_these, SIGHUP);
	sigaddset(&block_these, SIGUSR1);
	sigaddset(&block_these, SIGUSR2);
	sigprocmask(SIG_BLOCK, &block_these, NULL);

	do {
		ret = fcntl(fileno(fp), F_SETLKW, &zlock);
	} while (ret == -1 && errno == EINTR);

	sigprocmask(SIG_UNBLOCK, &block_these, NULL);
	if (ret == -1) {
		snprintf(zoo_error, ZELEN,
			 "failed to unlock zoo file, errno:%d %s",
			 errno, strerror(errno));
		return -1;
	}
	return 0;

}

int zoo_unlock(zoo_t z)
{
	FILE *fp = (FILE *) z;
	struct flock zlock;
	sigset_t block_these;
	int ret;

	if (fp == NULL)
		return -1;

	zlock.l_whence = zlock.l_start = zlock.l_len = 0;
	zlock.l_type = F_UNLCK;

	sigemptyset(&block_these);
	sigaddset(&block_these, SIGINT);
	sigaddset(&block_these, SIGTERM);
	sigaddset(&block_these, SIGHUP);
	sigaddset(&block_these, SIGUSR1);
	sigaddset(&block_these, SIGUSR2);
	sigprocmask(SIG_BLOCK, &block_these, NULL);

	do {
		ret = fcntl(fileno(fp), F_SETLKW, &zlock);
	} while (ret == -1 && errno == EINTR);

	sigprocmask(SIG_UNBLOCK, &block_these, NULL);

	if (ret == -1) {
		snprintf(zoo_error, ZELEN,
			 "failed to lock zoo file, errno:%d %s",
			 errno, strerror(errno));
		return -1;
	}
	return 0;
}

char *cat_args(int argc, char **argv)
{
	int a, size;
	char *cmd;

	for (size = a = 0; a < argc; a++) {
		size += strlen(argv[a]);
		size++;
	}

	if ((cmd = malloc(size)) == NULL) {
		snprintf(zoo_error, ZELEN,
			 "Malloc Error, %s/%d", __FILE__, __LINE__);
		return NULL;
	}

	*cmd = '\0';
	for (a = 0; a < argc; a++) {
		if (a != 0)
			strcat(cmd, " ");
		strcat(cmd, argv[a]);
	}

	return cmd;
}

#if defined(UNIT_TEST)

void zt_add(zoo_t z, int n)
{
	char cmdline[200];
	char tag[10];

	snprintf(tag, 10, "%s%d", "test", n);
	snprintf(cmdline, 200, "%s%d %s %s %s", "runtest", n, "one", "two",
		 "three");

	zoo_mark_cmdline(z, n, tag, cmdline);
}

int main(int argc, char *argv[])
{

	char *zooname;
	zoo_t test_zoo;
	char *test_tag = "unittest";
	int i, j;

	zooname = zoo_getname();

	if (!zooname) {
		zooname = strdup("test_zoo");
	}
	printf("Test zoo filename is %s\n", zooname);

	if ((test_zoo = zoo_open(zooname)) == NULL) {
		printf("Error opennning zoo\n");
		exit(-1);
	}

	zoo_mark_args(test_zoo, getpid(), test_tag, argc, argv);

	for (j = 0; j < 5; j++) {
		for (i = 0; i < 20; i++) {
			zt_add(test_zoo, i);
		}

		for (; i >= 0; i--) {
			zoo_clear(test_zoo, i);
		}
	}

	zoo_clear(test_zoo, getpid());

	return 0;
}

#endif
