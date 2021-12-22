// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2019 Google LLC
 * Copyright (c) Linux Test Project, 2019-2021
 */

#include <errno.h>
#include <stdlib.h>

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tst_af_alg.h"
#include "lapi/socket.h"

int tst_alg_create(void)
{
	const long ret = socket(AF_ALG, SOCK_SEQPACKET, 0);

	if (ret >= 0)
		return ret;
	if (errno == EAFNOSUPPORT)
		tst_brk(TCONF, "kernel doesn't support AF_ALG");
	tst_brk(TBROK | TERRNO, "unexpected error creating AF_ALG socket");
	return -1;
}

void tst_alg_bind_addr(int algfd, const struct sockaddr_alg *addr)
{
	const long ret = bind(algfd, (const struct sockaddr *)addr,
			      sizeof(*addr));

	if (ret == 0)
		return;
	if (errno == ENOENT) {
		tst_brk(TCONF, "kernel doesn't support %s algorithm '%s'",
			addr->salg_type, addr->salg_name);
	}
	tst_brk(TBROK | TERRNO,
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

int tst_try_alg(const char *algtype, const char *algname)
{
	long ret;
	int retval = 0;
	int algfd;
	struct sockaddr_alg addr;

	algfd = tst_alg_create();

	init_sockaddr_alg(&addr, algtype, algname);

	ret = bind(algfd, (const struct sockaddr *)&addr, sizeof(addr));

	if (ret != 0)
		retval = errno;

	close(algfd);
	return retval;
}

bool tst_have_alg(const char *algtype, const char *algname)
{
	int ret;

	ret = tst_try_alg(algtype, algname);

	switch (ret) {
	case 0:
		return true;
	case ENOENT:
		tst_res(TCONF, "kernel doesn't have %s algorithm '%s'",
			algtype, algname);
		return false;
	default:
		errno = ret;
		tst_brk(TBROK | TERRNO,
			"unexpected error binding AF_ALG socket to %s algorithm '%s'",
			algtype, algname);
		return false;
	}
}

void tst_require_alg(const char *algtype, const char *algname)
{
	int algfd = tst_alg_create();

	tst_alg_bind(algfd, algtype, algname);

	close(algfd);
}

void tst_alg_setkey(int algfd, const uint8_t *key, unsigned int keylen)
{
	long ret;
	uint8_t *keybuf = NULL;
	unsigned int i;

	if (key == NULL) {
		/* generate a random key */
		keybuf = SAFE_MALLOC(keylen);
		for (i = 0; i < keylen; i++)
			keybuf[i] = rand();
		key = keybuf;
	}
	ret = setsockopt(algfd, SOL_ALG, ALG_SET_KEY, key, keylen);
	if (ret != 0) {
		tst_brk(TBROK | TERRNO,
			"unexpected error setting key (len=%u)", keylen);
	}
	free(keybuf);
}

int tst_alg_accept(int algfd)
{
	const long ret = accept(algfd, NULL, NULL);

	if (ret < 0) {
		tst_brk(TBROK | TERRNO,
			"unexpected error accept()ing AF_ALG request socket");
	}
	return ret;
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

void tst_alg_sendmsg(int reqfd, const void *data, size_t datalen,
		     const struct tst_alg_sendmsg_params *params)
{
	struct iovec iov = {
		.iov_base = (void *)data,
		.iov_len = datalen,
	};
	struct msghdr msg = {
		.msg_iov = &iov,
		.msg_iovlen = 1,
		.msg_flags = params->msg_flags,
	};
	size_t controllen;
	uint8_t *control;
	struct cmsghdr *cmsg;
	struct af_alg_iv *alg_iv;

	if (params->encrypt && params->decrypt)
		tst_brk(TBROK, "Both encrypt and decrypt are specified");

	controllen = 0;
	if (params->encrypt || params->decrypt)
		controllen += CMSG_SPACE(sizeof(uint32_t));
	if (params->ivlen)
		controllen += CMSG_SPACE(sizeof(struct af_alg_iv) +
					 params->ivlen);
	if (params->assoclen)
		controllen += CMSG_SPACE(sizeof(uint32_t));

	control = SAFE_MALLOC(controllen);
	memset(control, 0, controllen);
	msg.msg_control = control;
	msg.msg_controllen = controllen;
	cmsg = CMSG_FIRSTHDR(&msg);

	if (params->encrypt || params->decrypt) {
		cmsg->cmsg_level = SOL_ALG;
		cmsg->cmsg_type = ALG_SET_OP;
		cmsg->cmsg_len = CMSG_LEN(sizeof(uint32_t));
		*(uint32_t *)CMSG_DATA(cmsg) =
			params->encrypt ? ALG_OP_ENCRYPT : ALG_OP_DECRYPT;
		cmsg = CMSG_NXTHDR(&msg, cmsg);
	}
	if (params->ivlen) {
		cmsg->cmsg_level = SOL_ALG;
		cmsg->cmsg_type = ALG_SET_IV;
		cmsg->cmsg_len = CMSG_LEN(sizeof(struct af_alg_iv) +
					  params->ivlen);
		alg_iv = (struct af_alg_iv *)CMSG_DATA(cmsg);
		alg_iv->ivlen = params->ivlen;
		memcpy(alg_iv->iv, params->iv, params->ivlen);
		cmsg = CMSG_NXTHDR(&msg, cmsg);
	}
	if (params->assoclen) {
		cmsg->cmsg_level = SOL_ALG;
		cmsg->cmsg_type = ALG_SET_AEAD_ASSOCLEN;
		cmsg->cmsg_len = CMSG_LEN(sizeof(uint32_t));
		*(uint32_t *)CMSG_DATA(cmsg) = params->assoclen;
		cmsg = CMSG_NXTHDR(&msg, cmsg);
	}

	SAFE_SENDMSG(datalen, reqfd, &msg, 0);
}
