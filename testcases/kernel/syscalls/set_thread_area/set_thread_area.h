#include <stdio.h>
#include <errno.h>

/* Harness Specific Include Files. */
#include "test.h"
#include "lapi/syscalls.h"
#include "config.h"

#if defined HAVE_ASM_LDT_H
#include <linux/unistd.h>
#include <asm/ldt.h>

#if defined HAVE_STRUCT_USER_DESC
typedef struct user_desc thread_area_s;
#elif defined HAVE_STRUCT_MODIFY_LDT_LDT_S
typedef struct modify_ldt_ldt_s thread_area_s;
#else
typedef struct user_desc {
	unsigned int entry_number;
	unsigned long int base_addr;
	unsigned int limit;
	unsigned int seg_32bit:1;
	unsigned int contents:2;
	unsigned int read_exec_only:1;
	unsigned int limit_in_pages:1;
	unsigned int seg_not_present:1;
	unsigned int useable:1;
	unsigned int empty:25;
} thread_area_s;
#endif /* HAVE_STRUCT_USER_DESC */
#endif /* HAVE_ASM_LDT_H */
