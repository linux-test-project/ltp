// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2022 SUSE LLC <rpalethorpe@suse.com>
 */

/*\
 * [Description]
 *
 * The verifier did not properly restrict some *_OR_NULL pointer
 * types. Including RET_PTR_TO_ALLOC_MEM_OR_NULL which is returned by
 * ringbuf_reserve. Somehow this means they can be used to perform
 * arbitrary pointer arithmetic.
 *
 * The test tries to do some pointer arithmetic on the return value of
 * ringbuf_reserve. Possibly with a trick to make the verifier believe
 * the pointer (in r1) is NULL. The test will pass if the eBPF is
 * rejected and will fail otherwise.
 *
 * This test does not try to cause a crash. Howver it does run the
 * eBPF if it can. This will result in an instant crash or memory
 * corruption which may later cause a crash.
 *
 * This test is adapted from a full reproducer which can be found here:
 * https://github.com/tr3ee/CVE-2022-23222
 *
 * It's recommended to disable unprivileged eBPF by setting
 * /proc/sys/kernel/unprivileged_bpf_disabled. Also there is a
 * specific fix for this issue:
 *
 * commit 64620e0a1e712a778095bd35cbb277dc2259281f
 * Author: Daniel Borkmann <daniel@iogearbox.net>
 * Date:   Tue Jan 11 14:43:41 2022 +0000
 *
 *  bpf: Fix out of bounds access for ringbuf helpers
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "config.h"
#include "tst_test.h"
#include "tst_taint.h"
#include "tst_capability.h"
#include "lapi/bpf.h"
#include "bpf_common.h"

static const char MSG[] = "Ahoj!";
static char *msg;

static int map_fd;
static uint32_t *key;
static uint64_t *val;
static char *log;
static union bpf_attr *attr;

static int load_prog(void)
{
	int ret;
	const struct bpf_insn prog_insn[] = {
		// r0 = bpf_ringbuf_reserve(ctx->ringbuf_fd, PAGE_SIZE, 0)
		BPF_LD_MAP_FD(BPF_REG_1, map_fd),
		BPF_MOV64_IMM(BPF_REG_2, getpagesize()),
		BPF_MOV64_IMM(BPF_REG_3, 0x00),
		BPF_EMIT_CALL(BPF_FUNC_ringbuf_reserve),

		BPF_MOV64_REG(BPF_REG_1, BPF_REG_0),
		BPF_ALU64_IMM(BPF_ADD, BPF_REG_1, 1),

		// if (r0 != NULL) { ringbuf_discard(r0, 1); exit(2); }
		BPF_JMP_IMM(BPF_JEQ, BPF_REG_0, 0, 5),
		BPF_MOV64_REG(BPF_REG_1, BPF_REG_0),
		BPF_MOV64_IMM(BPF_REG_2, 1),
		BPF_EMIT_CALL(BPF_FUNC_ringbuf_discard),
		BPF_MOV64_IMM(BPF_REG_0, 2),
		BPF_EXIT_INSN(),

		// *(sp + 4*r1) = INT32_MAX
		BPF_ALU64_IMM(BPF_MUL, BPF_REG_1, 8),
		BPF_MOV64_REG(BPF_REG_3, BPF_REG_10),
		BPF_ALU64_IMM(BPF_ADD, BPF_REG_3, -8),
		BPF_ALU64_REG(BPF_ADD, BPF_REG_3, BPF_REG_1),
		BPF_ST_MEM(BPF_DW, BPF_REG_3, 0, INT32_MAX),

		/* exit(0) */
		BPF_MOV64_IMM(BPF_REG_0, 0),
		BPF_EXIT_INSN()

	};

	bpf_init_prog_attr(attr, prog_insn, sizeof(prog_insn), log, BUFSIZE);

	ret = TST_RETRY_FUNC(bpf(BPF_PROG_LOAD, attr, sizeof(*attr)),
			     TST_RETVAL_GE0);

	if (ret >= 0)
		return ret;

	if (ret != -1)
		tst_brk(TBROK, "Invalid bpf() return value: %d", ret);

	if (log[0] != 0)
		tst_printf("%s\n", log);

	return ret;
}

static void setup(void)
{
	rlimit_bump_memlock();
	memcpy(msg, MSG, sizeof(MSG));
}

static void run(void)
{
	int prog_fd;

	map_fd = bpf_map_create(&(union bpf_attr){
			.map_type = BPF_MAP_TYPE_RINGBUF,
			.key_size = 0,
			.value_size = 0,
			.max_entries = getpagesize()
		});

	tst_res(TINFO, "Trying to load eBPF with OOB write");
	prog_fd = load_prog();
	if (prog_fd == -1) {
		tst_res(TPASS, "Failed verification");
		return;
	}

	tst_res(TFAIL, "Loaded program with OOB write");
	tst_res(TINFO, "Running eBPF with OOB");
	bpf_run_prog(prog_fd, msg, sizeof(MSG));
	tst_res(TINFO, "Ran eBPF");

	SAFE_CLOSE(prog_fd);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.min_kver = "5.8",
	.taint_check = TST_TAINT_W | TST_TAINT_D,
	.caps = (struct tst_cap []) {
		TST_CAP(TST_CAP_DROP, CAP_SYS_ADMIN),
		TST_CAP(TST_CAP_DROP, CAP_BPF),
		{}
	},
	.bufs = (struct tst_buffers []) {
		{&key, .size = sizeof(*key)},
		{&val, .size = sizeof(*val)},
		{&log, .size = BUFSIZE},
		{&attr, .size = sizeof(*attr)},
		{&msg, .size = sizeof(MSG)},
		{}
	},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "64620e0a1e71"},
		{"CVE", "CVE-2022-23222"},
		{}
	}
};
