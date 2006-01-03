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
 * Test Name: asapi_01
 *
 * Test Description:
 *  These tests are for the "Advanced Sockets API" (RFC 3542)
 *  Verify that in6 and sockaddr fields are present.
 *
 * Usage:  <for command-line>
 *  asapi_01
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

char *TCID="asapi_01";		/* Test program identifier.    */

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
/* section 2.1 structure & field definitions */
	{ "ip6_hdr un1_flow", EXISTS, IP6_H, "ip6_hdr",
		"ip6_ctlun.ip6_un1.ip6_un1_flow", "0", .ft_value="4" },
	{ "ip6_hdr ip6_flow", ALIAS, IP6_H, "ip6_hdr",
		"ip6_ctlun.ip6_un1.ip6_un1_flow", 0, .ft_dname="ip6_flow" },
	{ "ip6_hdr un1_plen", EXISTS, IP6_H, "ip6_hdr",
		"ip6_ctlun.ip6_un1.ip6_un1_plen", "4", .ft_value="2" },
	{ "ip6_hdr ip6_plen", ALIAS, IP6_H, "ip6_hdr",
		"ip6_ctlun.ip6_un1.ip6_un1_plen", "4", .ft_dname="ip6_plen" },
	{ "ip6_hdr un1_nxt", EXISTS, IP6_H, "ip6_hdr",
		"ip6_ctlun.ip6_un1.ip6_un1_nxt", "6", .ft_value="1" },
	{ "ip6_hdr ip6_nxt", ALIAS, IP6_H, "ip6_hdr",
		"ip6_ctlun.ip6_un1.ip6_un1_nxt", 0, .ft_dname="ip6_nxt" },
	{ "ip6_hdr un1_hlim", EXISTS, IP6_H, "ip6_hdr",
		"ip6_ctlun.ip6_un1.ip6_un1_hlim", "7", .ft_value="1" },
	{ "ip6_hdr ip6_hlim", ALIAS, IP6_H, "ip6_hdr",
		"ip6_ctlun.ip6_un1.ip6_un1_hlim", 0, .ft_dname="ip6_hlim" },
	{ "ip6_hdr un1_vfc", EXISTS, IP6_H, "ip6_hdr",
		"ip6_ctlun.ip6_un2_vfc", "0",
		.ft_value="1" },
	{ "ip6_hdr ip6_src", EXISTS, IP6_H, "ip6_hdr", "ip6_src",
		"sizeof(struct ip6_hdrctl)",
		.ft_value="sizeof(struct in6_addr)" },
	{ "ip6_hdr ip6_dst", EXISTS, IP6_H, "ip6_hdr", "ip6_dst",
		"(sizeof(struct ip6_hdrctl)+sizeof(struct in6_addr))",
		.ft_value="sizeof(struct in6_addr)" },
/* section 2.2 structure and field definitions */
	{ "IPPROTO_HOPOPTS", VALUE, IN_H, "IPPROTO_HOPOPTS",
		0, 0, .ft_dname="0" },
	{ "IPPROTO_IPV6", VALUE, IN_H, "IPPROTO_IPV6",
		0, 0, .ft_dname="41" },
	{ "IPPROTO_ROUTING", VALUE, IN_H, "IPPROTO_ROUTING",
		0, 0, .ft_dname="43" },
	{ "IPPROTO_FRAGMENT", VALUE, IN_H, "IPPROTO_FRAGMENT",
		0, 0, .ft_dname="44" },
	{ "IPPROTO_ESP", VALUE, IN_H, "IPPROTO_ESP",
		0, 0, .ft_dname="50" },
	{ "IPPROTO_AH", VALUE, IN_H, "IPPROTO_AH",
		0, 0, .ft_dname="51" },
	{ "IPPROTO_ICMPV6", VALUE, IN_H, "IPPROTO_ICMPV6",
		0, 0, .ft_dname="58" },
	{ "IPPROTO_NONE", VALUE, IN_H, "IPPROTO_NONE",
		0, 0, .ft_dname="59" },
	{ "IPPROTO_DSTOPTS", VALUE, IN_H, "IPPROTO_DSTOPTS",
		0, 0, .ft_dname="60" },
/* ip6_hbh */
	{ "ip6_hbh ip6h_nxt", EXISTS, IP6_H, "ip6_hbh",
		"ip6h_nxt", "0", .ft_value="1" },
	{ "ip6_hbh ip6h_nxt", EXISTS, IP6_H, "ip6_hbh",
		"ip6h_len", "1", .ft_value="1" },
/* ip6_dest */
	{ "ip6_dest ip6d_nxt", EXISTS, IP6_H, "ip6_dest",
		"ip6d_nxt", "0", .ft_value="1" },
	{ "ip6_dest ip6d_nxt", EXISTS, IP6_H, "ip6_dest",
		"ip6d_len", "1", .ft_value="1" },
/* ip6_rthdr0 */
	{ "ip6_rthdr0 ip6r0_nxt", EXISTS, IP6_H, "ip6_rthdr0",
		"ip6r0_nxt", "0", .ft_value="1" },
	{ "ip6_rthdr0 ip6r0_len", EXISTS, IP6_H, "ip6_rthdr0",
		"ip6r0_len", "1", .ft_value="1" },
	{ "ip6_rthdr0 ip6r0_type", EXISTS, IP6_H, "ip6_rthdr0",
		"ip6r0_type", "2", .ft_value="1" },
	{ "ip6_rthdr0 ip6r0_segleft", EXISTS, IP6_H, "ip6_rthdr0",
		"ip6r0_segleft", "3", .ft_value="1" },
	{ "ip6_rthdr0 ip6r0_reserved", EXISTS, IP6_H, "ip6_rthdr0",
		"ip6r0_reserved", "4", .ft_value="1" },
/* ip6_frag */
	{ "ip6_frag ip6f_nxt", EXISTS, IP6_H, "ip6_frag",
		"ip6f_nxt", "0", .ft_value="1" },
	{ "ip6_frag ip6f_reserved", EXISTS, IP6_H, "ip6_frag",
		"ip6f_reserved", "1", .ft_value="1" },
	{ "ip6_frag ip6f_offlg", EXISTS, IP6_H, "ip6_frag",
		"ip6f_offlg", "2", .ft_value="2" },
	{ "ip6_frag ip6f_ident", EXISTS, IP6_H, "ip6_frag",
		"ip6f_ident", "4", .ft_value="4" },

	{ "IP6F_OFF_MASK", VALUE, IP6_H, "IP6F_OFF_MASK",
		0, .ft_dname="htons(0xfff8)" },
	{ "IP6F_RESERVED_MASK", VALUE, IP6_H, "IP6F_RESERVED_MASK",
		0, .ft_dname="htons(0x0006)" },
	{ "IP6F_MORE_FRAG", VALUE, IP6_H, "IP6F_MORE_FRAG",
		0, .ft_dname="htons(0x0001)" },
	{ "IP6OPT_TYPE", VALUE, IP6_H, "IP6OPT_TYPE(0xff)",
		0, .ft_dname="0xc0" },
	{ "IP6OPT_TYPE_SKIP", VALUE, IP6_H, "IP6OPT_TYPE_SKIP",
		0, .ft_dname="0x00" },
	{ "IP6OPT_TYPE_DISCARD", VALUE, IP6_H, "IP6OPT_TYPE_DISCARD",
		0, .ft_dname="0x40" },
	{ "IP6OPT_TYPE_FORCEICMP", VALUE, IP6_H, "IP6OPT_TYPE_FORCEICMP",
		0, .ft_dname="0x80" },
	{ "IP6OPT_TYPE_ICMP", VALUE, IP6_H, "IP6OPT_TYPE_ICMP",
		0, .ft_dname="0xc0" },
	{ "IP6OPT_TYPE_MUTABLE", VALUE, IP6_H, "IP6OPT_TYPE_MUTABLE",
		0, .ft_dname="0x20" },
	{ "IP6OPT_PAD1", VALUE, IP6_H, "IP6OPT_PAD1",
		0, .ft_dname="0x00" },
	{ "IP6OPT_PADN", VALUE, IP6_H, "IP6OPT_PADN",
		0, .ft_dname="0x01" },
	{ "IP6OPT_JUMBO", VALUE, IP6_H, "IP6OPT_JUMBO",
		0, .ft_dname="0xc2" },
	{ "IP6OPT_NSAP_ADDR", VALUE, IP6_H, "IP6OPT_NSAP_ADDR",
		0, .ft_dname="0xc3" },
	{ "IP6OPT_TUNNEL_LIMIT", VALUE, IP6_H, "IP6OPT_TUNNEL_LIMIT",
		0, .ft_dname="0x04" },
	{ "IP6OPT_ROUTER_ALERT", VALUE, IP6_H, "IP6OPT_ROUTER_ALERT",
		0, .ft_dname="0x05" },
/* ip6_opt_jumbo */
	{ "ip6_opt_jumbo ip6oj_type", EXISTS, IP6_H, "ip6_opt_jumbo",
		"ip6oj_type", "0", .ft_value="1" },
	{ "ip6_opt_jumbo ip6oj_len", EXISTS, IP6_H, "ip6_opt_jumbo",
		"ip6oj_len", "1", .ft_value="1" },
	{ "ip6_opt_jumbo ip6oj_jumbo_len element", EXISTS, IP6_H,
		"ip6_opt_jumbo", "ip6oj_jumbo_len[0]", "2", .ft_value="1" },
	{ "ip6_opt_jumbo ip6oj_jumbo_len array", EXISTS, IP6_H,
		"ip6_opt_jumbo", "ip6oj_jumbo_len", "2", .ft_value="4" },
/* ip6_opt_nsap */
	{ "ip6_opt_nsap ip6on_type", EXISTS, IP6_H, "ip6_opt_nsap",
		"ip6on_type", "0", .ft_value="1" },
	{ "ip6_opt_nsap ip6on_len", EXISTS, IP6_H, "ip6_opt_nsap",
		"ip6on_len", "1", .ft_value="1" },
	{ "ip6_opt_nsap ip6on_src_nsap_len", EXISTS, IP6_H, "ip6_opt_nsap",
		"ip6on_src_nsap_len", "2", .ft_value="1" },
	{ "ip6_opt_nsap ip6on_dst_nsap_len", EXISTS, IP6_H, "ip6_opt_nsap",
		"ip6on_dst_nsap_len", "3", .ft_value="1" },
/* ip6_opt_tunnel */
	{ "ip6_opt_tunnel ip6ot_type", EXISTS, IP6_H, "ip6_opt_tunnel",
		"ip6ot_type", "0", .ft_value="1" },
	{ "ip6_opt_tunnel ip6ot_len", EXISTS, IP6_H, "ip6_opt_tunnel",
		"ip6ot_len", "1", .ft_value="1" },
	{ "ip6_opt_tunnel ip6ot_encap_limit", EXISTS, IP6_H, "ip6_opt_tunnel",
		"ip6ot_encap_limit", "2", .ft_value="1" },
/* ip6_opt_router */
	{ "ip6_opt_router ip6or_type", EXISTS, IP6_H, "ip6_opt_router",
		"ip6or_type", "0", .ft_value="1" },
	{ "ip6_opt_router ip6or_len", EXISTS, IP6_H, "ip6_opt_router",
		"ip6or_len", "1", .ft_value="1" },
	{ "ip6_opt_router ip6or_value element", EXISTS, IP6_H,
		"ip6_opt_router", "ip6or_value[0]", "2", .ft_value="1" },
	{ "ip6_opt_router ip6or_value array", EXISTS, IP6_H, "ip6_opt_router",
		"ip6or_value", "2", .ft_value="2" },
/* IP6_ALERT_* definitions */
	{ "IP6_ALERT_MLD", VALUE, IP6_H, "IP6_ALERT_MLD",
		0, 0, .ft_dname="0" },
	{ "IP6_ALERT_RSVP", VALUE, IP6_H, "IP6_ALERT_RSVP",
		0, 0, .ft_dname="htons(1)" },
	{ "IP6_ALERT_AN", VALUE, IP6_H, "IP6_ALERT_AN",
		0, 0, .ft_dname="htons(2)" },
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

