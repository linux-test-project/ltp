/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2008
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *     Renier Morales <renier@openhpi.org>
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <oHpi.h>
#include <oh_error.h>
#include <oh_domain.h>
#include <oh_client_conf.h>
#include <config.h>

/*******************************************************************************
 *  In order to use the glib lexical parser we need to define token
 *  types which we want to switch on
 ******************************************************************************/

enum {
        HPI_CLIENT_CONF_TOKEN_DOMAIN = G_TOKEN_LAST,
        HPI_CLIENT_CONF_TOKEN_DEFAULT,	
        HPI_CLIENT_CONF_TOKEN_HOST,
        HPI_CLIENT_CONF_TOKEN_PORT
} hpiClientConfType;

struct tokens {
        gchar *name;
        guint token;
};

static struct tokens oh_client_conf_tokens[] = {
        {
                .name = "domain",
                .token = HPI_CLIENT_CONF_TOKEN_DOMAIN
        },
        {
                .name = "default",
                .token = HPI_CLIENT_CONF_TOKEN_DEFAULT
        },
	
        {
                .name = "host",
                .token = HPI_CLIENT_CONF_TOKEN_HOST
        },
        {
                .name = "port",
                .token = HPI_CLIENT_CONF_TOKEN_PORT
        }

};

/*******************************************************************************
 * In order to initialize the lexical scanner, you need the following config.
 * This config was figured out by reading the glib sources, and lots of
 * trial and error (documentation for this isn't very good).
 *
 * G_TOKEN_STRING will be created when anything starts with a-zA-z_/.
 * due to cset_identifier_first and identifier2string values below.
 * Therefor, if you want 0 to be scanned as a string, you need to quote
 * it (e.g. "0")
 *
 *******************************************************************************/

static GScannerConfig oh_scanner_conf = {
                (
                        " \t\n"
                        )                       /* cset_skip_characters */,
                (
                        G_CSET_a_2_z
                        "_/."
                        G_CSET_A_2_Z
                        )                       /* cset_identifier_first */,
                (
                        G_CSET_a_2_z
                        "_-0123456789/."
                        G_CSET_A_2_Z
                        )                       /* cset_identifier_nth */,
                ( "#\n" )               /* cpair_comment_single */,
                FALSE                   /* case_sensitive */,
                TRUE                    /* skip_comment_multi */,
                TRUE                    /* skip_comment_single */,
                TRUE                    /* scan_comment_multi */,
                TRUE                    /* scan_identifier */,
                TRUE                    /* scan_identifier_1char */,
                TRUE                    /* scan_identifier_NULL */,
                TRUE                    /* scan_symbols */,
                TRUE                    /* scan_binary */,
                TRUE                    /* scan_octal */,
                TRUE                    /* scan_float */,
                TRUE                    /* scan_hex */,
                TRUE                    /* scan_hex_dollar */,
                TRUE                    /* scan_string_sq */,
                TRUE                    /* scan_string_dq */,
                TRUE                    /* numbers_2_int */,
                FALSE                   /* int_2_float */,
                TRUE                    /* identifier_2_string */,
                TRUE                    /* char_2_token */,
                TRUE                    /* symbol_2_token */,
                FALSE                   /* scope_0_fallback */,
};

static GTokenType get_next_good_token(GScanner *oh_scanner) {
        GTokenType next_token;

        next_token = g_scanner_get_next_token(oh_scanner);
        while (next_token != G_TOKEN_RIGHT_CURLY &&
               next_token != HPI_CLIENT_CONF_TOKEN_HOST &&
               next_token != HPI_CLIENT_CONF_TOKEN_PORT) {
                if (next_token == G_TOKEN_EOF) break;
                next_token = g_scanner_get_next_token(oh_scanner);
        }

        return next_token;
}

static int process_domain_token (GScanner *oh_scanner, GHashTable *domains)
{
        struct oh_domain_conf *domain_conf = NULL;
        GTokenType next_token;

        if (g_scanner_get_next_token(oh_scanner) != HPI_CLIENT_CONF_TOKEN_DOMAIN) {
                err("Processing domain: Expected a domain token");
                return -1;
        }

        /* Get the domain id and store in Hash Table */
        next_token = g_scanner_get_next_token(oh_scanner);
        if (next_token == HPI_CLIENT_CONF_TOKEN_DEFAULT) {
                // Default domain
                domain_conf = g_malloc0(sizeof(struct oh_domain_conf));
                // domain_conf->did = SAHPI_UNSPECIFIED_DOMAIN_ID;
                domain_conf->did = OH_DEFAULT_DOMAIN_ID;
                dbg("Processing domain: Found default domain definition");
        } else if (next_token == G_TOKEN_INT) {
                if (oh_scanner->value.v_int == 0) { // Domain Id of 0 is invalid
                        err("Processing domain: A domain id of 0 is invalid");
                        return -2;
                }
                domain_conf = g_malloc0(sizeof(struct oh_domain_conf));
                domain_conf->did = (SaHpiDomainIdT)oh_scanner->value.v_int;
                dbg("Processing domain: Found domain definition");
        } else {
                err("Processing domain: Expected int or string ('default') token");
                return -3;
        }

        /* Check for Left Brace token type. If we have it, then continue parsing. */
        if (g_scanner_get_next_token(oh_scanner) != G_TOKEN_LEFT_CURLY) {
                err("Processing domain: Expected left curly token.");
                goto free_and_exit;
        }

        next_token = get_next_good_token(oh_scanner);
        while (next_token != G_TOKEN_EOF && next_token != G_TOKEN_RIGHT_CURLY) {
                if (next_token == HPI_CLIENT_CONF_TOKEN_HOST) {
                        next_token = g_scanner_get_next_token(oh_scanner);
                        if (next_token != G_TOKEN_EQUAL_SIGN) {
                                err("Processing domain: Expected equal sign");
                                goto free_and_exit;
                        }
                        next_token = g_scanner_get_next_token(oh_scanner);
                        if (next_token != G_TOKEN_STRING) {
                                err("Processing domain: Expected a string");
                                goto free_and_exit;
                        }
                        if (domain_conf->host[0] == '\0') {
                                strncpy(domain_conf->host, oh_scanner->value.v_string, SAHPI_MAX_TEXT_BUFFER_LENGTH);
                        }
                } else if (next_token == HPI_CLIENT_CONF_TOKEN_PORT) {
                        next_token = g_scanner_get_next_token(oh_scanner);
                        if (next_token != G_TOKEN_EQUAL_SIGN) {
                                err("Processing domain: Expected equal sign");
                                goto free_and_exit;
                        }
                        next_token = g_scanner_get_next_token(oh_scanner);
                        if (next_token != G_TOKEN_INT) {
                                err("Processing domain: Expected an integer");
                                goto free_and_exit;
                        }
                        domain_conf->port = oh_scanner->value.v_int;
                } else {
                        err("Processing domain: Should not get here!");
                        goto free_and_exit;
                }
                next_token = g_scanner_get_next_token(oh_scanner);
        }

        if (next_token == G_TOKEN_EOF) {
                err("Processing domain: Expected a right curly");
                goto free_and_exit;
        } else if (domain_conf->host[0] == '\0') {
                err("Processing domain: Did not find the host parameter");
                goto free_and_exit;
        } else if (domain_conf->port == 0) {
                domain_conf->port = OPENHPI_DEFAULT_DAEMON_PORT;
        }

        g_hash_table_insert(domains, &domain_conf->did, domain_conf);
        //printf("domain %d: %s:%d\n", domain_conf->did, domain_conf->host, domain_conf->port);

        return 0;

free_and_exit:
        /**
        There was an error reading a token so we need to error out,
        but not before cleaning up.
        */
        g_free(domain_conf);

        return -10;
}

static void scanner_msg_handler (GScanner *scanner, gchar *message, gboolean is_error)
{
        g_return_if_fail (scanner != NULL);

        err("%s:%d: %s%s\n",
            scanner->input_name ? scanner->input_name : "<memory>",
            scanner->line, is_error ? "error: " : "", message );
}

int oh_load_client_config(const char *filename, GHashTable *domains)
{
        int oh_client_conf_file, i, done = 0;
        GScanner *oh_scanner;
        int num_tokens = sizeof(oh_client_conf_tokens) / sizeof(oh_client_conf_tokens[0]);

        if (!filename || !domains) {
                err("Error. Invalid parameters");
                return -1;
        }

        oh_scanner = g_scanner_new(&oh_scanner_conf);
        if (!oh_scanner) {
                err("Couldn't allocate g_scanner for file parsing");
                return -2;
        }

        oh_scanner->msg_handler = scanner_msg_handler;
        oh_scanner->input_name = filename;

        oh_client_conf_file = open(filename, O_RDONLY);
        if (oh_client_conf_file < 0) {
                err("Client configuration file '%s' could not be opened", filename);
                g_scanner_destroy(oh_scanner);
                return -3;
        }

        g_scanner_input_file(oh_scanner, oh_client_conf_file);

        for (i = 0; i < num_tokens; i++) {
                g_scanner_scope_add_symbol(
                        oh_scanner, 0,
                        oh_client_conf_tokens[i].name,
                        (void *)((unsigned long)oh_client_conf_tokens[i].token));
        }

        while (!done) {
                guint my_token;
                my_token = g_scanner_peek_next_token(oh_scanner);
                /*dbg("token: %d", my_token);*/
                switch (my_token)
                {
                case G_TOKEN_EOF:
                        done = 1;
                        break;
                case HPI_CLIENT_CONF_TOKEN_DOMAIN:
                        process_domain_token(oh_scanner, domains);
                        break;
                default:
                        /* need to advance it */
                        my_token = g_scanner_get_next_token(oh_scanner);
                        g_scanner_unexp_token(oh_scanner, G_TOKEN_SYMBOL,
                                              NULL, "\"domain\"", NULL, NULL, 1);
                        break;
                }
        }

        if (close(oh_client_conf_file) != 0) {
                err("Couldn't close file '%s'.", filename);
                g_scanner_destroy(oh_scanner);
                return -4;
        }

        done = oh_scanner->parse_errors;

        g_scanner_destroy(oh_scanner);

        dbg("Done processing conf file.\nNumber of parse errors:%d", done);

        return 0;
}
