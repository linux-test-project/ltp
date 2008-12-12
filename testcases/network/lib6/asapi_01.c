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
		"ip6_ctlun.ip6_un1.ip6_un1_flow", "0", {"4"} },
	{ "ip6_hdr ip6_flow", ALIAS, IP6_H, "ip6_hdr",
		"ip6_ctlun.ip6_un1.ip6_un1_flow", NULL, {"ip6_flow"} },
	{ "ip6_hdr un1_plen", EXISTS, IP6_H, "ip6_hdr",
		"ip6_ctlun.ip6_un1.ip6_un1_plen", "4", {"2"} },
	{ "ip6_hdr ip6_plen", ALIAS, IP6_H, "ip6_hdr",
		"ip6_ctlun.ip6_un1.ip6_un1_plen", "4", {"ip6_plen"} },
	{ "ip6_hdr un1_nxt", EXISTS, IP6_H, "ip6_hdr",
		"ip6_ctlun.ip6_un1.ip6_un1_nxt", "6", {"1"} },
	{ "ip6_hdr ip6_nxt", ALIAS, IP6_H, "ip6_hdr",
		"ip6_ctlun.ip6_un1.ip6_un1_nxt", NULL, {"ip6_nxt"} },
	{ "ip6_hdr un1_hlim", EXISTS, IP6_H, "ip6_hdr",
		"ip6_ctlun.ip6_un1.ip6_un1_hlim", "7", {"1"} },
	{ "ip6_hdr ip6_hlim", ALIAS, IP6_H, "ip6_hdr",
		"ip6_ctlun.ip6_un1.ip6_un1_hlim", NULL, {"ip6_hlim"} },
	{ "ip6_hdr un1_vfc", EXISTS, IP6_H, "ip6_hdr",
		"ip6_ctlun.ip6_un2_vfc", "0",
		{"1"} },
	{ "ip6_hdr ip6_src", EXISTS, IP6_H, "ip6_hdr", "ip6_src",
		"sizeof(struct ip6_hdrctl)",
		{"sizeof(struct in6_addr)"} },
	{ "ip6_hdr ip6_dst", EXISTS, IP6_H, "ip6_hdr", "ip6_dst",
		"(sizeof(struct ip6_hdrctl)+sizeof(struct in6_addr))",
		{"sizeof(struct in6_addr)"} },
/* section 2.2 structure and field definitions */
	{ "IPPROTO_HOPOPTS", VALUE, IN_H, "IPPROTO_HOPOPTS",
		NULL, NULL, {"0"} },
	{ "IPPROTO_IPV6", VALUE, IN_H, "IPPROTO_IPV6",
		NULL, NULL, {"41"} },
	{ "IPPROTO_ROUTING", VALUE, IN_H, "IPPROTO_ROUTING",
		NULL, NULL, {"43"} },
	{ "IPPROTO_FRAGMENT", VALUE, IN_H, "IPPROTO_FRAGMENT",
		NULL, NULL, {"44"} },
	{ "IPPROTO_ESP", VALUE, IN_H, "IPPROTO_ESP",
		NULL, NULL, {"50"} },
	{ "IPPROTO_AH", VALUE, IN_H, "IPPROTO_AH",
		NULL, NULL, {"51"} },
	{ "IPPROTO_ICMPV6", VALUE, IN_H, "IPPROTO_ICMPV6",
		NULL, NULL, {"58"} },
	{ "IPPROTO_NONE", VALUE, IN_H, "IPPROTO_NONE",
		NULL, NULL, {"59"} },
	{ "IPPROTO_DSTOPTS", VALUE, IN_H, "IPPROTO_DSTOPTS",
		NULL, NULL, {"60"} },
/* ip6_hbh */
	{ "ip6_hbh ip6h_nxt", EXISTS, IP6_H, "ip6_hbh",
		"ip6h_nxt", "0", {"1"} },
	{ "ip6_hbh ip6h_nxt", EXISTS, IP6_H, "ip6_hbh",
		"ip6h_len", "1", {"1"} },
/* ip6_dest */
	{ "ip6_dest ip6d_nxt", EXISTS, IP6_H, "ip6_dest",
		"ip6d_nxt", "0", {"1"} },
	{ "ip6_dest ip6d_nxt", EXISTS, IP6_H, "ip6_dest",
		"ip6d_len", "1", {"1"} },
/* ip6_rthdr0 */
	{ "ip6_rthdr0 ip6r0_nxt", EXISTS, IP6_H, "ip6_rthdr0",
		"ip6r0_nxt", "0", {"1"} },
	{ "ip6_rthdr0 ip6r0_len", EXISTS, IP6_H, "ip6_rthdr0",
		"ip6r0_len", "1", {"1"} },
	{ "ip6_rthdr0 ip6r0_type", EXISTS, IP6_H, "ip6_rthdr0",
		"ip6r0_type", "2", {"1"} },
	{ "ip6_rthdr0 ip6r0_segleft", EXISTS, IP6_H, "ip6_rthdr0",
		"ip6r0_segleft", "3", {"1"} },
	{ "ip6_rthdr0 ip6r0_reserved", EXISTS, IP6_H, "ip6_rthdr0",
		"ip6r0_reserved", "4", {"1"} },
/* ip6_frag */
	{ "ip6_frag ip6f_nxt", EXISTS, IP6_H, "ip6_frag",
		"ip6f_nxt", "0", {"1"} },
	{ "ip6_frag ip6f_reserved", EXISTS, IP6_H, "ip6_frag",
		"ip6f_reserved", "1", {"1"} },
	{ "ip6_frag ip6f_offlg", EXISTS, IP6_H, "ip6_frag",
		"ip6f_offlg", "2", {"2"} },
	{ "ip6_frag ip6f_ident", EXISTS, IP6_H, "ip6_frag",
		"ip6f_ident", "4", {"4"} },

	{ "IP6F_OFF_MASK", VALUE, IP6_H, "IP6F_OFF_MASK",
		NULL, NULL, {"htons(0xfff8)"} },
	{ "IP6F_RESERVED_MASK", VALUE, IP6_H, "IP6F_RESERVED_MASK",
		NULL, NULL, {"htons(0x0006)"} },
	{ "IP6F_MORE_FRAG", VALUE, IP6_H, "IP6F_MORE_FRAG",
		NULL, NULL, {"htons(0x0001)"} },
	{ "IP6OPT_TYPE", VALUE, IP6_H, "IP6OPT_TYPE(0xff)",
		NULL, NULL, {"0xc0"} },
	{ "IP6OPT_TYPE_SKIP", VALUE, IP6_H, "IP6OPT_TYPE_SKIP",
		NULL, NULL, {"0x00"} },
	{ "IP6OPT_TYPE_DISCARD", VALUE, IP6_H, "IP6OPT_TYPE_DISCARD",
		NULL, NULL, {"0x40"} },
	{ "IP6OPT_TYPE_FORCEICMP", VALUE, IP6_H, "IP6OPT_TYPE_FORCEICMP",
		NULL, NULL, {"0x80"} },
	{ "IP6OPT_TYPE_ICMP", VALUE, IP6_H, "IP6OPT_TYPE_ICMP",
		NULL, NULL, {"0xc0"} },
	{ "IP6OPT_TYPE_MUTABLE", VALUE, IP6_H, "IP6OPT_TYPE_MUTABLE",
		NULL, NULL, {"0x20"} },
	{ "IP6OPT_PAD1", VALUE, IP6_H, "IP6OPT_PAD1",
		NULL, NULL, {"0x00"} },
	{ "IP6OPT_PADN", VALUE, IP6_H, "IP6OPT_PADN",
		NULL, NULL, {"0x01"} },
	{ "IP6OPT_JUMBO", VALUE, IP6_H, "IP6OPT_JUMBO",
		NULL, NULL, {"0xc2"} },
	{ "IP6OPT_NSAP_ADDR", VALUE, IP6_H, "IP6OPT_NSAP_ADDR",
		NULL, NULL, {"0xc3"} },
	{ "IP6OPT_TUNNEL_LIMIT", VALUE, IP6_H, "IP6OPT_TUNNEL_LIMIT",
		NULL, NULL, {"0x04"} },
	{ "IP6OPT_ROUTER_ALERT", VALUE, IP6_H, "IP6OPT_ROUTER_ALERT",
		NULL, NULL, {"0x05"} },
/* ip6_opt_jumbo */
	{ "ip6_opt_jumbo ip6oj_type", EXISTS, IP6_H, "ip6_opt_jumbo",
		"ip6oj_type", "0", {"1"} },
	{ "ip6_opt_jumbo ip6oj_len", EXISTS, IP6_H, "ip6_opt_jumbo",
		"ip6oj_len", "1", {"1"} },
	{ "ip6_opt_jumbo ip6oj_jumbo_len element", EXISTS, IP6_H,
		"ip6_opt_jumbo", "ip6oj_jumbo_len[0]", "2", {"1"} },
	{ "ip6_opt_jumbo ip6oj_jumbo_len array", EXISTS, IP6_H,
		"ip6_opt_jumbo", "ip6oj_jumbo_len", "2", {"4"} },
/* ip6_opt_nsap */
	{ "ip6_opt_nsap ip6on_type", EXISTS, IP6_H, "ip6_opt_nsap",
		"ip6on_type", "0", {"1"} },
	{ "ip6_opt_nsap ip6on_len", EXISTS, IP6_H, "ip6_opt_nsap",
		"ip6on_len", "1", {"1"} },
	{ "ip6_opt_nsap ip6on_src_nsap_len", EXISTS, IP6_H, "ip6_opt_nsap",
		"ip6on_src_nsap_len", "2", {"1"} },
	{ "ip6_opt_nsap ip6on_dst_nsap_len", EXISTS, IP6_H, "ip6_opt_nsap",
		"ip6on_dst_nsap_len", "3", {"1"} },
/* ip6_opt_tunnel */
	{ "ip6_opt_tunnel ip6ot_type", EXISTS, IP6_H, "ip6_opt_tunnel",
		"ip6ot_type", "0", {"1"} },
	{ "ip6_opt_tunnel ip6ot_len", EXISTS, IP6_H, "ip6_opt_tunnel",
		"ip6ot_len", "1", {"1"} },
	{ "ip6_opt_tunnel ip6ot_encap_limit", EXISTS, IP6_H, "ip6_opt_tunnel",
		"ip6ot_encap_limit", "2", {"1"} },
/* ip6_opt_router */
	{ "ip6_opt_router ip6or_type", EXISTS, IP6_H, "ip6_opt_router",
		"ip6or_type", "0", {"1"} },
	{ "ip6_opt_router ip6or_len", EXISTS, IP6_H, "ip6_opt_router",
		"ip6or_len", "1", {"1"} },
	{ "ip6_opt_router ip6or_value element", EXISTS, IP6_H,
		"ip6_opt_router", "ip6or_value[0]", "2", {"1"} },
	{ "ip6_opt_router ip6or_value array", EXISTS, IP6_H, "ip6_opt_router",
		"ip6or_value", "2", {"2"} },
/* IP6_ALERT_* definitions */
	{ "IP6_ALERT_MLD", VALUE, IP6_H, "IP6_ALERT_MLD",
		NULL, NULL, {"0"} },
	{ "IP6_ALERT_RSVP", VALUE, IP6_H, "IP6_ALERT_RSVP",
		NULL, NULL, {"htons(1)"} },
	{ "IP6_ALERT_AN", VALUE, IP6_H, "IP6_ALERT_AN",
		NULL, NULL, {"htons(2)"} },
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

