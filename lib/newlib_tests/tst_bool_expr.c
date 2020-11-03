// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * Basic unit test for the bolean expression parser and evaluator.
 */

#include <string.h>
#include <stdio.h>
#include "tst_test.h"
#include "tst_bool_expr.h"

static int a, b, c;

static int map(struct tst_expr_tok *var)
{
	if (!strncmp(var->tok, "A", var->tok_len))
		return a;

	if (!strncmp(var->tok, "B", var->tok_len))
		return b;

	if (!strncmp(var->tok, "C", var->tok_len))
		return c;

	if (!strncmp(var->tok, "True", var->tok_len))
		return 1;

	if (!strncmp(var->tok, "False", var->tok_len))
		return 0;

	return -1;
}

static void parse_fail(const char *expr)
{
	struct tst_expr *res;

	tst_res(TINFO, "Parsing '%s'", expr);

	res = tst_bool_expr_parse(expr);

	if (res) {
		printf("In RPN: ");
		tst_bool_expr_print(stdout, res);
		printf("\n");
		tst_bool_expr_free(res);
		tst_res(TFAIL, "Expression was parsed");
	} else {
		tst_res(TPASS, "Parser returned an error");
	}
}

static void do_eval_test(const char *expr_str, int set_a, int set_b, int set_c, int exp_res)
{
	struct tst_expr *expr;
	int res;

	a = set_a;
	b = set_b;
	c = set_c;

	tst_res(TINFO, "'%s' A=%i B=%i C=%i == %i", expr_str, a, b, c, exp_res);

	expr = tst_bool_expr_parse(expr_str);

	if (!expr) {
		tst_res(TFAIL, "Parser returned error");
		return;
	}

	printf("In RPN: ");
	tst_bool_expr_print(stdout, expr);
	printf("\n");

	res = tst_bool_expr_eval(expr, map);

	if (res == exp_res)
		tst_res(TPASS, "Got %i", res);
	else
		tst_res(TFAIL, "Got %i", res);

	tst_bool_expr_free(expr);
}

static void do_test(void)
{
	do_eval_test("(A | B) & !!C", 0, 0, 0, 0);
	do_eval_test("(A | B) & !!C", 1, 0, 1, 1);
	do_eval_test("!A & B", 1, 0, 0, 0);
	do_eval_test("!A & B", 0, 1, 0, 1);
	do_eval_test("A & !B", 1, 0, 0, 1);
	do_eval_test("!!A & !!B", 0, 1, 0, 0);
	do_eval_test("!!A & !!B", 1, 1, 0, 1);
	do_eval_test("!(A & B) & C", 1, 1, 0, 0);
	do_eval_test("A & (B | C)", 1, 1, 0, 1);
	do_eval_test("A & B | C", 1, 1, 0, 1);
	do_eval_test("((((A)))&(B))", 1, 1, 0, 1);
	do_eval_test("   A  \t", 0, 0, 0, 0);
	do_eval_test("False & A", 1, 0, 0, 0);
	do_eval_test("! Undefined", 0, 0, 0, -1);

	parse_fail("A!");
	parse_fail("A &");
	parse_fail("A B");
	parse_fail("A ) B");
	parse_fail("A ( B");
	parse_fail("A ( B )");
	parse_fail("A |");
	parse_fail("A ! B");
	parse_fail("A! & B");
	parse_fail("A & | B");
	parse_fail("A & (B |)");
	parse_fail("A & ( | B)");
	parse_fail("A & B &");
	parse_fail("((A )");
	parse_fail("& A");
	parse_fail("! &");
	parse_fail(")");
	parse_fail("| A");
	parse_fail("");
}

static struct tst_test test = {
	.test_all = do_test,
};
