// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2019 Google LLC
 */

#ifndef LAPI_IF_ALG_H__
#define LAPI_IF_ALG_H__

#ifdef HAVE_LINUX_IF_ALG_H
#  include <linux/if_alg.h>
#endif
#  include <stdint.h>

#ifndef HAVE_STRUCT_SOCKADDR_ALG
struct sockaddr_alg {
	uint16_t	salg_family;
	uint8_t		salg_type[14];
	uint32_t	salg_feat;
	uint32_t	salg_mask;
	uint8_t		salg_name[64];
};
#endif

#ifndef HAVE_STRUCT_AF_ALG_IV
struct af_alg_iv {
	uint32_t	ivlen;
	uint8_t		iv[0];
};
#endif

#ifndef ALG_SET_KEY
# define ALG_SET_KEY		1
#endif

#ifndef ALG_SET_IV
# define ALG_SET_IV		2
#endif

#ifndef ALG_SET_OP
# define ALG_SET_OP		3
#endif

#ifndef ALG_SET_AEAD_ASSOCLEN
# define ALG_SET_AEAD_ASSOCLEN	4
#endif

#ifndef ALG_SET_AEAD_AUTHSIZE
# define ALG_SET_AEAD_AUTHSIZE	5
#endif

#ifndef ALG_OP_DECRYPT
# define ALG_OP_DECRYPT		0
#endif

#ifndef ALG_OP_ENCRYPT
# define ALG_OP_ENCRYPT		1
#endif

#endif /* LAPI_IF_ALG_H__ */
