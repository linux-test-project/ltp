# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2009, Cisco Systems Inc.
# Ngie Cooper, August 2009

top_srcdir		?= ../..

LTPLIBS = ujson
tst_run_shell: LTPLDLIBS = -lujson

include $(top_srcdir)/include/mk/testcases.mk

INSTALL_TARGETS		:= *.sh

MAKE_TARGETS		:= tst_sleep tst_random tst_checkpoint tst_rod tst_kvcmp\
			   tst_device tst_net_iface_prefix tst_net_ip_prefix tst_net_vars\
			   tst_getconf tst_supported_fs tst_check_drivers tst_get_unused_port\
			   tst_get_median tst_hexdump tst_get_free_pids tst_timeout_kill\
			   tst_check_kconfigs tst_cgctl tst_fsfreeze tst_ns_create tst_ns_exec\
			   tst_ns_ifmove tst_lockdown_enabled tst_secureboot_enabled tst_res_\
			   tst_run_shell tst_remaining_runtime tst_runas

include $(top_srcdir)/include/mk/generic_trunk_target.mk
