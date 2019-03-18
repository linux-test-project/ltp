// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2019 Google LLC
 */

#ifndef IF_ALG_H__
#define IF_ALG_H__

#ifdef HAVE_LINUX_IF_ALG_H
#  include <linux/if_alg.h>
#else
#  include <stdint.h>

struct sockaddr_alg {
	uint16_t	salg_family;
	uint8_t		salg_type[14];
	uint32_t	salg_feat;
	uint32_t	salg_mask;
	uint8_t		salg_name[64];
};

struct af_alg_iv {
	uint32_t	ivlen;
	uint8_t		iv[0];
};

#define ALG_SET_KEY		1
#define ALG_SET_IV		2
#define ALG_SET_OP		3
#define ALG_SET_AEAD_ASSOCLEN	4
#define ALG_SET_AEAD_AUTHSIZE	5

#define ALG_OP_DECRYPT		0
#define ALG_OP_ENCRYPT		1

#endif /* !HAVE_LINUX_IF_ALG_H */

#endif /* IF_ALG_H__ */
