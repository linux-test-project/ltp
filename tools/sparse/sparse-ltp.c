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

/* Check for LTP-003 and LTP-004
 *
 * Try to find cases where the static keyword was forgotten.
 */
static void check_symbol_visibility(const struct symbol *const sym)
{
	const unsigned long mod = sym->ctype.modifiers;
	const char *const name = show_ident(sym->ident);
	const int has_lib_prefix = !strncmp("tst_", name, 4) ||
		!strncmp("TST_", name, 4) ||
		!strncmp("ltp_", name, 4) ||
		!strncmp("safe_", name, 5);

	if (!(mod & MOD_TOPLEVEL))
		return;

	if (has_lib_prefix && (mod & MOD_STATIC)) {
		warning(sym->pos,
			"LTP-003: Symbol '%s' has the LTP public library prefix, but is static (private).",
			name);
		return;
	}

	if ((mod & MOD_STATIC))
		return;

	if (tu_kind == LTP_LIB && !has_lib_prefix) {
		warning(sym->pos,
			"LTP-003: Symbol '%s' is a public library function, but is missing the 'tst_' prefix",
			name);
		return;
	}

	if (sym->same_symbol)
		return;

	if (sym->ident == &main_ident)
		return;

	warning(sym->pos,
		"Symbol '%s' has no prototype or library ('tst_') prefix. Should it be static?",
		name);
}

/* AST level checks */
static void do_symbol_checks(struct symbol *sym)
{
	check_symbol_visibility(sym);
}

/* Compile the AST into a graph of basicblocks */
static void process_symbols(struct symbol_list *list)
{
	struct symbol *sym;

	FOR_EACH_PTR(list, sym) {
		struct entrypoint *ep;

		do_symbol_checks(sym);

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
