# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2009, Cisco Systems Inc.
# Ngie Cooper, July 2009

top_srcdir		?= ../../../..

include $(top_srcdir)/include/mk/env_pre.mk

INSTALL_TARGETS		:= check_envval get_ifname initialize_if set_ipv4addr \
			   add_ipv6addr check_icmpv4_connectivity \
			   check_icmpv6_connectivity check_netem check_setkey \
			   create_file find_portbundle killall_icmp_traffic \
			   killall_tcp_traffic killall_udp_traffic output_ipsec_conf \
			   ns-echoclient tst_net_stress.sh

FILTER_OUT_MAKE_TARGETS	:= ns-common

include $(top_srcdir)/include/mk/generic_leaf_target.mk

$(MAKE_TARGETS): %: ns-common.o
