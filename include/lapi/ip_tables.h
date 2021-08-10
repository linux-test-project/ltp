// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef LAPI_IP_TABLES__
#define LAPI_IP_TABLES__

#include "config.h"

#include <net/if.h>
#include <linux/netfilter_ipv4/ip_tables.h>

#ifndef HAVE_STRUCT_XT_ENTRY_MATCH
struct xt_entry_match {
	union {
		struct {
			uint16_t match_size;
			char name[29];
			uint8_t revision;
		} user;
		struct {
			uint16_t match_size;
			void *match;
		} kernel;
		uint16_t match_size;
	} u;
	unsigned char data[0];
};
#endif

#ifndef HAVE_STRUCT_XT_ENTRY_TARGET
struct xt_entry_target {
	union {
		struct {
			uint16_t target_size;
			char name[29];
			uint8_t revision;
		} user;
		struct {
			uint16_t target_size;
			void *target;
		} kernel;
		uint16_t target_size;
	} u;
	unsigned char data[0];
};
#endif

#endif /* LAPI_IP_TABLES__ */
