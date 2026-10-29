// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>
 */

#ifndef LAPI_IOCTL_H__
#define LAPI_IOCTL_H__

#include "config.h"
#include <sys/ioctl.h>
#include <stdint.h>

/* musl not including it in <sys/ioctl.h> */
#include <sys/ttydefaults.h>

#ifndef TIOCVHANGUP
# define TIOCVHANGUP 0x5437
#endif

#ifndef HAVE_STRUCT_TERMIO
# ifndef NCC
#  ifdef __powerpc__
#   define NCC 10
#  else
#   define NCC 8
#  endif
# endif /* NCC */

struct termio
  {
    unsigned short int c_iflag;		/* input mode flags */
    unsigned short int c_oflag;		/* output mode flags */
    unsigned short int c_cflag;		/* control mode flags */
    unsigned short int c_lflag;		/* local mode flags */
    unsigned char c_line;		/* line discipline */
    unsigned char c_cc[NCC];		/* control characters */
};
#endif /* HAVE_STRUCT_TERMIO */

#ifndef HAVE_STRUCT_PROCMAP_QUERY
#define PROCFS_IOCTL_MAGIC 'f'
#define PROCMAP_QUERY   _IOWR(PROCFS_IOCTL_MAGIC, 17, struct procmap_query)
enum procmap_query_flags {
        /*
         * VMA permission flags.
         *
         * Can be used as part of procmap_query.query_flags field to look up
         * only VMAs satisfying specified subset of permissions. E.g., specifying
         * PROCMAP_QUERY_VMA_READABLE only will return both readable and read/write VMAs,
         * while having PROCMAP_QUERY_VMA_READABLE | PROCMAP_QUERY_VMA_WRITABLE will only
         * return read/write VMAs, though both executable/non-executable and
         * private/shared will be ignored.
         *
         * PROCMAP_QUERY_VMA_* flags are also returned in procmap_query.vma_flags
         * field to specify actual VMA permissions.
         */
        PROCMAP_QUERY_VMA_READABLE              = 0x01,
        PROCMAP_QUERY_VMA_WRITABLE              = 0x02,
        PROCMAP_QUERY_VMA_EXECUTABLE            = 0x04,
        PROCMAP_QUERY_VMA_SHARED                = 0x08,
        /*
         * Query modifier flags.
         *
         * By default VMA that covers provided address is returned, or -ENOENT
         * is returned. With PROCMAP_QUERY_COVERING_OR_NEXT_VMA flag set, closest
         * VMA with vma_start > addr will be returned if no covering VMA is
         * found.
         *
         * PROCMAP_QUERY_FILE_BACKED_VMA instructs query to consider only VMAs that
         * have file backing. Can be combined with PROCMAP_QUERY_COVERING_OR_NEXT_VMA
         * to iterate all VMAs with file backing.
         */
        PROCMAP_QUERY_COVERING_OR_NEXT_VMA      = 0x10,
        PROCMAP_QUERY_FILE_BACKED_VMA           = 0x20,
};

struct procmap_query {
        /* Query struct size, for backwards/forward compatibility */
        uint64_t size;
        /*
         * Query flags, a combination of enum procmap_query_flags values.
         * Defines query filtering and behavior, see enum procmap_query_flags.
         *
         * Input argument, provided by user. Kernel doesn't modify it.
         */
        uint64_t query_flags;              /* in */
        /*
         * Query address. By default, VMA that covers this address will
         * be looked up. PROCMAP_QUERY_* flags above modify this default
         * behavior further.
         *
         * Input argument, provided by user. Kernel doesn't modify it.
         */
        uint64_t query_addr;               /* in */
        /* VMA starting (inclusive) and ending (exclusive) address, if VMA is found. */
        uint64_t vma_start;                /* out */
        uint64_t vma_end;                  /* out */
        /* VMA permissions flags. A combination of PROCMAP_QUERY_VMA_* flags. */
        uint64_t vma_flags;                /* out */
        /* VMA backing page size granularity. */
        uint64_t vma_page_size;            /* out */
        /*
         * VMA file offset. If VMA has file backing, this specifies offset
         * within the file that VMA's start address corresponds to.
         * Is set to zero if VMA has no backing file.
         */
        uint64_t vma_offset;               /* out */
        /* Backing file's inode number, or zero, if VMA has no backing file. */
        uint64_t inode;                    /* out */
        /* Backing file's device major/minor number, or zero, if VMA has no backing file. */
        uint32_t dev_major;                /* out */
        uint32_t dev_minor;                /* out */
        /*
         * If set to non-zero value, signals the request to return VMA name
         * (i.e., VMA's backing file's absolute path, with " (deleted)" suffix
         * appended, if file was unlinked from FS) for matched VMA. VMA name
         * can also be some special name (e.g., "[heap]", "[stack]") or could
         * be even user-supplied with prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME).
         *
         * Kernel will set this field to zero, if VMA has no associated name.
         * Otherwise kernel will return actual amount of bytes filled in
         * user-supplied buffer (see vma_name_addr field below), including the
         * terminating zero.
         *
         * If VMA name is longer that user-supplied maximum buffer size,
         * -E2BIG error is returned.
         *
         * If this field is set to non-zero value, vma_name_addr should point
         * to valid user space memory buffer of at least vma_name_size bytes.
         * If set to zero, vma_name_addr should be set to zero as well
         */
        uint32_t vma_name_size;            /* in/out */
        /*
         * If set to non-zero value, signals the request to extract and return
         * VMA's backing file's build ID, if the backing file is an ELF file
         * and it contains embedded build ID.
         *
         * Kernel will set this field to zero, if VMA has no backing file,
         * backing file is not an ELF file, or ELF file has no build ID
         * embedded.
         *
         * Build ID is a binary value (not a string). Kernel will set
         * build_id_size field to exact number of bytes used for build ID.
         * If build ID is requested and present, but needs more bytes than
         * user-supplied maximum buffer size (see build_id_addr field below),
         * -E2BIG error will be returned.
         *
         * If this field is set to non-zero value, build_id_addr should point
         * to valid user space memory buffer of at least build_id_size bytes.
         * If set to zero, build_id_addr should be set to zero as well
         */
        uint32_t build_id_size;            /* in/out */
        /*
         * User-supplied address of a buffer of at least vma_name_size bytes
         * for kernel to fill with matched VMA's name (see vma_name_size field
         * description above for details).
         *
         * Should be set to zero if VMA name should not be returned.
         */
        uint64_t vma_name_addr;            /* in */
        /*
         * User-supplied address of a buffer of at least build_id_size bytes
         * for kernel to fill with matched VMA's ELF build ID, if available
         * (see build_id_size field description above for details).
         *
         * Should be set to zero if build ID should not be returned.
         */
        uint64_t build_id_addr;            /* in */
};
#endif /* HAVE_STRUCT_PROCMAP_QUERY */

#endif /* LAPI_IOCTL_H__ */
