// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2019 Google LLC
 * Copyright (c) Linux Test Project, 2021
 */
/**
 * @file tst_af_alg.h
 *
 * Library for accessing kernel crypto algorithms via AF_ALG.
 *
 * See https://www.kernel.org/doc/html/latest/crypto/userspace-if.html
 * for more information about AF_ALG.
 */

#ifndef TST_AF_ALG_H
#define TST_AF_ALG_H

#include "lapi/if_alg.h"
#include <stdbool.h>

/**
 * Create an AF_ALG algorithm socket.
 *
 * This creates an AF_ALG algorithm socket that is initially not bound to any
 * particular algorithm.  On failure, tst_brk() is called with TCONF if the
 * kernel doesn't support AF_ALG, otherwise TBROK.
 *
 * @return a new AF_ALG algorithm socket
 */
int tst_alg_create(void);

/**
 * Bind an AF_ALG algorithm socket to an algorithm.
 *
 * @param algfd An AF_ALG algorithm socket
 * @param addr A structure which specifies the algorithm to use
 *
 * On failure, tst_brk() is called with TCONF if the kernel doesn't support the
 * specified algorithm, otherwise TBROK.
 */
void tst_alg_bind_addr(int algfd, const struct sockaddr_alg *addr);

/**
 * Bind an AF_ALG algorithm socket to an algorithm.
 *
 * @param algfd An AF_ALG algorithm socket
 * @param algtype The type of algorithm, such as "hash" or "skcipher"
 * @param algname The name of the algorithm, such as "sha256" or "xts(aes)"
 *
 * Like tst_alg_bind_addr(), except this just takes in the algorithm type and
 * name.  The 'feat' and 'mask' fields are left 0.
 *
 * On failure, tst_brk() is called with TCONF if the kernel doesn't support the
 * specified algorithm, otherwise TBROK.
 */
void tst_alg_bind(int algfd, const char *algtype, const char *algname);

/**
 * Check for the availability of an algorithm.
 *
 * @param algtype The type of algorithm, such as "hash" or "skcipher"
 * @param algname The name of the algorithm, such as "sha256" or "xts(aes)"
 *
 * Return 0 if the algorithm is available, or errno if unavailable.
 */
int tst_try_alg(const char *algtype, const char *algname);

/**
 * Check for the availability of an algorithm.
 *
 * @param algtype The type of algorithm, such as "hash" or "skcipher"
 * @param algname The name of the algorithm, such as "sha256" or "xts(aes)"
 *
 * Return true if the algorithm is available, or false if unavailable
 * and call tst_res() with TCONF. If another error occurs, tst_brk() is called
 * with TBROK unless algorithm is disabled due FIPS mode (errno ELIBBAD).
 */
bool tst_have_alg(const char *algtype, const char *algname);

/**
 * Require the availability of an algorithm.
 *
 * @param algtype The type of algorithm, such as "hash" or "skcipher"
 * @param algname The name of the algorithm, such as "sha256" or "xts(aes)"
 *
 * If the algorithm is unavailable, tst_brk() is called with TCONF.
 * If another error occurs, tst_brk() is called with TBROK.
 */
void tst_require_alg(const char *algtype, const char *algname);

/**
 * Assign a cryptographic key to an AF_ALG algorithm socket.
 *
 * @param algfd An AF_ALG algorithm socket
 * @param key Pointer to the key.  If NULL, a random key is generated.
 * @param keylen Length of the key in bytes
 *
 * On failure, tst_brk() is called with TBROK.
 */
void tst_alg_setkey(int algfd, const uint8_t *key, unsigned int keylen);

/**
 * Create an AF_ALG request socket for the given algorithm socket.
 *
 * @param algfd An AF_ALG algorithm socket
 *
 * This creates a request socket for the given algorithm socket, which must be
 * bound to an algorithm.  The same algorithm socket can have many request
 * sockets used concurrently to perform independent cryptographic operations,
 * e.g. hashing or encryption/decryption.  But the key, if any, that has been
 * assigned to the algorithm is shared by all request sockets.
 *
 * On failure, tst_brk() is called with TBROK.
 *
 * @return a new AF_ALG request socket
 */
int tst_alg_accept(int algfd);

/**
 * Set up an AF_ALG algorithm socket for the given algorithm w/ given key.
 *
 * @param algtype The type of algorithm, such as "hash" or "skcipher"
 * @param algname The name of the algorithm, such as "sha256" or "xts(aes)"
 * @param key The key to use (optional)
 * @param keylen The length of the key in bytes (optional)
 *
 * This is a helper function which creates an AF_ALG algorithm socket, binds it
 * to the specified algorithm, and optionally sets a key.  If keylen is 0 then
 * no key is set; otherwise if key is NULL a key of the given length is randomly
 * generated and set; otherwise the given key is set.
 *
 * @return the AF_ALG algorithm socket that was set up
 */
int tst_alg_setup(const char *algtype, const char *algname,
		  const uint8_t *key, unsigned int keylen);

/**
 * Set up an AF_ALG request socket for the given algorithm w/ given key.
 *
 * This is like tst_alg_setup(), except this returns a request fd instead of the
 * alg fd.  The alg fd is closed, so it doesn't need to be kept track of.
 *
 * @return the AF_ALG request socket that was set up
 */
int tst_alg_setup_reqfd(const char *algtype, const char *algname,
			const uint8_t *key, unsigned int keylen);

/** Specification of control data to send to an AF_ALG request socket */
struct tst_alg_sendmsg_params {

	/** If true, send ALG_SET_OP with ALG_OP_ENCRYPT */
	bool encrypt;

	/** If true, send ALG_SET_OP with ALG_OP_DECRYPT */
	bool decrypt;

	/** If ivlen != 0, send ALG_SET_IV */
	const uint8_t *iv;
	unsigned int ivlen;

	/** If assoclen != 0, send ALG_SET_AEAD_ASSOCLEN */
	unsigned int assoclen;

	/* Value to use as msghdr::msg_flags */
	uint32_t msg_flags;
};

/**
 * Send some data to an AF_ALG request socket, including control data.
 * @param reqfd An AF_ALG request socket
 * @param data The data to send
 * @param datalen The length of data in bytes
 * @param params Specification of the control data to send
 *
 * On failure, tst_brk() is called with TBROK.
 */
void tst_alg_sendmsg(int reqfd, const void *data, size_t datalen,
		     const struct tst_alg_sendmsg_params *params);

#endif /* TST_AF_ALG_H */
