// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017-2019 Petr Vorel <pvorel@suse.cz>
 */

#include <arpa/inet.h>

void tst_get_in_addr(const char *ip_str, struct in_addr *ip);
void tst_get_in6_addr(const char *ip_str, struct in6_addr *ip6);
