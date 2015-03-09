/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *   Author: David L Stevens
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
 *   along with this program;  if not, write to the Free Software Foundation,
 *   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
/*
 *   Description:
 *     These tests are for the "Advanced Sockets API" (RFC 3542)
 *     Section 20, ancillary data macros and structure definitions
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include <sys/wait.h>

#include <netinet/in.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>

#include "test.h"
#include "runcc.h"

enum ttype { EXISTS, ALIAS, VALUE, DEFINED };

static struct etent {
	char *et_tname;		/* test name */
	int et_type;		/* test type */
	char *et_incl;		/* include file list */
	char *et_struct;	/* structure name */
	char *et_field;		/* field name */
	char *et_offset;	/* field offset */
	union {
		char *fu_value;	/* field size or value */
		char *fu_dname;	/* #define name */
	} ftun;
#define et_value	ftun.fu_value
#define et_dname	ftun.fu_dname
} etab[] = {
	/* existence checks, RFC 3542 sections 5, 20 */
	{ "msghdr msg_name", EXISTS, SOCKET_H, "msghdr",
		"msg_name", NULL, {"sizeof(void *)"} },
	{ "msghdr msg_namelen", EXISTS, SOCKET_H, "msghdr",
		"msg_namelen", NULL, {"sizeof(socklen_t)"} },
	{ "msghdr msg_iov", EXISTS, SOCKET_H, "msghdr",
		"msg_iov", NULL, {"sizeof(struct iovec *)"} },
	{ "msghdr msg_iovlen", EXISTS, SOCKET_H, "msghdr",
		"msg_iovlen", NULL, {"sizeof(size_t)"} },
	{ "msghdr msg_control", EXISTS, SOCKET_H, "msghdr",
		"msg_control", NULL, {"sizeof(void *)"} },
	{ "msghdr msg_controllen", EXISTS, SOCKET_H, "msghdr",
		"msg_controllen", NULL, {"sizeof(size_t)"} },
	{ "msghdr msg_flags", EXISTS, SOCKET_H, "msghdr",
		"msg_flags", NULL, {"sizeof(int)"} },
	{ "cmsghdr cmsg_len", EXISTS, SOCKET_H, "cmsghdr",
		"cmsg_len", NULL, {"sizeof(size_t)"} },
	{ "cmsghdr cmsg_level", EXISTS, SOCKET_H, "cmsghdr",
		"cmsg_level", NULL, {"sizeof(int)"} },
	{ "cmsghdr cmsg_type", EXISTS, SOCKET_H, "cmsghdr",
		"cmsg_type", NULL, {"sizeof(int)"} },
	{ "CMSG_DATA", DEFINED, SOCKET_H, "CMSG_DATA",
		NULL, NULL, {0} },
	{ "CMSG_NXTHDR", DEFINED, SOCKET_H, "CMSG_NXTHDR",
		NULL, NULL, {0} },
	{ "CMSG_FIRSTHDR", DEFINED, SOCKET_H, "CMSG_FIRSTHDR",
		NULL, NULL, {0} },
	{ "CMSG_SPACE", DEFINED, SOCKET_H, "CMSG_SPACE",
		NULL, NULL, {0} },
	{ "CMSG_LEN", DEFINED, SOCKET_H, "CMSG_LEN",
		NULL, NULL, {0} },
};

static void setup(void);
static void adatet(const struct etent *etptr);

char *TCID = "asapi_07";
int TST_TOTAL = ARRAY_SIZE(etab);

int main(int argc, char *argv[])
{
	int i, lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++)
			adatet(&etab[i]);
	}

	tst_exit();
}

void setup(void)
{
	TEST_PAUSE;
}

/*  existence tests */
void adatet(const struct etent *etptr)
{
	switch (etptr->et_type) {
	case EXISTS:
		structcheck(etptr->et_tname, etptr->et_incl,
			etptr->et_struct, etptr->et_field,
			etptr->et_offset, etptr->et_value);
		break;
	case ALIAS:
		aliascheck(etptr->et_tname, etptr->et_incl,
			etptr->et_struct, etptr->et_field,
			etptr->et_dname);
		break;
	case VALUE:
		valuecheck(etptr->et_tname, etptr->et_incl,
			etptr->et_struct, etptr->et_dname);
		break;
	case DEFINED:
		funccheck(etptr->et_tname, etptr->et_incl,
			etptr->et_struct);
		break;
	default:
		tst_resm(TBROK, "invalid type %d",
			etptr->et_type);
		break;
	}
}
