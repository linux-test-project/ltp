// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
 * Copyright (c) Linux Test Project, 2021-2024
 * Author: Stanislav Kholmanskikh <stanislav.kholmanskikh@oracle.com>
 */

/**
 * DOC: libltpswap
 *
 * Contains common content for all swapon/swapoff tests.
 */

#ifndef __LIBSWAP_H__
#define __LIBSWAP_H__

enum swapfile_method {
    SWAPFILE_BY_SIZE,
    SWAPFILE_BY_BLKS
};

/*
 * Create a swapfile of a specified size or number of blocks.
 */
int make_swapfile(const char *file, const int lineno,
			const char *swapfile, unsigned int num,
			int safe, enum swapfile_method method);

/** 65536 bytes is minimum for 64kb page size, let's use 1 MB */
#define MINIMAL_SWAP_SIZE_MB 1

/**
 * MAKE_SMALL_SWAPFILE - create small swap file.
 *
 * Macro to create small swap file. Size defined with MINIMAL_SWAP_SIZE_MB.
 *
 * @swapfile: swap filename.
 */
#define MAKE_SMALL_SWAPFILE(swapfile) \
    make_swapfile(__FILE__, __LINE__, swapfile, MINIMAL_SWAP_SIZE_MB, 0, \
		  SWAPFILE_BY_SIZE)

/**
 * SAFE_MAKE_SMALL_SWAPFILE - create small swap file (safe version).
 *
 * Macro to create small swap file. Size defined with MINIMAL_SWAP_SIZE_MB.
 * Includes safety checks to handle potential errors.
 *
 * @swapfile: swap filename.
 */
#define SAFE_MAKE_SMALL_SWAPFILE(swapfile) \
    make_swapfile(__FILE__, __LINE__, swapfile, MINIMAL_SWAP_SIZE_MB, 1, \
		  SWAPFILE_BY_SIZE)

/**
 * MAKE_SWAPFILE_SIZE - create swap file (MB).
 *
 * Macro to create swap file, size specified in megabytes (MB).
 *
 * @swapfile: swap filename.
 * @size: swap size in MB.
 */
#define MAKE_SWAPFILE_SIZE(swapfile, size) \
    make_swapfile(__FILE__, __LINE__, swapfile, size, 0, SWAPFILE_BY_SIZE)

/**
 * MAKE_SWAPFILE_BLKS - create swap file (blocks).
 *
 * Macro to create swap file, size specified in block numbers.
 *
 * @swapfile: swap filename.
 * @blocks: number of blocks.
 */
#define MAKE_SWAPFILE_BLKS(swapfile, blocks) \
    make_swapfile(__FILE__, __LINE__, swapfile, blocks, 0, SWAPFILE_BY_BLKS)

/**
 * SAFE_MAKE_SWAPFILE_SIZE - create swap file (MB, safe version).
 *
 * Macro to safely create swap file, size specified in megabytes (MB).
 * Includes safety checks to handle potential errors.
 *
 * @swapfile: swap file name.
 * @size: swap size in MB.
 */
#define SAFE_MAKE_SWAPFILE_SIZE(swapfile, size) \
    make_swapfile(__FILE__, __LINE__, swapfile, size, 1, SWAPFILE_BY_SIZE)

/**
 * SAFE_MAKE_SWAPFILE_BLKS - create swap file (block, safe version)
 *
 * Macro to safely create swap file, size specified in block numbers.
 * Includes safety checks to handle potential errors.
 *
 * @swapfile: swap file name.
 * @blocks: number of blocks.
 */
#define SAFE_MAKE_SWAPFILE_BLKS(swapfile, blocks) \
    make_swapfile(__FILE__, __LINE__, swapfile, blocks, 1, SWAPFILE_BY_BLKS)

/**
 * is_swap_supported() - Check swapon/swapoff support.
 *
 * Check swapon/swapoff support status of filesystems or files
 * we are testing on.
 *
 * @filename: swap file name.
 * Return: true if swap is supported, false if not.
 */
bool is_swap_supported(const char *filename);

/**
 * tst_count_swaps() - Get the used swapfiles number.
 *
 * Return: used swapfiles number.
 */
int tst_count_swaps(void);

#endif /* __LIBSWAP_H__ */
