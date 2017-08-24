/*
 * Copyright (c) 2017 Petr Vorel <pvorel@suse.cz>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"

#include "tst_net.h"

#define DEFAULT_IPV4_PREFIX 24
#define DEFAULT_IPV6_PREFIX 64

static void usage(const char *cmd)
{
	fprintf(stderr, "USAGE:\n"
		"%s IP_LHOST[/PREFIX]\n"
		"%s -r IP_RHOST[/PREFIX]\n"
		"%s -h\n\n"
		"Set IP variables without prefix and prefix for given IP.\n"
		"EXPORTED VARIABLES:\n"
		"Export one of the following variables without /prefix:\n"
		"IPV4_LHOST: IPv4 address of the local host\n"
		"IPV4_RHOST: IPv4 address of the remote host\n"
		"IPV6_LHOST: IPv6 address of the local host\n"
		"IPV6_RHOST: IPv6 address of the remote host\n"
		"Export one of the following variables:\n"
		"IPV4_LPREFIX: IPv4 prefix for IPV4_LNETWORK\n"
		"IPV4_RPREFIX: IPv4 prefix for IPV4_RNETWORK\n"
		"IPV6_LPREFIX: IPv6 prefix for IPV6_LNETWORK\n"
		"IPV6_RPREFIX: IPv6 prefix for IPV6_RNETWORK\n"
		"Default IPv4 prefix: %d.\n"
		"Default IPv6 prefix: %d.\n\n"
		"PARAMS:\n"
		"-h this help\n"
		"-r export remote environment variables\n",
		cmd, cmd, cmd, DEFAULT_IPV4_PREFIX, DEFAULT_IPV6_PREFIX);
}

static void print_ivar(const char *name, unsigned int val)
{
	printf("export %s=%d\n", name, val);
}

int main(int argc, char *argv[])
{
	char *ip_str = NULL, *prefix_str = NULL;
	int is_ipv6, is_rhost = 0, prefix;
	struct in_addr ip;
	struct in6_addr ip6;

	int is_usage = argc > 1 && (!strcmp(argv[1], "-h") ||
		!strcmp(argv[1], "--help"));
	if (argc < 2 || is_usage) {
		usage(argv[0]);
		exit(is_usage ? EXIT_SUCCESS : EXIT_FAILURE);
	}
	if (!strcmp(argv[1], "-r"))
		is_rhost = 1;

	ip_str = argv[is_rhost ? 2 : 1];
	is_ipv6 = !!strchr(ip_str, ':');

	prefix_str = strchr(ip_str, '/');

	if (prefix_str)
		prefix = get_prefix(ip_str, is_ipv6);
	else
		prefix = is_ipv6 ? DEFAULT_IPV6_PREFIX : DEFAULT_IPV4_PREFIX;

	/* checks for validity of IP string */
	if (is_ipv6)
		get_in6_addr(ip_str, &ip6);
	else
		get_in_addr(ip_str, &ip);

	if (is_ipv6) {
		print_ivar(is_rhost ? "IPV6_RPREFIX" : "IPV6_LPREFIX", prefix);
		print_svar(is_rhost ? "IPV6_RHOST" : "IPV6_LHOST", ip_str);
	} else {
		print_ivar(is_rhost ? "IPV4_RPREFIX" : "IPV4_LPREFIX", prefix);
		print_svar(is_rhost ? "IPV4_RHOST" : "IPV4_LHOST", ip_str);
	}

	exit(EXIT_SUCCESS);
}
