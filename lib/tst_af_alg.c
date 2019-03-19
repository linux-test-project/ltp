// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2019 Google LLC
 */

#include <errno.h>
#include <stdlib.h>

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tst_af_alg.h"
#include "lapi/socket.h"

int tst_alg_create(void)
{
	TEST(socket(AF_ALG, SOCK_SEQPACKET, 0));
	if (TST_RET >= 0)
		return TST_RET;
	if (TST_ERR == EAFNOSUPPORT)
		tst_brk(TCONF, "kernel doesn't support AF_ALG");
	tst_brk(TBROK | TTERRNO, "unexpected error creating AF_ALG socket");
	return -1;
}

void tst_alg_bind_addr(int algfd, const struct sockaddr_alg *addr)
{
	TEST(bind(algfd, (const struct sockaddr *)addr, sizeof(*addr)));
	if (TST_RET == 0)
		return;
	if (TST_ERR == ENOENT) {
		tst_brk(TCONF, "kernel doesn't support %s algorithm '%s'",
			addr->salg_type, addr->salg_name);
	}
	tst_brk(TBROK | TTERRNO,
		"unexpected error binding AF_ALG socket to %s algorithm '%s'",
		addr->salg_type, addr->salg_name);
}

static void init_sockaddr_alg(struct sockaddr_alg *addr,
			      const char *algtype, const char *algname)
{
	memset(addr, 0, sizeof(*addr));

	addr->salg_family = AF_ALG;

	strncpy((char *)addr->salg_type, algtype, sizeof(addr->salg_type));
	if (addr->salg_type[sizeof(addr->salg_type) - 1] != '\0')
		tst_brk(TBROK, "algorithm type too long: '%s'", algtype);

	strncpy((char *)addr->salg_name, algname, sizeof(addr->salg_name));
	if (addr->salg_name[sizeof(addr->salg_name) - 1] != '\0')
		tst_brk(TBROK, "algorithm name too long: '%s'", algname);
}

void tst_alg_bind(int algfd, const char *algtype, const char *algname)
{
	struct sockaddr_alg addr;

	init_sockaddr_alg(&addr, algtype, algname);

	tst_alg_bind_addr(algfd, &addr);
}

bool tst_have_alg(const char *algtype, const char *algname)
{
	int algfd;
	struct sockaddr_alg addr;
	bool have_alg = true;

	algfd = tst_alg_create();

	init_sockaddr_alg(&addr, algtype, algname);

	TEST(bind(algfd, (const struct sockaddr *)&addr, sizeof(addr)));
	if (TST_RET != 0) {
		if (TST_ERR != ENOENT) {
			tst_brk(TBROK | TTERRNO,
				"unexpected error binding AF_ALG socket to %s algorithm '%s'",
				algtype, algname);
		}
		have_alg = false;
	}

	close(algfd);
	return have_alg;
}

void tst_require_alg(const char *algtype, const char *algname)
{
	int algfd = tst_alg_create();

	tst_alg_bind(algfd, algtype, algname);

	close(algfd);
}

void tst_alg_setkey(int algfd, const uint8_t *key, unsigned int keylen)
{
	uint8_t *keybuf = NULL;
	unsigned int i;

	if (key == NULL) {
		/* generate a random key */
		keybuf = SAFE_MALLOC(keylen);
		for (i = 0; i < keylen; i++)
			keybuf[i] = rand();
		key = keybuf;
	}
	TEST(setsockopt(algfd, SOL_ALG, ALG_SET_KEY, key, keylen));
	if (TST_RET != 0) {
		tst_brk(TBROK | TTERRNO,
			"unexpected error setting key (len=%u)", keylen);
	}
	free(keybuf);
}

int tst_alg_accept(int algfd)
{
	TEST(accept(algfd, NULL, NULL));
	if (TST_RET < 0) {
		tst_brk(TBROK | TTERRNO,
			"unexpected error accept()ing AF_ALG request socket");
	}
	return TST_RET;
}

int tst_alg_setup(const char *algtype, const char *algname,
		  const uint8_t *key, unsigned int keylen)
{
	int algfd = tst_alg_create();

	tst_alg_bind(algfd, algtype, algname);

	if (keylen != 0)
		tst_alg_setkey(algfd, key, keylen);

	return algfd;
}

int tst_alg_setup_reqfd(const char *algtype, const char *algname,
			const uint8_t *key, unsigned int keylen)
{
	int algfd = tst_alg_setup(algtype, algname, key, keylen);
	int reqfd = tst_alg_accept(algfd);

	close(algfd);
	return reqfd;
}
