/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2003
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Authors:
 *      Renier Morales <renierm@users.sf.net>
 *
 */

#include <snmp_util.h>

/**
 * snmp_get: Gets a single value indicated by the objectid
 * using snmp.
 * @handle: a handle to the snmp session needed to make an
 * snmp transaction.
 * @objid: string containing the OID entry.
 * @value: the value received from snmp will be put in this union.
 *
 * In the case of multiple values being returned, the type in 'value' will be
 * ASN_NULL (0x05). Nothing else in 'value' will be filled in.
 * Use snmp_get_all for doing gets that return multiple values.
 *
 * Return value: Returns 0 if successful, -1 if there was an error.
 **/
int snmp_get(struct snmp_session *ss, const char *objid, 
                struct snmp_value *value) 
{
        struct snmp_pdu *pdu;
        struct snmp_pdu *response;
        
        oid anOID[MAX_OID_LEN];
        size_t anOID_len = MAX_OID_LEN;
        struct variable_list *vars;
        int status;
        
        /*
         * Create the PDU for the data for our request.
         */
        pdu = snmp_pdu_create(SNMP_MSG_GET);
        read_objid(objid, anOID, &anOID_len);
        snmp_add_null_var(pdu, anOID, anOID_len);
        /*
         * Send the Request out.
         */
        status = snmp_synch_response(ss, pdu, &response);
        /*
         * Process the response.
         */
        if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
                vars = response->variables;
                value->type = vars->type;
                if (vars->next_variable != NULL) {
                        value->type = ASN_NULL;
                }/* If there are more values, set return type to null. */
                else if ( (vars->type == ASN_INTEGER) || (vars->type == ASN_COUNTER) ) {
                        value->integer = *(vars->val.integer);
                } else {
                        value->str_len = vars->val_len;
                        if (value->str_len >= MAX_ASN_STR_LEN)
                                value->str_len = MAX_ASN_STR_LEN;
                        else value->string[value->str_len] = '\0';
                        
                        strncpy(value->string,
                                (char *)vars->val.string,
                                value->str_len);                        
                }
        } else {
                value->type = (u_char)0x00; /* Set return type to 0 in case of error. */
                if (status == STAT_SUCCESS)
                        fprintf(stderr, "Error in packet %s\nReason: %s\n",
                                objid, snmp_errstring(response->errstat));
                else
                        snmp_sess_perror("snmpget", ss);
        }

        /* Clean up: free the response */
        if (response) snmp_free_pdu(response);

        return value->type? 0 : -1;
}

/**
 * snmp_set: Sets a value where indicated by the objectid
 * using snmp.
 * @handle: a handle to the snmp session needed to make an snmp transaction.
 * @objid: string containing the OID to set.
 * @value: the value to set the oid with.
 *
 * Return value: 0 if Success, less than 0 if Failure.
 **/
int snmp_set(
        struct snmp_session *ss,
        char *objid,
        struct snmp_value value) 
{
        struct snmp_pdu *pdu;
        struct snmp_pdu *response;

        oid anOID[MAX_OID_LEN];
        size_t anOID_len = MAX_OID_LEN;
        char datatype = 's';
        void *dataptr = NULL;
        int status = 0;
	int rtncode = 0;

        /*
         * Create the PDU for the data for our request.
         */
        pdu = snmp_pdu_create(SNMP_MSG_SET);
        read_objid(objid, anOID, &anOID_len);


        rtncode = 0; /* Default - All is OK */

        switch (value.type)
        {
                case ASN_INTEGER:
                case ASN_COUNTER:
                        datatype = 'i';
			(long *)dataptr = &value.integer;
                        break;
                case ASN_OCTET_STR:
                        datatype = 's';
			(u_char *)dataptr = &value.string;
                        break;
                default:
                        rtncode = -1;
                        fprintf(stderr, "datatype %c not yet supported by snmp_set()\n", value.type);
                        break;
        }

        if (rtncode == 0) {

		/*
		 * Set the data to send out
		 */
                snmp_add_var(pdu, anOID, anOID_len, datatype, dataptr);
        	/*
         	* Send the Request out.
         	*/
        	status = snmp_synch_response(ss, pdu, &response);
        	/*
         	* Process the response.
         	*/
        	if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
                	rtncode = 0;
                
        	} else {
                	rtncode = -1;
                	if (status == STAT_SUCCESS)
                        	fprintf(stderr, "Error in packet %s\nReason: %s\n",
                                		objid, snmp_errstring(response->errstat));
                	else
                        	snmp_sess_perror("snmpset", ss);
        	}

        	/* Clean up: free the response */
        	if (response) snmp_free_pdu(response);
	
	}

        return rtncode;
}

#if 0
int snmp_get_bulk(struct snmp_session *ss, const char *objid, 
                struct snmp_value *value) 
{
        struct snmp_pdu *pdu;
        struct snmp_pdu *response;
        
        oid anOID[MAX_OID_LEN];
        size_t anOID_len = MAX_OID_LEN;
        struct variable_list *vars;
        int status;
        
        /*
         * Create the PDU for the data for our request.
         */
        pdu = snmp_pdu_create(SNMP_MSG_GETNEXT);
        read_objid(objid, anOID, &anOID_len);
        snmp_add_null_var(pdu, anOID, anOID_len);
        /*
         * Send the Request out.
         */
        status = snmp_synch_response(ss, pdu, &response);
        /*
         * Process the response.
         */
        if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
                vars = response->variables;
                value->type = vars->type;
                if (vars->next_variable != NULL) {
                        value->type = ASN_NULL;
                }/* If there are more values, set return type to null. */
                else if (vars->type == ASN_INTEGER) {
                        value->integer = *(vars->val.integer);
                } else {
                        unsigned int str_len = vars->val_len;
                        strncpy(value->string,
                                (char *)vars->val.string,
                                MAX_ASN_STR_LEN);
                        if (str_len < MAX_ASN_STR_LEN) value->string[str_len] = '\0';
                        else value->string[MAX_ASN_STR_LEN-1] = '\0';
                }
        } else {
                value->type = (u_char)0x00; /* Set return type to 0 in case of error. */
                if (status == STAT_SUCCESS)
                        fprintf(stderr, "Error in packet %s\nReason: %s\n",
                                objid, snmp_errstring(response->errstat));
                else
                        snmp_sess_perror("snmpget", ss);
        }

        /* Clean up: free the response */
        if (response) snmp_free_pdu(response);

        return value->type? 0 : -1;
}
#endif
