/*
 * Copyright (C) 2007-2009, Hewlett-Packard Development Company, LLP
 *                     All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the distribution.
 *
 * Neither the name of the Hewlett-Packard Corporation, nor the names
 * of its contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Author(s)
 *     Bryan Sutula <Bryan.Sutula@hp.com>
 *      Mohan Devarajulu <mohan@fc.hp.com>
 *
 *
 * This file implements support functions that are used to perform SOAP
 * calls.  There are four general classes of functions implemented in this
 * file: session management, soap call requests, XML response tree parsing,
 * and general utility calls.  (Note that some of these are implemented as
 * macros, in the oa_soap_callsupport.h include file.)  The specific calls
 * are as follows:
 *
 * Session Management:
 *      soap_open()             - Opens a connection to an OA using the
 *                                SOAP/XML protocol
 *      soap_close()            - Close a connection opened with soap_open()
 *
 * Soap Call Requests:
 *      soap_request()          - Main XML/SOAP request function, used by the
 *                                individual SOAP client call functions to
 *                                communicate with the OA
 *
 * XML Response Tree Parsing:
 *      soap_find_node()        - Recursively searches an XML tree, starting
 *                                with a specified node, looking for the
 *                                specified named node
 *      soap_walk_tree()        - Beginning with the specified node, walk an
 *                                XML tree to find the destination node,
 *                                based on a pattern of named nodes
 *      soap_walk_doc()         - Same as soap_walk_tree() but begins at the
 *                                root of the XML document
 *      soap_value()            - Returns the value at this part of the XML
 *                                tree, in particular, the node's child's
 *                                content field
 *      soap_tree_value()       - A convenience combination of soap_walk_tree()
 *                                and soap_value, heavily used in response
 *                                parameter parsing
 *      soap_next_node()        - When walking a list (array) of soap call
 *                                response values, this function gets you to
 *                                the next response node
 *      soap_enum()             - Performs enum string matching, which would
 *                                otherwise require a large amount of parameter
 *                                parsing code
 *      soap_inv_enum()         - Performs the inverse of soap_enum()
 *
 * General Utility:
 *      soap_ignore_errors()    - Used when a SOAP call error is expected, and
 *                                does not necessarily indicate a problem.
 *      soap_error_number()     - Returns the last SOAP call error number;
 *                                perhaps used when soap_ignore_errors() has
 *                                disabled error reporting.
 *      soap_error_string()     - Returns the last SOAP call error string.
 *      soap_timeout()          - Reads or writes the current connection
 *                                timeout value, in seconds
 */


/* Include files */
#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <oh_error.h>
#include "oa_soap_callsupport.h"

/* Include file check */
#ifndef LIBXML_PUSH_ENABLED
#error This source file requires LIBXML_PUSH_ENABLED functionality in libxml2
#endif


/* Local data types */
enum soap_error_enum {
        SOAP_NO_ERROR,
        SOAP_ERROR,
        SOAP_INVALID_SESSION
};


/* Forward declarations of static functions */
static int      soap_login(SOAP_CON *connection);
static int      soap_logout(SOAP_CON *connection);


/**
 * soap_find_node
 * @node:       pointer to an XML subtree
 * @s_name:     string that is being searched
 *
 * Searches an XML subtree, looking for a named node.
 *
 * Return value: the XML node, if found, or NULL.
 **/
xmlNode         *soap_find_node(xmlNode *node, char *s_name)
{
        xmlNode         *c_node;

        while (node != NULL){
                if (!xmlStrcmp(node->name, (const xmlChar *)s_name)){
                        return(node);
                }
                c_node = soap_find_node(node->xmlChildrenNode, s_name);
                if (c_node != NULL){
                        return(c_node);
                }

                node = node->next;
        }

        return(NULL);
}


/**
 * soap_walk_tree
 * @node:       pointer to an XML subtree
 * @colonstring: node name strings that are being searched; for example, to
 *              search for nodes "a", then "b", then "c", pass "a:b:c"
 *
 * Searches an XML subtree, looking for a named node.  Very similar to
 * soap_find_node() above, but uses an entirely different algorithm, both for
 * efficiency and to make response parsing more pedantic.
 *
 * The passed colonstring is a list of tree node names separated by ':'.  At
 * each level of the tree, the corresponding name must match.  This means that
 * the whole XML tree doesn't need to be searched, and that you know you've
 * found the right node, not one that is similarly-named but is in a different
 * part of the XML tree.
 *
 * Return value: the XML node, if found, or NULL.
 **/
xmlNode         *soap_walk_tree(xmlNode *node, char *colonstring)
{
        char            *next;
        char            *c;
        int             len;            /* Length of node name string */

        if ((! node) ||
            (! colonstring) ||
            (! *colonstring) ||
            (*colonstring == ':')) {
                return(NULL);
        }

        /* Break string at ':' */
        c = strchr(colonstring, ':');
        if (c) {
                len = c - colonstring;
                next = c + 1;
        }
        else {
                len = strlen(colonstring);
                next = colonstring + len;
        }

        /* Look for this in the node tree's children */
        node = node->children;
        while (node) {
                if ((! xmlStrncmp(node->name,
                                  (const xmlChar *)colonstring,
                                  len)) &&
                    (xmlStrlen(node->name) == len)) {
                        if (*next) {
                                return(soap_walk_tree(node, next));
                        }
                        else {
                                /* Done searching */
                                return(node);
                        }
                }
                node = node->next;
        }
        return(NULL);
}


/**
 * soap_walk_doc
 * @doc:        pointer to an XML document
 * @colonstring: node name strings that are being searched; for example, to
 *              search for nodes "a", then "b", then "c", pass "a:b:c"
 *
 * Same as soap_walk_tree(), but first obtains the root XML node from an XML
 * document.
 *
 * Note: This could be a #define, but is generally called only once for each
 * OA SOAP call.  Having it as a function makes it easier to interpret any
 * compiler errors or warnings regarding parameter mismatches.
 *
 * Return value: the XML node, if found, or NULL.
 **/
xmlNode         *soap_walk_doc(xmlDocPtr doc, char *colonstring)
{
        /* Null doc pointer passes through without error, apparently */
        return(soap_walk_tree(xmlDocGetRootElement(doc), colonstring));
}


/**
 * soap_value
 * @node:       pointer to an XML subtree
 *
 * Returns the value at this part of the XML tree.  The value is the node's
 * child's content field.
 *
 * Return value: a string pointer to the XLM content field.  NULL is returned
 * if there is any sort of problem accessing the value.
 **/
char            *soap_value(xmlNode *node)
{
        if ((node) && (node->children) && (node->children->content)) {
                return((char *)(node->children->content));
        }
        return(NULL);
}


/**
 * soap_tree_value
 * @node:       pointer to an XML subtree
 * @colonstring: node name strings that are being searched; for example, to
 *              search for nodes "a", then "b", then "c", pass "a:b:c"
 *
 * soap_tree_value() is a heavily-used combination of soap_walk_tree() and
 * soap_value(), both described above.  Generally used to locate specific
 * response parameters and obtain their values.  NULL is returned if anything
 * goes wrong with the response, so callers must be prepared for that value.
 *
 * Return value: a string pointer to the XLM content field.  NULL is returned
 * if there is any sort of problem accessing the value.
 **/
char            *soap_tree_value(xmlNode *node, char *colonstring)
{
        return(soap_value(soap_walk_tree(node, colonstring)));
}


/**
 * soap_next_node
 * @node:       pointer to an XML subtree
 *
 * When walking a list (array) of soap call response values, it is necessary
 * to have a function that moves to the next response value.  This is that
 * function.  Think of it as the function used in place of "node = node->next",
 * for XML response trees.
 *
 * Extra work is required because the XML response tree can contain essentially
 * empty nodes.  For example, newlines and other formatting characters occupy
 * XML nodes, even though they don't contain useful response content.
 *
 * Return value: If there are more values in this response array, the next
 * array node is returned.  Otherwise, the return value is NULL.
 **/
xmlNode         *soap_next_node(xmlNode *node)
{
        if (! node) {
                return(NULL);
        }

        node = node->next;
        while (node) {
                if ((node->children) && (node->children->content)) {
                        return(node);
                }
                node = node->next;
        }

        return(NULL);
}


/**
 * soap_enum
 * @enums:      Combined enum value string, as generated by the OA_SOAP_ENUM()
 *              macro.  Generally, this is the type name of the enum followed
 *              by "_S".
 * @value:      The string that should match one of the enum value names
 *
 * Performs enum string matching, which would otherwise require a large amount
 * of parameter parsing code.
 *
 * The general problem is that the OA returnes a large number of enum values.
 * These come across as strings, and need to be matched back to enum values.
 * There would be a large amount of string matching code to do this job.
 *
 * Instead, during the definition of each enum, a single string is also created
 * containing the string values that need to be matched.  A single call to
 * strstr() will locate the enum value in the string, and then it's position in
 * the combined enum string determines the enum value to be returned.
 *
 * Return value: The enum value (a non-negative integer) if a match is found.
 * If there is no successful match, the value -1 is returned.
 **/
int             soap_enum(const char *enums, char *value)
{
        char            *found;
        const char      *start = enums;
        int             n = 0;
        int             len;

        if (! value) {                  /* Can't proceed without a string */
                err("could not find enum (NULL value) in \"%s\"", enums);
                return(-1);
        }

        len = strlen(value);
	/* We have to search repeatedly, in case the match is just a substring
	 * of an enum value.
	 */
	while (start) {
        	found = strstr(start, value);
		/* To match, a value must be found, it must either be at the
		 * start of the string or be preceeded by a space, and must
		 * be at the end of the string or be followed by a ','
		 */
        	if ((found) &&
		    ((found == start) || (*(found - 1) == ' ')) &&
		    ((*(found + len) == ',') || (*(found + len) == '\0'))) {
		    	/* Having found a match, count backwards to see which
			 * enum value should be returned.
			 */
                	while (--found >= enums) {
                        	if (*found == ',')
                                	n++;
                	}
                	return(n);
        	}
		if (found) {
			/* We found something but it was a substring.  We need
			 * to search again, but starting further along in the
			 * enums string.
			 */
			start = found + len;
		}
		else {
			start = NULL;	/* We won't search any more */
		}
	}

        err("could not find enum value \"%s\" in \"%s\"", value, enums);
        return(-1);
}


/**
 * soap_inv_enum
 * @result:     A string buffer big enough to hold any of the enum names.
 *              This is the ASCII string that can be sent to the OA.
 * @enums:      Combined enum value string, as generated by the OA_SOAP_ENUM()
 *              macro.  Generally, this is the type name of the enum followed
 *              by "_S".
 * @value:      The enum value that is to be converted to an ASCII string
 *
 * Performs roughly the inverse of soap_enum(), for very similar reasons.
 *
 * In this case, the user provides an enum value, and we need to send an ASCII
 * string to the OA.  Instead of a case statement, this function uses the same
 * combined enum value string to produce the string that needs to go to across
 * the wire.
 *
 * The process is to count the enum value separators (the ',') placed by the
 * OA_SOAP_ENUM() macro and find the right enum name string.  It is necessary
 * to copy this string to a temporary buffer, because the string will become
 * an argument to sprintf(), and needs to be null-terminated.  Therefore, the
 * caller must provide a buffer big enough for the resulting string.
 *
 * Note that there are no error checks on the buffer size, so please ensure
 * that "result" is big enough for any of the enum names.
 *
 * Return value: If "value" refers to a valid enum, soap_inv_enum() returns 0
 * and "result" will contain the desired ASCII string.  If there is no enum
 * string for the passed value, -1 is returned.
 **/
int             soap_inv_enum(char *result, const char *enums, int value)
{
        char            *found;
        int             len;

        if (value < 0) {                /* Error check */
                err("inappropriate value");
                return(-1);
        }

        while (value && enums) {
                enums = strchr(enums, ',') + 1;
                value--;
        }
        if (! enums) {
                err("can't find enum");
                return(-1);
        }

        if (*enums == ' ')
                enums++;

        found = strchr(enums, ',');
        if (found)
                len = found - enums;
        else
                len = strlen(enums);
        strncpy(result, enums, len);
        result[len] = '\0';
        return(0);
}


/**
 * soap_open
 * @server:     String containing a server and port, such as "ccc2:443" or
 *              "192.168.1.123:443".  This string is passed, unchanged, to
 *              BIO_set_conn_hostname(), so refer to that man page for details
 *              on what types of strings can be used.
 * @username:   Username to be used when performing OA SOAP calls.  Generally,
 *              this needs to be configured on the OA before the OA SOAP call
 *              library can be used.
 * @password:   Password for the above "username"
 * @timeout:    OA SOAP response timeout, in seconds.  If zero, SOAP requests
 *              will wait forever.
 *
 * Opens a session to an OA using the SOAP/XML protocol.  The rest of the SOAP
 * support calls use this session to perform SOAP requests.  Note that this
 * call performs the initial OA login, so that a caller can be assured that the
 * complete connection to the OA is available.
 *
 * SOAP sessions encapsulate the following functionality:
 * - Maintain the SSL information necessary to talk to the OA
 * - Keep the username and password used when performing SOAP requests
 * - Take care of any necessary login calls
 * - Implement optional timeouts while reading SOAP response values
 * - Manage request and response buffers needed by the OA SOAP protocol
 * - Perform basic error checking during SOAP requests
 *
 * Note that while thread-safe calls are used (depending on compile options),
 * the semantics of the SOAP support calls require that each thread maintain
 * it's own SOAP session.  So a threaded application will need to use
 * soap_open() in each thread.
 *
 * Return value: The new SOAP_CON connection structure if successful, or NULL
 * on failure.
 **/
SOAP_CON        *soap_open(char *server,
                           char *username, char *password,
                           long timeout)
{
        SOAP_CON        *connection;

        /* Basic pass parameter checking */
        if ((server == NULL) || (*server == '\0')) {
                err("missing remote server");
                return(NULL);
        }

        if ((username == NULL) || (*username == '\0')) {
                err("missing OA username");
                return(NULL);
        }

        if ((password == NULL) || (*password == '\0')) {
                err("missing OA password");
                return(NULL);
        }

        if (timeout < 0) {
                err("inappropriate timeout value");
                return(NULL);
        }

        /* This will check for potential ABI mismatches between the compiled
         * version and the installed library.  If there are mismatches, I'm
         * afraid the calling program will die here, but that may be better
         * than allowing the situation to continue.
         */
        LIBXML_TEST_VERSION


        /* Get a new OA SOAP connection structure and initialize it */
        connection = g_malloc(sizeof(SOAP_CON)); /* New connection */
        if (! connection) {
                err("out of memory");
                return(NULL);
        }
        strncpy(connection->server, server, OA_SOAP_SERVER_SIZE);
        strncpy(connection->username, username, OA_SOAP_USER_SIZE);
        strncpy(connection->password, password, OA_SOAP_USER_SIZE);
        connection->timeout = timeout;
        connection->session_id[0] = '\0';
        connection->doc = NULL;
        connection->req_buf[0] = '\0';
        connection->req_high_water = 0;
        soap_ignore_errors(connection, 0);
        connection->last_error_number = 0;
        connection->last_error_string = NULL;


        /* Create and initialize a new SSL_CTX structure */
        if (! (connection->ctx = oh_ssl_ctx_init())) {
                err("oh_ssl_ctx_init() failed");
                free(connection);
                return(NULL);
        }


        /* Login to the OA, saving session information */
        if (soap_login(connection)) {
                err("OA login failed for server %s", connection->server);
                if (oh_ssl_ctx_free(connection->ctx)) {
                        err("oh_ssl_ctx_free() failed");
                }
                if (connection->doc) {
                        xmlFreeDoc(connection->doc);
                }
                free(connection);
                return(NULL);
        }


        return(connection);             /* Successful soap_open() */
}


/**
 * soap_close
 * @connection: OA SOAP connection provided by soap_open()
 *
 * Closes a session previously opened with soap_open(), freeing any resources
 * allocated for this session.  After making this call, the SOAP_CON structure
 * pointer can no longer be used.
 *
 * Return value: (none)
 **/
void            soap_close(SOAP_CON *connection)
{
        /* Error checking */
        if (! connection) {
                err("NULL connection pointer in soap_close()");
                return;
        }

        /* Log out of the current OA session */
        if (connection->session_id[0] != '\0') {
                if (soap_logout(connection)) {
                        err("OA logout failed");
                        /* Continuing to try to free remaining resources */
                }
        }

        /* Free the SSL_CTX structure */
        if (oh_ssl_ctx_free(connection->ctx)) {
                err("oh_ssl_ctx_free() failed");
        }

        /* Free our OA SOAP connection data structure */
        if (connection->doc) {
                xmlFreeDoc(connection->doc);
        }

        /* Buffer size debug information */
        dbg("Request buffer used %d out of %d",
            connection->req_high_water,
            OA_SOAP_REQ_BUFFER_SIZE);

        g_free(connection);

        /* Note that soap_open() initialized the SSL library, but
         * soap_close() does nothing to clean up and free SSL objects.
         * This is because the SSL library may still be in use by other
         * SOAP connections, or even other plugins.  There is probably
         * no way to provide workable clean-up code for SSL in this
         * environment, so it won't get released completely until the
         * application (the OpenHPI daemon) exits.
         */
}


/**
 * soap_message
 * @connection: OA SOAP connection provided by soap_open()
 * @request:    Request SOAP command, NULL-terminated
 * @doc:        The address of an XML document pointer (filled in by this call)
 *
 * Used internally to communicate with the OA.  Request buffer is provided by
 * the caller.  The OA's response is parsed into an XML document, which is
 * then pointed to by @doc.
 *
 * This call includes creating the SOAP request header, sending it to the
 * server, sending the SOAP request, and reading the SOAP response.
 *
 * Return value: 0 for a successful SOAP call, -1 for a variety of errors,
 * and -2 for a response timeout.
 **/
static int      soap_message(SOAP_CON *connection, char *request,
                             xmlDocPtr *doc)
{
        int             nbytes;
        int             ret;
        char            header[OA_SOAP_HEADER_SIZE + 1];
        char            response[OA_SOAP_RESP_BUFFER_SIZE];
        xmlParserCtxtPtr parse;

        /* Error checking */
        if (! connection) {
                err("NULL connection pointer in soap_message()");
                return(-1);
        }
        if (! request) {
                err("NULL request buffer in soap_message()");
                return(-1);
        }

        /* Start SSL connection */
        connection->bio = oh_ssl_connect(connection->server,
                                         connection->ctx,
                                         connection->timeout);
        if (! connection->bio) {
                err("oh_ssl_connect() failed");
                return(-1);
        }

        /* Develop header string */
        nbytes = strlen(request);
        if (connection->req_high_water < nbytes)
                connection->req_high_water = nbytes;
        snprintf(header, OA_SOAP_HEADER_SIZE, OA_XML_HEADER,
                 connection->server, nbytes);
        /* TODO: On that last line, I think server includes port...fix that,
         * though it doesn't seem to be causing any problems.
         */

        /* Write header to server */
        dbg("OA request(1):\n%s\n", header);
        if (oh_ssl_write(connection->bio, header, strlen(header),
                         connection->timeout)) {
                (void) oh_ssl_disconnect(connection->bio, OH_SSL_BI);
                err("oh_ssl_write() failed");
                return(-1);
        }

        /* Write request to server */
        dbg("OA request(2):\n%s\n", request);
        if (oh_ssl_write(connection->bio, request, nbytes,
                         connection->timeout)) {
                (void) oh_ssl_disconnect(connection->bio, OH_SSL_BI);
                err("oh_ssl_write() failed");
                return(-1);
        }

        /* Read response from server using oh_ssl_read()
         *
         * Note that the response seems to come back in two pieces.  The
         * first is the http header.  The second is the XML document that
         * I generally want.  For now, I'm throwing away the first part.
         *
         * TODO: Need to check to see if this is always the case.
         */
        nbytes = oh_ssl_read(connection->bio,
                             response, OA_SOAP_RESP_BUFFER_SIZE - 1,
                             connection->timeout);
        if (nbytes <= 0) {
                (void) oh_ssl_disconnect(connection->bio, OH_SSL_UNI);
                if (nbytes != -2) {
                        err("oh_ssl_read() part 1 failed");
                        return(-1);
                }
                return(-2);             /* Timeout */
        }
        response[nbytes] = '\0';
        dbg("OA response(0):\n%s\n", response);

        /* OK...here's the plan:
         *
         * There's an XML parsing function that parses in chunks.  We'll get
         * the first buffer chunk and parse it.  If it's the last, we're done.
         * Otherwise, get more chunks and parse them until something complains.
         */
        nbytes = oh_ssl_read(connection->bio,
                             response, OA_SOAP_RESP_BUFFER_SIZE - 1,
                             connection->timeout);
        if (nbytes <= 0) {
                (void) oh_ssl_disconnect(connection->bio, OH_SSL_UNI);
                if (nbytes != -2) {
                        err("oh_ssl_read() first chunk failed");
                        return(-1);
                }
                return(-2);             /* Timeout */
        }
        response[nbytes] = '\0';
        dbg("OA response(1):\n%s\n", response);
        parse = xmlCreatePushParserCtxt(NULL, NULL, response, nbytes, NULL);
        if (! parse) {
                (void) oh_ssl_disconnect(connection->bio, OH_SSL_BI);
                err("failed to create XML push parser context");
                return(-1);
        }

        /* Remaining chunks */
        while ((nbytes = oh_ssl_read(connection->bio,
                                     response,
                                     OA_SOAP_RESP_BUFFER_SIZE - 1,
                                     connection->timeout)) > 0) {
                if (nbytes < 0) {
                        (void) oh_ssl_disconnect(connection->bio, OH_SSL_UNI);
                        xmlFreeParserCtxt(parse);
                        if (nbytes == -1) {
                                err("oh_ssl_read() other chunks failed");
                        }
                        return(nbytes);
                }
                else {
                        response[nbytes] = '\0';
                        dbg("OA response(2):\n%s\n", response);
                        ret = xmlParseChunk(parse, response, nbytes, 0);
                        if (ret) {
                                /* Parse error */
                                err("xmlParseChunk() failed with error %d",
                                    ret);
                                (void) oh_ssl_disconnect(connection->bio,
                                                         OH_SSL_BI);
                                xmlFreeParserCtxt(parse);
                                return(-1);
                        }
                }
        }

        /* We're done with the SSL connection.  Close it first. */
        if (oh_ssl_disconnect(connection->bio, OH_SSL_BI)) {
                err("oh_ssl_disconnect() failed");
                return(-1);
        }
        connection->bio = NULL;

        /* Finish up the XML parsing */
        xmlParseChunk(parse, response, 0, 1);
        *doc = parse->myDoc;
        if ((! doc) || (! parse->wellFormed)) {
                err("failed to parse XML response from OA");
                xmlFreeParserCtxt(parse);
                return(-1);
        }

        xmlFreeParserCtxt(parse);
        return(0);
}


/**
 * soap_login
 * @connection: OA SOAP connection provided by soap_open()
 *
 * Used internally to login to the OA.  This is executed when we don't have a
 * valid session ID, or the OA says the session ID is (no longer) valid.
 *
 * Internal buffers are used, so that the contents of the main SOAP command
 * buffer is not disturbed, in case the login needs to be done during a SOAP
 * call.
 *
 * Note that short timeout periods may cause a timeout error, but this
 * condition will not be separately reported.
 *
 * Return value: 0 for a successful SOAP call, and -1 for a variety of errors.
 **/
static int      soap_login(SOAP_CON *connection)
{
        char            buf[OA_SOAP_LOGIN_SIZE];
        xmlDocPtr       doc;
        xmlNode         *login_element;
        xmlNode         *fault;
        xmlNode         *oa_err;
        char            *sess_id;

        /* Error checking */
        if (! connection) {
                err("NULL connection pointer in soap_login()");
                return(-1);
        }

        if (connection->session_id[0] != '\0') {
                err("already have a session ID in soap_login()");
                /* Continuing for now */
                connection->session_id[0] = '\0'; /* in case of error later */
        }

        /* Generate login request */
        snprintf(buf, OA_SOAP_LOGIN_SIZE, OA_XML_LOGIN,
                 connection->username, connection->password);

        /* Perform login request */
        if (soap_message(connection, buf, &doc)) {
                err("failed to communicate with OA during login");
                return(-1);
        }


        /* Parse looking for session ID.
         *
         * Note that we don't want to use use the connection's doc pointer
         * because it will complicate the soap_call() logic.
         */
        login_element = soap_walk_doc(doc, "Body:userLogInResponse:"
                                           "HpOaSessionKeyToken:oaSessionKey");
        if ((sess_id = soap_value(login_element))) {
                strncpy(connection->session_id, sess_id,
                        OA_SOAP_SESSIONKEY_SIZE);
                dbg("Opened session ID %s", connection->session_id);
                /* Free the XML document */
                xmlFreeDoc(doc);
                return(0);              /* Normal, successful return */
        }

        /* Below this point, we have some sort of error.  All this is
         * to provide diagnostics for the user.
         */
        if ((fault = soap_walk_doc(doc, "Body:Fault"))) {
                if ((oa_err = soap_walk_tree(fault, "Detail:faultInfo"))) {
                        err("login failure: %s",
                            soap_tree_value(oa_err, "errorText"));
                }
                else {                  /* Assuming SOAP protocol error */
                        err("login failure: %s",
                            soap_tree_value(fault, "Reason:Text"));
                }
        }
        else {
                err("failed to find session ID during OA login");
        }
        xmlFreeDoc(doc);
        return(-1);
}


/**
 * soap_logout
 * @connection: OA SOAP connection provided by soap_open()
 *
 * Used internally to log out of the OA.  This is executed only while closing
 * the connection.
 *
 * Note that short timeout periods may cause a timeout error, but this
 * condition will not be separately reported.
 *
 * Return value: 0 for a successful SOAP call, and -1 for a variety of errors.
 **/
static int      soap_logout(SOAP_CON *connection)
{
        /* Error checking */
        if (! connection) {
                err("NULL connection pointer in soap_logout()");
                return(-1);
        }
        if (connection->session_id[0] == '\0') {
                err("missing session ID in soap_logout()");
                return(-1);
        }

        /* Perform logout request */
        if (soap_request(connection, "<hpoa:userLogOut/>\n")) {
                err("failed to communicate with OA during logout");
                /* Clearing session ID since there was something wrong with
                 * our current session.
                 */
                connection->session_id[0] = '\0';
                return(-1);
        }

        /* Clear session ID, since we're (probably) logged out */
        connection->session_id[0] = '\0';

        /* Parse looking for OK response */
        if (! soap_walk_doc(connection->doc,
                            "Body:userLogOutResponse:returnCodeOk")) {
                err("failed to logout of the OA session");
                return(-1);
        }

        return(0);
}


/**
 * soap_error_enum
 * @connection: OA SOAP connection provided by soap_open()
 *
 * Used internally to check for SOAP response errors.  This code is a bit
 * messy, and so has been pulled out of soap_call() to make the overall
 * structure easier to follow.
 *
 * Return value: see the soap_call() description for details on error number
 * return values.
 **/
static enum soap_error_enum soap_error_check(SOAP_CON *connection)
{
        xmlNode         *fault;
        xmlNode         *oa_err;

        if ((fault = soap_walk_doc(connection->doc, "Body:Fault"))) {
                /* This can be a generic SOAP protocol error, an invalid
                 * session key, or an error that is generated by the OA.
                 * Only the latter comes with OA error numbers and OA error
                 * strings.
                 *
                 * Try for the invalid login error first.
                 */
                if ((oa_err = soap_walk_tree(fault, "Code:Subcode:Value"))) {
                        if (strcmp(soap_value(oa_err),
                                   "wsse:FailedAuthentication") == 0) {
                                /* This is an invalid session key */
                                connection->last_error_number = -4;
                                connection->last_error_string =
                                        soap_tree_value(fault, "Reason:Text");
                                return SOAP_INVALID_SESSION;
                        }
                }
                /* OA errors have a "Detail:faultInfo" tree structure */
                if ((oa_err = soap_walk_tree(fault, "Detail:faultInfo"))) {
                        connection->last_error_number =
                                atoi(soap_tree_value(oa_err, "errorCode"));
                        connection->last_error_string =
                                soap_tree_value(oa_err, "errorText");
                }
                else {                  /* Assuming SOAP protocol error */
                        connection->last_error_number = -3;
                        connection->last_error_string =
                                soap_tree_value(fault, "Reason:Text");
                }
                if (! connection->ignore_errors) {
                        err("OA SOAP error %d: %s",
                            connection->last_error_number,
                            connection->last_error_string);
                }
                return SOAP_ERROR;
        }
        else { /* Node "Body:Fault" is not found */
                connection->last_error_number = 0;
                connection->last_error_string = NULL;
                return SOAP_NO_ERROR;
        }
}


/**
 * soap_call
 * @connection: OA SOAP connection provided by soap_open()
 *
 * Perform an entire OA SOAP call, including login (if necessary), sending the
 * request, reading the response, XML parsing, and checking for SOAP errors.
 *
 * Though this is the main public call, SOAP client functions will generally
 * use the soap_request() macro instead of calling this function directly.
 *
 * Functional notes:
 *      This routine is a bit complicated because we need to auto-login if
 *      we have a stale session_id, yet we don't want to continue to try to
 *      login forever.  We also don't know that the session_id is stale until
 *      we try to use it.
 *
 *      The overall structure of this routine is as follows:
 *
 *         Locate the session key position for later use
 *         Loop, up to two tries {
 *           If I have a session_id {
 *             Substitute the session key
 *             Perform SOAP request:
 *               Free previously-used XML memory
 *               Perform the SOAP call
 *             Check for error (using soap_error_check() above):
 *               Successful call --> Return good status
 *               Other error --> Return error information
 *               Invalid session error --> Clear session_id and drop through
 *           }
 *           If I don't have a session_id {
 *             Try to log in:
 *               Successful call --> Update session_id
 *               Error --> Return error information
 *           }
 *         }
 *
 * Return value: 0 for a successful SOAP call and negative numbers for a
 * variety of errors:
 *         -1   Some general error
 *         -2   Timeout
 *         -3   SOAP protocol error
 *         -4   Invalid session ID (not seen unless really we can't login)
 **/
int             soap_call(SOAP_CON *connection)
{
        char            *session_pos;   /* Position of session key */
        int             i;              /* Overall loop variable */
        int             err;

        /* Error checking */
        if (! connection) {
                err("NULL connection pointer in soap_call()");
                return(-1);
        }
        if (! connection->req_buf[0]) {
                err("missing command buffer in soap_call()");
                return(-1);
        }

        /* Locate the session key.
         *
         * TODO: Is there a better way?  I've also considered putting a
         * "%s" in the request string, and using an sprintf() to place the
         * key at the %s, but it involves a second buffer and wastes an
         * extra string copy.  The main problem with the current approach
         * is that it assumes a session key size, which could change in a
         * future version of the OA.
         */
        session_pos = strstr(connection->req_buf, OA_SOAP_SESS_KEY_LABEL);
        if (! session_pos) {
                err("failed to find session key location in passed message");
                return(-1);
        }

        /* Overall loop, up to two tries */
        for (i = 0; i < 2; i++) {

                /* We only try the SOAP request if we have a session_id */
                if (connection->session_id[0]) {

                        /* First, we need to free any previously-used XML
                         * memory
                         */
                        if (connection->doc) {
                                xmlFreeDoc(connection->doc);
                                connection->doc = NULL;
                        }

                        /* Substitute the current session key */
                        strncpy(session_pos, connection->session_id,
                                OA_SOAP_SESSIONKEY_SIZE);

                        /* Perform SOAP call */
                        err = soap_message(connection,
                                           connection->req_buf,
                                           &(connection->doc));
                        if (err) {
                                if (err == -2) {
                                        return(-2); /* Timeout */
                                }
                                err("failed to communicate with OA "
                                    "during soap_call()");
                                connection->req_buf[0] = '\0';
                                return(-1);
                        }

                        /* Test for successful call */
                        switch (soap_error_check(connection)) {
                                case SOAP_NO_ERROR:
                                        connection->req_buf[0] = '\0';
                                        return(0);
                                case SOAP_ERROR:
                                        connection->req_buf[0] = '\0';
                                        return(-1);
                                case SOAP_INVALID_SESSION:
                                        connection->session_id[0] = '\0';
                                        dbg("had an invalid session ID");
                                        break;
                                default:
                                        err("internal error");
                                        return(-1);
                        }

                } /* If we have a session key already */

                /* If we didn't (or no longer) have a valid session key,
                 * get one
                 */
                if (! connection->session_id[0]) {
                        if (soap_login(connection)) {
                                err("OA login failed in soap call");
                                return(-1);
                        }
                } /* If we don't have a session key */
        } /* Overall loop, up to two tries */

        /* If we fell through, we didn't succeed with the call.  The last
         * error information we have should already be stored in the connection
         * structure.
         */
        connection->req_buf[0] = '\0';  /* For safety */
        return(-1);
}
