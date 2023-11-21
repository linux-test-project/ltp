/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 SUSE LLC <mdoucha@suse.cz>
 */

#ifndef LAPI_NF_TABLES_H__
#define LAPI_NF_TABLES_H__

#include <linux/netfilter/nf_tables.h>

#ifndef HAVE_DECL_NFTA_CHAIN_ID
# define NFTA_CHAIN_ID 11
#endif

#ifndef HAVE_DECL_NFTA_VERDICT_CHAIN_ID
# define NFTA_VERDICT_CHAIN_ID 3
#endif

#endif /* LAPI_NF_TABLES_H__ */
