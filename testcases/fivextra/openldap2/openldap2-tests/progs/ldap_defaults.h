/* $OpenLDAP: pkg/ldap/include/ldap_defaults.h,v 1.1.8.7 2002/01/04 20:38:15 kurt Exp $ */
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
/* Portions
 * Copyright (c) 1994 Regents of the University of Michigan.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this notice is preserved and that due credit is given
 * to the University of Michigan at Ann Arbor. The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific prior written permission. This software
 * is provided ``as is'' without express or implied warranty.
 */

/*
 * This file controls defaults for OpenLDAP package.
 * You probably do not need to edit the defaults provided by this file.
 */

#ifndef _LDAP_DEFAULTS_H
#define _LDAP_DEFAULTS_H


#include "ldap_config.h"

#define LDAP_CONF_FILE	 LDAP_SYSCONFDIR LDAP_DIRSEP "ldap.conf"
#define LDAP_USERRC_FILE "ldaprc"
#define LDAP_ENV_PREFIX "LDAP"

/* default ldapi:// socket */
#define LDAPI_SOCK LDAP_RUNDIR LDAP_DIRSEP "ldapi"

/* default file: URI prefix */
#define LDAP_FILE_URI_PREFIX "file://" LDAP_TMPDIR LDAP_DIRSEP

/*
 * SHARED DEFINITIONS - other things you can change
 */
	/* default attribute to use when sorting entries, NULL => sort by DN */
#define SORT_ATTR	NULL
	/* default count of DN components to show in entry displays */
#define DEFAULT_RDNCOUNT	2
	/* default config file locations */
#define FILTERFILE	LDAP_SYSCONFDIR LDAP_DIRSEP "ldapfilter.conf"
#define TEMPLATEFILE	LDAP_SYSCONFDIR LDAP_DIRSEP "ldaptemplates.conf"
#define SEARCHFILE	LDAP_SYSCONFDIR LDAP_DIRSEP "ldapsearchprefs.conf"
#define FRIENDLYFILE	LDAP_DATADIR LDAP_DIRSEP "ldapfriendly"

/*
 * FINGER DEFINITIONS
 */
	/* banner to print */
#define FINGER_BANNER		"OpenLDAP Finger Service...\r\n"
	/* who to report errors to */
#define FINGER_ERRORS		"System Administrator"
	/* what to say if no matches are found */
#define FINGER_NOMATCH		"Search failed to find anything.\r\n"
	/* what to say if the service may be unavailable */
#define FINGER_UNAVAILABLE	\
"The directory service may be temporarily unavailable.\r\n\
Please try again later.\r\n"
	/* printed if a match has no email address - for disptmp default */
#define FINGER_NOEMAIL1	"None registered in this service."
#define FINGER_NOEMAIL2	NULL
#define FINGER_NOEMAIL	{ FINGER_NOEMAIL1, FINGER_NOEMAIL2, NULL }
	/* maximum number of matches returned */
#define FINGER_SIZELIMIT	50
	/* max number of hits displayed in full before a list is presented */
#define FINGER_LISTLIMIT	1
	/* what to exec for "finger @host" */
#define FINGER_CMD		LDAP_FINGER
	/* how to treat aliases when searching */
#define FINGER_DEREF		LDAP_DEREF_FINDING
	/* attribute to use when sorting results */
#define FINGER_SORT_ATTR	SORT_ATTR
	/* enable ufn support */
#define FINGER_UFN
	/* timeout for searches */
#define FINGER_TIMEOUT		60
	/* number of DN components to show in entry displays */
#define FINGER_RDNCOUNT		DEFAULT_RDNCOUNT	

/*
 * GO500 GOPHER GATEWAY DEFINITIONS
 */
	/* port on which to listen */
#define GO500_PORT	5555
	/* how to handle aliases */
#define GO500_DEREF	LDAP_DEREF_FINDING
	/* attribute to use when sorting results */
#define GO500_SORT_ATTR	SORT_ATTR
	/* timeout for searches */
#define GO500_TIMEOUT	180
	/* enable ufn support */
#define GO500_UFN
	/*
	 * only set and uncomment this if your hostname() does not return
	 * a fully qualified hostname
	 */
/* #define GO500_HOSTNAME	"fully.qualified.hostname.here" */
	/* number of DN components to show in entry displays */
#define GO500_RDNCOUNT		DEFAULT_RDNCOUNT	

/*
 * GO500GW GOPHER GATEWAY DEFINITIONS
 */
	/* where the helpfile lives */
#define GO500GW_HELPFILE	LDAP_DATADIR LDAP_DIRSEP "go500gw.help"
	/* port on which to listen */
#define GO500GW_PORT		7777
	/* timeout on all searches */
#define GO500GW_TIMEOUT		180
	/* enable ufn support */
#define GO500GW_UFN
	/* attribute to use when sorting results */
#define GO500GW_SORT_ATTR	SORT_ATTR
	/*
	 * only set and uncomment this if your hostname() does not return
	 * a fully qualified hostname
	 */
/* #define GO500GW_HOSTNAME	"fully.qualified.hostname.here" */
	/* number of DN components to show in entry displays */
#define GO500GW_RDNCOUNT	DEFAULT_RDNCOUNT	

/*
 * RCPT500 MAIL RESPONDER GATEWAY DEFINITIONS
 */
	/* where the helpfile lives */
#define RCPT500_HELPFILE	LDAP_DATADIR LDAP_DIRSEP "rcpt500.help"
	/* maximum number of matches returned */
#define RCPT500_SIZELIMIT	50
	/* address replies will appear to come from */
#define RCPT500_FROM		"\"Directory Query Program\" <Dir-Query>"
	/* command that will accept an RFC822 message text on standard
	   input, and send it.  sendmail -t does this nicely. */
#define RCPT500_PIPEMAILCMD	LDAP_SENDMAIL " -t"
	/* attribute to use when sorting results */
#define RCPT500_SORT_ATTR	SORT_ATTR
	/* max number of hits displayed in full before a list is presented */
#define RCPT500_LISTLIMIT	1
	/* enable ufn support */
#define RCPT500_UFN
	/* number of DN components to show in entry displays */
#define RCPT500_RDNCOUNT	DEFAULT_RDNCOUNT	

/*
 * MAIL500 MAILER DEFINITIONS
 */
	/* max number of ambiguous matches reported */
#define MAIL500_MAXAMBIGUOUS	10
	/* max subscribers allowed (size limit when searching for them ) */
#define MAIL500_MAXGROUPMEMBERS	LDAP_NO_LIMIT
	/* timeout for all searches */
#define MAIL500_TIMEOUT		180
	/* sendmail location - mail500 needs to exec this */
#define MAIL500_SENDMAIL	LDAP_SENDMAIL

/*
 * UD DEFINITIONS
 */
	/* ud configuration file */
#define UD_CONFIG_FILE		LDAP_SYSCONFDIR LDAP_DIRSEP "ud.conf"
	/* default editor */
#define UD_DEFAULT_EDITOR	LDAP_EDITOR
	/* default bbasename of user config file */
#define UD_USER_CONFIG_FILE	".udrc"
	/* default base where groups are created */
#define UD_WHERE_GROUPS_ARE_CREATED	""
	/* default base below which all groups live */
#define UD_WHERE_ALL_GROUPS_LIVE	""

/*
 * FAX500 DEFINITIONS
 */
	/* how long to wait for searches */
#define FAX_TIMEOUT		180
	/* maximum number of ambiguous matches reported */
#define FAX_MAXAMBIGUOUS	10
	/* maximum number of members allowed */
#define FAX_MAXMEMBERS		LDAP_NO_LIMIT
	/* program to send mail */
#define FAX_SENDMAIL		LDAP_SENDMAIL

/*
 * RP500 DEFINITIONS
 */
	/* prefix to add to non-fully-qualified numbers */
#define RP_PHONEPREFIX	""

/*
 * SLAPD DEFINITIONS
 */
	/* location of the default slapd config file */
#define SLAPD_DEFAULT_CONFIGFILE	LDAP_SYSCONFDIR LDAP_DIRSEP "slapd.conf"
	/* default max deref depth for aliases */
#define SLAPD_DEFAULT_MAXDEREFDEPTH	15	
	/* default sizelimit on number of entries from a search */
#define SLAPD_DEFAULT_SIZELIMIT		500
	/* default timelimit to spend on a search */
#define SLAPD_DEFAULT_TIMELIMIT		3600
	/* minimum max ids that a single index entry can map to in ldbm */
#define SLAPD_LDBM_MIN_MAXIDS		(8192-4)

/* the following DNs must be normalized! */
	/* dn of the default subschema subentry */
#define SLAPD_SCHEMA_DN			"cn=Subschema"
#if 0
	/* dn of the default "monitor" subentry */
#define SLAPD_MONITOR_DN		"cn=Monitor"
	/* dn of the default "config" subentry */
#define SLAPD_CONFIG_DN			"cn=Config"
#endif

#endif /* _LDAP_CONFIG_H */
