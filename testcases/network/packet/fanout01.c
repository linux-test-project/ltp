// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Richard Palethorpe <rpalethorpe@suse.com>
 * Original reproducer: https://blogs.securiteam.com/index.php/archives/3484
 * Other copyrights may apply.
 *
 * CVE-2017-15649
 *
 * Fixed by the following commits:
 * 4971613c "packet: in packet_do_bind, test fanout with bind_lock held"
 * 008ba2a1 "packet: hold bind lock when rebinding to fanout hook"
 *
 * See blogpost in copyright notice for more details.
 */
#include <errno.h>
#include <sys/types.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <string.h>

#include "tst_test.h"
#include "tst_fuzzy_sync.h"
#include "lapi/if_packet.h"

static struct tst_fzsync_pair pair;
static int fd;
static struct sockaddr_ll addr;

void setup(void)
{
	tst_setup_netns();
	tst_fzsync_pair_init(&pair);
}

void cleanup(void)
{
	tst_fzsync_pair_cleanup(&pair);
}

void *binder(void *unused)
{
	while (tst_fzsync_run_b(&pair)) {
		tst_fzsync_start_race_b(&pair);
		bind(fd, (struct sockaddr *)&addr, sizeof(addr));
		tst_fzsync_end_race_b(&pair);
	}

	return unused;
}

void run(void)
{
	int fanout_val = PACKET_FANOUT_ROLLOVER, index;
	struct ifreq ifr;

	memset(&ifr, 0, sizeof(struct ifreq));

	tst_fzsync_pair_reset(&pair, binder);
	while (tst_fzsync_run_a(&pair)) {
		fd = SAFE_SOCKET(AF_PACKET, SOCK_RAW, PF_PACKET);

		strcpy((char *)&ifr.ifr_name, "lo");
		SAFE_IOCTL(fd, SIOCGIFINDEX, &ifr);
		index = ifr.ifr_ifindex;

		SAFE_IOCTL(fd, SIOCGIFFLAGS, &ifr);
		ifr.ifr_flags &= ~(short)IFF_UP;
		SAFE_IOCTL(fd, SIOCSIFFLAGS, &ifr);

		addr.sll_family = AF_PACKET;
		/* need something different to rehook && 0 to skip register_prot_hook */
		addr.sll_protocol = 0x0;
		addr.sll_ifindex = index;

		tst_fzsync_start_race_a(&pair);
		setsockopt(fd, SOL_PACKET, PACKET_FANOUT,
			   &fanout_val, sizeof(fanout_val));
		tst_fzsync_end_race_a(&pair);

		/* UAF */
		close(fd);
	}

	tst_res(TPASS, "Nothing bad happened, probably...");
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.cleanup = cleanup,
	.needs_root = 1,
	.max_runtime = 180,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_USER_NS=y",
		"CONFIG_NET_NS=y",
		NULL
	},
	.save_restore = (const struct tst_path_val[]) {
		{"/proc/sys/user/max_user_namespaces", "1024", TST_SR_SKIP},
		{}
	},
	.tags = (const struct tst_tag[]) {
		{"CVE", "2017-15649"},
		{"linux-git", "4971613c1639"},
		{"linux-git", "008ba2a13f2d"},
		{}
	}
};
