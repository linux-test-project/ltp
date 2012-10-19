/******************************************************************************/
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                  */
/* the GNU General Public License for more details.                           */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program;  if not, write to the Free Software               */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    */
/*                                                                            */
/******************************************************************************/
/*
 * tomoyo_rewrite_test.c
 *
 * Testing program for security/tomoyo/
 *
 * Copyright (C) 2005-2010  NTT DATA CORPORATION
 */
#include "include.h"

static int should_fail = 0;

static void show_prompt(const char *str)
{
	printf("Testing %35s: (%s) ", str,
	       should_fail ? "must fail" : "must success");
	errno = 0;
}

static void show_result(int result)
{
	if (should_fail) {
		if (result == EOF) {
			if (errno == EPERM)
				printf("OK: Permission denied.\n");
			else
				printf("BUG!\n");
		} else {
			printf("BUG!\n");
		}
	} else {
		if (result != EOF)
			printf("OK\n");
		else
			printf("BUG!\n");
	}
}

#define REWRITE_PATH "/tmp/rewrite_test"

static void stage_rewrite_test(void)
{
	int fd;

	/* Start up */
	write_domain_policy("allow_read/write " REWRITE_PATH, 0);
	write_domain_policy("allow_truncate " REWRITE_PATH, 0);
	write_domain_policy("allow_create " REWRITE_PATH " 0600", 0);
	write_domain_policy("allow_unlink " REWRITE_PATH, 0);
	write_exception_policy("deny_rewrite " REWRITE_PATH, 0);
	set_profile(3, "file::open");
	set_profile(3, "file::create");
	set_profile(3, "file::truncate");
	set_profile(3, "file::rewrite");
	set_profile(3, "file::unlink");
	close(open(REWRITE_PATH, O_WRONLY | O_APPEND | O_CREAT, 0600));

	/* Enforce mode */
	should_fail = 0;

	show_prompt("open(O_RDONLY)");
	fd = open(REWRITE_PATH, O_RDONLY);
	show_result(fd);
	close(fd);

	show_prompt("open(O_WRONLY | O_APPEND)");
	fd = open(REWRITE_PATH, O_WRONLY | O_APPEND);
	show_result(fd);
	close(fd);

	should_fail = 1;
	show_prompt("open(O_WRONLY)");
	fd = open(REWRITE_PATH, O_WRONLY);
	show_result(fd);
	close(fd);

	show_prompt("open(O_WRONLY | O_TRUNC)");
	fd = open(REWRITE_PATH, O_WRONLY | O_TRUNC);
	show_result(fd);
	close(fd);

	show_prompt("open(O_WRONLY | O_TRUNC | O_APPEND)");
	fd = open(REWRITE_PATH, O_WRONLY | O_TRUNC | O_APPEND);
	show_result(fd);
	close(fd);

	show_prompt("truncate()");
	show_result(truncate(REWRITE_PATH, 0));

	fd = open(REWRITE_PATH, O_WRONLY | O_APPEND);
	show_prompt("ftruncate()");
	show_result(ftruncate(fd, 0));

	show_prompt("fcntl(F_SETFL, ~O_APPEND)");
	show_result(fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) & ~O_APPEND));
	close(fd);

	/* Permissive mode */
	set_profile(2, "file::open");
	set_profile(2, "file::create");
	set_profile(2, "file::truncate");
	set_profile(2, "file::rewrite");
	set_profile(2, "file::unlink");
	should_fail = 0;

	show_prompt("open(O_RDONLY)");
	fd = open(REWRITE_PATH, O_RDONLY);
	show_result(fd);
	close(fd);

	show_prompt("open(O_WRONLY | O_APPEND)");
	fd = open(REWRITE_PATH, O_WRONLY | O_APPEND);
	show_result(fd);
	close(fd);

	show_prompt("open(O_WRONLY)");
	fd = open(REWRITE_PATH, O_WRONLY);
	show_result(fd);
	close(fd);

	show_prompt("open(O_WRONLY | O_TRUNC)");
	fd = open(REWRITE_PATH, O_WRONLY | O_TRUNC);
	show_result(fd);
	close(fd);

	show_prompt("open(O_WRONLY | O_TRUNC | O_APPEND)");
	fd = open(REWRITE_PATH, O_WRONLY | O_TRUNC | O_APPEND);
	show_result(fd);
	close(fd);

	show_prompt("truncate()");
	show_result(truncate(REWRITE_PATH, 0));

	fd = open(REWRITE_PATH, O_WRONLY | O_APPEND);
	show_prompt("ftruncate()");
	show_result(ftruncate(fd, 0));

	show_prompt("fcntl(F_SETFL, ~O_APPEND)");
	show_result(fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) & ~O_APPEND));
	close(fd);

	/* Clean up */
	unlink(REWRITE_PATH);
	write_exception_policy("deny_rewrite " REWRITE_PATH, 0);
	printf("\n\n");
}

int main(int argc, char *argv[])
{
	tomoyo_test_init();
	stage_rewrite_test();
	clear_status();
	return 0;
}
