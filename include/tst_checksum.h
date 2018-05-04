/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2018 Oracle and/or its affiliates. All Rights Reserved.
 */

#ifndef TST_CHECKSUM_H__
#define TST_CHECKSUM_H__

#include <stdint.h>
#include <stddef.h>

/*
 * Generates CRC32c checksum.
 */
uint32_t tst_crc32c(uint8_t *buf, size_t buf_len);

#endif
