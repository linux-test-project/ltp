// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef TST_BOOL_EXPR_H__
#define TST_BOOL_EXPR_H__

enum tst_op {
	TST_OP_NOT,
	TST_OP_AND,
	TST_OP_OR,
	TST_OP_VAR,
	/* Used only internally */
	TST_OP_LPAR,
	TST_OP_RPAR,
};

struct tst_expr_tok {
	enum tst_op op;
	const char *tok;
	size_t tok_len;
	struct tst_expr_tok *next;
	const void *priv;
};

struct tst_expr {
	struct tst_expr_tok *rpn;
	struct tst_expr_tok buf[];
};

/*
 * Parses an boolean expression and returns a simplified RPN version.
 *
 * If expression is not valid the call prints error into stderr and returns
 * NULL. On success pointer to an expression is returned which can be evaluated
 * by the tst_bool_expr_eval() function and has to be later freed by the
 * caller.
 *
 * The boolean expression can consists of:
 *
 * - unary negation opeartion !
 * - two binary operations & and |
 * - correct sequence of parentheses ()
 * - strings that are treated as boolean variables
 *
 *  e.g. '(A | B) & C' or 'Variable_1 & Variable_2' are both a valid boolean
 *  expressions.
 *
 *  @expr String containing a boolean expression to be parsed.
 *  @return Pointer to an RPN expression.
 */
struct tst_expr *tst_bool_expr_parse(const char *expr);

/*
 * Prints an string representation of the expression into a FILE.
 *
 * @param A FILE to print to.
 * @expr An expression to print.
 */
void tst_bool_expr_print(FILE *f, struct tst_expr *expr);

/*
 * Evaluates an expression given a map for variables.
 *
 * The call will fail if:
 * - map function returns -1 which indicates undefined variable
 * - the eval function runs out of stack
 *
 * @param expr Boolean expression in RPN.
 * @param map Mapping function for boolean variables.
 *
 * @return Returns 0 or 1 if expression was evaluated correctly and -1 on error.
 */
int tst_bool_expr_eval(struct tst_expr *expr,
                       int (*map)(struct tst_expr_tok *var));

/*
 * Frees the memory allocated by the tst_bool_expr_parse().
 *
 * @param Boolean expression.
 */
void tst_bool_expr_free(struct tst_expr *expr);

#endif	/* TST_BOOL_EXPR_H__ */
