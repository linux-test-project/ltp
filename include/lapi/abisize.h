/*
 * Copyright (c) 2014 Linux Test Project
 *  Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef ABISIZE_H__
#define ABISIZE_H__

/*
 * Determines if we have to split up 64 bit arguments or not
 *
 * Deals with 32bit ABIs that have 64bit syscalls
 */
#define LTP_USE_64_ABI \
     (defined(__mips__) && _MIPS_SIM == _ABIN32) || \
     (defined(__x86_64__) && defined(__ILP32__)) || \
     (defined(__aarch64__) && defined(__ILP32__)) || \
     __WORDSIZE == 64

#endif /* ABISIZE_H__ */
