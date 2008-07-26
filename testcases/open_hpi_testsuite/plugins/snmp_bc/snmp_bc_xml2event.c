/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2004, 2006
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. This
 * file and program are licensed under a BSD style license. See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      Steve Sherman <stevees@us.ibm.com>
 *      W. David Ashley <dashley@us.ibm.com>
 */

#include <glib.h>
#include <string.h>

#include <snmp_bc_plugin.h>

GHashTable *errlog2event_hash = NULL;
ohpi_bc_lock snmp_bc_plock = {
        .lock = G_STATIC_REC_MUTEX_INIT,
        .count = 0
};

unsigned int errlog2event_hash_use_count = 0;

static void free_hash_data(gpointer key, gpointer value, gpointer user_data);
static void event_start_element(GMarkupParseContext *context,
                                const gchar         *element_name,
                                const gchar         **attribute_names,
                                const gchar         **attribute_values,
                                gpointer            user_data,
                                GError              **error);

/**********************************************************************
 * errlog2event_hash_init:
 * @custom_handle: Plugin's data pointer.
 * 
 * Initializes the Error Log to event translation hash table.
 *
 * Returns:
 * SA_OK - Normal operation.
 * SA_ERR_HPI_OUT_OF_MEMORY - No memory to allocate hash table structures.
 * SA_ERR_HPI_INVALID_PARAMS - @custom_handle NULL
 **********************************************************************/
SaErrorT errlog2event_hash_init(struct snmp_bc_hnd *custom_handle) {
        GMarkupParser parser;
        GMarkupParseContext *pcontext;
        gboolean rc;
        GError *err;
        struct errlog2event_hash_info user_data;
        if (!custom_handle) {
                err("Invalid parameter.");
                return(SA_ERR_HPI_INVALID_PARAMS);
        }
	
        err = NULL;
        snmp_bc_lock(snmp_bc_plock);
	/* Initialize hash table */
        errlog2event_hash = g_hash_table_new(g_str_hash, g_str_equal);
        if (errlog2event_hash == NULL) {
                err("No memory.");
                snmp_bc_unlock(snmp_bc_plock);
                return(SA_ERR_HPI_OUT_OF_MEMORY);
	}
	
	/* Initialize user data used in parsing XML events */
	user_data.hashtable = errlog2event_hash;

        /* Initialize XML parser */
        memset(&parser, 0, sizeof(parser));
        parser.start_element = event_start_element;
        pcontext = g_markup_parse_context_new(&parser, 0, &user_data, NULL);
        if (pcontext == NULL) {
		err("No memory.");
                snmp_bc_unlock(snmp_bc_plock);
		return(SA_ERR_HPI_OUT_OF_MEMORY);
        }

        /* Parse XML events */
        rc = g_markup_parse_context_parse(pcontext,
                                          (const gchar *)eventxml,
                                          (gssize)strlen(eventxml), &err);
        if (rc == FALSE || err != NULL) {
                if (err != NULL) {
                        err("Parse error=%s.", err->message);
                        g_error_free(err);
                }
                else {
                        err("Unknown XML parse error.");
                }
                g_markup_parse_context_free(pcontext);
                snmp_bc_unlock(snmp_bc_plock);
                return(SA_ERR_HPI_INTERNAL_ERROR);
        }
        g_markup_parse_context_end_parse(pcontext, &err);
        g_markup_parse_context_free(pcontext);

        /* Make sure there are elements in the hash table */
        if (g_hash_table_size(errlog2event_hash) == 0) {
                err("Hash table is empty.");
                snmp_bc_unlock(snmp_bc_plock);
                return(SA_ERR_HPI_INTERNAL_ERROR);
        }
        
        snmp_bc_unlock(snmp_bc_plock);
	return(SA_OK);
}

/**********************************************************************
 * errlog2event_hash_free:
 *
 * Frees Error Log to event translation hash table.
 *
 * Returns:
 * SA_OK - Normal operation.
 **********************************************************************/
SaErrorT errlog2event_hash_free()
{
        snmp_bc_lock(snmp_bc_plock);
        g_hash_table_foreach(errlog2event_hash, free_hash_data, NULL);
	g_hash_table_destroy(errlog2event_hash);
        snmp_bc_unlock(snmp_bc_plock);
	return(SA_OK);
}

static void free_hash_data(gpointer key, gpointer value, gpointer user_data)
{
	ErrLog2EventInfoT *xmlinfo;

        g_free(key);
        xmlinfo = (ErrLog2EventInfoT *)value;
        g_free(xmlinfo->event);
        g_free(value);
}

/* Note: Error messages are passed back to the caller via the GError
 * mechanism. There is no need for err calls in this function. */
static void event_start_element(GMarkupParseContext *context,
                                const gchar         *element_name,
                                const gchar         **attribute_names,
                                const gchar         **attribute_values,
                                gpointer            user_data,
                                GError              **error)
{
        int i = 0;
	gchar *key = NULL;
        gint line, pos;
	ErrLog2EventInfoT *xmlinfo, working;
	struct errlog2event_hash_info *hash_info;

	memset(&working, 0, sizeof(ErrLog2EventInfoT));
	hash_info = (struct errlog2event_hash_info *)user_data;
 
        /* Ignore all XML elements except the event tag */
        if (g_ascii_strncasecmp(element_name, "event", sizeof("event")) != 0) {
                /* This is normal - not an error condition! */
                return;
        }

        /* Fetch XML element attributes and values. Build event info */
        while (attribute_names[i] != NULL) {
                if (g_ascii_strncasecmp(attribute_names[i], "name", sizeof("name")) == 0) {
                        /* Don't use this attribute so ignore it */
                }
                else if (g_ascii_strncasecmp(attribute_names[i], "msg", sizeof("msg")) == 0) {
                        key = g_strdup(attribute_values[i]);
                        if (key == NULL) {
                                g_set_error(error, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
                                            "No memory for hash key=%s", attribute_values[i]);
                                return;
                        }
                }
                else if (g_ascii_strncasecmp(attribute_names[i], "hex", sizeof("hex")) == 0) {
                        working.event = g_strdup(attribute_values[i]);
                        if (working.event == NULL) {
                                g_set_error(error, G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
                                            "No memory for hash value=%s", attribute_values[i]);
                                return;
                        }
                }
                else if (g_ascii_strncasecmp(attribute_names[i], "severity", sizeof("severity")) == 0) {
                        if (g_ascii_strncasecmp(attribute_values[i], "SAHPI_CRITICAL", sizeof("SAHPI_CRITICAL")) == 0) {
                                working.event_sev = SAHPI_CRITICAL;
                        }
                        else if (g_ascii_strncasecmp(attribute_values[i], "SAHPI_MAJOR", sizeof("SAHPI_MAJOR")) == 0) {
                                working.event_sev = SAHPI_MAJOR;
                        }
                        else if (g_ascii_strncasecmp(attribute_values[i], "SAHPI_MINOR", sizeof("SAHPI_MINOR")) == 0) {
                                working.event_sev = SAHPI_MINOR;
                        }
                        else if (g_ascii_strncasecmp(attribute_values[i], "SAHPI_INFORMATIONAL", sizeof("SAHPI_INFORMATIONAL")) == 0) {
                                working.event_sev = SAHPI_INFORMATIONAL;
                        }
                        else {
                                g_markup_parse_context_get_position(context, &line, &pos);
                                g_set_error(error, G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT,
                                            "Bad severity=%s for XML event element line %d", 
					    attribute_values[i], line);
                                return;
                        }
                }
                else if (g_ascii_strncasecmp(attribute_names[i], "override", sizeof("override")) == 0) {
                        working.event_ovr |= NO_OVR;
                        if (strstr(attribute_values[i], "OVR_SEV") != NULL) {
                                working.event_ovr |= OVR_SEV;
                        }
                        if (strstr(attribute_values[i], "OVR_RID") != NULL) {
                                working.event_ovr |= OVR_RID;
                        }
                        if (strstr(attribute_values[i], "OVR_EXP") != NULL) {
                                working.event_ovr |= OVR_EXP;
                        }
                        if (strstr(attribute_values[i], "OVR_VMM") != NULL) {
                                working.event_ovr |= OVR_VMM;
                        }
                        if (strstr(attribute_values[i], "OVR_MM1") != NULL) {
                                working.event_ovr |= OVR_MM1;
                        }
                        if (strstr(attribute_values[i], "OVR_MM2") != NULL) {
                                working.event_ovr |= OVR_MM2;
                        }
                        if (strstr(attribute_values[i], "OVR_MM_STBY") != NULL) {
                                working.event_ovr |= OVR_MM_STBY;
                        }
                        if (strstr(attribute_values[i], "OVR_MM_PRIME") != NULL) {
                                working.event_ovr |= OVR_MM_PRIME;
                        }
                        /* Ignore any other values */
                }
                else if (g_ascii_strncasecmp(attribute_names[i], "dup", sizeof("dup")) == 0) {
                        working.event_dup = (short)atoi(attribute_values[i]);
                }
                else {
                        g_markup_parse_context_get_position(context, &line, &pos);
                        g_set_error(error, G_MARKUP_ERROR, G_MARKUP_ERROR_UNKNOWN_ATTRIBUTE,
                                    "Bad name for XML event element line %d", line);
                        return;
                }
                i++;
        }

	/* Check for valid key */
        if (key == NULL) {
                g_set_error(error, G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT,
                            "No key set from XML event element");
                return;
        }

        /* Malloc memory for hash value and set values */
        xmlinfo = g_malloc0(sizeof(ErrLog2EventInfoT));
        if (!xmlinfo) {
                g_set_error(error, G_MARKUP_ERROR,G_MARKUP_ERROR_PARSE,
                            "No memory for hash value");
                return;
        }
	*xmlinfo = working;

	/* Insert event into hash table */
        g_hash_table_insert(hash_info->hashtable, key, xmlinfo);
	dbg("Inserted event=%s into hash table. Sev=%s, OVR=%lld, Dup=%d",
	    xmlinfo->event, oh_lookup_severity(xmlinfo->event_sev),
	    xmlinfo->event_ovr, xmlinfo->event_dup);
        return;
}
