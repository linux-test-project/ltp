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

/* The old API can not comply with the rules. So when we see one of
 * these symbols we know that it will result in further
 * warnings. Probably these will suggest inappropriate things. Usually
 * these symbols should be removed and the new API used
 * instead. Otherwise they can be ignored until all tests have been
 * converted to the new API.
 */
static bool check_symbol_deprecated(const struct symbol *const sym)
{
	static struct ident *TCID_id, *TST_TOTAL_id;
	const struct ident *id = sym->ident;

	if (!TCID_id) {
		TCID_id = built_in_ident("TCID");
		TST_TOTAL_id = built_in_ident("TST_TOTAL");
	}

	if (id != TCID_id && id != TST_TOTAL_id)
		return false;

	warning(sym->pos,
		"Ignoring deprecated API symbol: '%s'. Should this code be converted to the new API?",
		show_ident(id));

	return true;
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

	if (has_lib_prefix && (mod & MOD_STATIC) && !(mod & MOD_INLINE)) {
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

/* See base_type() in dissect.c */
static struct symbol *unwrap_base_type(const struct symbol *sym)
{
	switch (sym->ctype.base_type->type) {
	case SYM_ARRAY:
	case SYM_NODE:
	case SYM_PTR:
		return unwrap_base_type(sym->ctype.base_type);
	default:
		return sym->ctype.base_type;
	}
}

/* Checks if some struct array initializer is terminated with a blank
 * (zeroed) item i.e. {}
 */
static bool is_terminated_with_null_struct(const struct symbol *const sym)
{
	const struct expression *const arr_init = sym->initializer;
	const struct expression *item_init =
		last_ptr_list((struct ptr_list *)arr_init->expr_list);

	if (item_init->type == EXPR_POS)
		item_init = item_init->init_expr;

	if (item_init->type != EXPR_INITIALIZER)
		return false;

	return ptr_list_empty((struct ptr_list *)item_init->expr_list);
}

/* LTP-005: Check array sentinel value
 *
 * This is most important for the tags array. It is only accessed when
 * the test fails. So we perform a static check to ensure it ends with
 * {}.
 */
static void check_struct_array_initializer(const struct symbol *const sym)
{
	if (is_terminated_with_null_struct(sym))
		return;

	warning(sym->pos,
		"LTP-005: Struct array doesn't appear to be null-terminated; did you forget to add '{}' as the final entry?");
}

/* Find struct tst_test test = { ... } and perform tests on its initializer */
static void check_test_struct(const struct symbol *const sym)
{
	static struct ident *tst_test, *tst_test_test;
	struct ident *ctype_name = NULL;
	struct expression *init = sym->initializer;
	struct expression *entry;

	if (!sym->ctype.base_type)
		return;

	ctype_name = sym->ctype.base_type->ident;

	if (!init)
		return;

	if (!tst_test_test) {
		tst_test = built_in_ident("tst_test");
		tst_test_test = built_in_ident("test");
	}

	if (sym->ident != tst_test_test)
		return;

	if (ctype_name != tst_test)
		return;

	FOR_EACH_PTR(init->expr_list, entry) {
		if (entry->init_expr->type != EXPR_SYMBOL)
			continue;

		switch (entry->ctype->ctype.base_type->type) {
		case SYM_PTR:
		case SYM_ARRAY:
			break;
		default:
			return;
		}

		const struct symbol *entry_init = entry->init_expr->symbol;
		const struct symbol *entry_ctype = unwrap_base_type(entry_init);

		if (entry_ctype->type == SYM_STRUCT)
			check_struct_array_initializer(entry_init);
	} END_FOR_EACH_PTR(entry);

}

/* AST level checks */
static void do_symbol_checks(struct symbol *sym)
{
	if (check_symbol_deprecated(sym))
		return;

	check_symbol_visibility(sym);
	check_test_struct(sym);
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
