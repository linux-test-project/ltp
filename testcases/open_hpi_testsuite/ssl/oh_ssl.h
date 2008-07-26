/*
 * Copyright (C) 2007-2008, Hewlett-Packard Development Company, LLP
 *		       All rights reserved.
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
 *     Shuah Khan <shuah.khan@hp.com>
 *     Richard White <richard.white@hp.com>
 *
 * This file defines prototype(s) for SSL initialization and support functions.
 *
 * The contents have been #ifdef'd so that it can be included by files
 * needing potential SSL initialization code, but during builds without
 * an SSL library.  In other words, it should be safe to include this
 * file from any OpenHPI source.
 */


#ifndef __OH_SSL_H
#define __OH_SSL_H


/* Include files */
#include <config.h>
#ifdef HAVE_OPENSSL
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/x509v3.h>
#endif


/* Data types used while using these routines */
#ifdef HAVE_OPENSSL
enum OH_SSL_SHUTDOWN_TYPE {             /* See SSL_shutdown man page */
        OH_SSL_UNI,                     /* Unidirectional SSL shutdown */
        OH_SSL_BI                       /* Bidirectional SSL shutdown */
};
#endif


/*
 * Prototypes for the SSL connection management functions that are
 * implemented in oh_ssl.c
 */
extern int oh_ssl_init(void);
#ifdef HAVE_OPENSSL
extern SSL_CTX *oh_ssl_ctx_init(void);
extern int oh_ssl_ctx_free(SSL_CTX *ctx);
extern BIO *oh_ssl_connect(char *hostname, SSL_CTX *ctx, long timeout);
extern int oh_ssl_disconnect(BIO *bio, enum OH_SSL_SHUTDOWN_TYPE shutdown);
extern int oh_ssl_read(BIO *bio, char *buf, int size, long timeout);
extern int oh_ssl_write(BIO *bio, char *buf, int size, long timeout);
#endif


#endif /* __OH_SSL_H */
