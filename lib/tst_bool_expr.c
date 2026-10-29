// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Cyril Hrubis <chrubis@suse.cz>
 */
/*
 * Boolean expression is evaluated in three steps.
 *
 * First of all the string containing the expression is tokenized. The
 * tokenizer runs twice and we only count number of tokens in the first pass in
 * order to simplify the memory allocation.
 *
 * Secondly the expression is transformed to a postfix (RPN) notation by
 * the shunting yard algorithm and the correctness of the expression is checked
 * during the transformation as well. The fact that parenthesis are matched is
 * asserted by the shunting yard algorithm itself while the rest is checked
 * simply by checking if the preceding token is in a set of allowed tokens.
 * This could be thought of as a simple open-coded state machine.
 *
 * An expression in the RPN form can be evaluated given a variable mapping
 * function. The evaluation ignores most of errors because invalid expression
 * will be rejected in the second step.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "tst_bool_expr.h"

static int char_to_op(char c)
{
	switch (c) {
	case '(':
		return TST_OP_LPAR;
	case ')':
		return TST_OP_RPAR;
	case '&':
		return TST_OP_AND;
	case '|':
		return TST_OP_OR;
	case '!':
		return TST_OP_NOT;
	default:
		return TST_OP_VAR;
	}
}

static int new_tok(struct tst_expr_tok **last, const char *tok, size_t tok_len)
{
	if (!tok_len)
		return 0;

	if (!(*last))
		return 1;

	(*last)->tok = tok;
	(*last)->tok_len = tok_len;
	(*last)->op = char_to_op(tok[0]);
	(*last)->priv = NULL;
	(*last)++;

	return 1;
}

static unsigned int tokenize(const char *expr, struct tst_expr_tok *last)
{
	size_t i, j;
	unsigned int token_cnt = 0;

	for (j = i = 0; expr[i]; i++) {
		switch (expr[i]) {
		case '(':
		case ')':
		case '!':
		case '&':
		case '|':
			token_cnt += new_tok(&last, &expr[j], i - j);
			token_cnt += new_tok(&last, &expr[i], 1);
			j = i+1;
		break;
		case '\t':
		case ' ':
			token_cnt += new_tok(&last, &expr[j], i - j);
			j = i+1;
		break;
		case '"':
			while (expr[i+1] != '"' && expr[i+1])
				i++;

			if (expr[i+1] == '"')
				i++;
		break;
		default:
		break;
		}
	}

	token_cnt += new_tok(&last, &expr[j], i - j);

	return token_cnt;
}

void tst_bool_expr_print(FILE *f, struct tst_expr *expr)
{
	struct tst_expr_tok *i;
	size_t j;

	for (i = expr->rpn; i; i = i->next) {
		for (j = 0; j < i->tok_len; j++)
			putc(i->tok[j], f);

		if (i->next)
			putc(' ', f);
	}
}

static void print_spaces(FILE *f, unsigned int spaces)
{
	while (spaces--)
		putc(' ', f);
}

static void tst_bool_expr_err(FILE *f, struct tst_expr *expr, unsigned int cnt)
{
	unsigned int i, spaces, err_len;
	const char *err;

	fprintf(f, "%s", expr->buf->tok);
	fprintf(f, "\n");

	for (i = 0; i < cnt; i++) {
		if (expr->buf[i].priv)
			break;
	}

	spaces = expr->buf[i].tok - expr->buf[0].tok;
	err = expr->buf[i].priv;
	err_len = strlen(err);

	print_spaces(f, spaces);

	fprintf(f, "^\n");

	if (err_len < spaces)
		print_spaces(f, spaces - err_len + 1);

	fprintf(f, "%s\n", err);
}

static inline void stack_push(struct tst_expr_tok *stack[], unsigned int *op_stack_pos,
                              struct tst_expr_tok *op)
{
	stack[(*op_stack_pos)++] = op;
}

static inline int stack_empty(unsigned int op_stack_pos)
{
	return op_stack_pos == 0;
}

static inline struct tst_expr_tok *stack_pop(struct tst_expr_tok *stack[],
                                             unsigned int *op_stack_pos)
{
	if (stack_empty(*op_stack_pos))
		return NULL;

	return stack[--(*op_stack_pos)];
}

#define TST_OP_NONE -1

static inline int stack_peek_op(struct tst_expr_tok *stack[],
                                unsigned int op_stack_pos)
{
	if (stack_empty(op_stack_pos))
		return TST_OP_NONE;

	return stack[op_stack_pos - 1]->op;
}

/*
 * This is a complete list of which tokens can happen before any of:
 *  - variable
 *  - left parentesis
 *  - negation
 *
 * The -1 represents start of the expression.
 */
static inline int check_one(int op)
{
	switch (op) {
	case TST_OP_AND:
	case TST_OP_OR:
	case TST_OP_NOT:
	case TST_OP_NONE:
	case TST_OP_LPAR:
		return 0;
	default:
		return 1;
	}
}

/*
 * And this checks for tokens that can happen before any of:
 * - right parentesis
 * - and
 * - or
 *
 * This is also used to check that the last token in expression is correct one.
 */
static inline int check_two(int op)
{
	switch (op) {
	case TST_OP_VAR:
	case TST_OP_RPAR:
		return 1;
	default:
		return 0;
	}
}

static int shunting_yard(struct tst_expr *expr, unsigned int cnt)
{
	struct tst_expr_tok *op_stack[cnt];
	unsigned int op_stack_pos = 0;
	struct tst_expr_tok *out[cnt + 1];
	unsigned int out_pos = 0;
	struct tst_expr_tok *i;
	unsigned int j;
	int prev_op = TST_OP_NONE;

	for (i = expr->buf; i < &(expr->buf[cnt]); i++) {
		switch (i->op) {
		case TST_OP_VAR:
			if (check_one(prev_op)) {
				i->priv = "Expected operation";
				goto err;
			}

			stack_push(out, &out_pos, i);

			while (stack_peek_op(op_stack, op_stack_pos) == TST_OP_NOT)
				stack_push(out, &out_pos, stack_pop(op_stack, &op_stack_pos));
		break;
		case TST_OP_LPAR:
			if (check_one(prev_op)) {
				i->priv = "Expected operation";
				goto err;
			}

			stack_push(op_stack, &op_stack_pos, i);
		break;
		case TST_OP_RPAR:
			if (!check_two(prev_op)) {
				i->priv = "Expected variable or )";
				goto err;
			}

			/* pop everything till ( */
			for (;;) {
				struct tst_expr_tok *op = stack_pop(op_stack, &op_stack_pos);

				if (!op) {
					i->priv = "Missing (";
					goto err;
				}

				if (op->op == TST_OP_LPAR)
					break;

				stack_push(out, &out_pos, op);
			}

			while (stack_peek_op(op_stack, op_stack_pos) == TST_OP_NOT)
				stack_push(out, &out_pos, stack_pop(op_stack, &op_stack_pos));
		break;
		case TST_OP_NOT:
			if (check_one(prev_op)) {
				i->priv = "Expected operation";
				goto err;
			}
			stack_push(op_stack, &op_stack_pos, i);
		break;
		case TST_OP_AND:
		case TST_OP_OR:
			if (!check_two(prev_op)) {
				i->priv = "Expected variable or (";
				goto err;
			}

			/*
			 * There can be at most one binary op on the stack
			 * since we pop the one present on the stack before we
			 * attempt to push new one they so never accumulate.
			 */
			switch (stack_peek_op(op_stack, op_stack_pos)) {
			case TST_OP_AND:
			case TST_OP_OR:
				stack_push(out, &out_pos, stack_pop(op_stack, &op_stack_pos));
			break;
			}

			stack_push(op_stack, &op_stack_pos, i);
		break;
		}

		prev_op = i->op;
	}

	if (!check_two(expr->buf[cnt-1].op)) {
		expr->buf[cnt-1].priv = "Unfinished expression";
		goto err;
	}

	/* pop remaining operations */
	while ((i = stack_pop(op_stack, &op_stack_pos))) {
		if (i->op == TST_OP_LPAR) {
			i->priv = "Missing )";
			goto err;
		}

		stack_push(out, &out_pos, i);
	}

	/* construct the list */
	out[out_pos] = NULL;

	for (j = 0; j < out_pos; j++)
		out[j]->next = out[j + 1];

	expr->rpn = out[0];

	return 0;
err:
	return 1;
}

struct tst_expr *tst_bool_expr_parse(const char *expr)
{
	struct tst_expr *ret;
	unsigned int tok_cnt = tokenize(expr, NULL);

	if (!tok_cnt)
		return NULL;

	ret = malloc(sizeof(struct tst_expr) + sizeof(struct tst_expr_tok) * tok_cnt);
	if (!ret)
		return NULL;

	tokenize(expr, ret->buf);

	if (shunting_yard(ret, tok_cnt)) {
		tst_bool_expr_err(stderr, ret, tok_cnt);
		tst_bool_expr_free(ret);
		return NULL;
	}

	return ret;
}

#define MAX_STACK 16

int tst_bool_expr_eval(struct tst_expr *expr,
                       int (*map)(struct tst_expr_tok *var))
{
	struct tst_expr_tok *i;
	int stack[MAX_STACK];
	int pos = -1;

	for (i = expr->rpn; i; i = i->next) {
		switch (i->op) {
		case TST_OP_NOT:
			stack[pos] = !stack[pos];
		break;
		case TST_OP_OR:
			stack[pos-1] = stack[pos] || stack[pos-1];
			pos--;
		break;
		case TST_OP_AND:
			stack[pos-1] = stack[pos] && stack[pos-1];
			pos--;
		break;
		case TST_OP_VAR:
			if (pos + 1 >= MAX_STACK) {
				fprintf(stderr, "Eval out of stack!\n");
				return -1;
			}

			stack[++pos] = map(i);

			/* map reported undefined variable -> abort */
			if (stack[pos] == -1)
				return -1;
		break;
		/* does not happen */
		default:
		break;
		}
	}

	return stack[0];
}

void tst_bool_expr_free(struct tst_expr *expr)
{
	free(expr);
}
