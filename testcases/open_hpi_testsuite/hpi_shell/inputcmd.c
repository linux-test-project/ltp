/*      -*- linux-c -*-
 *
 * Copyright (c) 2005 by Intel Corp.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *	Kouzmich	< Mikhail.V.Kouzmich@intel.com >
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <termios.h>
#include <fcntl.h>
#include "hpi_cmd.h"

#define CTRL_A_KEY		0x01
#define CTRL_B_KEY		0x02
#define CTRL_D_KEY		0x04
#define CTRL_E_KEY		0x05
#define CTRL_F_KEY		0x06
#define CTRL_G_KEY		0x07
#define BELL_KEY		0x07
#define CTRL_H_KEY		0x08
#define TAB_KEY			0x09
#define NL_KEY			0x0A
#define CTRL_K_KEY		0x0B
#define CTRL_L_KEY		0x0C
#define CTRL_N_KEY		0x0E
#define CTRL_R_KEY		0x12
#define CTRL_S_KEY		0x13
#define CTRL1_KEY		0x1B
#define CTRL2_KEY		0x5B
#define BACKSP_KEY		0x7F

#define INSERT_KEY		0x32
#define DELETE_KEY		0x33
#define PGUP_KEY		0x35
#define PGDOWN_KEY		0x36
#define UP_KEY			0x41
#define DOWN_KEY		0x42
#define RIGHT_KEY		0x43
#define LEFT_KEY		0x44
#define END_KEY			0x46
#define HOME_KEY		0x48

#define HISTORY_DELTA		5

typedef struct {
	int	n_items;
	int	comp_len;
	char	**items;
} compl_t;

compl_t	complition_struct;
int	termfd = -1;

static char	clear_buf[READ_BUF_SIZE];
static int	no_stty = 1;
static struct termios saved_termio;
static int	is_insert_key = 0;
static char	**History;
static int	hist_ind = 0;
static int	hist_size = 0;
static int	current_hist_ind = -1;

void init_history(void)
{
	History = (char **)malloc(sizeof(char *) * HISTORY_DELTA);
	hist_size = HISTORY_DELTA;
	memset(History, 0, sizeof(char *) * HISTORY_DELTA);
	current_hist_ind = -1;
	hist_ind = 0;
	complition_struct.n_items = 0;
}

static void get_history_new(int new_cmd)
{
	char	**tmp;

	if (current_hist_ind < 0) new_cmd = 1;
	if ((current_hist_ind >= 0) &&
		(*(History[current_hist_ind]) == 0)) {
		hist_ind = current_hist_ind;
		return;
	};
	if (new_cmd) current_hist_ind++;
	else return;
	if (current_hist_ind >= hist_size) {
		hist_size += HISTORY_DELTA;
		tmp = (char **)malloc(sizeof(char *) * hist_size);
		memset(tmp, 0, sizeof(char *) * hist_size);
		if (current_hist_ind > 1) {
			memcpy(tmp, History,
				sizeof(char *) * current_hist_ind);
			free(History);
		};
		History = tmp;
	};
	hist_ind = current_hist_ind;
	History[current_hist_ind] = (char *)malloc(1);
	*(History[current_hist_ind]) = 0;
}

static void add_to_history(char *line, int index)
{
	if (line == (char *)NULL) return;
	if (strlen(line) == 0) return;
	if (index > current_hist_ind) return;
	if(strcmp(line, History[index]) == 0) return;
	free(History[index]);
	History[index] = strdup(line);
}

static char *get_history_next(char *str)
{
	add_to_history(str, hist_ind);
	if (current_hist_ind > hist_ind) hist_ind++;
	else printf("%c", BELL_KEY);
	return(History[hist_ind]);
}

static char *get_history_prev(char *str)
{
	
	add_to_history(str, hist_ind);
	hist_ind--;
	if (hist_ind < 0) {
		hist_ind = 0;
		printf("%c", BELL_KEY);
	};
	return(History[hist_ind]);
}

static void go_to_begin(int index)
{
	while (index-- > 0) printf("%c", '\b');
}

static int print_str_by_index(char *buf, int index, int cursor_pos)
// index - current cursor position
// cursor_pos - new cursor position,
//	if cursor_pos == -1 set to the end of the buf
//     return value: new cursor position
{
	int	n;

	n = strlen(buf) - index;
	if (n > 0)
		printf("%s", buf + index);
	if ((cursor_pos == -1) || (cursor_pos > strlen(buf)))
		cursor_pos = strlen(buf);
	n = strlen(buf) - cursor_pos;
	go_to_begin(n);
	return(cursor_pos);
}

static int clear_line(int index, int length)
{
	go_to_begin(index);
	memset(clear_buf, ' ', length);
	clear_buf[length] = 0;
	return(print_str_by_index(clear_buf, 0, 0));
}

static int add_char(char *buf, int length, int c, int index)
//    return value : new corsor position
{
	int	i;

	if (index >= length) {
		buf[length++] = c;
		return(length);
	};
	if ( ! is_insert_key)
		for (i = length; i > index; i--) buf[i] = buf[i - 1];
	buf[index] = c;
	i = (is_insert_key) ? length - 1 : length;
	print_str_by_index(buf, index, index + 1);
	return(index + 1);
}

static int delete_char(char *buf, int length, int index, int as)
// as = 0 - backspace key, 1 - delete key
//    return value : new corsor position
{
	int	n, ind;

	if (index < 0) return(0);
	if ((index == length) && as) return(length);
	ind = (as) ? index : index - 1;
	memcpy(buf + ind, buf + ind + 1, length - ind - 1);
	buf[length - 1] = ' ';
	if (as == 0) printf("%c", '\b');
	n = print_str_by_index(buf, ind, ind);
	buf[length - 1] = 0;
	return(n);
}

static int find_cmd_by_text(char *text, int current_index, int forward)
{
	int	i, len, is_cmp = 0;

	len = strlen(text);
	if (len == 0) return(-1);
	for (i = current_index; (i >= 0) && (i <= current_hist_ind);) {
		if (strncmp(History[i], text, len) == 0) {
			is_cmp = 1;
			break;
		};
		if (forward) i++;
		else i--;
	};
	if (is_cmp) return(i);
	return(-1);
}

static int find_command(char *line, int curr_index, int forward)
{
	int	res, cmd_ind, len, ind, c, line_size;
	char	str[READ_BUF_SIZE];
	char	text[READ_BUF_SIZE];

	len = strlen(line) + strlen(Title);
	clear_line(len, len);
	if (forward) {
		if (curr_index == current_hist_ind) {
			printf("%c", BELL_KEY);
			return(curr_index);
		}
	} else {
		if (curr_index <= 0) {
			printf("%c", BELL_KEY);
			return(0);
		}
	};
	memset(text, 0, READ_BUF_SIZE);
	len = 0;
	ind = 0;
	cmd_ind = curr_index;
	for (;;) {
		line_size = ind + strlen(History[cmd_ind]);
		if (forward)
			res = find_cmd_by_text(text, cmd_ind, 1);
		else
			res = find_cmd_by_text(text, cmd_ind, 0);
		if (res != -1)
			cmd_ind = res;
		else
			printf("%c", BELL_KEY);
		if (forward)
			snprintf(str, READ_BUF_SIZE,
				"(i-search)`%s': ", text);
		else
			snprintf(str, READ_BUF_SIZE,
				"(revers-i-search)`%s': ", text);
		clear_line(ind, line_size);
		ind = print_str_by_index(str, 0, -1);
		print_str_by_index(History[cmd_ind], 0, 0);
		c = getchar();
		if (c == BACKSP_KEY) {
			len--;
			if (len < 0) len = 0;
			text[len] = 0;
		} else if ((c < ' ') || (c > 'z')) {
			ungetc(c, stdin);
			break;
		};
		text[len++] = c;
		if (len >= READ_BUF_SIZE) break;
	};
	res = ind + strlen(History[cmd_ind]);
	clear_line(ind, res);
	return(cmd_ind);
}

static void check_compl(compl_t *compl_def)
{
	int	i, j, len;
	char	*str;

	if (compl_def->n_items == 0) {
		compl_def->comp_len = 0;
		return;
	};
	if (compl_def->n_items == 1) {
		compl_def->comp_len = strlen(compl_def->items[0]);
		return;
	};
	str = compl_def->items[0];
	len = strlen(str);
	for (i = 1; i < compl_def->n_items; i++) {
		for (j = len; j > 0; j--) {
			if (strncmp(str, compl_def->items[i], j) == 0)
				break;
		};
		if (j == 0) {
			compl_def->comp_len = 0;
			return;
		};
		len = j;
	};
	compl_def->comp_len = len;
}

static void add_to_compl(char *text, compl_t *compl_def)
{
	char	**tmp;
	int	n;

	n = compl_def->n_items + 1;
	tmp = (char **)malloc(sizeof(char *) * n);
	if (compl_def->n_items > 0) {
		memcpy(tmp, compl_def->items,
			sizeof(char *) * compl_def->n_items);
		free(compl_def->items);
	};
	compl_def->items = tmp;
	tmp[compl_def->n_items] = strdup(text);
	compl_def->n_items = n;
}

static int completion_func(int compl_type, char *text, compl_t *compl_def)
{
	int		i, len;
	command_def_t	*cmd = NULL;

	if (compl_def == (compl_t *)NULL) return(0);
	if (compl_def->n_items > 0) {
		for (i = 0; i < compl_def->n_items; i++)
			free(compl_def->items[i]);
		free(compl_def->items);
		compl_def->n_items = 0;
	};
	compl_def->comp_len = 0;
	len = strlen(text);
	switch (compl_type) {
		case COMPL_CMD:
			for (cmd = commands; cmd->cmd != NULL; cmd++) {
				if ((cmd->type != MAIN_COM) &&
					(cmd->type != block_type) &&
					(cmd->type != UNDEF_COM))
					continue;
				if (strncmp(text, cmd->cmd, len) == 0)
					add_to_compl(cmd->cmd, compl_def);
			};
			break;
		case COMPL_NULL:
			return(0);
	};
	check_compl(compl_def);
	return(compl_def->n_items);
}

static int set_term_flags(void)
{
	int		res, c;
	char		name[1024];
	struct termios	termio;

	if (no_stty == 0) return(0);
	ctermid(name);
	termfd = open(name, O_RDWR);
	if (termfd < 0) {
		printf("Can not open terminal\n");
		return(1);
	};
	c = tcgetattr(termfd, &saved_termio);
	if (c != 0) {
		printf("Can not read terminal attrs\n");
		return(1);
	};
	termio = saved_termio;
	c = ICANON | ECHO | ECHOCTL;
	c = ~c;
	termio.c_lflag &= c;
	termio.c_cc[VMIN] = 1;
	termio.c_cc[VTIME] = 0;
	res = tcsetattr(termfd, TCSANOW, &termio);
	no_stty = 0;
	return(0);
}

void restore_term_flags(void)
{
	if (no_stty) return;
	tcsetattr(termfd, TCSANOW, &saved_termio);
	no_stty = 1;
}

char *get_command_line(int new_cmd, int comp_type)
{
	int	c, ind = 0, len = 0, res;
	char	input_buf[READ_BUF_SIZE];
	char	*str;

	if (set_term_flags() != 0)
		exit(1);
	get_history_new(new_cmd);
	memset(input_buf, 0, READ_BUF_SIZE);
	for (;;) {
		c = getchar();
		len = strlen(input_buf);
		if (len >= (READ_BUF_SIZE - 1)) c = NL_KEY;
		switch (c) {
		case CTRL_A_KEY:
			go_to_begin(ind);
			ind = print_str_by_index(input_buf, 0, 0);
			break;
		case CTRL_B_KEY:
			printf("%c", '\b');
			ind--;
			if (ind < 0) {
				ind = 0;
				printf("%c", ' ');
			};
			break;
		case CTRL_D_KEY:
			if (ind == len) break;
			ind = delete_char(input_buf, len, ind, 1);
			break;
		case CTRL_E_KEY:
			ind = print_str_by_index(input_buf, ind, len);
			break;
		case CTRL_G_KEY:
		case CTRL_L_KEY:
			printf("%c", c);
			break;
		case CTRL_F_KEY:
			ind = print_str_by_index(input_buf, ind, ind + 1);
			break;
		case TAB_KEY:
			res = completion_func(comp_type, input_buf,
				&complition_struct);
			if (res == 0) break;
			if (res == 1) {
				strcpy(input_buf, complition_struct.items[0]);
				strcat(input_buf, " ");
				ind = print_str_by_index(input_buf, ind, -1);
				break;
			};
			memset(input_buf, 0, READ_BUF_SIZE);
			strncpy(input_buf, complition_struct.items[0],
				complition_struct.comp_len);
			ind = print_str_by_index(input_buf, ind, -1);
			break;
		case NL_KEY:
			printf("%c", c);
			if (current_hist_ind != hist_ind)
				add_to_history(input_buf, hist_ind);
			if (strlen(input_buf) == 0) return("");
			add_to_history(input_buf, current_hist_ind);
			return(History[current_hist_ind]);
		case CTRL_K_KEY:
			clear_line(ind, len);
			memset(input_buf + ind, 0, len - ind);
			print_str_by_index(input_buf, 0, ind);
			break;
		case CTRL_N_KEY:
			ungetc(DOWN_KEY, stdin);
			ungetc(CTRL2_KEY, stdin);
			ungetc(CTRL1_KEY, stdin);
			break;
		case CTRL_R_KEY:
		case CTRL_S_KEY:
			res = find_command(input_buf, hist_ind,
				(c == CTRL_S_KEY));
			if (res != hist_ind) {
				hist_ind = res;
				memset(input_buf, 0, READ_BUF_SIZE);
				strcpy(input_buf, History[hist_ind]);
			};
			print_str_by_index(Title, 0, -1);
			ind = print_str_by_index(input_buf, 0, -1);
			break;
		case CTRL1_KEY:
			c = getchar();
			if (c != CTRL2_KEY) break;
			c = getchar();
			switch (c) {
			case INSERT_KEY:
				getchar();
				is_insert_key = (is_insert_key) ? 0 : 1;
				break;
			case DELETE_KEY:
				getchar();
				if (ind == len) break;
				ind = delete_char(input_buf, len, ind, 1);
				break;
			case LEFT_KEY:
				printf("%c", '\b');
				ind--;
				if (ind < 0) {
					ind = 0;
					printf("%c", ' ');
				};
				break;
			case RIGHT_KEY:
				ind++;
				if (ind > len) ind = len;
				else print_str_by_index(input_buf, ind - 1, ind);
				break;
			case UP_KEY:
			case DOWN_KEY:
				clear_line(ind, len);
				if (c == UP_KEY)
					str = get_history_prev(input_buf);
				else
					str = get_history_next(input_buf);
				memset(input_buf, 0, READ_BUF_SIZE);
				strcpy(input_buf, str);
				len = strlen(input_buf);
				ind = print_str_by_index(input_buf, 0, len);
				break;
			case PGUP_KEY:
			case PGDOWN_KEY:
				getchar();
				add_to_history(input_buf, hist_ind);
				clear_line(ind, len);
				hist_ind = (c == PGUP_KEY) ? 0 : current_hist_ind;
				str = History[hist_ind];
				memset(input_buf, 0, READ_BUF_SIZE);
				strcpy(input_buf, str);
				len = strlen(input_buf);
				ind = print_str_by_index(input_buf, 0, len);
				break;
			case HOME_KEY:
				go_to_begin(ind);
				ind = print_str_by_index(input_buf, 0, 0);
				break;
			case END_KEY:
				ind = print_str_by_index(input_buf, ind, len);
				break;
			};
			break;
		case CTRL_H_KEY:
		case BACKSP_KEY:
			ind = (ind <= 0) ? 0 : delete_char(input_buf, len, ind, 0);
			break;
		default:
			if (ind == len) {
				input_buf[ind++] = c;
				printf("%c", c);
				break;
			};
			ind = add_char(input_buf, len, c, ind);
			break;
		};
		if (ind >= (READ_BUF_SIZE - 1)) {
			if (current_hist_ind != hist_ind)
				add_to_history(input_buf, hist_ind);
			add_to_history(input_buf, current_hist_ind);
			return(History[current_hist_ind]);
		}
	};
	return((char *)NULL);
}

void set_current_history(char *line)
{
	if (line == (char *)NULL) return;
	if (strlen(line) == 0) return;
	if (strcmp(History[current_hist_ind], line) == 0) return;
	free(History[current_hist_ind]);
	History[current_hist_ind] = strdup(line);
}

char *get_last_history(void)
{
	if (current_hist_ind > 0)
		return(History[current_hist_ind - 1]);
	return((char *)NULL);
}

char *get_def_history(char *text, int *count)
{
	int	ind, i, res;
	char	*tmp, c;

	tmp = text;
	i = *count;
	while((*tmp != ' ') && (*tmp != 0)) {
		tmp++;
		i++;
	};
	c = *tmp;
	*tmp = 0;
	if (isdigit(*text)) {
		ind = atoi(text);
		*tmp = c;
		if ((ind < 0) || (ind > current_hist_ind))
			return((char *)NULL);
		*count = i;
		return(History[ind]);
	};
	res = find_cmd_by_text(text, current_hist_ind - 1, 0);
	*tmp = c;
	if (res == -1) return((char *)NULL);
	*count = i;
	return(History[res]);
}

ret_code_t history_cmd(void)
{
	int i;

	for (i = 0; i <= current_hist_ind; i++) {
		printf("[ %d ] %s\n", i, History[i]);
	};
	return(HPI_SHELL_OK);
}
