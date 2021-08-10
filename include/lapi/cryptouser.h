// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Richard Palethorpe <rpalethorpe@suse.com>
 */

#ifndef LAPI_CRYPTOUSER_H__
#define LAPI_CRYPTOUSER_H__

#ifdef HAVE_LINUX_CRYPTOUSER_H
#  include <linux/cryptouser.h>
#else
#  include <stdint.h>
#  define CRYPTO_MAX_NAME 64

enum {
	CRYPTO_MSG_BASE = 0x10,
	CRYPTO_MSG_NEWALG = 0x10,
	CRYPTO_MSG_DELALG,
	CRYPTO_MSG_UPDATEALG,
	CRYPTO_MSG_GETALG,
	CRYPTO_MSG_DELRNG,
	__CRYPTO_MSG_MAX
};

enum crypto_attr_type_t {
	CRYPTOCFGA_UNSPEC,
	CRYPTOCFGA_PRIORITY_VAL,	/* uint32_t */
	CRYPTOCFGA_REPORT_LARVAL,	/* struct crypto_report_larval */
	CRYPTOCFGA_REPORT_HASH,		/* struct crypto_report_hash */
	CRYPTOCFGA_REPORT_BLKCIPHER,	/* struct crypto_report_blkcipher */
	CRYPTOCFGA_REPORT_AEAD,		/* struct crypto_report_aead */
	CRYPTOCFGA_REPORT_COMPRESS,	/* struct crypto_report_comp */
	CRYPTOCFGA_REPORT_RNG,		/* struct crypto_report_rng */
	CRYPTOCFGA_REPORT_CIPHER,	/* struct crypto_report_cipher */
	CRYPTOCFGA_REPORT_AKCIPHER,	/* struct crypto_report_akcipher */
	CRYPTOCFGA_REPORT_KPP,		/* struct crypto_report_kpp */
	CRYPTOCFGA_REPORT_ACOMP,	/* struct crypto_report_acomp */
	__CRYPTOCFGA_MAX

#define CRYPTOCFGA_MAX (__CRYPTOCFGA_MAX - 1)
};

struct crypto_user_alg {
	char cru_name[CRYPTO_MAX_NAME];
	char cru_driver_name[CRYPTO_MAX_NAME];
	char cru_module_name[CRYPTO_MAX_NAME];
	uint32_t cru_type;
	uint32_t cru_mask;
	uint32_t cru_refcnt;
	uint32_t cru_flags;
};

struct crypto_report_larval {
	char type[CRYPTO_MAX_NAME];
};

struct crypto_report_hash {
	char type[CRYPTO_MAX_NAME];
	unsigned int blocksize;
	unsigned int digestsize;
};

struct crypto_report_cipher {
	char type[CRYPTO_MAX_NAME];
	unsigned int blocksize;
	unsigned int min_keysize;
	unsigned int max_keysize;
};

struct crypto_report_blkcipher {
	char type[CRYPTO_MAX_NAME];
	char geniv[CRYPTO_MAX_NAME];
	unsigned int blocksize;
	unsigned int min_keysize;
	unsigned int max_keysize;
	unsigned int ivsize;
};

struct crypto_report_aead {
	char type[CRYPTO_MAX_NAME];
	char geniv[CRYPTO_MAX_NAME];
	unsigned int blocksize;
	unsigned int maxauthsize;
	unsigned int ivsize;
};

struct crypto_report_comp {
	char type[CRYPTO_MAX_NAME];
};

struct crypto_report_rng {
	char type[CRYPTO_MAX_NAME];
	unsigned int seedsize;
};

struct crypto_report_akcipher {
	char type[CRYPTO_MAX_NAME];
};

struct crypto_report_kpp {
	char type[CRYPTO_MAX_NAME];
};

struct crypto_report_acomp {
	char type[CRYPTO_MAX_NAME];
};

#endif	/* HAVE_LINUX_CRYPTOUSER_H */

/* These are taken from include/crypto.h in the kernel tree. They are not
 * currently included in the user API.
 */
#ifndef CRYPTO_MAX_ALG_NAME
#  define CRYPTO_MAX_ALG_NAME		128
#endif

#ifndef CRYPTO_ALG_TYPE_MASK
#  define CRYPTO_ALG_TYPE_MASK		0x0000000f
#endif
#ifndef CRYPTO_ALG_TYPE_CIPHER
#  define CRYPTO_ALG_TYPE_CIPHER	0x00000001
#endif
#ifndef CRYPTO_ALG_TYPE_COMPRESS
#  define CRYPTO_ALG_TYPE_COMPRESS	0x00000002
#endif
#ifndef CRYPTO_ALG_TYPE_AEAD
#  define CRYPTO_ALG_TYPE_AEAD		0x00000003
#endif
#ifndef CRYPTO_ALG_TYPE_BLKCIPHER
#  define CRYPTO_ALG_TYPE_BLKCIPHER	0x00000004
#endif
#ifndef CRYPTO_ALG_TYPE_ABLKCIPHER
#  define CRYPTO_ALG_TYPE_ABLKCIPHER	0x00000005
#endif
#ifndef CRYPTO_ALG_TYPE_SKCIPHER
#  define CRYPTO_ALG_TYPE_SKCIPHER	0x00000005
#endif
#ifndef CRYPTO_ALG_TYPE_GIVCIPHER
#  define CRYPTO_ALG_TYPE_GIVCIPHER	0x00000006
#endif
#ifndef CRYPTO_ALG_TYPE_KPP
#  define CRYPTO_ALG_TYPE_KPP		0x00000008
#endif
#ifndef CRYPTO_ALG_TYPE_ACOMPRESS
#  define CRYPTO_ALG_TYPE_ACOMPRESS	0x0000000a
#endif
#ifndef CRYPTO_ALG_TYPE_SCOMPRESS
#  define CRYPTO_ALG_TYPE_SCOMPRESS	0x0000000b
#endif
#ifndef CRYPTO_ALG_TYPE_RNG
#  define CRYPTO_ALG_TYPE_RNG		0x0000000c
#endif
#ifndef CRYPTO_ALG_TYPE_AKCIPHER
#  define CRYPTO_ALG_TYPE_AKCIPHER	0x0000000d
#endif
#ifndef CRYPTO_ALG_TYPE_DIGEST
#  define CRYPTO_ALG_TYPE_DIGEST	0x0000000e
#endif
#ifndef CRYPTO_ALG_TYPE_HASH
#  define CRYPTO_ALG_TYPE_HASH		0x0000000e
#endif
#ifndef CRYPTO_ALG_TYPE_SHASH
#  define CRYPTO_ALG_TYPE_SHASH		0x0000000e
#endif
#ifndef CRYPTO_ALG_TYPE_AHASH
#  define CRYPTO_ALG_TYPE_AHASH		0x0000000f
#endif

#ifndef CRYPTO_ALG_TYPE_HASH_MASK
#  define CRYPTO_ALG_TYPE_HASH_MASK	0x0000000e
#endif
#ifndef CRYPTO_ALG_TYPE_AHASH_MASK
#  define CRYPTO_ALG_TYPE_AHASH_MASK	0x0000000e
#endif
#ifndef CRYPTO_ALG_TYPE_BLKCIPHER_MASK
#  define CRYPTO_ALG_TYPE_BLKCIPHER_MASK	0x0000000c
#endif
#ifndef CRYPTO_ALG_TYPE_ACOMPRESS_MASK
#  define CRYPTO_ALG_TYPE_ACOMPRESS_MASK	0x0000000e
#endif

#endif	/* LAPI_CRYPTOUSER_H__ */
