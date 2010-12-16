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
 * Test Name: asapi_02
 *
 * Test Description:
 *  These tests are for the "Advanced Sockets API" (RFC 3542)
 *  Verify that in6 and sockaddr fields are present.
 *
 * Usage:  <for command-line>
 *  asapi_02
 *
 * HISTORY
 *	01/2005 written by David L Stevens
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

char *TCID="asapi_02";		/* Test program identifier.    */

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
/* Section 2.2, icmp6_hdr & defines */
	{ "icmp6_hdr icmp6_type", EXISTS, ICMP6_H, "icmp6_hdr",
		"icmp6_type", "0", {"1"} },
	{ "icmp6_hdr icmp6_code", EXISTS, ICMP6_H, "icmp6_hdr",
		"icmp6_code", "1", {"1"} },
	{ "icmp6_hdr icmp6_cksum", EXISTS, ICMP6_H, "icmp6_hdr",
		"icmp6_cksum", "2", {"2"} },
	{ "icmp6_hdr icmp6_un_data32 element", EXISTS, ICMP6_H, "icmp6_hdr",
		"icmp6_dataun.icmp6_un_data32[0]", "4", {"4"} },
	{ "icmp6_hdr icmp6_un_data32 array", EXISTS, ICMP6_H, "icmp6_hdr",
		"icmp6_dataun.icmp6_un_data32", "4", {"4"} },
	{ "icmp6_hdr icmp6_un_data16 element", EXISTS, ICMP6_H, "icmp6_hdr",
		"icmp6_dataun.icmp6_un_data16[0]", "4", {"2"} },
	{ "icmp6_hdr icmp6_un_data16 array", EXISTS, ICMP6_H, "icmp6_hdr",
		"icmp6_dataun.icmp6_un_data16", "4", {"4"} },
	{ "icmp6_hdr icmp6_un_data8 element", EXISTS, ICMP6_H, "icmp6_hdr",
		"icmp6_dataun.icmp6_un_data8[0]", "4", {"1"} },
	{ "icmp6_hdr icmp6_un_data8 array", EXISTS, ICMP6_H, "icmp6_hdr",
		"icmp6_dataun.icmp6_un_data8", "4", {"4"} },
/* icmp6_hdr definitions */
	{ "icmp6_hdr icmp6_data32 define", ALIAS, ICMP6_H, "icmp6_hdr",
		"icmp6_dataun.icmp6_un_data32", NULL, {"icmp6_data32"} },
	{ "icmp6_hdr icmp6_data16 define", ALIAS, ICMP6_H, "icmp6_hdr",
		"icmp6_dataun.icmp6_un_data16", NULL, {"icmp6_data16"} },
	{ "icmp6_hdr icmp6_data8 define", ALIAS, ICMP6_H, "icmp6_hdr",
		"icmp6_dataun.icmp6_un_data8", NULL, {"icmp6_data8"} },
	{ "icmp6_hdr icmp6_pptr define", ALIAS, ICMP6_H, "icmp6_hdr",
		"icmp6_dataun.icmp6_un_data32[0]", NULL, {"icmp6_pptr"} },
	{ "icmp6_hdr icmp6_mtu define", ALIAS, ICMP6_H, "icmp6_hdr",
		"icmp6_dataun.icmp6_un_data32[0]", NULL, {"icmp6_mtu"} },
	{ "icmp6_hdr icmp6_id define", ALIAS, ICMP6_H, "icmp6_hdr",
		"icmp6_dataun.icmp6_un_data16[0]", NULL, {"icmp6_id"} },
	{ "icmp6_hdr icmp6_seq define", ALIAS, ICMP6_H, "icmp6_hdr",
		"icmp6_dataun.icmp6_un_data16[1]", NULL, {"icmp6_seq"} },
	{ "icmp6_hdr icmp6_maxdelay define", ALIAS, ICMP6_H, "icmp6_hdr",
		"icmp6_dataun.icmp6_un_data16[0]", NULL,
		{"icmp6_maxdelay"} },
/* Section 2.2.1 ICMPv6 Type and Code Values */
	{ "ICMP6_DST_UNREACH", VALUE, ICMP6_H, "ICMP6_DST_UNREACH",
		NULL, NULL, {"1"} },
	{ "ICMP6_PACKET_TOO_BIG", VALUE,ICMP6_H, "ICMP6_PACKET_TOO_BIG",
		NULL, NULL, {"2"} },
	{ "ICMP6_TIME_EXCEEDED", VALUE, ICMP6_H, "ICMP6_TIME_EXCEEDED",
		NULL, NULL, {"3"} },
	{ "ICMP6_PARAM_PROB", VALUE, ICMP6_H, "ICMP6_PARAM_PROB",
		NULL, NULL, {"4"} },
	{ "ICMP6_INFOMSG_MASK", VALUE, ICMP6_H, "ICMP6_INFOMSG_MASK",
		NULL, NULL, {"0x80"} },
	{ "ICMP6_ECHO_REQUEST", VALUE, ICMP6_H, "ICMP6_ECHO_REQUEST",
		NULL, NULL, {"128"} },
	{ "ICMP6_ECHO_REPLY", VALUE, ICMP6_H, "ICMP6_ECHO_REPLY",
		NULL, NULL, {"129"} },
	{ "ICMP6_DST_UNREACH_NOROUTE", VALUE, ICMP6_H,
		"ICMP6_DST_UNREACH_NOROUTE", NULL, NULL, {"0"} },
	{ "ICMP6_DST_UNREACH_ADMIN", VALUE, ICMP6_H,
		"ICMP6_DST_UNREACH_ADMIN", NULL, NULL, {"1"} },
	{ "ICMP6_DST_UNREACH_BEYONDSCOPE", VALUE, ICMP6_H,
		"ICMP6_DST_UNREACH_BEYONDSCOPE", NULL, NULL, {"2"} },
	{ "ICMP6_DST_UNREACH_ADDR", VALUE, ICMP6_H,
		"ICMP6_DST_UNREACH_ADDR", NULL, NULL, {"3"} },
	{ "ICMP6_DST_UNREACH_NOPORT", VALUE, ICMP6_H,
		"ICMP6_DST_UNREACH_NOPORT", NULL, NULL, {"4"} },
	{ "ICMP6_TIME_EXCEED_TRANSIT", VALUE, ICMP6_H,
		"ICMP6_TIME_EXCEED_TRANSIT", NULL, NULL, {"0"} },
	{ "ICMP6_TIME_EXCEED_REASSEMBLY", VALUE, ICMP6_H,
		"ICMP6_TIME_EXCEED_REASSEMBLY", NULL, NULL, {"1"} },
	{ "ICMP6_PARAMPROB_HEADER", VALUE, ICMP6_H,
		"ICMP6_PARAMPROB_HEADER", NULL, NULL, {"0"} },
	{ "ICMP6_PARAMPROB_NEXTHEADER", VALUE, ICMP6_H,
		"ICMP6_PARAMPROB_NEXTHEADER", NULL, NULL, {"1"} },
	{ "ICMP6_PARAMPROB_OPTION", VALUE, ICMP6_H,
		"ICMP6_PARAMPROB_OPTION", NULL, NULL, {"2"} },
/* section 2.2.2, Neighbor Discovery */
	{ "ND_ROUTER_SOLICIT", VALUE, ICMP6_H, "ND_ROUTER_SOLICIT",
		NULL, NULL, {"133"} },
	{ "ND_ROUTER_ADVERT", VALUE, ICMP6_H, "ND_ROUTER_ADVERT",
		NULL, NULL, {"134"} },
	{ "ND_NEIGHBOR_SOLICIT", VALUE, ICMP6_H, "ND_NEIGHBOR_SOLICIT",
		NULL, NULL, {"135"} },
	{ "ND_NEIGHBOR_ADVERT", VALUE, ICMP6_H, "ND_NEIGHBOR_ADVERT",
		NULL, NULL, {"136"} },
	{ "ND_REDIRECT", VALUE, ICMP6_H, "ND_REDIRECT",
		NULL, NULL, {"137"} },

	{ "nd_router_solicit nd_rs_hdr", EXISTS, ICMP6_H, "nd_router_solicit",
		"nd_rs_hdr", "0", {"sizeof(struct icmp6_hdr)"} },
	{ "nd_router_solicit nd_rs_type define", ALIAS, ICMP6_H,
		"nd_router_solicit", "nd_rs_hdr.icmp6_type",
		NULL, {"nd_rs_type"} },
	{ "nd_router_solicit nd_rs_code define", ALIAS, ICMP6_H,
		"nd_router_solicit", "nd_rs_hdr.icmp6_code",
		NULL, {"nd_rs_code"} },
	{ "nd_router_solicit nd_rs_cksum define", ALIAS, ICMP6_H,
		"nd_router_solicit", "nd_rs_hdr.icmp6_cksum",
		NULL, {"nd_rs_cksum"} },
	{ "nd_router_solicit nd_rs_reserved define", ALIAS, ICMP6_H,
		"nd_router_solicit", "nd_rs_hdr.icmp6_data32[0]",
		NULL, {"nd_rs_reserved"} },

	{ "nd_router_advert nd_ra_hdr", EXISTS, ICMP6_H, "nd_router_advert",
		"nd_ra_hdr", "0", {"sizeof(struct icmp6_hdr)"} },
	{ "nd_router_advert nd_ra_reachable", EXISTS, ICMP6_H,
		"nd_router_advert", "nd_ra_reachable",
		"sizeof(struct icmp6_hdr)", {"4"} },
	{ "nd_router_advert nd_ra_retransmit", EXISTS, ICMP6_H,
		"nd_router_advert", "nd_ra_retransmit",
		"sizeof(struct icmp6_hdr)+4", {"4"} },
	{ "nd_router_advert nd_ra_type define", ALIAS, ICMP6_H,
		"nd_router_advert", "nd_ra_hdr.icmp6_type",
		NULL, {"nd_ra_type"} },
	{ "nd_router_advert nd_ra_code define", ALIAS, ICMP6_H,
		"nd_router_advert", "nd_ra_hdr.icmp6_code",
		NULL, {"nd_ra_code"} },
	{ "nd_router_advert nd_ra_cksum define", ALIAS, ICMP6_H,
		"nd_router_advert", "nd_ra_hdr.icmp6_cksum",
		NULL, {"nd_ra_cksum"} },
	{ "nd_router_advert nd_ra_curhoplimit define", ALIAS, ICMP6_H,
		"nd_router_advert", "nd_ra_hdr.icmp6_data8[0]",
		NULL, {"nd_ra_curhoplimit"} },
	{ "nd_router_advert nd_ra_flags_reserved define", ALIAS, ICMP6_H,
		"nd_router_advert", "nd_ra_hdr.icmp6_data8[1]",
		NULL, {"nd_ra_flags_reserved"} },
	{ "ND_RA_FLAG_MANAGED", VALUE, ICMP6_H, "ND_RA_FLAG_MANAGED",
		NULL, NULL, {"0x80"} },
	{ "ND_RA_FLAG_OTHER", VALUE, ICMP6_H, "ND_RA_FLAG_OTHER",
		NULL, NULL, {"0x40"} },
	{ "nd_router_advert nd_ra_router_lifetime define", ALIAS, ICMP6_H,
		"nd_router_advert", "nd_ra_hdr.icmp6_data16[1]",
		NULL, {"nd_ra_router_lifetime"} },

	{ "nd_neighbor_solicit nd_ns_hdr", EXISTS, ICMP6_H,
		"nd_neighbor_solicit", "nd_ns_hdr",
		"0", {"sizeof(struct icmp6_hdr)"} },
	{ "nd_neighbor_solicit nd_ns_target", EXISTS, ICMP6_H,
		"nd_neighbor_solicit", "nd_ns_target",
		"sizeof(struct icmp6_hdr)",
		{"sizeof(struct in6_addr)"} },
	{ "nd_neighbor_solicit nd_ns_type define", ALIAS, ICMP6_H,
		"nd_neighbor_solicit", "nd_ns_hdr.icmp6_type",
		NULL, {"nd_ns_type"} },
	{ "nd_neighbor_solicit nd_ns_code define", ALIAS, ICMP6_H,
		"nd_neighbor_solicit", "nd_ns_hdr.icmp6_code",
		NULL, {"nd_ns_code"} },
	{ "nd_neighbor_solicit nd_ns_cksum define", ALIAS, ICMP6_H,
		"nd_neighbor_solicit", "nd_ns_hdr.icmp6_cksum",
		NULL, {"nd_ns_cksum"} },
	{ "nd_neighbor_solicit nd_ns_reserved define", ALIAS, ICMP6_H,
		"nd_neighbor_solicit", "nd_ns_hdr.icmp6_data32[0]",
		NULL, {"nd_ns_reserved"} },

	{ "nd_neighbor_advert nd_na_hdr", EXISTS, ICMP6_H,
		"nd_neighbor_advert", "nd_na_hdr",
		"0", {"sizeof(struct icmp6_hdr)"} },
	{ "nd_neighbor_advert nd_na_target", EXISTS, ICMP6_H,
		"nd_neighbor_advert", "nd_na_target",
		"sizeof(struct icmp6_hdr)",
		{"sizeof(struct in6_addr)"} },
	{ "nd_neighbor_advert nd_na_type define", ALIAS, ICMP6_H,
		"nd_neighbor_advert", "nd_na_hdr.icmp6_type",
		NULL, {"nd_na_type"} },
	{ "nd_neighbor_advert nd_na_code define", ALIAS, ICMP6_H,
		"nd_neighbor_advert", "nd_na_hdr.icmp6_code",
		NULL, {"nd_na_code"} },
	{ "nd_neighbor_advert nd_na_cksum define", ALIAS, ICMP6_H,
		"nd_neighbor_advert", "nd_na_hdr.icmp6_cksum",
		NULL, {"nd_na_cksum"} },
	{ "nd_neighbor_advert nd_na_flags_reserved define", ALIAS, ICMP6_H,
		"nd_neighbor_advert", "nd_na_hdr.icmp6_data32[0]",
		NULL, {"nd_na_flags_reserved"} },

	{ "ND_NA_FLAG_ROUTER", VALUE, ICMP6_H, "ND_NA_FLAG_ROUTER",
		NULL, NULL, {"htonl(0x80000000)"} },
	{ "ND_NA_FLAG_SOLICITED", VALUE, ICMP6_H, "ND_NA_FLAG_SOLICITED",
		NULL, NULL, {"htonl(0x40000000)"} },
	{ "ND_NA_FLAG_OVERRIDE", VALUE, ICMP6_H, "ND_NA_FLAG_OVERRIDE",
		NULL, NULL, {"htonl(0x20000000)"} },

	{ "nd_redirect nd_rd_hdr", EXISTS, ICMP6_H, "nd_redirect", "nd_rd_hdr",
		"0", {"sizeof(struct icmp6_hdr)"} },
	{ "nd_redirect nd_rd_target", EXISTS, ICMP6_H,
		"nd_redirect", "nd_rd_target", "sizeof(struct icmp6_hdr)",
		{"sizeof(struct in6_addr)"} },
	{ "nd_redirect nd_rd_dst", EXISTS, ICMP6_H, "nd_redirect",
		"nd_rd_dst", "sizeof(struct icmp6_hdr)+sizeof(struct in6_addr)",
		{"sizeof(struct in6_addr)"} },
	{ "nd_redirect nd_rd_type define", ALIAS, ICMP6_H,
		"nd_neighbor_advert", "nd_na_hdr.icmp6_data32[0]",
		NULL, {"nd_na_flags_reserved"} },

	{ "nd_opt_hdr nd_rd_hdr", EXISTS, ICMP6_H, "nd_opt_hdr", "nd_opt_type",
		"0", {"1"} },
	{ "nd_opt_hdr nd_rd_hdr", EXISTS, ICMP6_H, "nd_opt_hdr", "nd_opt_len",
		"1", {"1"} },
	{ "ND_OPT_SOURCE_LINKADDR", VALUE, ICMP6_H, "ND_OPT_SOURCE_LINKADDR",
		NULL, NULL, {"1"} },
	{ "ND_OPT_TARGET_LINKADDR", VALUE, ICMP6_H, "ND_OPT_TARGET_LINKADDR",
		NULL, NULL, {"2"} },
	{ "ND_OPT_PREFIX_INFORMATION", VALUE, ICMP6_H,
		"ND_OPT_PREFIX_INFORMATION", NULL, NULL, {"3"} },
	{ "ND_OPT_REDIRECTED_HEADER", VALUE, ICMP6_H,
		"ND_OPT_REDIRECTED_HEADER", NULL, NULL, {"4"} },
	{ "ND_OPT_MTU", VALUE, ICMP6_H, "ND_OPT_MTU", NULL, NULL, {"5"} },

	{ "nd_opt_prefix_info nd_opt_pi_type", EXISTS, ICMP6_H,
		"nd_opt_prefix_info", "nd_opt_pi_type",
		"0", {"1"} },
	{ "nd_opt_prefix_info nd_opt_pi_len", EXISTS, ICMP6_H,
		"nd_opt_prefix_info", "nd_opt_pi_len",
		"1", {"1"} },
	{ "nd_opt_prefix_info nd_opt_pi_prefix_len", EXISTS, ICMP6_H,
		"nd_opt_prefix_info", "nd_opt_pi_prefix_len",
		"2", {"1"} },
	{ "nd_opt_prefix_info nd_opt_pi_flags_reserved", EXISTS, ICMP6_H,
		"nd_opt_prefix_info", "nd_opt_pi_flags_reserved",
		"3", {"1"} },
	{ "nd_opt_prefix_info nd_opt_pi_valid_time", EXISTS, ICMP6_H,
		"nd_opt_prefix_info", "nd_opt_pi_valid_time",
		"4", {"4"} },
	{ "nd_opt_prefix_info nd_opt_pi_preferred_time", EXISTS, ICMP6_H,
		"nd_opt_prefix_info", "nd_opt_pi_preferred_time",
		"8", {"4"} },
	{ "nd_opt_prefix_info nd_opt_pi_reserved2", EXISTS, ICMP6_H,
		"nd_opt_prefix_info", "nd_opt_pi_reserved2",
		"12", {"4"} },
	{ "nd_opt_prefix_info nd_opt_pi_prefix", EXISTS, ICMP6_H,
		"nd_opt_prefix_info", "nd_opt_pi_prefix",
		"16", {"sizeof(struct in6_addr)"} },
	{ "ND_OPT_PI_FLAG_ONLINK", VALUE, ICMP6_H, "ND_OPT_PI_FLAG_ONLINK",
		NULL, NULL, {"0x80"} },
	{ "ND_OPT_PI_FLAG_AUTO", VALUE, ICMP6_H, "ND_OPT_PI_FLAG_AUTO",
		NULL, NULL, {"0x40"} },

	{ "nd_opt_rd_hdr nd_opt_rh_type", EXISTS, ICMP6_H, "nd_opt_rd_hdr",
		"nd_opt_rh_type", "0", {"1"} },
	{ "nd_opt_rd_hdr nd_opt_rh_len", EXISTS, ICMP6_H, "nd_opt_rd_hdr",
		"nd_opt_rh_len", "1", {"1"} },
	{ "nd_opt_rd_hdr nd_opt_rh_reserved1", EXISTS, ICMP6_H, "nd_opt_rd_hdr",
		"nd_opt_rh_reserved1", "2", {"2"} },
	{ "nd_opt_rd_hdr nd_opt_rh_reserved2", EXISTS, ICMP6_H, "nd_opt_rd_hdr",
		"nd_opt_rh_reserved2", "4", {"4"} },

	{ "nd_opt_mtu nd_opt_mtu_type", EXISTS, ICMP6_H, "nd_opt_mtu",
		"nd_opt_mtu_type", "0", {"1"} },
	{ "nd_opt_mtu nd_opt_mtu_len", EXISTS, ICMP6_H, "nd_opt_mtu",
		"nd_opt_mtu_len", "1", {"1"} },
	{ "nd_opt_mtu nd_opt_mtu_reserved", EXISTS, ICMP6_H, "nd_opt_mtu",
		"nd_opt_mtu_reserved", "2", {"2"} },
	{ "nd_opt_mtu nd_opt_mtu_mtu", EXISTS, ICMP6_H, "nd_opt_mtu",
		"nd_opt_mtu_mtu", "4", {"4"} },
};

#define FTCOUNT	(sizeof(ftab)/sizeof(ftab[0]))

int TST_TOTAL = FTCOUNT;

int
main(int argc, char *argv[])
{
	int	i, lc;
	char *msg;

	/* Parse standard options given to run the test. */
	msg = parse_opts(argc, argv, NULL, NULL);
	if (msg != NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
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

	tst_exit();
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
}