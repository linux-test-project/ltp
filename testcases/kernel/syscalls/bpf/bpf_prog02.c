// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Richard Palethorpe <rpalethorpe@suse.com>
 *
 * Check if eBPF can do arithmetic with 64bits. This targets a specific
 * regression which only effects unprivileged users who are subject to extra
 * pointer arithmetic checks during verification.
 *
 * Fixed by commit 3612af783cf52c74a031a2f11b82247b2599d3cd.
 * https://new.blog.cloudflare.com/ebpf-cant-count/
 *
 * This test is very similar in structure to bpf_prog01 which is better
 * annotated.
 */

#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include "config.h"
#include "tst_test.h"
#include "tst_capability.h"
#include "lapi/socket.h"
#include "lapi/bpf.h"
#include "bpf_common.h"

#define A64INT (((uint64_t)1) << 60)

const char MSG[] = "Ahoj!";
static char *msg;

static char *log;
static uint32_t *key;
static uint64_t *val;
static union bpf_attr *attr;

static int load_prog(int fd)
{
	struct bpf_insn insn[] = {
		BPF_MOV64_IMM(BPF_REG_6, 1),            /* 0: r6 = 1 */

		BPF_LD_MAP_FD(BPF_REG_1, fd),	        /* 1: r1 = &fd */
		BPF_MOV64_REG(BPF_REG_2, BPF_REG_10),   /* 3: r2 = fp */
		BPF_ALU64_IMM(BPF_ADD, BPF_REG_2, -4),  /* 4: r2 = r2 - 4 */
		BPF_ST_MEM(BPF_W, BPF_REG_2, 0, 0),     /* 5: *r2 = 0 */
		BPF_EMIT_CALL(BPF_FUNC_map_lookup_elem),/* 6: map_lookup_elem */
		BPF_JMP_IMM(BPF_JEQ, BPF_REG_0, 0, 17), /* 7: if(!r0) goto 25 */
		BPF_MOV64_REG(BPF_REG_3, BPF_REG_0),    /* 8: r3 = r0 */
		BPF_LD_IMM64(BPF_REG_4, A64INT),        /* 9: r4 = 2^61 */
		BPF_ALU64_REG(BPF_ADD, BPF_REG_4, BPF_REG_6), /* 11: r4 += r6 */
		BPF_STX_MEM(BPF_DW, BPF_REG_3, BPF_REG_4, 0), /* 12: *r3 = r4 */

		BPF_LD_MAP_FD(BPF_REG_1, fd),	        /* 13: r1 = &fd */
		BPF_MOV64_REG(BPF_REG_2, BPF_REG_10),   /* 15: r2 = fp */
		BPF_ALU64_IMM(BPF_ADD, BPF_REG_2, -4),  /* 16: r2 = r2 - 4 */
		BPF_ST_MEM(BPF_W, BPF_REG_2, 0, 1),     /* 17: *r2 = 1 */
		BPF_EMIT_CALL(BPF_FUNC_map_lookup_elem),/* 18: map_lookup_elem */
		BPF_JMP_IMM(BPF_JEQ, BPF_REG_0, 0, 5),  /* 19: if(!r0) goto 25 */
		BPF_MOV64_REG(BPF_REG_3, BPF_REG_0),    /* 20: r3 = r0 */
		BPF_LD_IMM64(BPF_REG_4, A64INT),        /* 21: r4 = 2^60 */
		BPF_ALU64_REG(BPF_SUB, BPF_REG_4, BPF_REG_6), /* 23: r4 -= r6 */
		BPF_STX_MEM(BPF_DW, BPF_REG_3, BPF_REG_4, 0), /* 24: *r3 = r4 */

		BPF_MOV64_IMM(BPF_REG_0, 0),            /* 25: r0 = 0 */
		BPF_EXIT_INSN(),		        /* 26: return r0 */
	};

	bpf_init_prog_attr(attr, insn, sizeof(insn), log, BUFSIZ);
	return bpf_load_prog(attr, log);
}

static void setup(void)
{
	rlimit_bump_memlock();
	memcpy(msg, MSG, sizeof(MSG));
}

static void run(void)
{
	int map_fd, prog_fd;
	int sk[2];

	memset(attr, 0, sizeof(*attr));
	attr->map_type = BPF_MAP_TYPE_ARRAY;
	attr->key_size = 4;
	attr->value_size = 8;
	attr->max_entries = 2;

	map_fd = bpf_map_create(attr);

	prog_fd = load_prog(map_fd);

	SAFE_SOCKETPAIR(AF_UNIX, SOCK_DGRAM, 0, sk);
	SAFE_SETSOCKOPT(sk[1], SOL_SOCKET, SO_ATTACH_BPF,
			&prog_fd, sizeof(prog_fd));

	SAFE_WRITE(1, sk[0], msg, sizeof(MSG));

	memset(attr, 0, sizeof(*attr));
	attr->map_fd = map_fd;
	attr->key = ptr_to_u64(key);
	attr->value = ptr_to_u64(val);
	*key = 0;

	TEST(bpf(BPF_MAP_LOOKUP_ELEM, attr, sizeof(*attr)));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "array map lookup");
		goto exit;
	}

	if (*val != A64INT + 1) {
		tst_res(TFAIL,
			"val = %"PRIu64", but should be val = %"PRIu64" + 1",
			*val, A64INT);
		goto exit;
	}

	tst_res(TPASS, "val = %"PRIu64" + 1", A64INT);

	*key = 1;

	TEST(bpf(BPF_MAP_LOOKUP_ELEM, attr, sizeof(*attr)));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "array map lookup");
		goto exit;
	}

	if (*val != A64INT - 1) {
		tst_res(TFAIL,
			"val = %"PRIu64", but should be val = %"PRIu64" - 1",
			*val, A64INT);
		goto exit;
	}

	tst_res(TPASS, "val = %"PRIu64" - 1", A64INT);

exit:
	SAFE_CLOSE(prog_fd);
	SAFE_CLOSE(map_fd);
	SAFE_CLOSE(sk[0]);
	SAFE_CLOSE(sk[1]);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.min_kver = "3.18",
	.caps = (struct tst_cap []) {
		TST_CAP(TST_CAP_DROP, CAP_SYS_ADMIN),
		{}
	},
	.bufs = (struct tst_buffers []) {
		{&key, .size = sizeof(*key)},
		{&val, .size = sizeof(*val)},
		{&log, .size = BUFSIZ},
		{&attr, .size = sizeof(*attr)},
		{&msg, .size = sizeof(MSG)},
		{},
	},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "3612af783cf5"},
		{}
	}
};
