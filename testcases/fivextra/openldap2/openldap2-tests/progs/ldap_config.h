/* Generated from ./ldap_config.h.in on Tue Apr 15 20:29:16 CDT 2003 */
/* $OpenLDAP: pkg/ldap/include/ldap_config.h.in,v 1.1.8.5 2002/01/04 20:38:15 kurt Exp $ */
/*
 * Copyright 1998-2002 The OpenLDAP Foundation, Redwood City, California, USA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted only as authorized by the OpenLDAP
 * Public License.  A copy of this license is available at
 * http://www.OpenLDAP.org/license.html or in file LICENSE in the
 * top-level directory of the distribution.
 */

/*
 * This file works in confunction with OpenLDAP configure system.
 * If you do no like the values below, adjust your configure options.
 */

#ifndef _LDAP_CONFIG_H
#define _LDAP_CONFIG_H

/* directory separator */
#ifndef LDAP_DIRSEP
#ifndef _WIN32
#define LDAP_DIRSEP "/"
#else
#define LDAP_DIRSEP "\\"
#endif
#endif

/* directory for temporary files */
#if defined( _P_tmpdir )
# define LDAP_TMPDIR _P_tmpdir
#elif defined( P_tmpdir )
# define LDAP_TMPDIR P_tmpdir
#elif defined( _PATH_TMPDIR )
# define LDAP_TMPDIR _PATH_TMPDIR
#else
# define LDAP_TMPDIR LDAP_DIRSEP "tmp"
#endif

/* directories */
#ifndef LDAP_BINDIR
#define LDAP_BINDIR			"/usr/bin"
#endif
#ifndef LDAP_SBINDIR
#define LDAP_SBINDIR		"/usr/sbin"
#endif
#ifndef LDAP_DATADIR
#define LDAP_DATADIR		"/usr/share/openldap"
#endif
#ifndef LDAP_SYSCONFDIR
#define LDAP_SYSCONFDIR		"/usr/etc/openldap"
#endif
#ifndef LDAP_LIBEXECDIR
#define LDAP_LIBEXECDIR		"/usr/libexec"
#endif
#ifndef LDAP_RUNDIR
#define LDAP_RUNDIR			"/usr/var"
#endif

/* command locations */
#ifndef LDAP_EDITOR
#define LDAP_EDITOR			"/usr/bin/vi"
#endif
#ifndef LDAP_FINGER
#define LDAP_FINGER			"/usr/bin/finger"
#endif
#ifndef LDAP_SENDMAIL
#define LDAP_SENDMAIL		"/usr/lib/sendmail"
#endif

#endif /* _LDAP_CONFIG_H */
