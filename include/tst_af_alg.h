// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2019 Google LLC
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
 * Return true if the algorithm is available, or false if unavailable.
 * If another error occurs, tst_brk() is called with TBROK.
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

#endif /* TST_AF_ALG_H */
