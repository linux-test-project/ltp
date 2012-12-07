/* Debugging routines for the control program. */

/*
 * Copyright (C) 2003-2006 IBM
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/time.h>

#include "debug.h"

#define BUF_LEN 256

int pounder_fprintf(FILE * stream, const char *format, ...)
{
	struct timeval tv;
	struct tm *time;
	char buf[BUF_LEN];
	int ret;
	va_list args;
	FILE *logfile;

	snprintf(buf, BUF_LEN, "%s/POUNDERLOG", getenv("POUNDER_LOGDIR"));
	logfile = fopen(buf, "a");
	if (logfile == NULL) {
		perror(buf);
	}

	gettimeofday(&tv, NULL);
	time = localtime(&tv.tv_sec);
	strftime(buf, BUF_LEN, "[%Y-%m-%d %H:%M:%S]", time);

	fprintf(stream, "%s (%d) ", buf, getpid());

	va_start(args, format);
	ret = vfprintf(stream, format, args);
	va_end(args);

	if (logfile != NULL) {
		fprintf(logfile, "%s (%d) ", buf, getpid());
		va_start(args, format);
		vfprintf(logfile, format, args);
		va_end(args);
		fclose(logfile);
	}

	fflush(stream);

	return ret;
}

const char *fail_msg = "\e[33;1mFAIL\e[0m";
const char *pass_msg = "\e[32;1mPASS\e[0m";
const char *abort_msg = "\e[31;1mABORT\e[0m";
const char *start_msg = "\e[36;1mSTART\e[0m";
