/* test-saslauthd.c: saslauthd test utility
 * Rob Siemborski
 */
/* 
 * Copyright (c) 2002 Carnegie Mellon University.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The name "Carnegie Mellon University" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For permission or any other legal
 *    details, please contact  
 *      Office of Technology Transfer
 *      Carnegie Mellon University
 *      5000 Forbes Avenue
 *      Pittsburgh, PA  15213-3890
 *      (412) 268-4387, fax: (412) 268-7395
 *      tech-transfer@andrew.cmu.edu
 *
 * 4. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Computing Services
 *     at Carnegie Mellon University (http://www.cmu.edu/computing/)."
 *
 * CARNEGIE MELLON UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO
 * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY BE LIABLE
 * FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "saslauthd.h"
#include <stdio.h>

#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef USE_DOORS
#include <door.h>
#endif
#include <assert.h>

extern int errno;

/*
 * Keep calling the writev() system call with 'fd', 'iov', and 'iovcnt'
 * until all the data is written out or an error occurs.
 */
static int retry_writev(int fd, struct iovec *iov, int iovcnt)
{
    int n;
    int i;
    int written = 0;
    static int iov_max =
#ifdef MAXIOV
	MAXIOV
#else
#ifdef IOV_MAX
	IOV_MAX
#else
	8192
#endif
#endif
	;
    
    for (;;) {
	while (iovcnt && iov[0].iov_len == 0) {
	    iov++;
	    iovcnt--;
	}

	if (!iovcnt) return written;

	n = writev(fd, iov, iovcnt > iov_max ? iov_max : iovcnt);
	if (n == -1) {
	    if (errno == EINVAL && iov_max > 10) {
		iov_max /= 2;
		continue;
	    }
	    if (errno == EINTR) continue;
	    return -1;
	}

	written += n;

	for (i = 0; i < iovcnt; i++) {
	    if (iov[i].iov_len > (unsigned) n) {
		iov[i].iov_base = (char *)iov[i].iov_base + n;
		iov[i].iov_len -= n;
		break;
	    }
	    n -= iov[i].iov_len;
	    iov[i].iov_len = 0;
	}

	if (i == iovcnt) return written;
    }
}


/*
 * Keep calling the read() system call with 'fd', 'buf', and 'nbyte'
 * until all the data is read in or an error occurs.
 */
int retry_read(int fd, void *buf, unsigned nbyte)
{
    int n;
    int nread = 0;

    if (nbyte == 0) return 0;

    for (;;) {
	n = read(fd, buf, nbyte);
	if (n == -1 || n == 0) {
	    if (errno == EINTR || errno == EAGAIN) continue;
	    return -1;
	}

	nread += n;

	if (nread >= (int) nbyte) return nread;

	buf += n;
	nbyte -= n;
    }
}

/* saslauthd-authenticated login */
static int saslauthd_verify_password(const char *saslauthd_path,
				   const char *userid, 
				   const char *passwd,
				   const char *service,
				   const char *user_realm)
{
    char response[1024];
    char query[8192];
    char *query_end = query;
    int s;
    struct sockaddr_un srvaddr;
    int r;
    unsigned short count;
    void *context;
    char pwpath[sizeof(srvaddr.sun_path)];
    const char *p = NULL;
#ifdef USE_DOORS
    door_arg_t arg;
#endif

    if(!service) service = "imap";
    if(!user_realm) user_realm = "";
    if(!userid || !passwd) return -1;
    
    if (saslauthd_path) {
	strncpy(pwpath, saslauthd_path, sizeof(pwpath));
    } else {
	if (strlen(PATH_SASLAUTHD_RUNDIR) + 4 + 1 > sizeof(pwpath))
	    return -1;

	strcpy(pwpath, PATH_SASLAUTHD_RUNDIR);
	strcat(pwpath, "/mux");
    }

    /*
     * build request of the form:
     *
     * count authid count password count service count realm
     */
    {
 	unsigned short u_len, p_len, s_len, r_len;
 	struct iovec iov[8];
 
 	u_len = htons(strlen(userid));
 	p_len = htons(strlen(passwd));
	s_len = htons(strlen(service));
	r_len = htons((user_realm ? strlen(user_realm) : 0));

	memcpy(query_end, &u_len, sizeof(unsigned short));
	query_end += sizeof(unsigned short);
	while (*userid) *query_end++ = *userid++;

	memcpy(query_end, &p_len, sizeof(unsigned short));
	query_end += sizeof(unsigned short);
	while (*passwd) *query_end++ = *passwd++;

	memcpy(query_end, &s_len, sizeof(unsigned short));
	query_end += sizeof(unsigned short);
	while (*service) *query_end++ = *service++;

	memcpy(query_end, &r_len, sizeof(unsigned short));
	query_end += sizeof(unsigned short);
	if (user_realm) while (*user_realm) *query_end++ = *user_realm++;
    }

#ifdef USE_DOORS
    s = open(pwpath, O_RDONLY);
    if (s < 0) {
	perror("open");
	return -1;
    }

    arg.data_ptr = query;
    arg.data_size = query_end - query;
    arg.desc_ptr = NULL;
    arg.desc_num = 0;
    arg.rbuf = response;
    arg.rsize = sizeof(response);

    door_call(s, &arg);

    assert(arg.data_size < sizeof(response));
    response[arg.data_size] = '\0';

#else
    s = socket(AF_UNIX, SOCK_STREAM, 0);
    if (s == -1) {
	perror("socket() ");
	return -1;
    }

    memset((char *)&srvaddr, 0, sizeof(srvaddr));
    srvaddr.sun_family = AF_UNIX;
    strncpy(srvaddr.sun_path, pwpath, sizeof(srvaddr.sun_path));

    r = connect(s, (struct sockaddr *) &srvaddr, sizeof(srvaddr));
    if (r == -1) {
        perror("connect() ");
	return -1;
    }

    {
 	struct iovec iov[8];
 
	iov[0].iov_len = query_end - query;
	iov[0].iov_base = query;

	if (retry_writev(s, iov, 1) == -1) {
            fprintf(stderr,"write failed\n");
  	    return -1;
  	}
    }
  
    /*
     * read response of the form:
     *
     * count result
     */
    if (retry_read(s, &count, sizeof(count)) < (int) sizeof(count)) {
        fprintf(stderr,"size read failed\n");
	return -1;
    }
  
    count = ntohs(count);
    if (count < 2) { /* MUST have at least "OK" or "NO" */
	close(s);
        fprintf(stderr,"bad response from saslauthd\n");
	return -1;
    }
  
    count = (int)sizeof(response) < count ? sizeof(response) : count;
    if (retry_read(s, response, count) < count) {
	close(s);
        fprintf(stderr,"read failed\n");
	return -1;
    }
    response[count] = '\0';
  
    close(s);
#endif /* USE_DOORS */
  
    if (!strncmp(response, "OK", 2)) {
	printf("OK \"Success.\"\n");
	return 0;
    }
  
    printf("NO \"authentication failed\"\n");
    return -1;
}

int
main(int argc, char *argv[])
{
  const char *user = NULL, *password = NULL;
  const char *realm = NULL, *service = NULL, *path = NULL;
  int c;
  int flag_error = 0;
  unsigned passlen, verifylen;
  const char *errstr = NULL;
  int result;
  char *user_domain = NULL;

  while ((c = getopt(argc, argv, "p:u:r:s:f:")) != EOF)
      switch (c) {
      case 'f':
	  path = optarg;
	  break;
      case 's':
	  service = optarg;
	  break;
      case 'r':
	  realm = optarg;
	  break;
      case 'u':
	  user = optarg;
	  break;
      case 'p':
	  password = optarg;
	  break;
      default:
	  flag_error = 1;
	  break;
    }

  if (!user || !password)
    flag_error = 1;

  if (flag_error) {
    (void)fprintf(stderr,
		 "%s: usage: %s -u username -p password\n"
		 "              [-r realm] [-s servicename] [-f socket path]\n",
		  argv[0], argv[0]);
    exit(1);
  }

  /* saslauthd-authenticated login */
  return saslauthd_verify_password(path, user, password, service, realm);
}

