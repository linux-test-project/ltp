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
 * Test Name: asapi_03
 *
 * Test Description:
 *  These tests are for the "Advanced Sockets API" (RFC 3542)
 *  Verify that in6 and sockaddr fields are present.
 *
 * Usage:  <for command-line>
 *  asapi_03
 *
 * HISTORY
 *	03/2005 written by David L Stevens
 *
 * RESTRICTIONS:
 *  None.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include <sys/wait.h>

#include <netinet/ip6.h>

#include "test.h"
#include "usctest.h"
#include "runcc.h"

char *TCID="asapi_03";		/* Test program identifier.    */

void setup(void), cleanup(void);

enum ttype { EXISTS, ALIAS, VALUE };

struct ftent {
	char	*ft_tname;		/* test name */
	int	ft_type;		/* test type */
	char	*ft_incl;		/* include file list */
	char	*ft_struct;		/* structure name */
	char	*ft_field;		/* field name */
	char	*ft_offset;		/* field offset */
	union {
		char	*fu_value;	/* field size or value */
		char	*fu_dname;	/* #define name */
	} ftun;
#define ft_value	ftun.fu_value
#define ft_dname	ftun.fu_dname
} ftab[] = {
/* section 2.2.3, MLDv1 */
	{ "MLD_LISTENER_QUERY", VALUE, ICMP6_H, "MLD_LISTENER_QUERY",
		0, 0, .ft_dname="130" },
	{ "MLD_LISTENER_REPORT", VALUE, ICMP6_H, "MLD_LISTENER_REPORT",
		0, 0, .ft_dname="131" },
	{ "MLD_LISTENER_REDUCTION", VALUE, ICMP6_H, "MLD_LISTENER_REDUCTION",
		0, 0, .ft_dname="132" },
	{ "mld_hdr mld_icmp6_hdr", EXISTS, ICMP6_H, "mld_hdr",
		"mld_icmp6_hdr", "0", .ft_value="sizeof(struct icmp6_hdr)" },
	{ "mld_hdr mld_addr", EXISTS, ICMP6_H, "mld_hdr", "mld_addr",
		"sizeof(struct icmp6_hdr)",
		.ft_value="sizeof(struct in6_addr)" },
	{ "mld_hdr mld_type define", ALIAS, ICMP6_H, "mld_hdr",
		"mld_icmp6_hdr.icmp6_type", 0, .ft_dname="mld_type" },
	{ "mld_hdr mld_code define", ALIAS, ICMP6_H, "mld_hdr",
		"mld_icmp6_hdr.icmp6_code", 0, .ft_dname="mld_code" },
	{ "mld_hdr mld_cksum define", ALIAS, ICMP6_H, "mld_hdr",
		"mld_icmp6_hdr.icmp6_cksum", 0, .ft_dname="mld_cksum" },
	{ "mld_hdr mld_maxdelay define", ALIAS, ICMP6_H, "mld_hdr",
		"mld_icmp6_hdr.icmp6_data16[0]", 0, .ft_dname="mld_maxdelay" },
	{ "mld_hdr mld_reserved define", ALIAS, ICMP6_H, "mld_hdr",
		"mld_icmp6_hdr.icmp6_data16[1]", 0, .ft_dname="mld_reserved" },
/* section 2.2.4, Router renumbering */
	{ "ICMP6_ROUTER_RENUMBERING", VALUE, ICMP6_H,"ICMP6_ROUTER_RENUMBERING",
		0, 0, .ft_dname="138" },
	{ "icmp6_router_renum rr_hdr", EXISTS, ICMP6_H, "icmp6_router_renum",
		"rr_hdr", "0", .ft_value="sizeof(struct icmp6_hdr)" },
	{ "icmp6_router_renum rr_segnum", EXISTS, ICMP6_H, "icmp6_router_renum",
		"rr_segnum", "sizeof(struct icmp6_hdr)", .ft_value="1" },
	{ "icmp6_router_renum rr_flags", EXISTS, ICMP6_H, "icmp6_router_renum",
		"rr_flags", "sizeof(struct icmp6_hdr)+1", .ft_value="1" },
	{ "icmp6_router_renum rr_maxdelay", EXISTS, ICMP6_H,
		"icmp6_router_renum", "rr_maxdelay",
		"sizeof(struct icmp6_hdr)+2",  .ft_value="2" },
	{ "icmp6_router_renum rr_reserved", EXISTS, ICMP6_H,
		"icmp6_router_renum", "rr_reserved",
		"sizeof(struct icmp6_hdr)+4", .ft_value="4" },
	{ "icmp6_router_renum rr_type define", ALIAS, ICMP6_H,
		"icmp6_router_renum", "rr_hdr.icmp6_type",
		0, .ft_dname="rr_type" },
	{ "icmp6_router_renum rr_code define", ALIAS, ICMP6_H,
		"icmp6_router_renum", "rr_hdr.icmp6_code",
		0, .ft_dname="rr_code" },
	{ "icmp6_router_renum rr_cksum define", ALIAS, ICMP6_H,
		"icmp6_router_renum", "rr_hdr.icmp6_cksum",
		0, .ft_dname="rr_cksum" },
	{ "icmp6_router_renum rr_seqnum define", ALIAS, ICMP6_H,
		"icmp6_router_renum", "rr_hdr.icmp6_data32[0]",
		0, .ft_dname="rr_seqnum" },
	{ "ICMP6_RR_FLAGS_TEST", VALUE, ICMP6_H, "ICMP6_RR_FLAGS_TEST",
		0, 0, .ft_dname="0x80" },
	{ "ICMP6_RR_FLAGS_REQRESULT", VALUE, ICMP6_H,
		"ICMP6_RR_FLAGS_REQRESULT", 0, 0, .ft_dname="0x40" },
	{ "ICMP6_RR_FLAGS_FORCEAPPLY", VALUE, ICMP6_H,
		"ICMP6_RR_FLAGS_FORCEAPPLY", 0, 0, .ft_dname="0x20" },
	{ "ICMP6_RR_FLAGS_SPECSITE", VALUE, ICMP6_H,
		"ICMP6_RR_FLAGS_SPECSITE", 0, 0, .ft_dname="0x10" },
	{ "ICMP6_RR_FLAGS_PREVDONE", VALUE, ICMP6_H,
		"ICMP6_RR_FLAGS_PREVDONE", 0, 0, .ft_dname="0x08" },

	{ "rr_pco_match rpm_code", EXISTS, ICMP6_H, "rr_pco_match",
		"rpm_code", "0", .ft_value="1" },
	{ "rr_pco_match rpm_len", EXISTS, ICMP6_H, "rr_pco_match",
		"rpm_len", "1", .ft_value="1" },
	{ "rr_pco_match rpm_ordinal", EXISTS, ICMP6_H, "rr_pco_match",
		"rpm_ordinal", "2", .ft_value="1" },
	{ "rr_pco_match rpm_matchlen", EXISTS, ICMP6_H, "rr_pco_match",
		"rpm_matchlen", "3", .ft_value="1" },
	{ "rr_pco_match rpm_minlen", EXISTS, ICMP6_H, "rr_pco_match",
		"rpm_minlen", "4", .ft_value="1" },
	{ "rr_pco_match rpm_maxlen", EXISTS, ICMP6_H, "rr_pco_match",
		"rpm_maxlen", "5", .ft_value="1" },
	{ "rr_pco_match rpm_reserved", EXISTS, ICMP6_H, "rr_pco_match",
		"rpm_reserved", "6", .ft_value="2" },
	{ "rr_pco_match rpm_prefix", EXISTS, ICMP6_H, "rr_pco_match",
		"rpm_prefix", "8", .ft_value="sizeof(struct in6_addr)" },

	{ "RPM_PCO_ADD", VALUE, ICMP6_H, "RPM_PCO_ADD", 0, 0, .ft_dname="1" },
	{ "RPM_PCO_CHANGE", VALUE, ICMP6_H, "RPM_PCO_CHANGE", 0, 0,
		.ft_dname="2" },
	{ "RPM_PCO_SETGLOBAL", VALUE, ICMP6_H, "RPM_PCO_SETGLOBAL", 0, 0,
		.ft_dname="3" },

	{ "rr_pco_use rpu_uselen", EXISTS, ICMP6_H, "rr_pco_use",
		"rpu_uselen", "0", .ft_value="1" },
	{ "rr_pco_use rpu_keeplen", EXISTS, ICMP6_H, "rr_pco_use",
		"rpu_keeplen", "1", .ft_value="1" },
	{ "rr_pco_use rpu_ramask", EXISTS, ICMP6_H, "rr_pco_use",
		"rpu_ramask", "2", .ft_value="1" },
	{ "rr_pco_use rpu_raflags", EXISTS, ICMP6_H, "rr_pco_use",
		"rpu_raflags", "3", .ft_value="1" },
	{ "rr_pco_use rpu_vltime", EXISTS, ICMP6_H, "rr_pco_use",
		"rpu_vltime", "4", .ft_value="4" },
	{ "rr_pco_use rpu_pltime", EXISTS, ICMP6_H, "rr_pco_use",
		"rpu_pltime", "8", .ft_value="4" },
	{ "rr_pco_use rpu_flags", EXISTS, ICMP6_H, "rr_pco_use",
		"rpu_flags", "12", .ft_value="4" },
	{ "rr_pco_use rpu_prefix", EXISTS, ICMP6_H, "rr_pco_use",
		"rpu_prefix", "16", .ft_value="sizeof(struct in6_addr)" },

	{ "ICMP6_RR_PCOUSE_RAFLAGS_ONLINK", VALUE, ICMP6_H,
		"ICMP6_RR_PCOUSE_RAFLAGS_ONLINK", 0, 0, .ft_dname="0x20" },
	{ "ICMP6_RR_PCOUSE_RAFLAGS_AUTO", VALUE, ICMP6_H,
		"ICMP6_RR_PCOUSE_RAFLAGS_AUTO", 0, 0, .ft_dname="0x10" },
	{ "ICMP6_RR_PCOUSE_DECRVLTIME", VALUE, ICMP6_H,
		"ICMP6_RR_PCOUSE_DECRVLTIME", 0, 0,
		.ft_dname="htonl(0x80000000)" },
	{ "ICMP6_RR_PCOUSE_DECRPLTIME", VALUE, ICMP6_H,
		"ICMP6_RR_PCOUSE_DECRPLTIME", 0, 0,
		.ft_dname="htonl(0x40000000)" },

	{ "rr_result rrr_flags", EXISTS, ICMP6_H, "rr_result",
		"rrr_flags", "0", .ft_value="2" },
	{ "rr_result rrr_ordinal", EXISTS, ICMP6_H, "rr_result",
		"rrr_ordinal", "2", .ft_value="1" },
	{ "rr_result rrr_matchedlen", EXISTS, ICMP6_H, "rr_result",
		"rrr_matchedlen", "3", .ft_value="1" },
	{ "rr_result rrr_ifid", EXISTS, ICMP6_H, "rr_result",
		"rrr_ifid", "4", .ft_value="4" },
	{ "rr_result rrr_prefix", EXISTS, ICMP6_H, "rr_result",
		"rrr_prefix", "8", .ft_value="sizeof(struct in6_addr)" },

	{ "ICMP6_RR_RESULT_FLAGS_OOB", VALUE, ICMP6_H,
		"ICMP6_RR_RESULT_FLAGS_OOB", 0, 0,
		.ft_dname="htons(0x0002)" },
	{ "ICMP6_RR_RESULT_FLAGS_FORBIDDEN", VALUE, ICMP6_H,
		"ICMP6_RR_RESULT_FLAGS_FORBIDDEN", 0, 0,
		.ft_dname="htons(0x0001)" },
};

#define FTCOUNT	(sizeof(ftab)/sizeof(ftab[0]))

int TST_TOTAL = FTCOUNT;

int
main(int argc, char *argv[])
{
	int	i, lc;
	char *msg;

	/* Parse standard options given to run the test. */
	msg = parse_opts(argc, argv, (option_t *)NULL, NULL);
	if (msg != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	setup();

	for (lc = 0; TEST_LOOPING(lc); ++lc) {
		for (i=0; i<FTCOUNT; ++i) {
			switch (ftab[i].ft_type) {
			case EXISTS:
				structcheck(ftab[i].ft_tname, ftab[i].ft_incl,
					ftab[i].ft_struct, ftab[i].ft_field,
					ftab[i].ft_offset, ftab[i].ft_value);
				break;
			case ALIAS:
				aliascheck(ftab[i].ft_tname, ftab[i].ft_incl,
					ftab[i].ft_struct, ftab[i].ft_field,
					ftab[i].ft_dname);
				break;
			case VALUE:
				valuecheck(ftab[i].ft_tname, ftab[i].ft_incl,
					ftab[i].ft_struct, ftab[i].ft_dname);
				break;
			default:
				tst_resm(TBROK, "invalid type %d",
					ftab[i].ft_type);
				break;
			}
		}
	}

	cleanup();
	/* NOTREACHED */
	return 0;
}

void
setup(void)
{
	TEST_PAUSE;	/* if -P option specified */
}

void
cleanup(void)
{
	TEST_CLEANUP;
	tst_exit();
}

