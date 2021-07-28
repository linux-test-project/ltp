// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2021 SUSE LLC <rpalethorpe@suse.com>
 *
 * Sparse allows us to perform checks on the AST (struct symbol) or on
 * a linearized representation. In the latter case we are given a set
 * of entry points (functions) containing basic blocks of
 * instructions.
 *
 * The basic blocks contain byte code in SSA form. This is similar to
 * the intermediate representation most compilers use during
 * optimisation.
 */
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>

#include "lib.h"
#include "allocate.h"
#include "opcode.h"
#include "token.h"
#include "parse.h"
#include "symbol.h"
#include "expression.h"
#include "linearize.h"

/* The rules for test, library and tool code are different */
enum ltp_tu_kind {
	LTP_LIB,
	LTP_OTHER,
};

static enum ltp_tu_kind tu_kind = LTP_OTHER;

/* Check for LTP-002
 *
 * Inspects the destination symbol of each store instruction. If it is
 * TST_RET or TST_ERR then emit a warning.
 */
static void check_lib_sets_TEST_vars(const struct instruction *insn)
{
	static struct ident *TST_RES_id, *TST_ERR_id;

	if (!TST_RES_id) {
		TST_RES_id = built_in_ident("TST_RET");
		TST_ERR_id = built_in_ident("TST_ERR");
	}

	if (insn->opcode != OP_STORE)
		return;
	if (insn->src->ident != TST_RES_id &&
	    insn->src->ident != TST_ERR_id)
		return;

	warning(insn->pos,
		"LTP-002: Library should not write to TST_RET or TST_ERR");
}

static void do_basicblock_checks(struct basic_block *bb)
{
	struct instruction *insn;

	FOR_EACH_PTR(bb->insns, insn) {
		if (!bb_reachable(insn->bb))
			continue;

		if (tu_kind == LTP_LIB)
			check_lib_sets_TEST_vars(insn);
	} END_FOR_EACH_PTR(insn);
}

static void do_entrypoint_checks(struct entrypoint *ep)
{
	struct basic_block *bb;

	FOR_EACH_PTR(ep->bbs, bb) {
		do_basicblock_checks(bb);
	} END_FOR_EACH_PTR(bb);
}

/* Compile the AST into a graph of basicblocks */
static void process_symbols(struct symbol_list *list)
{
	struct symbol *sym;

	FOR_EACH_PTR(list, sym) {
		struct entrypoint *ep;

		expand_symbol(sym);
		ep = linearize_symbol(sym);
		if (!ep || !ep->entry)
			continue;

		do_entrypoint_checks(ep);

		if (dbg_entry)
			show_entry(ep);
	} END_FOR_EACH_PTR(sym);
}

static void collect_info_from_args(const int argc, char *const *const argv)
{
	int i;

	for (i = 0; i < argc; i++) {
		if (!strcmp("-DLTPLIB", argv[i]))
			tu_kind = LTP_LIB;
	}
}

int main(int argc, char **argv)
{
	struct string_list *filelist = NULL;
	char *file;

	Waddress_space = 0;
	Wbitwise = 0;
	Wcast_truncate = 0;
	Wcontext = 0;
	Wdecl = 0;
	Wexternal_function_has_definition = 0;
	Wflexible_array_array = 0;
	Wimplicit_int = 0;
	Wint_to_pointer_cast = 0;
	Wmemcpy_max_count = 0;
	Wnon_pointer_null = 0;
	Wone_bit_signed_bitfield = 0;
	Woverride_init = 0;
	Wpointer_to_int_cast = 0;
	Wvla = 0;

	do_output = 0;

	collect_info_from_args(argc, argv);

	process_symbols(sparse_initialize(argc, argv, &filelist));
	FOR_EACH_PTR(filelist, file) {
		process_symbols(sparse(file));
	} END_FOR_EACH_PTR(file);

	report_stats();
	return 0;
}
