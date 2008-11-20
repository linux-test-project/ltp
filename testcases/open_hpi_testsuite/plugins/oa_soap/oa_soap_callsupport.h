/*
 * Copyright (C) 2007-2008, Hewlett-Packard Development Company, LLP
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
 *
 *
 * This file implements support functions that are used to perform SOAP
 * calls.  See also oa_soap_callsupport.c for further details.
 */


#ifndef _INC_OA_SOAP_CALLSUPPORT_H_
#define _INC_OA_SOAP_CALLSUPPORT_H_


/* Include files */
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <oh_ssl.h>


/* Size limits */
/* TODO: Is there a way to check all of these, to make sure they are big
 * enough, but not too big?
 */
#define OA_SOAP_SERVER_SIZE             160
#define OA_SOAP_USER_SIZE               80
#define OA_SOAP_HEADER_SIZE             (200+OA_SOAP_SERVER_SIZE)
#define OA_SOAP_LOGIN_SIZE              2000 /* Careful...needs to be big enough
                                              * for failed login attempts
                                              */
#define OA_SOAP_REQ_BUFFER_SIZE         2000
#define OA_SOAP_RESP_BUFFER_SIZE        4000 /* 8000 seems to be the maximum
                                              * usable size, based on the
                                              * choices made by the SSL library
                                              */

/* These need to match up */
#define OA_SOAP_SESSIONKEY_SIZE         16
#define OA_SOAP_SESS_KEY_LABEL          "0123456_hi_there"


/* Macros available for callers of these routines */


/**
 * soap_request
 * @connection: OA SOAP connection provided by soap_open()
 * @fmt:        printf-style format string, used to build the SOAP command
 * @...:        printf-style variable arguments, used with "fmt"
 *
 * This is the main function used by most of the individual SOAP routines.
 * It creates the SOAP request string and uses soap_call() to do the real
 * work.
 *
 * Return value: 0 for successful SOAP call, various negative values for
 * errors.  (See soap_call() for details on various error return values.)
 **/
#define soap_request(connection, fmt, ...) \
( \
        snprintf(connection->req_buf, OA_SOAP_REQ_BUFFER_SIZE, \
                 OA_XML_REQUEST fmt OA_XML_TAIL, ## __VA_ARGS__), \
        soap_call(connection) \
)


/**
 * soap_ignore_errors
 * @connection: OA SOAP connection provided by soap_open()
 * @boolean:    True if errors are not to be printed, false (0) if errors
 *              are to be reported through the normal OpenHPI err() macro
 *
 * Generally, OpenHPI error macros are used to report any errors that
 * occur.  Some of this error reporting can be masked, so that the user
 * isn't bothered by errors that may be expected.  This routines enables
 * and disables this error reporting.
 *
 * Return value: (none)
 **/
#define soap_ignore_errors(connection, boolean) \
        connection->ignore_errors = boolean


/**
 * soap_timeout
 * @connection: OA SOAP connection provided by soap_open()
 *
 * Gives read and write access to the connection timeout value.  Can be used
 * on either left-hand or right-hand side of an assignment.  Value read from
 * or written to this macro should be an integer, and is interpreted as the
 * timeout in seconds.
 *
 * Return value: (none)
 **/
#define soap_timeout(connection) \
        (connection->timeout)


/**
 * soap_error_number and soap_error_string
 * @connection: OA SOAP connection provided by soap_open()
 *
 * Related to soap_ignore_errors() above, it is sometimes useful to retrieve
 * the error number or string from the most recent OA SOAP call.  These macros
 * provide this.  The number values are as documented for soap_call(), and the
 * error string comes from either the OA's SOAP server engine or is generated
 * by the OA as a result of a failed SOAP call.  Note that the string is not
 * valid when the error is a general error (-1) or a timeout (-2).
 *
 * Return values:
 *   soap_error_number() returns an integer number
 *   soap_error_string() returns a string pointer
 * Both values are valid until the next OA SOAP call.
 **/
#define soap_error_number(connection) \
        (connection->last_error_number)

#define soap_error_string(connection) \
        (connection->last_error_string)


/* Error codes returned by OA on event session failure */
#define ERR_EVENT_PIPE 201
#define ERR_EVENT_DAEMON_KILLED 204


/* Define the enum strings only in the oa_soap_calls.c file */
#ifdef OA_SOAP_CALLS_FILE
#define OA_SOAP_ENUM_STRING(name, ...) \
        const char name##_S[] = #__VA_ARGS__;
#else
#define OA_SOAP_ENUM_STRING(name, ...)
#endif

#define OA_SOAP_ENUM(name, ...) \
        enum name { __VA_ARGS__ }; \
        OA_SOAP_ENUM_STRING(name, __VA_ARGS__)


/* Data structures */
struct soap_con {
    SSL_CTX     *ctx;
    BIO         *bio;
    long        timeout;                /* Timeout value, or zero for none */
    char        server[OA_SOAP_SERVER_SIZE + 1];
    char        username[OA_SOAP_USER_SIZE + 1];
    char        password[OA_SOAP_USER_SIZE + 1];
    char        session_id[OA_SOAP_SESSIONKEY_SIZE + 1];
    xmlDocPtr   doc;                    /* We keep this here so that memory can
                                         * be freed during the next call
                                         */
    char        req_buf[OA_SOAP_REQ_BUFFER_SIZE];
    int         req_high_water;
    int         ignore_errors;
    int         last_error_number;
    char        *last_error_string;
};
typedef struct soap_con         SOAP_CON;


/* Function prototypes */
SOAP_CON        *soap_open(char *server, char *username, char *password,
                           long timeout);
void            soap_close(SOAP_CON *connection);
int             soap_call(SOAP_CON *connection);
xmlNode         *soap_find_node(xmlNode *node, char *findstring);
xmlNode         *soap_walk_tree(xmlNode *node, char *colonstring);
xmlNode         *soap_walk_doc(xmlDocPtr doc, char *colonstring);
char            *soap_value(xmlNode *node);
char            *soap_tree_value(xmlNode *node, char *colonstring);
xmlNode         *soap_next_node(xmlNode *node);
int             soap_enum(const char *enums, char *value);
int             soap_inv_enum(char *result, const char *enums, int value);


/* Various strings that are used to communicate with the OA */
#define OA_XML_HEADER \
        "POST /hpoa HTTP/1.1\n" \
        "Host: %s\n" \
        "Content-Type: application/soap+xml; charset=\"utf-8\"\n" \
        "Content-Length: %d\n\n"

#define OA_XML_VERSION \
        "<?xml version=\"1.0\"?>\n"

#define OA_XML_ENVELOPE \
        "<SOAP-ENV:Envelope " \
        "xmlns:SOAP-ENV=\"http://www.w3.org/2003/05/soap-envelope\" " \
        "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" " \
        "xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" " \
        "xmlns:wsu=\"http://docs.oasis-open.org/wss/2004/01/" \
                "oasis-200401-wss-wssecurity-utility-1.0.xsd\" " \
        "xmlns:wsse=\"http://docs.oasis-open.org/wss/2004/01/" \
                "oasis-200401-wss-wssecurity-secext-1.0.xsd\" " \
        "xmlns:hpoa=\"hpoa.xsd\">\n"

#define OA_XML_SECURITY \
        "<SOAP-ENV:Header>" \
        "<wsse:Security SOAP-ENV:mustUnderstand=\"true\">\n" \
        "<hpoa:HpOaSessionKeyToken>\n" \
        "<hpoa:oaSessionKey>" OA_SOAP_SESS_KEY_LABEL "</hpoa:oaSessionKey>\n" \
        "</hpoa:HpOaSessionKeyToken>\n" \
        "</wsse:Security>\n" \
        "</SOAP-ENV:Header>\n"

#define OA_XML_TAIL \
        "</SOAP-ENV:Body>\n" \
        "</SOAP-ENV:Envelope>\n"

#define OA_XML_LOGIN \
        OA_XML_VERSION \
        OA_XML_ENVELOPE \
        "<SOAP-ENV:Body>\n" \
        "<hpoa:userLogIn>\n" \
        "<hpoa:username>%s</hpoa:username>\n" \
        "<hpoa:password>%s</hpoa:password>\n" \
        "</hpoa:userLogIn>\n" \
        OA_XML_TAIL

#define OA_XML_REQUEST \
        OA_XML_VERSION \
        OA_XML_ENVELOPE \
        OA_XML_SECURITY \
        "<SOAP-ENV:Body>\n"


#endif  /* _INC_OASOAP_CALLSUPPORT_H_ */
