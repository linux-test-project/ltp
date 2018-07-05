/*
 * Copyright (c) 2018 Richard Palethorpe <rpalethorpe@suse.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CRYPTOUSER_H__
#define CRYPTOUSER_H__

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

struct crypto_user_alg {
	char cru_name[CRYPTO_MAX_NAME];
	char cru_driver_name[CRYPTO_MAX_NAME];
	char cru_module_name[CRYPTO_MAX_NAME];
	uint32_t cru_type;
	uint32_t cru_mask;
	uint32_t cru_refcnt;
	uint32_t cru_flags;
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

#endif	/* CRYPTOUSER_H__ */
