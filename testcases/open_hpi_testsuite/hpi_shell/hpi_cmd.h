/*      -*- linux-c -*-
 *
 * Copyright (c) 2004 by Intel Corp.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Changes:
 *	11.30.2004 - Kouzmich: porting to HPI-B
 *
 *
 */

#ifndef _INC_HPI_CMD_H
#define _INC_HPI_CMD_H
#include <SaHpi.h>
#include <hpi_ui.h>
#include <glib.h>

#define KUZ_DEBUG0

#define READ_BUF_SIZE	1024
#define LINE_BUF_SIZE	4096

#define COMPL_NULL	0
#define COMPL_CMD	1

typedef enum {
	HPI_SHELL_OK = 0,
	HPI_SHELL_CMD_ERROR = -1,
	HPI_SHELL_PARM_ERROR = -2,
	HPI_SHELL_SYNTAX_ERROR = -3
} ret_code_t;

typedef enum {
	UNDEF_COM,
	MAIN_COM,
	SEN_COM,
	ANN_COM,
	CTRL_COM,
	INV_COM,
	HS_COM,
    DIMI_COM,
    FUMI_COM,
} com_enum_t;

typedef struct {
    char	*cmd;
    ret_code_t	(*fun)(void);
    const char	*help;
    com_enum_t	type;
} command_def_t;

typedef enum {
	CMD_TERM,
	ITEM_TERM,
	CMD_END_TERM,
	CMD_REDIR_TERM,
	CMD_ERROR_TERM,
	EMPTY_TERM
} term_t;

typedef struct {
	term_t	term_type;
	char	*term;
} term_def_t;

extern command_def_t	commands[];
extern int		prt_flag;
extern int		show_event_short;
extern Domain_t		*Domain;
extern GSList		*domainlist;
extern term_def_t	*terms;
extern int		read_stdin;
extern int		read_file;
extern FILE		*input_file;
extern com_enum_t	block_type;
extern ret_code_t	shell_error;
extern int		debug_flag;
extern char		Title[];
extern int		termfd;
extern int		is_more;

extern int		add_domain(Domain_t *domain);
extern int		add_input_file(char *name);
extern ret_code_t	ann_block(void);
extern ret_code_t	ann_block_acknow(void);
extern ret_code_t	ann_block_add(void);
extern ret_code_t	ann_block_delete(void);
extern ret_code_t	ann_block_list(void);
extern ret_code_t	ann_block_modeget(void);
extern ret_code_t	ann_block_modeset(void);
extern ret_code_t	ann_block_show(void);
extern ret_code_t	ask_rdr(SaHpiResourceIdT rptid, SaHpiRdrTypeT type,
				SaHpiInstrumentIdT *ret);
extern ret_code_t	ask_rpt(SaHpiResourceIdT *ret);
extern int		close_session(void);
extern ret_code_t	cmd_parser(char *mes, int as, int new_cmd, int *redirect);
extern void		cmd_shell(void);
extern ret_code_t	ctrl_block(void);
extern ret_code_t	ctrl_block_setst(void);
extern ret_code_t	ctrl_block_show(void);
extern ret_code_t	ctrl_block_state(void);
extern void		delete_progress(void);
extern int		do_discover(Domain_t *domain);
extern void		do_progress(char *mes);
extern ret_code_t	domain_proc(void);
extern char		*get_command_line(int new_cmd, int type);
extern char		*get_def_history(char *text, int *count);
extern int		get_hex_int_param(char *mes, int *val);
extern int		get_hex_string_param(char *mes, char *val, int max_length);
extern int		get_int_param(char *mes, int *val);
extern char		*get_last_history(void);
extern int		get_new_command(char *mes);
extern int		get_string_param(char *mes, char *string, int len);
extern term_def_t	*get_next_term(void);
extern int		get_sessionId(Domain_t *domain);
extern void		help(int as);
extern ret_code_t	history_cmd(void);
extern ret_code_t	hs_block(void);
extern ret_code_t	hs_block_action(void);
extern ret_code_t	hs_block_active(void);
extern ret_code_t	hs_block_gtime(void);
extern ret_code_t	hs_block_inact(void);
extern ret_code_t	hs_block_ind(void);
extern ret_code_t	hs_block_policy(void);
extern ret_code_t	hs_block_state(void);
extern ret_code_t	hs_block_stime(void);
extern void		init_history(void);
extern ret_code_t	inv_block(void);
extern ret_code_t	inv_block_show(void);
extern ret_code_t	inv_block_addarea(void);
extern ret_code_t	inv_block_addfield(void);
extern ret_code_t	inv_block_delarea(void);
extern ret_code_t	inv_block_delfield(void);
extern ret_code_t	inv_block_setfield(void);
extern ret_code_t	list_sensor(void);
extern ret_code_t	open_file(char *path);
extern int		open_session(int eflag);
extern void		restore_term_flags(void);
extern int		run_command(void);
extern ret_code_t	sen_block(void);
extern ret_code_t	sen_block_disable(void);
extern ret_code_t	sen_block_enable(void);
extern ret_code_t	sen_block_evtdis(void);
extern ret_code_t	sen_block_evtenb(void);
extern ret_code_t	sen_block_maskadd(void);
extern ret_code_t	sen_block_maskrm(void);
extern ret_code_t	sen_block_setthres(void);
extern ret_code_t	sen_block_show(void);
extern void		set_current_history(char *line);
extern void		set_Subscribe(Domain_t *domain, int as);
extern int		set_text_buffer(SaHpiTextBufferT *buf);
extern ret_code_t	show_inv(void);
extern ret_code_t   dimi_block(void);
extern ret_code_t   dimi_block_info(void);
extern ret_code_t   dimi_block_testinfo(void);
extern ret_code_t   dimi_block_ready(void);
extern ret_code_t   dimi_block_start(void);
extern ret_code_t   dimi_block_cancel(void);
extern ret_code_t   dimi_block_status(void);
extern ret_code_t   dimi_block_results(void);
extern ret_code_t   fumi_block(void);
extern ret_code_t   fumi_block_setsource(void);
extern ret_code_t   fumi_block_validatesource(void);
extern ret_code_t   fumi_block_getsource(void);
extern ret_code_t   fumi_block_targetinfo(void);
extern ret_code_t   fumi_block_backup(void);
extern ret_code_t   fumi_block_setbootorder(void);
extern ret_code_t   fumi_block_bankcopy(void);
extern ret_code_t   fumi_block_install(void);
extern ret_code_t   fumi_block_status(void);
extern ret_code_t   fumi_block_verifytarget(void);
extern ret_code_t   fumi_block_cancel(void);
extern ret_code_t   fumi_block_rollback(void);
extern ret_code_t   fumi_block_activate(void);
extern int		ui_print(char *Str);
extern ret_code_t	unget_term(void);

#endif

