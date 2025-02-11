// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2021 SUSE LLC <rpalethorpe@suse.com>
 */

/*\
 * Compare the effects of 32-bit div/mod by zero with the "expected"
 * behaviour.
 *
 * The commit "bpf: fix subprog verifier bypass by div/mod by 0
 * exception", changed div/mod by zero from exiting the current
 * program to setting the destination register to zero (div) or
 * leaving it untouched (mod).
 *
 * This solved one verfier bug which allowed dodgy pointer values, but
 * it turned out that the source register was being 32-bit truncated
 * when it should not be. Also the destination register for mod was
 * not being truncated when it should be.
 *
 * So then we have the following two fixes:
 * "bpf: Fix 32 bit src register truncation on div/mod"
 * "bpf: Fix truncation handling for mod32 dst reg wrt zero"
 *
 * Testing for all of these issues is a problem. Not least because
 * division by zero is undefined, so in theory any result is
 * acceptable so long as the verifier and runtime behaviour
 * match.
 *
 * However to keep things simple we just check if the source and
 * destination register runtime values match the current upstream
 * behaviour at the time of writing.
 *
 * If the test fails you may have one or more of the above patches
 * missing. In this case it is possible that you are not vulnerable
 * depending on what other backports and fixes have been applied. If
 * upstream changes the behaviour of division by zero, then the test
 * will need updating.
 *
 * Note that we use r6 as the src register and r7 as the dst. w6 and
 * w7 are the same registers treated as 32bit.
 */

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "config.h"
#include "tst_test.h"
#include "tst_taint.h"
#include "tst_capability.h"
#include "bpf_common.h"

static const char MSG[] = "Ahoj!";
static char *msg;

static int map_fd;
static uint32_t *key;
static uint64_t *val;
static char *log;
static union bpf_attr *attr;

static void ensure_ptr_arithmetic(void)
{
	const struct bpf_insn prog_insn[] = {
		/* r2 = r10
		 * r3 = -1
		 * r2 += r3
		 * *(char *)r2 = 0
		 */
		BPF_MOV64_REG(BPF_REG_2, BPF_REG_10),
		BPF_MOV64_IMM(BPF_REG_3, -1),
		BPF_ALU64_REG(BPF_ADD, BPF_REG_2, BPF_REG_3),
		BPF_ST_MEM(BPF_B, BPF_REG_2, 0, 0),

		/* exit(0) */
		BPF_MOV64_IMM(BPF_REG_0, 0),
		BPF_EXIT_INSN()
	};
	int ret;

	bpf_init_prog_attr(attr, prog_insn, sizeof(prog_insn), log, BUFSIZE);

	ret = TST_RETRY_FUNC(bpf(BPF_PROG_LOAD, attr, sizeof(*attr)),
				       TST_RETVAL_GE0);

	if (ret >= 0) {
		tst_res(TINFO, "Have pointer arithmetic");
		SAFE_CLOSE(ret);
		return;
	}

	if (ret != -1)
		tst_brk(TBROK, "Invalid bpf() return value: %d", ret);

	if (log[0] != 0)
		tst_brk(TCONF | TERRNO, "No pointer arithmetic:\n %s", log);

	tst_brk(TBROK | TERRNO, "Failed to load program");
}

static int load_prog(void)
{
	const struct bpf_insn prog_insn[] = {
		/* r6 = 1 << 32
		 * r7 = -1
		 */
		BPF_LD_IMM64(BPF_REG_6, 1ULL << 32),
		BPF_MOV64_IMM(BPF_REG_7, -1LL),

		/* w7 /= w6 */
		BPF_ALU32_REG(BPF_DIV, BPF_REG_7, BPF_REG_6),

		/* map[1] = r6
		 * map[2] = r7
		 */
		BPF_MAP_ARRAY_STX(map_fd, 0, BPF_REG_6),
		BPF_MAP_ARRAY_STX(map_fd, 1, BPF_REG_7),

		/* r6 = 1 << 32
		 * r7 = -1
		 */
		BPF_LD_IMM64(BPF_REG_6, 1ULL << 32),
		BPF_MOV64_IMM(BPF_REG_7, -1LL),

		/* w7 %= w6 */
		BPF_ALU32_REG(BPF_MOD, BPF_REG_7, BPF_REG_6),

		/* map[3] = r6
		 * map[4] = r7
		 */
		BPF_MAP_ARRAY_STX(map_fd, 2, BPF_REG_6),
		BPF_MAP_ARRAY_STX(map_fd, 3, BPF_REG_7),

		/* exit(0) */
		BPF_MOV64_IMM(BPF_REG_0, 0),
		BPF_EXIT_INSN()
	};

	bpf_init_prog_attr(attr, prog_insn, sizeof(prog_insn), log, BUFSIZE);

	return bpf_load_prog(attr, log);
}

static void expect_reg_val(const char *const reg_name,
			   const uint64_t expected_val)
{
	bpf_map_array_get(map_fd, key, val);

	(*key)++;

	if (*val != expected_val) {
		tst_res(TFAIL,
			"%s = %"PRIu64", but should be %"PRIu64,
			reg_name, *val, expected_val);
	} else {
		tst_res(TPASS, "%s = %"PRIu64, reg_name, *val);
	}
}

static void setup(void)
{
	rlimit_bump_memlock();
	memcpy(msg, MSG, sizeof(MSG));
}

static void run(void)
{
	int prog_fd;

	map_fd = bpf_map_array_create(8);

	ensure_ptr_arithmetic();
	prog_fd = load_prog();
	bpf_run_prog(prog_fd, msg, sizeof(MSG));
	SAFE_CLOSE(prog_fd);

	*key = 0;

	tst_res(TINFO, "Check w7(-1) /= w6(0) [r7 = -1, r6 = 1 << 32]");
	expect_reg_val("src(r6)", 1ULL << 32);
	expect_reg_val("dst(r7)", 0);

	tst_res(TINFO, "Check w7(-1) %%= w6(0) [r7 = -1, r6 = 1 << 32]");
	expect_reg_val("src(r6)", 1ULL << 32);
	expect_reg_val("dst(r7)", (uint32_t)-1);

	SAFE_CLOSE(map_fd);
}

static struct tst_test test = {
	.timeout = 20,
	.setup = setup,
	.test_all = run,
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
		{"linux-git", "f6b1b3bf0d5f"},
		{"linux-git", "468f6eafa6c4"},
		{"linux-git", "e88b2c6e5a4d"},
		{"linux-git", "9b00f1b78809"},
		{"CVE", "CVE-2021-3444"},
		{}
	}
};
