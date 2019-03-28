// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2014-2019 Linux Test Project
 *  Cyril Hrubis <chrubis@suse.cz>
 *  Petr Vorel <petr.vorel@gmail.com>
 */

#ifndef ABISIZE_H__
#define ABISIZE_H__

/* __WORDSIZE replacement */
#if defined(__LP64__) || defined(_LP64)
# define TST_ABI64
# define TST_ABI 64
#else
# define TST_ABI32
# define TST_ABI 32
#endif

/*
 * Determines if we have to split up 64 bit arguments or not
 *
 * Deals with 32bit ABIs that have 64bit syscalls
 */
#define LTP_USE_64_ABI \
     (defined(__mips__) && _MIPS_SIM == _ABIN32) || \
     (defined(__x86_64__) && defined(__ILP32__)) || \
     (defined(__aarch64__) && defined(__ILP32__)) || \
     defined(TST_ABI64)

#endif /* ABISIZE_H__ */
