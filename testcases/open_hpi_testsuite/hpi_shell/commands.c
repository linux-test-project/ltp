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
 * Authors:
 *     Hu Yin     <hu.yin@intel.com>
 *     Racing Guo <racing.guo@intel.com>
 * Changes:
 *      11.30.2004 - Kouzmich < Mikhail.V.Kouzmich@intel.com >:
 *                      porting to HPI-B
 *      09.07.2005 - Renier Morales <renier@openhpi.org>:
 *                      Changes due to move of oh_add_config_file to config.c
 *
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>
#include <config.h>
#include <oh_config.h>
#include <oHpi.h>
#include <hpi_ui.h>
#include "hpi_cmd.h"

#define SEV_BUF_SIZE    32

typedef struct {
        char            *name;
        SaHpiSeverityT  val;
} Sev_def_t;

static Sev_def_t Sev_array[] = {
        {"crit",        SAHPI_CRITICAL},
        {"maj",         SAHPI_MAJOR},
        {"min",         SAHPI_MINOR},
        {"inf",         SAHPI_INFORMATIONAL},
        {"ok",          SAHPI_OK},
        {"debug",       SAHPI_DEBUG},
        {"all",         SAHPI_ALL_SEVERITIES},
        {NULL,          0}
};

void help(int as)
//  as = 0  - Available commands
//  as = 1  - help command
{
        command_def_t   *cmd = NULL, *res = (command_def_t *)NULL;
        int             len;
        term_def_t      *term;

        if ((as == 0) || ((term = get_next_term()) == NULL)) {
                int width = 0;

                printf("Available commands are: \n\n");
                for (cmd = commands; cmd->cmd != NULL; cmd++) {
                        if ((cmd->type != MAIN_COM) &&
                                (cmd->type != block_type) &&
                                (cmd->type != UNDEF_COM))
                                continue;
                        printf("%-20s", cmd->cmd);
                        if ((++width % 4) == 0)
                                printf("\n");
                }
                printf("\n");
                return;
        }

        for (;;) {
                register char   *arg;
                int             n;

                arg = term->term;
                len = strlen(arg);
                n = 0;
                for (cmd = commands; cmd->cmd != NULL; cmd++) {
                        if ((cmd->type != MAIN_COM) &&
                                (cmd->type != block_type) &&
                                (cmd->type != UNDEF_COM))
                                continue;
                        if (strncmp(cmd->cmd, arg, len) == 0) {
                                if (n == 0) res = cmd;
                                n++;
                        };
                        if (strcmp(cmd->cmd, arg) == 0) {
                                res = cmd;
                                n = 1;
                                break;
                        }
                }
                if (n != 1)
                        printf("Invalid help command %s\n", arg);
                else
                        printf("%s\n", res->help);
                term = get_next_term();
                if (term == NULL) break;
        }
}

static ret_code_t help_cmd(void)
{
        help(1);
        return(HPI_SHELL_OK);
}

static ret_code_t add_config(void)
{
#if 0
// This code needs to call oHpi APIs instead of internal functions

        SaErrorT        rv;
        term_def_t      *term;
        struct oh_parsed_config config = {NULL, NULL, 0, 0, 0, 0};

        term = get_next_term();
        if (term == NULL) {
                printf("no config file\n");
                return HPI_SHELL_CMD_ERROR;
        }
        rv = oh_load_config(term->term, &config);
        if (rv == SA_ERR_HPI_BUSY) {
                printf("Hold on. Another configuration changing is"
                " processing\n");
        }
        if (rv == SA_ERR_HPI_NOT_PRESENT) {
                printf("Hold on. Initialization is processing\n");
        }
        if (rv != SA_OK) return HPI_SHELL_CMD_ERROR;
        rv = oh_process_config(&config);
        oh_clean_config(&config);
        if (rv == SA_OK) return HPI_SHELL_OK;
#endif
        return HPI_SHELL_CMD_ERROR;
}

static ret_code_t event(void)
{
        term_def_t      *term;

        term = get_next_term();
        if (term == NULL) {
                printf("Event display: %s\n", prt_flag?"Enable":"Disable");
                return(HPI_SHELL_OK);
        };
        if (strcmp(term->term, "enable") == 0) {
                prt_flag = 1;
                printf("Event display enable successfully\n");
        } else if (strcmp(term->term, "disable") == 0) {
                prt_flag = 0;
                printf("Event display disable successfully\n");
        } else if (strcmp(term->term, "short") == 0) {
                show_event_short = 1;
                prt_flag = 1;
                printf("Event short display enable successfully\n");
        } else if (strcmp(term->term, "full") == 0) {
                show_event_short = 0;
                prt_flag = 1;
                printf("Event full display enable successfully\n");
        } else {
                return HPI_SHELL_PARM_ERROR;
        };
        set_Subscribe((Domain_t *)NULL, prt_flag);

        return HPI_SHELL_OK;
}

static ret_code_t debugset(void)
{
        char            *val;
        term_def_t      *term;

        if (debug_flag) printf("debugset:\n");
        term = get_next_term();
        if (term == NULL) {
                val = getenv("OPENHPI_ERROR");
                if (val == (char *)NULL) val = "NO";
                printf("OPENHPI_ERROR=%s\n", val);
                return(HPI_SHELL_OK);
        };
        if (strcmp(term->term, "on") == 0)
                val = "YES";
        else if (strcmp(term->term, "off") == 0)
                val = "NO";
        else
                return HPI_SHELL_PARM_ERROR;
        setenv("OPENHPI_ERROR", val, 1);

        return HPI_SHELL_OK;
}

static ret_code_t moreset(void)
{
        char            *val;
        term_def_t      *term;

        term = get_next_term();
        if (term == NULL) {
                 if (is_more) val = "ON";
		 else val = "OFF";
                printf("more = %s\n", val);
                return(HPI_SHELL_OK);
        };
        if (strcmp(term->term, "on") == 0)
                is_more = 1;
        else if (strcmp(term->term, "off") == 0)
                is_more = 0;
        else
                return HPI_SHELL_PARM_ERROR;

        return HPI_SHELL_OK;
}

static ret_code_t power(void)
{
        SaErrorT                rv;
        SaHpiResourceIdT        resourceid;
        SaHpiPowerStateT        state;
        int                     do_set = 1;
        term_def_t              *term;
        ret_code_t              ret;

        ret = ask_rpt(&resourceid);
        if (ret != HPI_SHELL_OK) return(ret);

        term = get_next_term();
        if (term == NULL) do_set = 0;
        else if (!strcmp(term->term, "on")) {
                state = SAHPI_POWER_ON;
        } else if (!strcmp(term->term, "off")) {
                state = SAHPI_POWER_OFF;
        } else if (!strcmp(term->term, "cycle")) {
                state = SAHPI_POWER_CYCLE;
        } else {
                return HPI_SHELL_PARM_ERROR;
        }

        if (do_set) {
                rv = saHpiResourcePowerStateSet(Domain->sessionId, resourceid, state);
                if (rv != SA_OK) {
                        printf("saHpiResourcePowerStateSet error %s\n",
                                oh_lookup_error(rv));
                        return HPI_SHELL_CMD_ERROR;
                };
                return HPI_SHELL_OK;
        }

        rv = saHpiResourcePowerStateGet(Domain->sessionId, resourceid, &state);
        if (rv != SA_OK) {
                printf("saHpiResourcePowerStateGet error %s\n", oh_lookup_error(rv));
                return HPI_SHELL_CMD_ERROR;
        }
        if (state == SAHPI_POWER_ON) {
                printf("Resource %d is power on now.\n",resourceid);
        } else if (state == SAHPI_POWER_OFF) {
                printf("Resource %d is power off now.\n",resourceid);
        }

        return HPI_SHELL_OK;
}

static ret_code_t reset(void)
{
        SaErrorT                rv;
        SaHpiResourceIdT        resourceid;
        SaHpiResetActionT       state;
        int                     do_set = 1;
        term_def_t              *term;
        ret_code_t              ret;

        ret = ask_rpt(&resourceid);
        if (ret != HPI_SHELL_OK) return(ret);

        term = get_next_term();
        if (term == NULL) do_set = 0;
        else if (!strcmp(term->term, "cold")) {
                state = SAHPI_COLD_RESET;
        } else if (!strcmp(term->term, "warm")) {
                state = SAHPI_WARM_RESET;
        } else if (!strcmp(term->term, "assert")) {
                state = SAHPI_RESET_ASSERT;
        } else if (!strcmp(term->term, "deassert")) {
                state = SAHPI_RESET_DEASSERT;
        } else {
                return HPI_SHELL_PARM_ERROR;
        }

        if (do_set) {
                rv = saHpiResourceResetStateSet(Domain->sessionId, resourceid, state);
                if (rv != SA_OK) {
                        printf("saHpiResourceResetStateSet error %s\n",
                                oh_lookup_error(rv));
                        return HPI_SHELL_CMD_ERROR;
                }
        }

        rv = saHpiResourceResetStateGet(Domain->sessionId, resourceid, &state);
        if (rv != SA_OK) {
                printf("saHpiResourceResetStateGet error %s\n", oh_lookup_error(rv));
                return HPI_SHELL_CMD_ERROR;
        }
        if (state == SAHPI_RESET_ASSERT) {
                printf("Entity's reset of %d is asserted now.\n",resourceid);
        } else if (state == SAHPI_RESET_DEASSERT) {
                printf("Entity's reset of %d is not asserted now.\n",resourceid);
        } else {
                printf("Entity's reset of %d is not setted now.\n",resourceid);
        }

        return HPI_SHELL_OK;
}

static ret_code_t clear_evtlog(void)
{
        SaHpiResourceIdT        resourceid;
        term_def_t              *term;
        SaErrorT                rv;

        term = get_next_term();
        if (term == NULL)
                resourceid = SAHPI_UNSPECIFIED_RESOURCE_ID;
        else
                resourceid = (SaHpiResourceIdT)atoi(term->term);

        rv = saHpiEventLogClear(Domain->sessionId, resourceid);
        if (rv != SA_OK) {
                printf("EventLog clear, error = %s\n", oh_lookup_error(rv));
                return HPI_SHELL_CMD_ERROR;
        }

        printf("EventLog successfully cleared\n");
        return HPI_SHELL_OK;
}

static ret_code_t set_tag(void)
{
        SaHpiResourceIdT        resid = 0;
        SaHpiTextBufferT        tbuf;
        int                     i;
        char                    buf[SAHPI_MAX_TEXT_BUFFER_LENGTH + 1];
        SaErrorT                rv;
        SaHpiRptEntryT          rpt_entry;
        Rpt_t                   tmp_rpt;
        ret_code_t              ret;

        ret = ask_rpt(&resid);
        if (ret != HPI_SHELL_OK) return(ret);
        i = get_string_param("New tag: ", buf, SAHPI_MAX_TEXT_BUFFER_LENGTH);
        if (i != 0) {
                printf("Invalid tag: %s\n", buf);
                return(HPI_SHELL_PARM_ERROR);
        };
        strcpy((char *)(tbuf.Data), buf);
        tbuf.DataType = SAHPI_TL_TYPE_TEXT;
        tbuf.Language = SAHPI_LANG_ENGLISH;
        tbuf.DataLength = strlen(buf);
        rv = saHpiResourceTagSet(Domain->sessionId, resid, &tbuf);
        if (rv != SA_OK) {
                printf("saHpiResourceTagSet error = %s\n", oh_lookup_error(rv));
                return HPI_SHELL_CMD_ERROR;
        };
        rv = saHpiRptEntryGetByResourceId(Domain->sessionId, resid, &rpt_entry);
        make_attrs_rpt(&tmp_rpt, &rpt_entry);
        show_Rpt(&tmp_rpt, ui_print);
        free_attrs(&(tmp_rpt.Attrutes));
        return (HPI_SHELL_OK);
}

static ret_code_t parmctrl(void)
{
        SaHpiResourceIdT        resid;
        int                     i;
        char                    buf[10];
        SaErrorT                rv;
        SaHpiParmActionT        act;
        ret_code_t              ret;

        ret = ask_rpt(&resid);
        if (ret != HPI_SHELL_OK) return(ret);
        i = get_string_param("Action (default,save,restore): ", buf, 9);
        if (i != 0) {
                printf("Invalid action: %s\n", buf);
                return(HPI_SHELL_PARM_ERROR);
        };
        if (strcmp(buf, "default") == 0)
                act = SAHPI_DEFAULT_PARM;
        else if (strcmp(buf, "save") == 0)
                act = SAHPI_SAVE_PARM;
        else if (strcmp(buf, "restore") == 0)
                act = SAHPI_RESTORE_PARM;
        else {
                printf("Invalid action: %s\n", buf);
                return(HPI_SHELL_PARM_ERROR);
        };

        rv = saHpiParmControl(Domain->sessionId, resid, act);
        if (rv != SA_OK) {
                printf("saHpiParmControl error = %s\n", oh_lookup_error(rv));
                return HPI_SHELL_CMD_ERROR;
        };
        return (HPI_SHELL_OK);
}

static ret_code_t set_sever(void)
{
        SaHpiResourceIdT        resid;
        SaHpiSeverityT          sev = SAHPI_OK;
        int                     i;
        char                    buf[SEV_BUF_SIZE + 1];
        SaErrorT                rv;
        SaHpiRptEntryT          rpt_entry;
        Rpt_t                   tmp_rpt;
        ret_code_t              ret;

        ret = ask_rpt(&resid);
        if (ret != HPI_SHELL_OK) return(ret);
        i = get_string_param(
                "New severity (crit, maj, min, inf, ok, debug, all): ",
                buf, SEV_BUF_SIZE);
        if (i != 0) {
                printf("Invalid sevetity: %s\n", buf);
                return(HPI_SHELL_PARM_ERROR);
        };
        for (i = 0; Sev_array[i].name != (char *)NULL; i++)
                if (strcmp(buf, Sev_array[i].name) == 0) {
                        sev = Sev_array[i].val;
                        break;
                };
        if (Sev_array[i].name == (char *)NULL) {
                printf("Invalid sevetity type: %s\n", buf);
                return(HPI_SHELL_PARM_ERROR);
        };
        rv = saHpiResourceSeveritySet(Domain->sessionId, resid, sev);
        if (rv != SA_OK) {
                printf("saHpiResourceSeveritySet error = %s\n",
                        oh_lookup_error(rv));
                return HPI_SHELL_CMD_ERROR;
        };
        rv = saHpiRptEntryGetByResourceId(Domain->sessionId, resid, &rpt_entry);
        make_attrs_rpt(&tmp_rpt, &rpt_entry);
        show_Rpt(&tmp_rpt, ui_print);
        free_attrs(&(tmp_rpt.Attrutes));
        return (HPI_SHELL_OK);
}

static ret_code_t discovery(void)
{
        SaErrorT        ret;

        do_progress("Discover");
        ret = saHpiDiscover(Domain->sessionId);
        if (SA_OK != ret) {
                printf("saHpiResourcesDiscover error = %s\n",
                        oh_lookup_error(ret));
                delete_progress();
                return HPI_SHELL_CMD_ERROR;
        };
        delete_progress();
        return HPI_SHELL_OK;
}

static ret_code_t dat_list(void)
{
        return show_dat(Domain, ui_print);
}

static ret_code_t listres(void)
{
	term_def_t	*term;
	int		mask = SHORT_LSRES;
	

	term = get_next_term();
        while (term != NULL) {
		if (strcmp(term->term, "stat") == 0)
			mask |= STATE_LSRES;
		else if (strcmp(term->term, "path") == 0)
			mask |= PATH_LSRES;
		else {
			printf("Invalid argument: %s\n", term->term);
			return(HPI_SHELL_PARM_ERROR);
		};
		term = get_next_term();
	};

       show_rpt_list(Domain, SHOW_ALL_RPT, 0, mask, ui_print);
        return(HPI_SHELL_OK);
}

static ret_code_t show_evtlog(void)
{
        SaHpiResourceIdT        rptid = 0;
        term_def_t              *term;

        term = get_next_term();
        if (term == NULL)
                rptid = SAHPI_UNSPECIFIED_RESOURCE_ID;
        else
                rptid = (SaHpiResourceIdT)atoi(term->term);

        return show_event_log(Domain->sessionId, rptid,
                show_event_short, ui_print);
}

static ret_code_t evtlog_time(void)
{
        SaHpiResourceIdT        rptid = 0;
        SaErrorT                rv;
        SaHpiTimeT              logtime;
        SaHpiTextBufferT        buffer;
        term_def_t              *term;

        term = get_next_term();
        if (term == NULL)
                rptid = SAHPI_UNSPECIFIED_RESOURCE_ID;
        else
                rptid = (SaHpiResourceIdT)atoi(term->term);

        rv = saHpiEventLogTimeGet(Domain->sessionId, rptid, &logtime);
        if (rv != SA_OK)
        {
                printf("saHpiEventLogTimeGet %s\n", oh_lookup_error(rv));
                return (HPI_SHELL_CMD_ERROR);
        }

        oh_decode_time(logtime, &buffer);
        printf ("Current event log time: %s\n", buffer.Data);
        return HPI_SHELL_OK;
}

static ret_code_t evtlog_state(void)
{
        SaHpiResourceIdT        rptid = 0;
        SaErrorT                rv;
        SaHpiBoolT              state = SAHPI_TRUE;
        int                     do_set = 0;
        char                    *str;
        term_def_t              *term;

        rptid = SAHPI_UNSPECIFIED_RESOURCE_ID;
        term = get_next_term();
        if ((term != NULL) && (isdigit(term->term[0])))
                rptid = (SaHpiResourceIdT)atoi(term->term);

        term = get_next_term();
        if (term != NULL) {
                do_set = 1;
                if (strcmp(term->term, "enable") == 0)
                        state = SAHPI_TRUE;
                else if (strcmp(term->term, "disable") == 0)
                        state = SAHPI_FALSE;
                else return(HPI_SHELL_PARM_ERROR);
        };
        if (do_set) {
                rv = saHpiEventLogStateSet(Domain->sessionId, rptid, state);
                if (rv != SA_OK) {
                        printf("saHpiEventLogStateSet %s\n",
                                oh_lookup_error(rv));
                        return(HPI_SHELL_CMD_ERROR);
                };
                return(HPI_SHELL_OK);
        };
        rv = saHpiEventLogStateGet(Domain->sessionId, rptid, &state);
        if (rv != SA_OK) {
                printf("saHpiEventLogStateGet %s\n", oh_lookup_error(rv));
                return(HPI_SHELL_CMD_ERROR);
        };
        if (state == SAHPI_TRUE) str = "Enable";
        else str = "Disable";
        printf("Event Log State: %s\n", str);
        return HPI_SHELL_OK;
}

static ret_code_t evtlog_reset(void)
{
        SaHpiResourceIdT        rptid = 0;
        SaErrorT                rv;
        term_def_t              *term;

        rptid = SAHPI_UNSPECIFIED_RESOURCE_ID;
        term = get_next_term();
        if ((term != NULL) && (isdigit(term->term[0])))
                rptid = (SaHpiResourceIdT)atoi(term->term);

        rv = saHpiEventLogOverflowReset(Domain->sessionId, rptid);
        if (rv != SA_OK) {
                printf("saHpiEventLogOverflowReset %s\n",
                        oh_lookup_error(rv));
                return(HPI_SHELL_CMD_ERROR);
        };
        return HPI_SHELL_OK;
}

static ret_code_t settime_evtlog(void)
{
        SaHpiResourceIdT        rptid = 0;
        SaErrorT                rv;
        SaHpiTimeT              newtime;
        struct tm               new_tm_time;
        char                    buf[READ_BUF_SIZE];
        int                     day_array[] = { 31, 28, 31, 30, 31, 30, 31,
                                                31, 30, 31, 30, 31 };
        term_def_t              *term;
        int                     i;

        rptid = SAHPI_UNSPECIFIED_RESOURCE_ID;
        term = get_next_term();
        if ((term != NULL) && (isdigit(term->term[0]))) {
                if (strchr(term->term, ':') != (char *)NULL) {
                        unget_term();
                        rptid = SAHPI_UNSPECIFIED_RESOURCE_ID;
                } else
                        rptid = (SaHpiResourceIdT)atoi(term->term);
        } else
                rptid = SAHPI_UNSPECIFIED_RESOURCE_ID;

        if (rptid == SAHPI_UNSPECIFIED_RESOURCE_ID)
                printf("Set date and time for Domain Event Log!\n");
        else
                printf("Set date and time for Resource %d!\n", rptid);

        memset(&new_tm_time, 0, sizeof(struct tm));
        i = get_string_param("format: MM:DD:YYYY:hh:mm:ss ==> ",
                buf, READ_BUF_SIZE);
        if (i != 0) return(HPI_SHELL_PARM_ERROR);
        sscanf(buf, "%d:%d:%d:%d:%d:%d",
                &new_tm_time.tm_mon, &new_tm_time.tm_mday,
                &new_tm_time.tm_year, &new_tm_time.tm_hour,
                &new_tm_time.tm_min, &new_tm_time.tm_sec);
        if ((new_tm_time.tm_mon < 1) || (new_tm_time.tm_mon > 12)) {
                printf("Month out of range: (%d)\n", new_tm_time.tm_mon);
                return(HPI_SHELL_PARM_ERROR);
        };
        new_tm_time.tm_mon--;
        if (new_tm_time.tm_year < 1900) {
                printf("Year out of range: (%d)\n", new_tm_time.tm_year);
                return(HPI_SHELL_PARM_ERROR);
        };
        if (new_tm_time.tm_mon == 1) {
        /* if the given year is a leap year */
                if ((((new_tm_time.tm_year % 4) == 0)
                        && ((new_tm_time.tm_year % 100) != 0))
                        || ((new_tm_time.tm_year % 400) == 0))
                        day_array[1] = 29;
        };

        if ((new_tm_time.tm_mday < 1) ||
                (new_tm_time.tm_mday > day_array[new_tm_time.tm_mon])) {
                printf("Day out of range: (%d)\n", new_tm_time.tm_mday);
                return(HPI_SHELL_PARM_ERROR);
        };

        new_tm_time.tm_year -= 1900;

        if ((new_tm_time.tm_hour < 0) || (new_tm_time.tm_hour > 24)) {
                printf("Hours out of range: (%d)\n", new_tm_time.tm_hour);
                return(HPI_SHELL_PARM_ERROR);
        };
        if ((new_tm_time.tm_min < 0) || (new_tm_time.tm_min > 60)) {
                printf("Minutes out of range: (%d)\n", new_tm_time.tm_min);
                return(HPI_SHELL_PARM_ERROR);
        };
        if ((new_tm_time.tm_sec < 0) || (new_tm_time.tm_sec > 60)) {
                printf("Seconds out of range: (%d)\n", new_tm_time.tm_sec);
                return(HPI_SHELL_PARM_ERROR);
        };

        newtime = (SaHpiTimeT) mktime(&new_tm_time) * 1000000000;
        rv = saHpiEventLogTimeSet(Domain->sessionId, rptid, newtime);
        if (rv != SA_OK)
        {
                printf("saHpiEventLogTimeSet %s\n", oh_lookup_error(rv));
                return(HPI_SHELL_CMD_ERROR);
        }

        return (HPI_SHELL_OK);
}

static ret_code_t show_rpt(void)
{
        Rpt_t                   tmp_rpt;
        SaHpiRptEntryT          rpt_entry;
        SaErrorT                rv;
        SaHpiResourceIdT        resid;
        ret_code_t              ret;

        ret = ask_rpt(&resid);
        if (ret != HPI_SHELL_OK) return(ret);
        rv = saHpiRptEntryGetByResourceId(Domain->sessionId, resid, &rpt_entry);
        if (rv != SA_OK) {
                printf("NO rpt: %d\n", resid);
                return(HPI_SHELL_CMD_ERROR);
        };
        make_attrs_rpt(&tmp_rpt, &rpt_entry);
        show_Rpt(&tmp_rpt, ui_print);
        free_attrs(&(tmp_rpt.Attrutes));
        return (HPI_SHELL_OK);
}

static ret_code_t show_rdr(void)
{
        Rdr_t                   tmp_rdr;
        SaHpiRdrT               rdr_entry;
        SaHpiResourceIdT        rptid = 0;
        SaHpiInstrumentIdT      rdrnum;
        SaHpiRdrTypeT           type;
        SaErrorT                rv;
        int                     i;
        char                    buf[10], t;
        term_def_t              *term;
        ret_code_t              ret;

        term = get_next_term();
        if (term == NULL) {
                if (read_file) return(HPI_SHELL_CMD_ERROR);
                i = show_rpt_list(Domain, SHOW_ALL_RPT, rptid,
			SHORT_LSRES, ui_print);
                if (i == 0) {
                        printf("NO rpt!\n");
                        return(HPI_SHELL_CMD_ERROR);
                };
                i = get_string_param("RPT (ID | all) ==> ", buf, 9);
                if (i != 0) return HPI_SHELL_CMD_ERROR;
                if (strncmp(buf, "all", 3) == 0) {
                        show_rpt_list(Domain, SHOW_ALL_RDR, rptid,
				SHORT_LSRES, ui_print);
                        return(HPI_SHELL_OK);
                };
                rptid = (SaHpiResourceIdT)atoi(buf);
        } else {
                if (strcmp(term->term, "all") == 0) {
                        show_rpt_list(Domain, SHOW_ALL_RDR, rptid,
				SHORT_LSRES, ui_print);
                        return(HPI_SHELL_OK);
                };
                if (isdigit(term->term[0]))
                        rptid = (SaHpiResourceIdT)atoi(term->term);
                else
                        return HPI_SHELL_PARM_ERROR;
        };
        term = get_next_term();
        if (term == NULL) {
                if (read_file) return(HPI_SHELL_CMD_ERROR);
                i = get_string_param("RDR Type (s|a|c|w|i|d|f) ==> ",
                        buf, 9);
                if ( (i != 0) || (buf[0] == '\0') || (buf[1] != 0) )
			return HPI_SHELL_PARM_ERROR;
        } else {
                memset(buf, 0, 10);
                strncpy(buf, term->term, 3);
        };
        t = *buf;
        if (t == 'c') type = SAHPI_CTRL_RDR;
        else if (t == 's') type = SAHPI_SENSOR_RDR;
        else if (t == 'i') type = SAHPI_INVENTORY_RDR;
        else if (t == 'w') type = SAHPI_WATCHDOG_RDR;
        else if (t == 'a') type = SAHPI_ANNUNCIATOR_RDR;
        else if (t == 'd') type = SAHPI_DIMI_RDR;
        else if (t == 'f') type = SAHPI_FUMI_RDR;
        else return HPI_SHELL_PARM_ERROR;
        ret = ask_rdr(rptid, type, &rdrnum);
        if (ret != HPI_SHELL_OK) return(ret);

	rv = saHpiRdrGetByInstrumentId(Domain->sessionId,
		rptid, type, rdrnum, &rdr_entry);
        if (rv != SA_OK) {
                printf("ERROR!!! Get rdr: ResourceId=%d RdrType=%d"
                        "RdrNum=%d: %s\n",
                        rptid, type, rdrnum, oh_lookup_error(rv));
                return(HPI_SHELL_CMD_ERROR);
        };
        make_attrs_rdr(&tmp_rdr, &rdr_entry);
        show_Rdr(&tmp_rdr, ui_print);
        free_attrs(&(tmp_rdr.Attrutes));
        return HPI_SHELL_OK;
}

static ret_code_t show_ver(void)
{
   SaHpiVersionT ver = saHpiVersionGet();

   printf("\nPackage version: %s\n", PACKAGE_VERSION);
   printf("HPI specification version: SAI_HPI-%c.%02d.%02d\n\n",
   	  ('A' + (ver >> 16) -1), (ver >> 8) & 0xFF, (ver) & 0xFF);

   return(HPI_SHELL_OK);
}

static ret_code_t wtd_get(void)
{
        SaHpiResourceIdT        rptid;
        SaHpiWatchdogNumT       wtdnum;
        SaHpiWatchdogT          watchdog;
        SaHpiWatchdogExpFlagsT  flags;
        SaErrorT                rv;
        ret_code_t              ret;
        char                    *str;
        char                    tmp[256];

        ret = ask_rpt(&rptid);
        if (ret != HPI_SHELL_OK) return(ret);
        ret = ask_rdr(rptid, SAHPI_WATCHDOG_RDR, &wtdnum);
        if (ret != HPI_SHELL_OK) return(ret);

        rv = saHpiWatchdogTimerGet(Domain->sessionId,
                rptid, wtdnum, &watchdog);
        if (rv != SA_OK) {
                printf("ERROR!!! Get Watchdog: ResourceId=%d "
                        "WatchdogNum=%d: %s\n",
                        rptid, wtdnum, oh_lookup_error(rv));
                return(HPI_SHELL_CMD_ERROR);
        };
        if (watchdog.Log) str = "TRUE";
        else str = "FALSE";
        printf("  Watchdogtimer (%d/%d): Log=%s", rptid, wtdnum, str);
        if (watchdog.Running) str = "Running";
        else str = "Stopped";
        printf("  %s\n", str);
        switch (watchdog.TimerUse) {
                case SAHPI_WTU_NONE:
                        str = "NONE"; break;
                case SAHPI_WTU_BIOS_FRB2:
                        str = "BIOS_FRB2"; break;
                case SAHPI_WTU_BIOS_POST:
                        str = "BIOS_POST"; break;
                case SAHPI_WTU_OS_LOAD:
                        str = "OS_LOAD"; break;
                case SAHPI_WTU_SMS_OS:
                        str = "SMS_OS"; break;
                case SAHPI_WTU_OEM:
                        str = "OEM"; break;
                case SAHPI_WTU_UNSPECIFIED:
                        str = "UNSPEC"; break;
                default: str = "Unknown"; break;
        };
        printf("  Timer Use: %s", str);
        switch (watchdog.TimerAction) {
                case SAHPI_WAE_NO_ACTION:
                        str = "NO_ACTION"; break;
                case SAHPI_WAE_RESET:
                        str = "RESET"; break;
                case SAHPI_WAE_POWER_DOWN:
                        str = "POWER_DOWN"; break;
                case SAHPI_WAE_POWER_CYCLE:
                        str = "POWER_CYCLE"; break;
                case SAHPI_WAE_TIMER_INT:
                        str = "TIMER_INT"; break;
                default: str = "Unknown"; break;
        };
        printf("  Action: %s", str);
        switch (watchdog.PretimerInterrupt) {
                case SAHPI_WPI_NONE:
                        str = "NONE"; break;
                case SAHPI_WPI_SMI:
                        str = "SMI"; break;
                case SAHPI_WPI_NMI:
                        str = "NMI"; break;
                case SAHPI_WPI_MESSAGE_INTERRUPT:
                        str = "MESSAGE"; break;
                case SAHPI_WPI_OEM:
                        str = "OEM"; break;
                default: str = "Unknown"; break;
        };
        printf("  Interrupt: %s", str);
        printf("  TimeOut: %d\n", watchdog.PreTimeoutInterval);
        tmp[0] = 0;
        flags = watchdog.TimerUseExpFlags;
        if (flags & SAHPI_WATCHDOG_EXP_BIOS_FRB2)
                strcat(tmp, " BIOS_FRB2 |");
        if (flags & SAHPI_WATCHDOG_EXP_BIOS_POST)
                strcat(tmp, " BIOS_POST |");
        if (flags & SAHPI_WATCHDOG_EXP_OS_LOAD)
                strcat(tmp, " OS_LOAD |");
        if (flags & SAHPI_WATCHDOG_EXP_SMS_OS)
                strcat(tmp, " SMS_OS |");
        if (flags & SAHPI_WATCHDOG_EXP_OEM)
                strcat(tmp, " OEM |");
        if (strlen(tmp) > 0) {
                tmp[strlen(tmp) - 1] = 0;
                printf("  Flags: {%s}\n", tmp);
        } else
                printf("  Flags: (null)\n");
        printf("  InitialCount = %d  PresentCount = %d\n",
                watchdog.InitialCount, watchdog.PresentCount);
        return HPI_SHELL_OK;
}

static ret_code_t wtd_set(void)
{
        SaHpiResourceIdT        rptid;
        SaHpiWatchdogNumT       wtdnum;
        SaHpiWatchdogT          watchdog;
        SaHpiWatchdogExpFlagsT  flags;
        SaErrorT                rv;
        ret_code_t              ret;
        int                     i, res;
        char                    *str, *str1;
        char                    tmp[256];

        ret = ask_rpt(&rptid);
        if (ret != HPI_SHELL_OK) return(ret);
        ret = ask_rdr(rptid, SAHPI_WATCHDOG_RDR, &wtdnum);
        if (ret != HPI_SHELL_OK) return(ret);

        i = get_string_param("Log(0 | 1): ", tmp, 255);
        if (i != 0) {
                printf("Invalid Log value: %s\n", tmp);
                return(HPI_SHELL_PARM_ERROR);
        };
        if (tmp[0] == '1') watchdog.Log = SAHPI_TRUE;
        else watchdog.Log = SAHPI_FALSE;

        i = get_string_param("Running(0 | 1): ", tmp, 255);
        if (i != 0) {
                printf("Invalid Running value: %s\n", tmp);
                return(HPI_SHELL_PARM_ERROR);
        };
        if (tmp[0] == '1') watchdog.Running = SAHPI_TRUE;
        else watchdog.Running = SAHPI_FALSE;

        i = get_string_param(
                "TimerUse(none|bios_frb2|bios_post|os_load|sms_os|oem): ",
                tmp, 255);
        if (i != 0) {
                printf("Invalid TimerUse value: %s\n", tmp);
                return(HPI_SHELL_PARM_ERROR);
        };
        if (strcmp(tmp, "none") == 0)
                watchdog.TimerUse = SAHPI_WTU_NONE;
        else if (strcmp(tmp, "bios_frb2") == 0)
                watchdog.TimerUse = SAHPI_WTU_BIOS_FRB2;
        else if (strcmp(tmp, "bios_post") == 0)
                watchdog.TimerUse = SAHPI_WTU_BIOS_POST;
        else if (strcmp(tmp, "os_load") == 0)
                watchdog.TimerUse = SAHPI_WTU_OS_LOAD;
        else if (strcmp(tmp, "sms_os") == 0)
                watchdog.TimerUse = SAHPI_WTU_SMS_OS;
        else if (strcmp(tmp, "oem") == 0)
                watchdog.TimerUse = SAHPI_WTU_OEM;
        else {
                printf("Invalid TimerUse value: %s\n", tmp);
                return(HPI_SHELL_PARM_ERROR);
        };

        i = get_string_param(
                "TimerAction(no|reset|pwr_down|pwr_cycle|int): ",
                tmp, 255);
        if (i != 0) {
                printf("Invalid TimerAction value: %s\n", tmp);
                return(HPI_SHELL_PARM_ERROR);
        };
        if (strcmp(tmp, "no") == 0)
                watchdog.TimerAction = SAHPI_WAE_NO_ACTION;
        else if (strcmp(tmp, "reset") == 0)
                watchdog.TimerAction = SAHPI_WAE_RESET;
        else if (strcmp(tmp, "pwr_down") == 0)
                watchdog.TimerAction = SAHPI_WAE_POWER_DOWN;
        else if (strcmp(tmp, "pwr_cycle") == 0)
                watchdog.TimerAction = SAHPI_WAE_POWER_CYCLE;
        else if (strcmp(tmp, "int") == 0)
                watchdog.TimerAction = SAHPI_WAE_TIMER_INT;
        else {
                printf("Invalid TimerAction value: %s\n", tmp);
                return(HPI_SHELL_PARM_ERROR);
        };

        i = get_string_param("PretimerInterrupt(no|smi|nmi|mess|oem): ",
                tmp, 255);
        if (i != 0) {
                printf("Invalid PretimerInterrupt value: %s\n", tmp);
                return(HPI_SHELL_PARM_ERROR);
        };
        if (strcmp(tmp, "no") == 0)
                watchdog.PretimerInterrupt = SAHPI_WPI_NONE;
        else if (strcmp(tmp, "smi") == 0)
                watchdog.PretimerInterrupt = SAHPI_WPI_SMI;
        else if (strcmp(tmp, "nmi") == 0)
                watchdog.PretimerInterrupt = SAHPI_WPI_NMI;
        else if (strcmp(tmp, "mess") == 0)
                watchdog.PretimerInterrupt = SAHPI_WPI_MESSAGE_INTERRUPT;
        else if (strcmp(tmp, "oem") == 0)
                watchdog.PretimerInterrupt = SAHPI_WPI_OEM;
        else {
                printf("Invalid TimerAction value: %s\n", tmp);
                return(HPI_SHELL_PARM_ERROR);
        };

        i = get_int_param("TimeOut: ", &res);
        if (i != 1) {
                printf("Invalid TimeOut value\n");
                return(HPI_SHELL_PARM_ERROR);
        };
        watchdog.PreTimeoutInterval = res;

        i = get_string_param("Flags(\"bios_frb2|bios_post|os_load|sms_os|oem\"): ",
                tmp, 255);
        if (i != 0) *tmp = 0;
        flags = 0;
        str = tmp;
        while (strlen(str) != 0) {
                while (isspace(*str)) str++;
                str1 = str;
                while ((*str1 != ' ') && (*str1 != 0) && (*str1 != '|')) str1++;
                if (*str1 != 0) *str1++ = 0;
                else *str1 = 0;
                if (strcmp(str, "bios_frb2") == 0)
                        flags |= SAHPI_WATCHDOG_EXP_BIOS_FRB2;
                if (strcmp(str, "bios_post") == 0)
                        flags |= SAHPI_WATCHDOG_EXP_BIOS_POST;
                if (strcmp(str, "os_load") == 0)
                        flags |= SAHPI_WATCHDOG_EXP_OS_LOAD;
                if (strcmp(str, "sms_os") == 0)
                        flags |= SAHPI_WATCHDOG_EXP_SMS_OS;
                if (strcmp(str, "oem") == 0)
                        flags |= SAHPI_WATCHDOG_EXP_OEM;
                str = str1;
        };

        watchdog.TimerUseExpFlags = flags;
        i = get_int_param("InitialCount: ", &res);
        if (i != 1) {
                printf("Invalid InitialCount value\n");
                return(HPI_SHELL_PARM_ERROR);
        };
        watchdog.InitialCount = res;

        rv = saHpiWatchdogTimerSet(Domain->sessionId, rptid, wtdnum, &watchdog);
        if (rv != SA_OK) {
                printf("ERROR!!! Set Watchdog: ResourceId=%d WatchdogNum=%d: %s\n",
                        rptid, wtdnum, oh_lookup_error(rv));
                return(HPI_SHELL_CMD_ERROR);
        };
        return HPI_SHELL_OK;
}

static ret_code_t wtd_reset(void)
{
        SaHpiResourceIdT        rptid;
        SaHpiWatchdogNumT       wtdnum;
        SaErrorT                rv;
        ret_code_t              ret;

        ret = ask_rpt(&rptid);
        if (ret != HPI_SHELL_OK) return(ret);
        ret = ask_rdr(rptid, SAHPI_WATCHDOG_RDR, &wtdnum);
        if (ret != HPI_SHELL_OK) return(ret);

        rv = saHpiWatchdogTimerReset(Domain->sessionId, rptid, wtdnum);
        if (rv != SA_OK) {
                printf("ERROR!!! Reset Watchdog: ResourceId=%d WatchdogNum=%d: %s\n",
                        rptid, wtdnum, oh_lookup_error(rv));
                return(HPI_SHELL_CMD_ERROR);
        };
        return HPI_SHELL_OK;
}

static ret_code_t quit(void)
{
        if (block_type != MAIN_COM) {
                unget_term();
                return(HPI_SHELL_OK);
        };
        printf("quit\n");
        restore_term_flags();
        close_session();
        exit(0);
}

static ret_code_t reopen_session(void)
{
        int              eflag = 0, fflag = 0;
        term_def_t      *term;
        SaErrorT         rv;
   
        term = get_next_term();
        while (term != NULL) {
                if (strcmp(term->term, "force") == 0) {
                        fflag = 1;
                } else {
                        printf("Invalid argument: %s\n", term->term);
                        return(HPI_SHELL_PARM_ERROR);
                };
                term = get_next_term();
        };
        do {
           rv = saHpiSessionClose(Domain->sessionId);
           sleep( 1 );
        } while ( fflag == 0 && rv != SA_OK && rv != SA_ERR_HPI_NO_RESPONSE );
        if (rv != SA_OK) {
                printf("saHpiSessionClose error %s\n", oh_lookup_error(rv));
        }
        if (open_session(eflag) != 0) {
                printf("Can not open session\n");
                return(HPI_SHELL_CMD_ERROR);
        }
        return(HPI_SHELL_OK);
}

static ret_code_t run(void)
{
        term_def_t      *term;
        char            *path;

        term = get_next_term();
        if (term == NULL) return(HPI_SHELL_PARM_ERROR);
        path = term->term;
        return(open_file(path));
}

static ret_code_t exec_proc(void)
{
        term_def_t      *term;
        char            buf[4096];

        term = get_next_term();
        if (term == NULL) return(HPI_SHELL_PARM_ERROR);
        strcpy(buf, term->term);
        while (term != NULL) {
                term = get_next_term();
                if (term == NULL) break;
                if (term->term_type != ITEM_TERM) break;
                strcat(buf, " ");
                strcat(buf, term->term);
        };
        if (system(buf) <  0)
		return(HPI_SHELL_CMD_ERROR);
        return(HPI_SHELL_OK);
}

static ret_code_t echo(void)
{
        term_def_t      *term;

        term = get_next_term();
        if (term != NULL)
                printf("%s\n", term->term);
        return(HPI_SHELL_OK);
}

static ret_code_t domain_info(void)
{
        SaHpiDomainInfoT        info;
        SaHpiTextBufferT        *buf;
        SaErrorT                rv;
        char                    date[30];

        rv = saHpiDomainInfoGet(Domain->sessionId, &info);
        if (rv != SA_OK) {
                printf("ERROR!!! saHpiDomainInfoGet: %s\n",
                        oh_lookup_error(rv));
                return(HPI_SHELL_CMD_ERROR);
        };
        printf("Domain: %d   Capabil: 0x%x   IsPeer: %d   Guid: %s\n",
                info.DomainId, info.DomainCapabilities,
                info.IsPeer, info.Guid);
        buf = &(info.DomainTag);
        print_text_buffer_text("    Tag: ", buf, NULL, ui_print);
        printf("\n");
        time2str(info.DrtUpdateTimestamp, date, 30);
        printf("    DRT update count: %d   DRT Timestamp : %s\n",
                info.DrtUpdateCount, date);
        time2str(info.RptUpdateTimestamp, date, 30);
        printf("    RPT update count: %d   RPT Timestamp : %s\n",
                info.RptUpdateCount, date);
        time2str(info.DatUpdateTimestamp, date, 30);
        printf("    DAT update count: %d   DAT Timestamp : %s\n",
                info.DatUpdateCount, date);
        printf("        ActiveAlarms: %d   CriticalAlarms: %d   Major: %d "
                "Minor: %d   Limit: %d\n",
                info.ActiveAlarms, info.CriticalAlarms, info.MajorAlarms,
                info.MinorAlarms, info.DatUserAlarmLimit);
        printf("        DatOverflow : %d\n", info.DatOverflow);
        return(HPI_SHELL_OK);
}

ret_code_t domain_proc(void)
{
        SaHpiDomainInfoT        info;
        SaHpiEntryIdT           entryid, nextentryid;
        SaHpiDrtEntryT          drtentry;
        SaErrorT                rv;
        SaHpiDomainIdT          id;
        SaHpiSessionIdT         sessionId;
        int                     i, n, first;
        gpointer                ptr;
        Domain_t                *domain = (Domain_t *)NULL;
        Domain_t                *new_domain;
        term_def_t              *term;

        term = get_next_term();
        if (term == NULL) {
                printf("Domain list:\n");
                printf("    ID: %d   SessionId: %d", Domain->domainId,
                        Domain->sessionId);
                rv = saHpiDomainInfoGet(Domain->sessionId, &info);
                if (rv == SA_OK) {
                        print_text_buffer_text("    Tag: ",
                                &(info.DomainTag), NULL, ui_print);
                };
                printf("\n");
                entryid = SAHPI_FIRST_ENTRY;
                first = 1;
                while (entryid != SAHPI_LAST_ENTRY) {
                        rv = saHpiDrtEntryGet(Domain->sessionId, entryid,
                                &nextentryid, &drtentry);
                        if (rv != SA_OK) break;
                        if (first) {
                                first = 0;
                                printf("        Domain Reference Table:\n");
                        };
                        printf("            ID: %d", drtentry.DomainId);
                        entryid = nextentryid;
                        rv = saHpiSessionOpen(drtentry.DomainId,
                                                &sessionId, NULL);
                        if (rv != SA_OK) {
                                printf("\n");
                                continue;
                        };
                        rv = saHpiDomainInfoGet(sessionId, &info);
                        if (rv == SA_OK) {
                                print_text_buffer_text("    Tag: ",  &(info.DomainTag), NULL, ui_print);
                        };
                        saHpiSessionClose(sessionId);
                        printf("\n");
                }
                return(HPI_SHELL_OK);
        };

        if (isdigit(term->term[0]))
                id = (int)atoi(term->term);
        else
                return HPI_SHELL_PARM_ERROR;
        n = g_slist_length(domainlist);
        for (i = 0; i < n; i++) {
                ptr = g_slist_nth_data(domainlist, i);
                if (ptr == (gpointer)NULL) break;
                domain = (Domain_t *)ptr;
                if (domain->domainId == id) break;
        };
        if (i >= n) {
                new_domain = (Domain_t *)malloc(sizeof(Domain_t));
                memset(new_domain, 0, sizeof(Domain_t));
                new_domain->domainId = id;
                if (add_domain(new_domain) < 0) {
                        free(new_domain);
                        printf("Can not open domain: %d\n", id);
                        return HPI_SHELL_PARM_ERROR;
                };
                domain = new_domain;
        };
        Domain = domain;
        set_Subscribe(Domain, prt_flag);
        add_domain(Domain);
        return(HPI_SHELL_OK);
}
#ifdef KUZ_DEBUG
static ret_code_t test_cmd(void)
{
        char    ar[256], s[10];
        int     c, n;

        while (1) {
                c = getchar();
                memset(s, 0, 5);
                n = *s;
                snprintf(ar, 256, "input: <%c>  <0x%x>  <%d>\n", c, c, c);
                printf("%s", ar);
        };
        return(HPI_SHELL_OK);
}
#endif

/* command table */
const char addcfghelp[] = "addcfg: add plugins, domains, handlers from"
                        " config file\nUsage: addcfg <config file>\n";
const char annhelp[] =  "ann: annunciator command block\n"
                        "Usage: ann <resourceId> <num>\n";
const char clevtloghelp[] = "clearevtlog: clear system event logs\n"
                        "Usage: clearevtlog [<resource id>]";
const char ctrlhelp[] = "ctrl: control command block\n"
                        "Usage: ctrl [<ctrlId>]\n"
                        "       ctrlId:: <resourceId> <num>\n";
const char dathelp[] = "dat: domain alarm table list\n"
                        "Usage: dat";
const char dimiblockhelp[] = "dimi: DIMI command block\n"
                        "Usage: dimi [<DimiId>]\n"
                        "       DimiId:: <resourceId> <DimiNum>\n";
const char debughelp[] = "debug: set or unset OPENHPI_ERROR environment\n"
                        "Usage: debug [ on | off ]";
const char domainhelp[] = "domain: show domain list and set current domain\n"
                        "Usage: domain [<domain id>]";
const char domaininfohelp[] = "domaininfo: show current domain info\n"
                        "Usage: domaininfo";
const char dscvhelp[] = "dscv: discovery resources\n"
                        "Usage: dscv ";
const char echohelp[] = "echo: pass string to the stdout\n"
                        "Usage: echo <string>";
const char eventhelp[] = "event: enable or disable event display on screen\n"
                        "Usage: event [enable|disable|short|full] ";
const char evtlresethelp[] = "evtlogreset: reset the OverflowFlag in the event log\n"
                        "Usage: evtlogreset [<resource id>]";
const char evtlstatehelp[] = "evtlogstate: show and set the event log state\n"
                        "Usage: evtlogstate [<resource id>] [enable|disable]";
const char evtlogtimehelp[] = "evtlogtime: show the event log's clock\n"
                        "Usage: evtlogtime [<resource id>]";
const char exechelp[] = "exec: execute external program\n"
                        "Usage: exec <filename> [parameters]";
const char fumiblockhelp[] = "fumi: FUMI command block\n"
                        "Usage: fumi [<FumiId>]\n"
                        "       DimiId:: <resourceId> <FumiNum>\n";
const char helphelp[] = "help: help information for OpenHPI commands\n"
                        "Usage: help [optional commands]";
const char historyhelp[] = "history: show input commands history\n"
                        "Usage: history";
const char hsblockhelp[] = "hs: hot swap command block\n"
                        "Usage: hs <resourceId>\n";
const char hsindhelp[] = "hotswap_ind: show hot swap indicator state\n"
                        "Usage: hotswap_ind <resource id>";
const char hsstathelp[] = "hotswapstat: retrieve hot swap state of a resource\n"
                        "Usage: hotswapstat <resource id>";
const char invhelp[] =  "inv: inventory command block\n"
                        "Usage: inv [<InvId>]\n"
                        "       InvId:: <resourceId> <IdrId>\n";
const char lreshelp[] = "lsres: list resources\n"
                        "Usage: lsres [stat] [path]";
const char lsorhelp[] = "lsensor: list sensors\n"
                        "Usage: lsensor";
const char morehelp[] = "more: set or unset more enable\n"
                        "Usage: more [ on | off ]";
const char parmctrlhelp[] = "parmctrl: save and restore parameters for a resource\n"
                        "Usage: parmctrl <resource id> <action>\n"
                        "    action - default | save | restore";
const char powerhelp[] = "power: power the resource on, off or cycle\n"
                        "Usage: power <resource id> [on|off|cycle]";
const char quithelp[] = "quit: close session and quit console\n"
                        "Usage: quit";
const char resethelp[] = "reset: perform specified reset on the entity\n"
                        "Usage: reset <resource id> [cold|warm|assert|deassert]";
const char reopenhelp[] = "reopen: reopens session\n"
                        "Usage: reopen [force]\n"
                          "force flag skips old session closing check";
const char runhelp[] = "run: execute command file\n"
                        "Usage: run <file name>";
const char senhelp[] =  "sen: sensor command block\n"
                        "Usage: sen [<sensorId>]\n"
                        "       sensorId:: <resourceId> <num>\n";
const char setseverhelp[] = "setsever: set severity for a resource\n"
                        "Usage: setsever [<resource id>]";
const char settaghelp[] = "settag: set tag for a particular resource\n"
                        "Usage: settag [<resource id>]";
const char settmevtlhelp[] = "settimeevtlog: sets the event log's clock\n"
                        "Usage: settimeevtlog [<resource id>]";
const char showevtloghelp[] = "showevtlog: show system event logs\n"
                        "Usage: showevtlog [<resource id>]";
const char showinvhelp[] = "showinv: show inventory data of a resource\n"
                        "Usage: showinv [<resource id>]";
const char showrdrhelp[] = "showrdr: show resource data record\n"
                        "Usage: showrdr [<resource id> [type [<rdr num>]]]\n"
                        "   or  rdr [<resource id> [type [<rdr num>]]]\n"
                        "       type =  c - control rdr, s - sensor, i - inventory rdr\n"
                        "               w - watchdog, a - annunciator";
const char showrpthelp[] = "showrpt: show resource information\n"
                        "Usage: showrpt [<resource id>]\n"
                        "   or  rpt [<resource id>]";
const char versionhelp[] = "ver: show HPI specification, package versions\n"
                        "Usage: ver";
const char wtdgethelp[] = "wtdget: show watchdog timer\n"
                        "Usage: wtdget <resource id> <watchdogNum>";
const char wtdresethelp[] = "wtdreset: reset watchdog timer\n"
                        "Usage: wtdreset <resource id>";
const char wtdsethelp[] = "wtdset: set watchdog timer\n"
                        "Usage: wtdset <resource id> <watchdogNum> <values>";
//  sensor command block
const char sen_dishelp[] = "disable: set sensor disable\n"
                        "Usage: disable";
const char sen_enbhelp[] = "enable: set sensor enable\n"
                        "Usage: enable";
const char sen_evtenbhelp[] = "evtenb: set sensor event enable\n"
                        "Usage: evtenb";
const char sen_evtdishelp[] = "evtdis: set sensor event disable\n"
                        "Usage: evtdis";
const char sen_mskaddhelp[] = "maskadd: add sensor event masks\n"
                        "Usage: maskadd";
const char sen_mskrmhelp[] = "maskrm: remove sensor event masks\n"
                        "Usage: maskrm";
const char sen_setthhelp[] = "setthres: set sensor thresholds\n"
                        "Usage: setthres";
const char sen_showhelp[] = "show: show sensor state\n"
                        "Usage: show";
//  inventory command block
const char inv_addahelp[] = "addarea: add inventory area\n"
                        "Usage: addarea";
const char inv_addfhelp[] = "addfield: add inventory field\n"
                        "Usage: addfield";
const char inv_delahelp[] = "delarea: delete inventory area\n"
                        "Usage: delarea";
const char inv_delfhelp[] = "delfield: delete inventory field\n"
                        "Usage: delfield";
const char inv_setfhelp[] = "setfield: set inventory field\n"
                        "Usage: setfield";
const char inv_showhelp[] = "show: show inventory\n"
                        "Usage: show [<area id>]";
//  control command block
const char ctrl_setsthelp[] = "setstate: set control state\n"
                        "Usage: setstate <values>";
const char ctrl_showhelp[] = "show: show control\n"
                        "Usage: show";
const char ctrl_sthelp[] = "state: show control state\n"
                        "Usage: state";
//  annunciator command block
const char ann_acknowhelp[] = "acknow: set acknowledge flag\n"
                        "Usage: acknow [<EntryId>] | [all <Severity>]";
const char ann_addhelp[] = "add: add Announcement\n"
                        "Usage: add <values>";
const char ann_delhelp[] = "delete: delete Announcement\n"
                        "Usage: delete <EntryId>";
const char ann_listhelp[] = "list: show Announcement list\n"
                        "Usage: list";
const char ann_mgethelp[] = "modeget: show annunciator mode\n"
                        "Usage: modeget";
const char ann_msethelp[] = "modeset: set annunciator mode\n"
                        "Usage: modeset <mode>";
const char ann_showhelp[] = "show: show annunciator or condition\n"
                        "Usage: show [num]";
//  Hot swap command block
const char hs_actionhelp[] = "action: set action process\n"
                        "Usage: action insert|extract";
const char hs_activehelp[] = "active: set active state\n"
                        "Usage: active";
const char hs_gettohelp[] = "gettimeout: show timeout\n"
                        "Usage: gettimeout insert|extract";
const char hs_inactivehelp[] = "inactive: set inactive state\n"
                        "Usage: inactive";
const char hs_indhelp[] = "ind: set and show indicator state\n"
                        "Usage: ind get|on|off";
const char hs_policyhelp[] = "policycancel: set default policy\n"
                        "Usage: policycancel";
const char hs_settohelp[] = "settimeout: set timeout\n"
                        "Usage: settimeout insert|extract <value>";
const char hs_statehelp[] = "state: show hot swap state\n"
                        "Usage: state";
//  DIMI command block
const char dimi_infohelp[] = "info: shows information about DIMI\n"
                        "Usage: info";
const char dimi_testinfohelp[] = "testinfo: shows information about specified test\n"
                        "Usage: testinfo [<testNum>]";
const char dimi_readyhelp[] = "ready: shows information about the DIMI readiness to run specified test\n"
                        "Usage: ready <testNum>";
const char dimi_starthelp[] = "start: starts execution of specified test\n"
                        "Usage: start <testNum>";
const char dimi_cancelhelp[] = "cancel: cancels specified test running\n"
                        "Usage: cancel <testNum>";
const char dimi_statushelp[] = "status: shows status of specified test\n"
                        "Usage: status <testNum>";
const char dimi_resultshelp[] = "results: show results from the last run of specified test\n"
                        "Usage: results <testNum>";
//  FUMI command block
const char fumi_setsourcehelp[] = "setsource : set new source information to the specified bank\n"
                        "Usage: setsource <bankNum> <sourceURI>";
const char fumi_validatesourcehelp[] = "validatesource : initiates the validation of the integrity of "
                        "the source image associated with the designated bank\n"
                        "Usage: validatesource <bankNum>";
const char fumi_getsourcehelp[] = "getsource : shows information about the source image assigned to "
                        "designated bank\n"
                        "Usage: getsource <bankNum>";
const char fumi_targetinfohelp[] = "targetinfo : shows information about the specified bank\n"
                        "Usage: targetinfo <bankNum>";
const char fumi_backuphelp[] = "backup : initiates a backup of currently active bank\n"
                        "Usage: backup";
const char fumi_setbootorderhelp[] = "setbootorder : set the position of a bank in the boot order\n"
                        "Usage: setbootorder <bankNum> <position>";
const char fumi_bankcopyhelp[] = "bankcopy : initiates a copy of the contents of one bank to another bank\n"
                        "Usage: bankcopy <srcBankNum> <dstBankNum>";
const char fumi_installhelp[] = "install : starts an installation process, loading firmware to "
                        "a specified bank\n"
                        "Usage: install <bankNum>";
const char fumi_statushelp[] = "status : shows upgrade status of the FUMI\n"
                        "Usage: status <bankNum>";
const char fumi_verifytargethelp[] = "verifytarget : starts the verification process of the upgraded image\n"
                        "Usage: verifytarget <bankNum>";
const char fumi_cancelhelp[] = "cancel : stops upgrade asynchronous operation in progress\n"
                        "Usage: cancel <bankNum>";
const char fumi_rollbackhelp[] = "rollback : initiates a rollback operation to "
                        "restore the currently active bank with a backup version\n"
                        "Usage: rollback ";
const char fumi_activatehelp[] = "activate : starts execution of the active image on the FUMI\n"
                        "Usage: activate ";

command_def_t commands[] = {
    { "addcfg",         add_config,     addcfghelp,     MAIN_COM },
    { "ann",            ann_block,      annhelp,        MAIN_COM },
    { "clearevtlog",    clear_evtlog,   clevtloghelp,   MAIN_COM },
    { "ctrl",           ctrl_block,     ctrlhelp,       MAIN_COM },
    { "dat",            dat_list,       dathelp,        MAIN_COM },
    { "debug",          debugset,       debughelp,      UNDEF_COM },
    { "dimi",           dimi_block,     dimiblockhelp,  MAIN_COM },
    { "domain",         domain_proc,    domainhelp,     MAIN_COM },
    { "domaininfo",     domain_info,    domaininfohelp, MAIN_COM },
    { "dscv",           discovery,      dscvhelp,       MAIN_COM },
    { "echo",           echo,           echohelp,       UNDEF_COM },
    { "event",          event,          eventhelp,      UNDEF_COM },
    { "evtlogtime",     evtlog_time,    evtlogtimehelp, MAIN_COM },
    { "evtlogreset",    evtlog_reset,   evtlresethelp,  MAIN_COM },
    { "evtlogstate",    evtlog_state,   evtlstatehelp,  MAIN_COM },
    { "exec",           exec_proc,      exechelp,       UNDEF_COM },
    { "fumi",           fumi_block,     fumiblockhelp,  MAIN_COM },
    { "help",           help_cmd,       helphelp,       UNDEF_COM },
    { "history",        history_cmd,    historyhelp,    UNDEF_COM },
    { "hs",             hs_block,       hsblockhelp,    MAIN_COM },
    { "inv",            inv_block,      invhelp,        MAIN_COM },
    { "lsres",          listres,        lreshelp,       UNDEF_COM },
    { "lsensor",        list_sensor,    lsorhelp,       MAIN_COM },
    { "more",           moreset,        morehelp,       UNDEF_COM },
    { "parmctrl",       parmctrl,       parmctrlhelp,   MAIN_COM },
    { "power",          power,          powerhelp,      MAIN_COM },
    { "quit",           quit,           quithelp,       UNDEF_COM },
    { "rdr",            show_rdr,       showrdrhelp,    MAIN_COM },
    { "reopen",         reopen_session, reopenhelp,     UNDEF_COM },
    { "reset",          reset,          resethelp,      MAIN_COM },
    { "rpt",            show_rpt,       showrpthelp,    MAIN_COM },
    { "run",            run,            runhelp,        MAIN_COM },
    { "sen",            sen_block,      senhelp,        MAIN_COM },
    { "settag",         set_tag,        settaghelp,     MAIN_COM },
    { "setsever",       set_sever,      setseverhelp,   MAIN_COM },
    { "settimeevtlog",  settime_evtlog, settmevtlhelp,  MAIN_COM },
    { "showevtlog",     show_evtlog,    showevtloghelp, MAIN_COM },
    { "showinv",        show_inv,       showinvhelp,    MAIN_COM },
    { "showrdr",        show_rdr,       showrdrhelp,    MAIN_COM },
    { "showrpt",        show_rpt,       showrpthelp,    MAIN_COM },
    { "ver",            show_ver,       versionhelp,    UNDEF_COM },
    { "wtdget",         wtd_get,        wtdgethelp,     MAIN_COM },
    { "wtdreset",       wtd_reset,      wtdresethelp,   MAIN_COM },
    { "wtdset",         wtd_set,        wtdsethelp,     MAIN_COM },
    { "?",              help_cmd,       helphelp,       UNDEF_COM },
#ifdef KUZ_DEBUG
    { "test",           test_cmd,       helphelp,       UNDEF_COM },
#endif
//  sensor command block
    { "disable",        sen_block_disable,      sen_dishelp,    SEN_COM },
    { "enable",         sen_block_enable,       sen_enbhelp,    SEN_COM },
    { "evtdis",         sen_block_evtdis,       sen_evtdishelp, SEN_COM },
    { "evtenb",         sen_block_evtenb,       sen_evtenbhelp, SEN_COM },
    { "maskadd",        sen_block_maskadd,      sen_mskaddhelp, SEN_COM },
    { "maskrm",         sen_block_maskrm,       sen_mskrmhelp,  SEN_COM },
    { "setthres",       sen_block_setthres,     sen_setthhelp,  SEN_COM },
    { "show",           sen_block_show,         sen_showhelp,   SEN_COM },
//  inventory command block
    { "addarea",        inv_block_addarea,      inv_addahelp,   INV_COM },
    { "addfield",       inv_block_addfield,     inv_addfhelp,   INV_COM },
    { "delarea",        inv_block_delarea,      inv_delahelp,   INV_COM },
    { "delfield",       inv_block_delfield,     inv_delfhelp,   INV_COM },
    { "setfield",       inv_block_setfield,     inv_setfhelp,   INV_COM },
    { "show",           inv_block_show,         inv_showhelp,   INV_COM },
//  control command block
    { "setstate",       ctrl_block_setst,       ctrl_setsthelp, CTRL_COM },
    { "show",           ctrl_block_show,        ctrl_showhelp,  CTRL_COM },
    { "state",          ctrl_block_state,       ctrl_sthelp,    CTRL_COM },
//  annunciator command block
    { "acknow",         ann_block_acknow,       ann_acknowhelp, ANN_COM },
    { "add",            ann_block_add,          ann_addhelp,    ANN_COM },
    { "delete",         ann_block_delete,       ann_delhelp,    ANN_COM },
    { "list",           ann_block_list,         ann_listhelp,   ANN_COM },
    { "modeget",        ann_block_modeget,      ann_mgethelp,   ANN_COM },
    { "modeset",        ann_block_modeset,      ann_msethelp,   ANN_COM },
    { "show",           ann_block_show,         ann_showhelp,   ANN_COM },
//  Hot swap command block
    { "action",         hs_block_action,hs_actionhelp,  HS_COM },
    { "active",         hs_block_active,hs_activehelp,  HS_COM },
    { "gettimeout",     hs_block_gtime, hs_gettohelp,   HS_COM },
    { "inactive",       hs_block_inact, hs_inactivehelp,HS_COM },
    { "ind",            hs_block_ind,   hs_indhelp,     HS_COM },
    { "policycancel",   hs_block_policy,hs_policyhelp,  HS_COM },
    { "settimeout",     hs_block_stime, hs_settohelp,   HS_COM },
    { "state",          hs_block_state, hs_statehelp,   HS_COM },
// DIMI command block
    { "info",           dimi_block_info,     dimi_infohelp,     DIMI_COM },
    { "testinfo",       dimi_block_testinfo, dimi_testinfohelp, DIMI_COM },
    { "ready",          dimi_block_ready,    dimi_readyhelp,    DIMI_COM },
    { "start",          dimi_block_start,    dimi_starthelp,    DIMI_COM },
    { "cancel",         dimi_block_cancel,   dimi_cancelhelp,   DIMI_COM },
    { "status",         dimi_block_status,   dimi_statushelp,   DIMI_COM },
    { "results",        dimi_block_results,  dimi_resultshelp,  DIMI_COM },
// FUMI command block
    { "setsource",      fumi_block_setsource,      fumi_setsourcehelp,       FUMI_COM },
    { "validatesource", fumi_block_validatesource, fumi_validatesourcehelp,  FUMI_COM },
    { "getsource",      fumi_block_getsource,      fumi_getsourcehelp,       FUMI_COM },
    { "targetinfo",     fumi_block_targetinfo,     fumi_targetinfohelp,      FUMI_COM },
    { "backup",         fumi_block_backup,         fumi_backuphelp,          FUMI_COM },
    { "setbootorder",   fumi_block_setbootorder,   fumi_setbootorderhelp,    FUMI_COM },
    { "bankcopy",       fumi_block_bankcopy,       fumi_bankcopyhelp,        FUMI_COM },
    { "install",        fumi_block_install,        fumi_installhelp,         FUMI_COM },
    { "status",         fumi_block_status,         fumi_statushelp,          FUMI_COM },
    { "verifytarget",   fumi_block_verifytarget,   fumi_verifytargethelp,    FUMI_COM },
    { "cancel",         fumi_block_cancel,         fumi_cancelhelp,          FUMI_COM },
    { "rollback",       fumi_block_rollback,       fumi_rollbackhelp,        FUMI_COM },
    { "activate",       fumi_block_activate,       fumi_activatehelp,        FUMI_COM },
// Terminator
    { NULL,             NULL,           NULL,           MAIN_COM }
};
