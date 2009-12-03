/*      -*- linux-c -*-
 *
 * (C) Copyright IBM Corp. 2004-2008
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * Author(s):
 *      W. David Ashley <dashley@us.ibm.com>
 *      David Judkoivcs <djudkovi@us.ibm.com>
 * 	Renier Morales <renier@openhpi.org>
 *      Anton Pak <anton.pak@pigeonpoint.com>
 *
 */


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <glib.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include "strmsock.h"
#include "oh_domain.h"

extern "C"
{
#include <SaHpi.h>
#include <oHpi.h>
#include <oh_error.h>
#include <oh_init.h>
}

#include "marshal_hpi.h"



/*--------------------------------------------------------------------*/
/* Local definitions                                                  */
/*--------------------------------------------------------------------*/

extern "C"
{

enum tResult
{
   eResultOk,
   eResultError,
   eResultReply,
   eResultClose
};

static bool morph2daemon(void);
static void service_thread(gpointer data, gpointer user_data);
static void HandleInvalidRequest(psstrmsock thrdinst);
static tResult HandleMsg(psstrmsock thrdinst,
			 char *data,
			 SaHpiSessionIdT * sid);

}

#define CLIENT_TIMEOUT 0  // Unlimited
#define PID_FILE "/var/run/openhpid.pid"

static bool stop_server = FALSE;
static bool runasdaemon = TRUE;
static int sock_timeout = CLIENT_TIMEOUT;
static int max_threads = -1;  // unlimited

/* options set by the command line */
static char *pid_file = NULL;
static int verbose_flag = 0;
static struct option long_options[] = {
        {"verbose",   no_argument,       NULL, 'v'},
        {"nondaemon", no_argument,       NULL, 'n'},
        {"config",    required_argument, NULL, 'c'},
        {"port",      required_argument, NULL, 'p'},
        {"pidfile",   required_argument, NULL, 'f'},
        {"timeout",   required_argument, NULL, 's'},
        {"threads",   required_argument, NULL, 't'},
        {0, 0, 0, 0}
};

/* verbose macro */
#define PVERBOSE1(msg, ...) if (verbose_flag) dbg(msg, ## __VA_ARGS__)
#define PVERBOSE2(msg, ...) if (verbose_flag) err(msg, ## __VA_ARGS__)
#define PVERBOSE3(msg, ...) if (verbose_flag) printf("CRITICAL: "msg, ## __VA_ARGS__)

/*--------------------------------------------------------------------*/
/* Function: display_help                                             */
/*--------------------------------------------------------------------*/

void display_help(void)
{
        printf("Help for openhpid:\n\n");
        printf("   openhpid -c conf_file [-v] [-p port] [-f pidfile]\n\n");
        printf("   -c conf_file  conf_file is the path/name of the configuration file.\n");
        printf("                 This option is required unless the environment\n");
        printf("                 variable OPENHPI_CONF has been set to a valid\n");
        printf("                 configuration file.\n");
        printf("   -v            This option causes the daemon to display verbose\n");
        printf("                 messages. This option is optional.\n");
        printf("   -p port       This overrides the default listening port (%d) of\n",
	       OPENHPI_DEFAULT_DAEMON_PORT);
        printf("                 the daemon. This option is optional.\n");
        printf("   -f pidfile    This overrides the default name/location for the daemon.\n");
        printf("                 pid file. This option is optional.\n");
        printf("   -s seconds    This overrides the default socket read timeout of 30\n");
        printf("                 minutes. This option is optional.\n");
        printf("   -t threads    This sets the maximum number of connection threads.\n");
        printf("                 The default is umlimited. This option is optional.\n");
        printf("   -n            This forces the code to run as a foreground process\n");
        printf("                 and NOT as a daemon. The default is to run as\n");
        printf("                 a daemon. This option is optional.\n\n");
        printf("A typical invocation might be\n\n");
        printf("   ./openhpid -c /etc/openhpi/openhpi.conf\n\n");
}

/*--------------------------------------------------------------------*/
/* Function: main                                                     */
/*--------------------------------------------------------------------*/

int main (int argc, char *argv[])
{
	GThreadPool *thrdpool;
        int port, c, option_index = 0;
        char *portstr;
        char * configfile = NULL;
        char pid_buf[256];
        int pfile, len, pid = 0;

        /* get the command line options */
        while (1) {
                c = getopt_long(argc, argv, "nvc:p:f:s:t:", long_options,
                                &option_index);
                /* detect when done scanning options */
                if (c == -1) {
                        break;
                }
                switch (c) {
                case 0:
                        /* no need to do anything here */
                        break;
                case 'c':
                        setenv("OPENHPI_CONF", optarg, 1);
                        break;
                case 'p':
                        setenv("OPENHPI_DAEMON_PORT", optarg, 1);
                        break;
                case 'v':
                        verbose_flag = 1;
                        break;
                case 'f':
                        pid_file = (char *)malloc(strlen(optarg) + 1);
                        strcpy(pid_file, optarg);
                        break;
                case 's':
                        sock_timeout = atoi(optarg);
                        if (sock_timeout < 0) {
                                sock_timeout = CLIENT_TIMEOUT;
                        }
                        break;
                case 't':
                        max_threads = atoi(optarg);
                        if (max_threads < -1 || max_threads == 0) {
                                max_threads = -1;
                        }
                        break;
                case 'n':
                        runasdaemon = FALSE;
                        break;
                case '?':
                        display_help();
                        exit(0);
                default:
			/* they obviously need it */
			display_help();
                        exit(-1);
                }
        }

        if (pid_file == NULL) {
                pid_file = (char *)malloc(strlen(PID_FILE) + 1);
                strcpy(pid_file, PID_FILE);
        }

        if (optind < argc) {
                err("Error: Unknown command line option specified .\n");
                err("       Aborting execution.\n\n");
		display_help();
                exit(-1);
        }

        // see if we are already running
        if ((pfile = open(pid_file, O_RDONLY)) > 0) {
                len = read(pfile, pid_buf, sizeof(pid_buf) - 1);
                close(pfile);
                pid_buf[len] = '\0';
                pid = atoi(pid_buf);
                if (pid && (pid == getpid() || kill(pid, 0) < 0)) {
                        unlink(pid_file);
                } else {
                        // there is already a server running
                        err("Error: There is already a server running .\n");
                        err("       Aborting execution.\n");
                        exit(1);
                }
        }

        // write the pid file
        pfile = open(pid_file, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP);
        if (pfile == -1) {
                // there is already a server running
                err("Error: Cannot open PID file .\n");
                err("       Aborting execution.\n\n");
                display_help();
                exit(1);
        }
        snprintf(pid_buf, sizeof(pid_buf), "%d\n", (int)getpid());
        len = write(pfile, pid_buf, strlen(pid_buf));
        close(pfile);

        // see if we have a valid configuration file
        char *cfgfile = getenv("OPENHPI_CONF");
        if (cfgfile == NULL) {
                err("Error: Configuration file not specified .\n");
                err("       Aborting execution.\n\n");
		display_help();
                exit(-1);
        }
        if (!g_file_test(cfgfile, G_FILE_TEST_EXISTS)) {
                err("Error: Configuration file does not exist.\n");
                err("       Aborting execution.\n\n");
		display_help();
                exit(-1);
        }
        configfile = (char *) malloc(strlen(cfgfile) + 1);
        strcpy(configfile, cfgfile);

        // get our listening port
        portstr = getenv("OPENHPI_DAEMON_PORT");
        if (portstr == NULL) {
                port =  OPENHPI_DEFAULT_DAEMON_PORT;
        }
        else {
                port =  atoi(portstr);
        }

        // become a daemon
        if (!morph2daemon()) {
		exit(8);
	}

        if (!g_thread_supported()) {
                g_thread_init(NULL);
        }

	if (oh_init()) { // Initialize OpenHPI
		err("There was an error initializing OpenHPI");
		return 8;
	}

	// create the thread pool
        thrdpool = g_thread_pool_new(service_thread, NULL, max_threads, FALSE, NULL);

        // create the server socket
	psstrmsock servinst = new sstrmsock;
	if (servinst->Create(port)) {
		err("Error creating server socket.\n");
		g_thread_pool_free(thrdpool, FALSE, TRUE);
                delete servinst;
		return 8;
	}

        // announce ourselves
        dbg("%s started.\n", argv[0]);
        dbg("OPENHPI_CONF = %s\n", configfile);
        dbg("OPENHPI_DAEMON_PORT = %d\n\n", port);

        // wait for a connection and then service the connection
	while (TRUE) {

		if (stop_server) {
			break;
		}

		if (servinst->Accept()) {
			PVERBOSE1("Error accepting server socket.");
			break;
		}

		PVERBOSE1("### Spawning thread to handle connection. ###");
		psstrmsock thrdinst = new sstrmsock(*servinst);
		g_thread_pool_push(thrdpool, (gpointer)thrdinst, NULL);

        
	}

	servinst->CloseSrv();
	PVERBOSE1("Server socket closed.");

        // ensure all threads are complete
	g_thread_pool_free(thrdpool, FALSE, TRUE);

	delete servinst;
	return 0;
}


/*--------------------------------------------------------------------*/
/* Function: morph2daemon                                             */
/*--------------------------------------------------------------------*/

static bool morph2daemon(void)
{
        char pid_buf[SA_HPI_MAX_NAME_LENGTH];
        int pid_fd;
	int ret;

	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
		return false;
	}

	if (runasdaemon) {
		pid_t pid = fork();
		if (pid < 0) {
			return false;
		} else if (pid != 0) { // parent process
			exit( 0 );
		}

                // Become the session leader
		setsid();
                // Second fork to make sure we are detached from any
                // controlling terminal.
		pid = fork();
		if (pid < 0) {
			return false;
		} else if (pid != 0) { // parent process
			exit(0);
		}

                // Rreate the pid file (overwrite of old pid file is ok)
        	unlink(pid_file);
        	pid_fd =
                        open(pid_file, O_WRONLY|O_CREAT,
                             S_IRUSR|S_IWUSR|S_IRGRP);
        	snprintf(pid_buf, SA_HPI_MAX_NAME_LENGTH,
                         "%d\n", (int)getpid());
        	ret = write(pid_fd, pid_buf, strlen(pid_buf));
        	close(pid_fd);

		//chdir("/");
		umask(0); // Reset default file permissions
                
                // Close unneeded inherited file descriptors
                // Keep stdout and stderr open if they already are.
#ifdef NR_OPEN
		for (int i = 3; i < NR_OPEN; i++) {
#else
                for (int i = 3; i < 1024; i++) {
#endif
			close(i);
		}
	}

	return true;
}


/*--------------------------------------------------------------------*/
/* Function: service_thread                                           */
/*--------------------------------------------------------------------*/

static void service_thread(gpointer data, gpointer user_data)
{
	psstrmsock thrdinst = (psstrmsock) data;
        bool stop = false;
	char buf[dMaxMessageLength];
        tResult result;
        gpointer thrdid = g_thread_self();
        SaHpiSessionIdT session_id = 0;

	PVERBOSE1("%p Servicing connection.", thrdid);

        /* set the read timeout for the socket */
        thrdinst->SetReadTimeout(sock_timeout);

        PVERBOSE1("### service_thread, thrdid [%p] ###", (void *)thrdid);

	while (stop == false) {
                if (thrdinst->ReadMsg(buf)) {
                        if (thrdinst->GetErrcode() == EWOULDBLOCK) {
                                PVERBOSE3("%p Timeout reading socket.\n", thrdid);
                        } else {
                                PVERBOSE3("%p Error reading socket.\n", thrdid);
                        }
                        goto thrd_cleanup;
                }
                else {
                        switch( thrdinst->header.m_type ) {
                        case eMhMsg:
                                result = HandleMsg(thrdinst, buf, &session_id);
                                // marshal error ?
                                if (result == eResultError) {
                                        PVERBOSE3("%p Invalid API found.\n", thrdid);
                                        HandleInvalidRequest(thrdinst);
                                }
                                // done ?
                                if (result == eResultClose) {
                                        stop = true;
                                }
                                break;
                        default:
                                PVERBOSE3("%p Error in socket read buffer data.\n", thrdid);
                                HandleInvalidRequest(thrdinst);
                                break;
                        }
                }
	}

        thrd_cleanup:
        // if necessary, clean up HPI lib data
        if (session_id != 0) {
                saHpiSessionClose( session_id );
        }
        delete thrdinst; // cleanup thread instance data

	PVERBOSE1("%p Connection ended.", thrdid);
	return; // do NOT use g_thread_exit here!
}


/*--------------------------------------------------------------------*/
/* Function: HandleInvalidRequest                                     */
/*--------------------------------------------------------------------*/

void HandleInvalidRequest(psstrmsock thrdinst) {
       gpointer thrdid = g_thread_self();

       /* create and deliver a pong message */
       PVERBOSE2("%p Invalid request.", thrdid);
       thrdinst->MessageHeaderInit(eMhError, 0, thrdinst->header.m_id, 0 );
       thrdinst->WriteMsg(NULL);

       return;
}


/*--------------------------------------------------------------------*/
/* Function: HandleMsg                                                */
/*--------------------------------------------------------------------*/

static tResult HandleMsg(psstrmsock thrdinst,
			 char *data,
                         SaHpiSessionIdT * sid)
{
        cHpiMarshal *hm;
        SaErrorT ret;
        tResult result = eResultReply;
        char *pReq = data + sizeof(cMessageHeader);
        gpointer thrdid = g_thread_self();
        unsigned char request_mFlags;
        
        hm = HpiMarshalFind(thrdinst->header.m_id);

        // keep a local copy of the requests flags
        // before it is overwritten in the MessageHeaderInit below.
        request_mFlags = thrdinst->header.m_flags;

        // init reply header
        thrdinst->MessageHeaderInit((tMessageType) thrdinst->header.m_type, 0,
                                        thrdinst->header.m_id, hm->m_reply_len );


        switch( thrdinst->header.m_id ) {
                case eFsaHpiSessionOpen: {
                        SaHpiDomainIdT  domain_id;
                        SaHpiSessionIdT session_id = 0;
                        void            *securityparams = NULL;
                
                        PVERBOSE1("%p Processing saHpiSessionOpen.", thrdid);
                
                        if ( HpiDemarshalRequest1( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &domain_id ) < 0 )
                                return eResultError;
                
			// patched version ret = saHpiSessionOpen( SAHPI_UNSPECIFIED_DOMAIN_ID, &session_id, securityparams );
                        ret = saHpiSessionOpen( OH_DEFAULT_DOMAIN_ID, &session_id, securityparams );
                        thrdinst->header.m_len = HpiMarshalReply1( hm, pReq, &ret, &session_id );
                
                        // this is used in case the connection ever breaks!
                        *sid = session_id;
                }
                break;
                
                case eFsaHpiSessionClose: {
                        SaHpiSessionIdT session_id;
                
                        PVERBOSE1("%p Processing saHpiSessionClose.", thrdid);
                
                        if ( HpiDemarshalRequest1( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id ) < 0 )
                                return eResultError;
                
                        ret = saHpiSessionClose( session_id );
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                        result = eResultClose;
                        *sid = 0;
                }
                break;
                
                case eFsaHpiDiscover: {
                        SaHpiSessionIdT session_id;
                
                        PVERBOSE1("%p Processing saHpiDiscover.", thrdid);
                
                        if ( HpiDemarshalRequest1( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id ) < 0 )
                                return eResultError;
                
                        ret = saHpiDiscover( session_id );
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                }
                break;
                
                case eFsaHpiDomainInfoGet: {
                        SaHpiSessionIdT  session_id;
                        SaHpiDomainInfoT domain_info;
                
                        PVERBOSE1("%p Processing saHpiDomainInfoGet.", thrdid);
                
                        if ( HpiDemarshalRequest1( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id ) < 0 )
                                return eResultError;
                
                        ret = saHpiDomainInfoGet( session_id, &domain_info );
                
                        thrdinst->header.m_len = HpiMarshalReply1( hm, pReq, &ret, &domain_info );
                }
                break;
                
                case eFsaHpiDrtEntryGet: {
                        SaHpiSessionIdT session_id;
                        SaHpiEntryIdT   entry_id;
                        SaHpiEntryIdT   next_entry_id = 0;
                        SaHpiDrtEntryT  drt_entry;
                
                        PVERBOSE1("%p Processing saHpiDrtEntryGet.", thrdid);
                
                        if ( HpiDemarshalRequest2( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &entry_id ) < 0 )
                                return eResultError;
                
                        ret = saHpiDrtEntryGet( session_id, entry_id, &next_entry_id,
                                                &drt_entry );
                
                        thrdinst->header.m_len = HpiMarshalReply2( hm, pReq, &ret, &next_entry_id, &drt_entry );
                }
                break;
                
                case eFsaHpiDomainTagSet: {
                        SaHpiSessionIdT  session_id;
                        SaHpiTextBufferT domain_tag;
                
                        PVERBOSE1("%p Processing saHpiDomainTagSet.", thrdid);
                
                        if ( HpiDemarshalRequest2( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &domain_tag ) < 0 )
                                return eResultError;
                
                        ret = saHpiDomainTagSet( session_id, &domain_tag );
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                }
                break;
                
                case eFsaHpiRptEntryGet: {
                        SaHpiSessionIdT session_id;
                        SaHpiEntryIdT   entry_id;
                        SaHpiEntryIdT   next_entry_id = 0; // for valgring
                        SaHpiRptEntryT  rpt_entry;
                
                        PVERBOSE1("%p Processing saHpiRptEntryGet.", thrdid);
                
                        if ( HpiDemarshalRequest2( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &entry_id ) < 0 )
                                return eResultError;
                
                        ret = saHpiRptEntryGet( session_id, entry_id, &next_entry_id, &rpt_entry );
                
                        thrdinst->header.m_len = HpiMarshalReply2( hm, pReq, &ret, &next_entry_id, &rpt_entry );
                }
                break;
                
                case eFsaHpiRptEntryGetByResourceId: {
                        SaHpiSessionIdT  session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiRptEntryT   rpt_entry;
                
                        PVERBOSE1("%p Processing saHpiRptEntryGetByResourceId.", thrdid);
                
                        if ( HpiDemarshalRequest2( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id ) < 0 )
                                return eResultError;
                
                        ret = saHpiRptEntryGetByResourceId( session_id, resource_id, &rpt_entry );
                
                        thrdinst->header.m_len = HpiMarshalReply1( hm, pReq, &ret, &rpt_entry );
                }
                break;
                
                case eFsaHpiResourceSeveritySet: {
                        SaHpiSessionIdT  session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiSeverityT   severity;
                
                        PVERBOSE1("%p Processing saHpiResourceSeveritySet.", thrdid);
                
                        if ( HpiDemarshalRequest3( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &severity ) < 0 )
                                return eResultError;
                
                        ret = saHpiResourceSeveritySet( session_id,
                                                        resource_id, severity );
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                }
                break;
                
                case eFsaHpiResourceTagSet:
                        {
                        SaHpiSessionIdT  session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiTextBufferT resource_tag;
                
                        PVERBOSE1("%p Processing saHpiResourceTagSet.", thrdid);
                
                        if ( HpiDemarshalRequest3( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &resource_tag ) < 0 )
                                return eResultError;
                
                        ret = saHpiResourceTagSet( session_id, resource_id, &resource_tag );
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                }
                break;

                case eFsaHpiMyEntityPathGet: {
                        SaHpiSessionIdT session_id;
                        SaHpiEntityPathT entity_path;

                        PVERBOSE1("%p Processing saHpiMyEntityPathGet.", thrdid);

                        if ( HpiDemarshalRequest1( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id ) < 0 )
                                return eResultError;

                        ret = saHpiMyEntityPathGet( session_id, &entity_path );

                        thrdinst->header.m_len = HpiMarshalReply1( hm, pReq, &ret, &entity_path );
                }
                break;
                
                case eFsaHpiResourceIdGet: {
                        SaHpiSessionIdT session_id;
                        SaHpiResourceIdT resource_id = 0;
                
                        PVERBOSE1("%p Processing saHpiResourceIdGet.", thrdid);
                
                        if ( HpiDemarshalRequest1( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id ) < 0 )
                                return eResultError;
                
                        ret = saHpiResourceIdGet( session_id, &resource_id );
                
                        thrdinst->header.m_len = HpiMarshalReply1( hm, pReq, &ret, &resource_id );
                }
                break;

                case eFsaHpiGetIdByEntityPath: {
                        /* IN params */
                        SaHpiSessionIdT session_id;
                        SaHpiEntityPathT entity_path;
                        SaHpiRdrTypeT instrument_type;
                        /* INOUT params */
                        SaHpiUint32T instance_id;
                        /* OUT params */
                        SaHpiResourceIdT resource_id;
                        SaHpiInstrumentIdT instrument_id;
                        SaHpiUint32T rpt_update_count;
                
                        PVERBOSE1("%p Processing saHpiGetIdByEntityPath.", thrdid);
                
                        if ( HpiDemarshalRequest4( request_mFlags & dMhEndianBit,
                                                   hm, pReq,
                                                   &session_id, &entity_path,
                                                   &instrument_type, &instance_id ) < 0 )
                                return eResultError;
                
                        ret = saHpiGetIdByEntityPath( session_id, entity_path,
                                                      instrument_type, &instance_id,
                                                      &resource_id, &instrument_id,
                                                      &rpt_update_count );
                
                        thrdinst->header.m_len = HpiMarshalReply4( hm, pReq, &ret,
                                                                   &instance_id,
                                                                   &resource_id,
                                                                   &instrument_id,
                                                                   &rpt_update_count );
                }
                break;

                case eFsaHpiGetChildEntityPath: {
                        /* IN params */
                        SaHpiSessionIdT session_id;
                        SaHpiEntityPathT parent_ep;
                        /* INOUT params */
                        SaHpiUint32T instance_id;
                        /* OUT params */
                        SaHpiEntityPathT child_ep;
                        SaHpiUint32T rpt_update_count;
                
                        PVERBOSE1("%p Processing saHpiGetChildEntityPath.", thrdid);
                
                        if ( HpiDemarshalRequest3( request_mFlags & dMhEndianBit,
                                                   hm, pReq,
                                                   &session_id, &parent_ep,
                                                   &instance_id ) < 0 )
                                return eResultError;
                
                        ret = saHpiGetChildEntityPath( session_id, parent_ep,
                                                       &instance_id, &child_ep,
                                                       &rpt_update_count );
                
                        thrdinst->header.m_len = HpiMarshalReply3( hm, pReq, &ret,
                                                                   &instance_id,
                                                                   &child_ep,
                                                                   &rpt_update_count );
                }
                break;

                case eFsaHpiResourceFailedRemove: {
                        SaHpiSessionIdT session_id;
                        SaHpiResourceIdT resource_id;

                        PVERBOSE1("%p Processing saHpiResourceFailedRemove.", thrdid);
                
                        if ( HpiDemarshalRequest2( request_mFlags & dMhEndianBit,
                                                   hm, pReq,
                                                   &session_id, &resource_id ) < 0 )
                                return eResultError;
                
                        ret = saHpiResourceFailedRemove( session_id, resource_id );
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                }
                break;
                
                case eFsaHpiEventLogInfoGet: {
                        SaHpiSessionIdT    session_id;
                        SaHpiResourceIdT   resource_id;
                        SaHpiEventLogInfoT info;
                
                        PVERBOSE1("%p Processing saHpiEventLogInfoGet.", thrdid);
                
                        if ( HpiDemarshalRequest2( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id ) < 0 )
                                return eResultError;
                
                        ret = saHpiEventLogInfoGet( session_id, resource_id, &info );
                
                        thrdinst->header.m_len = HpiMarshalReply1( hm, pReq, &ret, &info );
                }
                break;

                case eFsaHpiEventLogCapabilitiesGet: {
                        SaHpiSessionIdT    session_id;
                        SaHpiResourceIdT   resource_id;
                        SaHpiEventLogCapabilitiesT elcaps;
                
                        PVERBOSE1("%p Processing saHpiEventLogCapabilitiesGet.", thrdid);
                
                        if ( HpiDemarshalRequest2( request_mFlags & dMhEndianBit,
                                                   hm, pReq, &session_id, &resource_id ) < 0 )
                                return eResultError;
                
                        ret = saHpiEventLogCapabilitiesGet( session_id, resource_id, &elcaps );
                
                        thrdinst->header.m_len = HpiMarshalReply1( hm, pReq, &ret, &elcaps );
                }
                break;
                
                case eFsaHpiEventLogEntryGet: {
                        SaHpiSessionIdT       session_id;
                        SaHpiResourceIdT      resource_id;
                        SaHpiEventLogEntryIdT entry_id;
                        SaHpiEventLogEntryIdT prev_entry_id = 0;
                        SaHpiEventLogEntryIdT next_entry_id = 0;
                        SaHpiEventLogEntryT   event_log_entry;
                        SaHpiRdrT             rdr;
                        SaHpiRptEntryT        rpt_entry;
                
                        PVERBOSE1("%p Processing saHpiEventLogEntryGet.", thrdid);
                
                        memset( &rdr, 0, sizeof( SaHpiRdrT ) );
                        memset( &rpt_entry, 0, sizeof( SaHpiRptEntryT ) );
                        memset( &event_log_entry, 0, sizeof( SaHpiEventLogEntryT ) );
                
                        if ( HpiDemarshalRequest3( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &entry_id ) < 0 )
                                return eResultError;
                
                        ret = saHpiEventLogEntryGet( session_id, resource_id, entry_id,
                                                        &prev_entry_id, &next_entry_id,
                                                        &event_log_entry, &rdr, &rpt_entry );
                
                        thrdinst->header.m_len = HpiMarshalReply5( hm, pReq, &ret, &prev_entry_id, &next_entry_id,
                                                                        &event_log_entry, &rdr, &rpt_entry );
                }
                break;
                
                case eFsaHpiEventLogEntryAdd: {
                        SaHpiSessionIdT  session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiEventT      evt_entry;
                
                        PVERBOSE1("%p Processing saHpiEventLogEntryAdd.", thrdid);
                
                        if ( HpiDemarshalRequest3( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &evt_entry ) < 0 )
                                return eResultError;
                
                        ret = saHpiEventLogEntryAdd( session_id, resource_id,
                                                        &evt_entry );
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                }
                break;
                
                case eFsaHpiEventLogClear: {
                        SaHpiSessionIdT  session_id;
                        SaHpiResourceIdT resource_id;
                
                        PVERBOSE1("%p Processing saHpiEventLogClear.", thrdid);
                
                        if ( HpiDemarshalRequest2( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id ) < 0 )
                                return eResultError;
                
                        ret = saHpiEventLogClear( session_id, resource_id );
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                }
                break;
                
                case eFsaHpiEventLogTimeGet: {
                        SaHpiSessionIdT  session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiTimeT       ti;
                
                        PVERBOSE1("%p Processing saHpiEventLogTimeGet.", thrdid);
                
                        if ( HpiDemarshalRequest2( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id ) < 0 )
                                return eResultError;
                
                        ret = saHpiEventLogTimeGet( session_id, resource_id, &ti );
                
                        thrdinst->header.m_len = HpiMarshalReply1( hm, pReq, &ret, &ti );
                }
                break;
                
                case eFsaHpiEventLogTimeSet: {
                        SaHpiSessionIdT  session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiTimeT       ti;
                
                        PVERBOSE1("%p Processing saHpiEventLogTimeSet.", thrdid);
                
                        if ( HpiDemarshalRequest3( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &ti ) < 0 )
                                return eResultError;
                
                        ret = saHpiEventLogTimeSet( session_id, resource_id, ti );
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                }
                break;
                
                case eFsaHpiEventLogStateGet: {
                        SaHpiSessionIdT  session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiBoolT       enable;
                
                        PVERBOSE1("%p Processing saHpiEventLogStateGet.", thrdid);
                
                        if ( HpiDemarshalRequest2( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id ) < 0 )
                                return eResultError;
                
                        ret = saHpiEventLogStateGet( session_id, resource_id, &enable );
                
                        thrdinst->header.m_len = HpiMarshalReply1( hm, pReq, &ret, &enable );
                }
                break;
                
                case eFsaHpiEventLogStateSet: {
                        SaHpiSessionIdT  session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiBoolT       enable;
                
                        PVERBOSE1("%p Processing saHpiEventLogStateSet.", thrdid);
                
                        if ( HpiDemarshalRequest3( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &enable ) < 0 )
                                return eResultError;
                
                        ret = saHpiEventLogStateSet( session_id, resource_id, enable );
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                }
                break;
                
                case eFsaHpiEventLogOverflowReset: {
                        SaHpiSessionIdT  session_id;
                        SaHpiResourceIdT resource_id;
                
                        PVERBOSE1("%p Processing saHpiEventLogOverflowReset.", thrdid);
                
                        if ( HpiDemarshalRequest2( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id ) < 0 )
                                return eResultError;
                
                        ret = saHpiEventLogOverflowReset( session_id, resource_id );
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                }
                break;
                
                case eFsaHpiSubscribe: {
                        SaHpiSessionIdT session_id;
                
                        PVERBOSE1("%p Processing saHpiSubscribe.", thrdid);
                
                        if ( HpiDemarshalRequest1( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id ) < 0 )
                                return eResultError;
                
                        ret = saHpiSubscribe( session_id );
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                }
                break;
                
                case eFsaHpiUnsubscribe: {
                        SaHpiSessionIdT session_id;
                
                        PVERBOSE1("%p Processing saHpiUnsubscribe.", thrdid);
                
                        if ( HpiDemarshalRequest1( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id ) < 0 )
                                return eResultError;
                
                        ret = saHpiUnsubscribe( session_id );
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                }
                break;
                
                case eFsaHpiEventGet: {
                        SaHpiSessionIdT      session_id;
                        SaHpiTimeoutT        timeout;
                        SaHpiEventT          event;
                        SaHpiRdrT            rdr;
                        SaHpiRptEntryT       rpt_entry;
                        SaHpiEvtQueueStatusT status;
                
                        PVERBOSE1("%p Processing saHpiEventGet.", thrdid);
                
                        if ( HpiDemarshalRequest2( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &timeout ) < 0 )
                                return eResultError;
                
                        ret = saHpiEventGet( session_id, timeout, &event, &rdr,
                                                &rpt_entry, &status );
                
                        thrdinst->header.m_len = HpiMarshalReply4( hm, pReq, &ret, &event, &rdr, &rpt_entry, &status );
                }
                break;
                
                case eFsaHpiEventAdd: {
                        SaHpiSessionIdT session_id;
                        SaHpiEventT     event;
                
                        PVERBOSE1("%p Processing saHpiEventAdd.", thrdid);
                
                        if ( HpiDemarshalRequest2( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &event ) < 0 )
                                return eResultError;
                
                        ret = saHpiEventAdd( session_id, &event );
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                }
                break;
                
                case eFsaHpiAlarmGetNext: {
                        SaHpiSessionIdT session_id;
                        SaHpiSeverityT  severity;
                        SaHpiBoolT      unack;
                        SaHpiAlarmT     alarm;
                
                        PVERBOSE1("%p Processing saHpiAlarmGetNext.", thrdid);
                
                        if ( HpiDemarshalRequest4( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &severity,
                                                        &unack, &alarm ) < 0 )
                                return eResultError;
                
                        ret = saHpiAlarmGetNext( session_id, severity, unack, &alarm );
                
                        thrdinst->header.m_len = HpiMarshalReply1( hm, pReq, &ret, &alarm );
                }
                break;
                
                case eFsaHpiAlarmGet: {
                        SaHpiSessionIdT session_id;
                        SaHpiAlarmIdT   alarm_id;
                        SaHpiAlarmT     alarm;
                
                        PVERBOSE1("%p Processing saHpiAlarmGet.", thrdid);
                
                        if ( HpiDemarshalRequest2( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &alarm_id ) < 0 )
                                return eResultError;
                
                        ret = saHpiAlarmGet( session_id, alarm_id, &alarm );
                
                        thrdinst->header.m_len = HpiMarshalReply1( hm, pReq, &ret, &alarm );
                }
                break;
                
                case eFsaHpiAlarmAcknowledge: {
                        SaHpiSessionIdT session_id;
                        SaHpiAlarmIdT   alarm_id;
                        SaHpiSeverityT  severity;
                
                        PVERBOSE1("%p Processing saHpiAlarmAcknowledge.", thrdid);
                
                        if ( HpiDemarshalRequest3( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &alarm_id,
                                                        &severity ) < 0 )
                                return eResultError;
                
                        ret = saHpiAlarmAcknowledge( session_id, alarm_id, severity );
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                }
                break;
                
                case eFsaHpiAlarmAdd: {
                        SaHpiSessionIdT session_id;
                        SaHpiAlarmT     alarm;
                
                        PVERBOSE1("%p Processing saHpiAlarmAdd.", thrdid);
                
                        if ( HpiDemarshalRequest2( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &alarm ) < 0 )
                                return eResultError;
                
                        ret = saHpiAlarmAdd( session_id, &alarm );
                
                        thrdinst->header.m_len = HpiMarshalReply1( hm, pReq, &ret, &alarm );
                }
                break;
                
                case eFsaHpiAlarmDelete: {
                        SaHpiSessionIdT session_id;
                        SaHpiAlarmIdT   alarm_id;
                        SaHpiSeverityT  severity;
                
                        PVERBOSE1("%p Processing saHpiAlarmDelete.", thrdid);
                
                        if ( HpiDemarshalRequest3( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &alarm_id,
                                                        &severity ) < 0 )
                                return eResultError;
                
                        ret = saHpiAlarmDelete( session_id, alarm_id, severity );
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                }
                break;
                
                case eFsaHpiRdrGet: {
                        SaHpiSessionIdT  session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiEntryIdT    entry_id;
                        SaHpiEntryIdT    next_entry_id;
                        SaHpiRdrT        rdr;
                
                        PVERBOSE1("%p Processing saHpiRdrGet.", thrdid);
                
                        if ( HpiDemarshalRequest3( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &entry_id ) < 0 )
                                return eResultError;
                
                        ret = saHpiRdrGet( session_id, resource_id, entry_id,
                                                &next_entry_id, &rdr );
                
                        thrdinst->header.m_len = HpiMarshalReply2( hm, pReq, &ret, &next_entry_id, &rdr );
                }
                break;
                
                case eFsaHpiRdrGetByInstrumentId: {
                        SaHpiSessionIdT    session_id;
                        SaHpiResourceIdT   resource_id;
                        SaHpiRdrTypeT      rdr_type;
                        SaHpiInstrumentIdT inst_id;
                        SaHpiRdrT          rdr;
                
                        PVERBOSE1("%p Processing saHpiRdrGetByInstrumentId.", thrdid);
                
                        if ( HpiDemarshalRequest4( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &rdr_type, &inst_id ) < 0 )
                                return eResultError;
                
                        ret = saHpiRdrGetByInstrumentId( session_id, resource_id, rdr_type,
                                                        inst_id, &rdr );
                
                        thrdinst->header.m_len = HpiMarshalReply1( hm, pReq, &ret, &rdr );
                }
                break;
                
                case eFsaHpiRdrUpdateCountGet: {
                        SaHpiSessionIdT    session_id;
                        SaHpiResourceIdT   resource_id;
                        SaHpiUint32T       rdr_update_count;

                        PVERBOSE1("%p Processing saHpiRdrUpdateCountGet.", thrdid);

                        if ( HpiDemarshalRequest2( request_mFlags & dMhEndianBit,
                                                   hm, pReq, &session_id, &resource_id ) < 0 )
                                return eResultError;

                        ret = saHpiRdrUpdateCountGet( session_id, resource_id, &rdr_update_count );

                        thrdinst->header.m_len = HpiMarshalReply1( hm, pReq, &ret, &rdr_update_count );
                }
                break;

                case eFsaHpiSensorReadingGet: {
                        SaHpiSessionIdT     session_id;
                        SaHpiResourceIdT    resource_id;
                        SaHpiSensorNumT     sensor_num;
                        SaHpiSensorReadingT reading;
                        SaHpiEventStateT    state;
                
                        PVERBOSE1("%p Processing saHpiSensorReadingGet.", thrdid);
                
                        if ( HpiDemarshalRequest3( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &sensor_num ) < 0 )
                                return eResultError;
                
                        ret = saHpiSensorReadingGet( session_id, resource_id,
                                                        sensor_num, &reading, &state );
                
                        thrdinst->header.m_len = HpiMarshalReply2( hm, pReq, &ret, &reading, &state );
                }
                break;
                
                case eFsaHpiSensorThresholdsGet: {
                        SaHpiSessionIdT        session_id;
                        SaHpiResourceIdT       resource_id;
                        SaHpiSensorNumT        sensor_num;
                        SaHpiSensorThresholdsT sensor_thresholds;
                
                        PVERBOSE1("%p Processing saHpiSensorThresholdsGet.", thrdid);
                
                        if ( HpiDemarshalRequest3( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &sensor_num ) < 0 )
                                return eResultError;
                
                        ret = saHpiSensorThresholdsGet( session_id,
                                                        resource_id, sensor_num,
                                                        &sensor_thresholds);
                
                        thrdinst->header.m_len = HpiMarshalReply1( hm, pReq, &ret, &sensor_thresholds );
                }
                break;
                
                case eFsaHpiSensorThresholdsSet: {
                        SaHpiSessionIdT        session_id;
                        SaHpiResourceIdT       resource_id;
                        SaHpiSensorNumT        sensor_num;
                        SaHpiSensorThresholdsT sensor_thresholds;
                
                        PVERBOSE1("%p Processing saHpiSensorThresholdsSet.", thrdid);
                
                        if ( HpiDemarshalRequest4( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &sensor_num, &sensor_thresholds ) < 0 )
                                return eResultError;
                
                        ret = saHpiSensorThresholdsSet( session_id, resource_id,
                                                        sensor_num,
                                                        &sensor_thresholds );
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                }
                break;
                
                case eFsaHpiSensorTypeGet: {
                        SaHpiSessionIdT     session_id;
                        SaHpiResourceIdT    resource_id;
                        SaHpiSensorNumT     sensor_num;
                        SaHpiSensorTypeT    type;
                        SaHpiEventCategoryT category;
                
                        PVERBOSE1("%p Processing saHpiSensorTypeGet.", thrdid);
                
                        if ( HpiDemarshalRequest3( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &sensor_num ) < 0 )
                                return eResultError;
                
                        ret = saHpiSensorTypeGet( session_id, resource_id,
                                                        sensor_num, &type, &category );
                
                        thrdinst->header.m_len = HpiMarshalReply2( hm, pReq, &ret, &type, &category );
                }
                break;
                
                case eFsaHpiSensorEnableGet: {
                        SaHpiSessionIdT  session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiSensorNumT  sensor_num;
                        SaHpiBoolT       enabled;
                
                        PVERBOSE1("%p Processing saHpiSensorEnableGet.", thrdid);
                
                        if ( HpiDemarshalRequest3( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &sensor_num ) < 0 )
                                return eResultError;
                
                        ret = saHpiSensorEnableGet( session_id, resource_id,
                                                        sensor_num, &enabled );
                
                        thrdinst->header.m_len = HpiMarshalReply1( hm, pReq, &ret, &enabled );
                }
                break;
                
                case eFsaHpiSensorEnableSet: {
                        SaHpiSessionIdT  session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiSensorNumT  sensor_num;
                        SaHpiBoolT       enabled;
                
                        PVERBOSE1("%p Processing saHpiSensorEnableSet.", thrdid);
                
                        if ( HpiDemarshalRequest4( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &sensor_num, &enabled ) < 0 )
                                return eResultError;
                
                        ret = saHpiSensorEnableSet( session_id, resource_id,
                                                        sensor_num, enabled );
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                }
                break;
                
                case eFsaHpiSensorEventEnableGet: {
                        SaHpiSessionIdT  session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiSensorNumT  sensor_num;
                        SaHpiBoolT       enables;
                
                        PVERBOSE1("%p Processing saHpiSensorEventEnableGet.", thrdid);
                
                        if ( HpiDemarshalRequest3( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &sensor_num ) < 0 )
                                return eResultError;
                
                        ret = saHpiSensorEventEnableGet( session_id, resource_id,
                                                        sensor_num, &enables );
                
                        thrdinst->header.m_len = HpiMarshalReply1( hm, pReq, &ret, &enables );
                }
                break;
                
                case eFsaHpiSensorEventEnableSet: {
                        SaHpiSessionIdT  session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiSensorNumT  sensor_num;
                        SaHpiBoolT       enables;
                
                        PVERBOSE1("%p Processing saHpiSensorEventEnableSet.", thrdid);
                
                        if ( HpiDemarshalRequest4( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &sensor_num, &enables ) < 0 )
                                return eResultError;
                
                        ret = saHpiSensorEventEnableSet( session_id, resource_id,
                                                        sensor_num, enables );
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                }
                break;
                
                case eFsaHpiSensorEventMasksGet: {
                        SaHpiSessionIdT  session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiSensorNumT  sensor_num;
                        SaHpiEventStateT assert_mask;
                        SaHpiEventStateT deassert_mask;
                
                        PVERBOSE1("%p Processing saHpiSensorEventMasksGet.", thrdid);
                
                        if ( HpiDemarshalRequest5( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &sensor_num, &assert_mask,
                                                        &deassert_mask ) < 0 )
                                return eResultError;
                
                        ret = saHpiSensorEventMasksGet( session_id, resource_id, sensor_num,
                                                        &assert_mask, &deassert_mask );
                
                        thrdinst->header.m_len = HpiMarshalReply2( hm, pReq, &ret, &assert_mask, &deassert_mask );
                }
                break;
                
                case eFsaHpiSensorEventMasksSet: {
                        SaHpiSessionIdT             session_id;
                        SaHpiResourceIdT            resource_id;
                        SaHpiSensorNumT             sensor_num;
                        SaHpiSensorEventMaskActionT action;
                        SaHpiEventStateT            assert_mask;
                        SaHpiEventStateT            deassert_mask;
                
                        PVERBOSE1("%p Processing saHpiSensorEventMasksSet.", thrdid);
                
                        if ( HpiDemarshalRequest6( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &sensor_num, &action, &assert_mask,
                                                        &deassert_mask ) < 0 )
                                return eResultError;
                
                        ret = saHpiSensorEventMasksSet( session_id, resource_id, sensor_num,
                                                        action, assert_mask, deassert_mask );
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                }
                break;
                
                case eFsaHpiControlTypeGet: {
                        SaHpiSessionIdT  session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiCtrlNumT    ctrl_num;
                        SaHpiCtrlTypeT   type;
                
                        PVERBOSE1("%p Processing saHpiControlTypeGet.", thrdid);
                
                        if ( HpiDemarshalRequest3( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &ctrl_num ) < 0 )
                                return eResultError;
                
                        ret = saHpiControlTypeGet( session_id, resource_id, ctrl_num,
                                                        &type );
                
                        thrdinst->header.m_len = HpiMarshalReply1( hm, pReq, &ret, &type );
                }
                break;
                
                case eFsaHpiControlGet: {
                        SaHpiSessionIdT  session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiCtrlNumT    ctrl_num;
                        SaHpiCtrlModeT   ctrl_mode;
                        SaHpiCtrlStateT  ctrl_state;
                
                        PVERBOSE1("%p Processing saHpiControlGet.", thrdid);
                
                        if ( HpiDemarshalRequest4( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &ctrl_num, &ctrl_state ) < 0 )
                                return eResultError;
                
                        ret = saHpiControlGet( session_id, resource_id, ctrl_num,
                                                &ctrl_mode, &ctrl_state );
                
                        thrdinst->header.m_len = HpiMarshalReply2( hm, pReq, &ret, &ctrl_mode, &ctrl_state );
                }
                break;
                
                case eFsaHpiControlSet: {
                        SaHpiSessionIdT  session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiCtrlNumT    ctrl_num;
                        SaHpiCtrlModeT   ctrl_mode;
                        SaHpiCtrlStateT  ctrl_state;
                
                        PVERBOSE1("%p Processing saHpiControlSet.", thrdid);
                
                        if ( HpiDemarshalRequest5( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &ctrl_num, &ctrl_mode, &ctrl_state ) < 0 )
                                return eResultError;
                
                        ret = saHpiControlSet( session_id, resource_id,
                                                ctrl_num, ctrl_mode, &ctrl_state );
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                }
                break;
                
                case eFsaHpiIdrInfoGet: {
                        SaHpiSessionIdT  session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiIdrIdT      idr_id;
                        SaHpiIdrInfoT    info;
                
                        PVERBOSE1("%p Processing saHpiIdrInfoGet.", thrdid);
                
                        if ( HpiDemarshalRequest3( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &idr_id ) < 0 )
                                return eResultError;
                
                        ret = saHpiIdrInfoGet( session_id, resource_id,
                                                idr_id, &info );
                
                        thrdinst->header.m_len = HpiMarshalReply1( hm, pReq, &ret, &info );
                }
                break;
                
                case eFsaHpiIdrAreaHeaderGet: {
                        SaHpiSessionIdT     session_id;
                        SaHpiResourceIdT    resource_id;
                        SaHpiIdrIdT         idr_id;
                        SaHpiIdrAreaTypeT   area;
                        SaHpiEntryIdT       area_id;
                        SaHpiEntryIdT       next;
                        SaHpiIdrAreaHeaderT header;
                
                        PVERBOSE1("%p Processing saHpiIdrAreaHeaderGet.", thrdid);
                
                        if ( HpiDemarshalRequest5( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &idr_id, &area, &area_id ) < 0 )
                                return eResultError;
                
                        ret = saHpiIdrAreaHeaderGet( session_id, resource_id, idr_id,
                                                        area, area_id, &next, &header );
                
                        thrdinst->header.m_len = HpiMarshalReply2( hm, pReq, &ret, &next, &header );
                }
                break;
                
                case eFsaHpiIdrAreaAdd: {
                        SaHpiSessionIdT     session_id;
                        SaHpiResourceIdT    resource_id;
                        SaHpiIdrIdT         idr_id;
                        SaHpiIdrAreaTypeT   area;
                        SaHpiEntryIdT       area_id;
                
                        PVERBOSE1("%p Processing saHpiIdrAreaAdd.", thrdid);
                
                        if ( HpiDemarshalRequest4( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &idr_id, &area ) < 0 )
                                return eResultError;
                
                        ret = saHpiIdrAreaAdd( session_id, resource_id, idr_id,
                                                area, &area_id  );
                
                        thrdinst->header.m_len = HpiMarshalReply1( hm, pReq, &ret, &area_id );
                }
                break;

                case eFsaHpiIdrAreaAddById: {
                        SaHpiSessionIdT     session_id;
                        SaHpiResourceIdT    resource_id;
                        SaHpiIdrIdT         idr_id;
                        SaHpiIdrAreaTypeT   area_type;
                        SaHpiEntryIdT       area_id;
                
                        PVERBOSE1("%p Processing saHpiIdrAreaAddById.", thrdid);
                
                        if ( HpiDemarshalRequest5( request_mFlags & dMhEndianBit,
                                                   hm, pReq,
                                                   &session_id, &resource_id,
                                                   &idr_id, &area_type, &area_id ) < 0 )
                                return eResultError;
                
                        ret = saHpiIdrAreaAddById( session_id, resource_id, idr_id,
                                                   area_type, area_id  );
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                }
                break;
                
                case eFsaHpiIdrAreaDelete: {
                        SaHpiSessionIdT     session_id;
                        SaHpiResourceIdT    resource_id;
                        SaHpiIdrIdT         idr_id;
                        SaHpiEntryIdT       area_id;
                
                        PVERBOSE1("%p Processing saHpiIdrAreaAdd.", thrdid);
                
                        if ( HpiDemarshalRequest4( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &idr_id, &area_id ) < 0 )
                                return eResultError;
                
                        ret = saHpiIdrAreaDelete( session_id, resource_id, idr_id,
                                                        area_id  );
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                }
                break;
                
                case eFsaHpiIdrFieldGet: {
                        SaHpiSessionIdT    session_id;
                        SaHpiResourceIdT   resource_id;
                        SaHpiIdrIdT        idr_id;
                        SaHpiEntryIdT      area_id;
                        SaHpiIdrFieldTypeT type;
                        SaHpiEntryIdT      field_id;
                        SaHpiEntryIdT      next;
                        SaHpiIdrFieldT     field;
                
                        PVERBOSE1("%p Processing saHpiIdrFieldGet.", thrdid);
                
                        if ( HpiDemarshalRequest6( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &idr_id, &area_id, &type, &field_id ) < 0 )
                                return eResultError;
                
                        ret = saHpiIdrFieldGet( session_id, resource_id, idr_id, area_id,
                                                type, field_id, &next, &field );
                
                        thrdinst->header.m_len = HpiMarshalReply2( hm, pReq, &ret, &next, &field );
                }
                break;
                
                case eFsaHpiIdrFieldAdd: {
                        SaHpiSessionIdT    session_id;
                        SaHpiResourceIdT   resource_id;
                        SaHpiIdrIdT        idr_id;
                        SaHpiIdrFieldT     field;
                
                        PVERBOSE1("%p Processing saHpiIdrFieldAdd.", thrdid);
                
                        if ( HpiDemarshalRequest4( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &idr_id, &field ) < 0 )
                                return eResultError;
                
                        ret = saHpiIdrFieldAdd( session_id, resource_id, idr_id,
                                                &field );
                
                        thrdinst->header.m_len = HpiMarshalReply1( hm, pReq, &ret, &field );
                }
                break;

                case eFsaHpiIdrFieldAddById: {
                        SaHpiSessionIdT    session_id;
                        SaHpiResourceIdT   resource_id;
                        SaHpiIdrIdT        idr_id;
                        SaHpiIdrFieldT     field;
                
                        PVERBOSE1("%p Processing saHpiIdrFieldAddById.", thrdid);
                
                        if ( HpiDemarshalRequest4( request_mFlags & dMhEndianBit,
                                                   hm, pReq,
                                                   &session_id, &resource_id,
                                                   &idr_id, &field ) < 0 )
                                return eResultError;
                
                        ret = saHpiIdrFieldAddById( session_id, resource_id,
                                                    idr_id, &field );
                
                        thrdinst->header.m_len = HpiMarshalReply1( hm, pReq, &ret, &field );
                }
                break;
                
                case eFsaHpiIdrFieldSet: {
                        SaHpiSessionIdT    session_id;
                        SaHpiResourceIdT   resource_id;
                        SaHpiIdrIdT        idr_id;
                        SaHpiIdrFieldT     field;
                
                        PVERBOSE1("%p Processing saHpiIdrFieldSet.", thrdid);
                
                        if ( HpiDemarshalRequest4( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &idr_id, &field ) < 0 )
                                return eResultError;
                
                        ret = saHpiIdrFieldSet( session_id, resource_id, idr_id,
                                                &field );
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                }
                break;
                
                case eFsaHpiIdrFieldDelete: {
                        SaHpiSessionIdT  session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiIdrIdT      idr_id;
                        SaHpiEntryIdT    area_id;
                        SaHpiEntryIdT    field_id;
                
                        PVERBOSE1("%p Processing saHpiIdrFieldSet.", thrdid);
                
                        if ( HpiDemarshalRequest5( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &idr_id, &area_id, &field_id ) < 0 )
                                return eResultError;
                
                        ret = saHpiIdrFieldDelete( session_id, resource_id, idr_id,
                                                        area_id, field_id );
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                }
                break;
                
                case eFsaHpiWatchdogTimerGet: {
                        SaHpiSessionIdT   session_id;
                        SaHpiResourceIdT  resource_id;
                        SaHpiWatchdogNumT watchdog_num;
                        SaHpiWatchdogT    watchdog;
                
                        PVERBOSE1("%p Processing saHpiWatchdogTimerGet.", thrdid);
                
                        if ( HpiDemarshalRequest3( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &watchdog_num ) < 0 )
                                return eResultError;
                
                        ret = saHpiWatchdogTimerGet( session_id, resource_id,
                                                        watchdog_num, &watchdog );
                
                        thrdinst->header.m_len = HpiMarshalReply1( hm, pReq, &ret, &watchdog );
                }
                break;
                
                case eFsaHpiWatchdogTimerSet: {
                        SaHpiSessionIdT   session_id;
                        SaHpiResourceIdT  resource_id;
                        SaHpiWatchdogNumT watchdog_num;
                        SaHpiWatchdogT    watchdog;
                
                        PVERBOSE1("%p Processing saHpiWatchdogTimerSet.", thrdid);
                
                        if ( HpiDemarshalRequest4( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &watchdog_num, &watchdog ) < 0 )
                                return eResultError;
                
                        ret = saHpiWatchdogTimerSet( session_id, resource_id,
                                                        watchdog_num, &watchdog );
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                }
                break;
                
                case eFsaHpiWatchdogTimerReset: {
                        SaHpiSessionIdT   session_id;
                        SaHpiResourceIdT  resource_id;
                        SaHpiWatchdogNumT watchdog_num;
                
                        PVERBOSE1("%p Processing saHpiWatchdogTimerReset.", thrdid);
                
                        if ( HpiDemarshalRequest3( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &watchdog_num ) < 0 )
                                return eResultError;
                
                        ret = saHpiWatchdogTimerReset( session_id, resource_id,
                                                        watchdog_num );
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                }
                break;
                
                case eFsaHpiAnnunciatorGetNext: {
                        SaHpiSessionIdT      session_id;
                        SaHpiResourceIdT     resource_id;
                        SaHpiAnnunciatorNumT annun_num;
                        SaHpiSeverityT       severity;
                        SaHpiBoolT           unack;
                        SaHpiAnnouncementT   announcement;
                
                        PVERBOSE1("%p Processing saHpiAnnunciatorGetNext.", thrdid);
                
                        if ( HpiDemarshalRequest6( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &annun_num, &severity, &unack,
                                                        &announcement ) < 0 )
                                return eResultError;
                
                        ret = saHpiAnnunciatorGetNext( session_id, resource_id, annun_num,
                                                        severity, unack, &announcement );
                
                        thrdinst->header.m_len = HpiMarshalReply1( hm, pReq, &ret, &announcement );
                }
                break;
                
                case eFsaHpiAnnunciatorGet: {
                        SaHpiSessionIdT      session_id;
                        SaHpiResourceIdT     resource_id;
                        SaHpiAnnunciatorNumT annun_num;
                        SaHpiEntryIdT        entry_id;
                        SaHpiAnnouncementT   announcement;
                
                        PVERBOSE1("%p Processing saHpiAnnunciatorGet.", thrdid);
                
                        if ( HpiDemarshalRequest4( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &annun_num, &entry_id ) < 0 )
                                return eResultError;
                
                        ret = saHpiAnnunciatorGet( session_id, resource_id, annun_num,
                                                        entry_id, &announcement );
                
                        thrdinst->header.m_len = HpiMarshalReply1( hm, pReq, &ret, &announcement );
                }
                break;
                
                case eFsaHpiAnnunciatorAcknowledge: {
                        SaHpiSessionIdT      session_id;
                        SaHpiResourceIdT     resource_id;
                        SaHpiAnnunciatorNumT annun_num;
                        SaHpiEntryIdT        entry_id;
                        SaHpiSeverityT       severity;
                
                        PVERBOSE1("%p Processing saHpiAnnunciatorAcknowledge.", thrdid);
                
                        if ( HpiDemarshalRequest5( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &annun_num, &entry_id, &severity ) < 0 )
                                return eResultError;
                
                        ret = saHpiAnnunciatorAcknowledge( session_id, resource_id, annun_num,
                                                                entry_id, severity );
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                }
                break;
                
                case eFsaHpiAnnunciatorAdd: {
                        SaHpiSessionIdT      session_id;
                        SaHpiResourceIdT     resource_id;
                        SaHpiAnnunciatorNumT annun_num;
                        SaHpiAnnouncementT   announcement;
                
                        PVERBOSE1("%p Processing saHpiAnnunciatorAdd.", thrdid);
                
                        if ( HpiDemarshalRequest4( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &annun_num, &announcement ) < 0 )
                                return eResultError;
                
                        ret = saHpiAnnunciatorAdd( session_id, resource_id, annun_num,
                                                        &announcement );
                
                        thrdinst->header.m_len = HpiMarshalReply1( hm, pReq, &ret, &announcement );
                }
                break;
                
                case eFsaHpiAnnunciatorDelete: {
                        SaHpiSessionIdT      session_id;
                        SaHpiResourceIdT     resource_id;
                        SaHpiAnnunciatorNumT annun_num;
                        SaHpiEntryIdT        entry_id;
                        SaHpiSeverityT       severity;
                
                        PVERBOSE1("%p Processing saHpiAnnunciatorAdd.", thrdid);
                
                        if ( HpiDemarshalRequest5( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &annun_num, &entry_id, &severity ) < 0 )
                                return eResultError;
                
                        ret = saHpiAnnunciatorDelete( session_id, resource_id, annun_num,
                                                        entry_id, severity );
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                }
                break;
                
                case eFsaHpiAnnunciatorModeGet: {
                        SaHpiSessionIdT       session_id;
                        SaHpiResourceIdT      resource_id;
                        SaHpiAnnunciatorNumT  annun_num;
                        SaHpiAnnunciatorModeT mode;
                
                        PVERBOSE1("%p Processing saHpiAnnunciatorModeGet.", thrdid);
                
                        if ( HpiDemarshalRequest3( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &annun_num ) < 0 )
                                return eResultError;
                
                        ret = saHpiAnnunciatorModeGet( session_id, resource_id, annun_num,
                                                        &mode );
                
                        thrdinst->header.m_len = HpiMarshalReply1( hm, pReq, &ret, &mode );
                }
                break;
                
                case eFsaHpiAnnunciatorModeSet: {
                        SaHpiSessionIdT       session_id;
                        SaHpiResourceIdT      resource_id;
                        SaHpiAnnunciatorNumT  annun_num;
                        SaHpiAnnunciatorModeT mode;
                
                        PVERBOSE1("%p Processing saHpiAnnunciatorModeSet.", thrdid);
                
                        if ( HpiDemarshalRequest4( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &annun_num, &mode ) < 0 )
                                return eResultError;
                
                        ret = saHpiAnnunciatorModeSet( session_id, resource_id, annun_num,
                                                        mode );
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                }
                break;
                
                case eFsaHpiDimiInfoGet: {
                        SaHpiSessionIdT       session_id;
                        SaHpiResourceIdT      resource_id;
                        SaHpiDimiNumT	      dimi_num;
                        SaHpiDimiInfoT        info;
                
                        PVERBOSE1("%p Processing saHpiDimiInfoGet.", thrdid);
                
                        if ( HpiDemarshalRequest3( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &dimi_num ) < 0 )
                                return eResultError;
                
                        ret = saHpiDimiInfoGet( session_id, resource_id, dimi_num,
                                                        &info );
                
                        thrdinst->header.m_len = HpiMarshalReply1( hm, pReq, &ret, &info );
                }
                break;

                case eFsaHpiDimiTestInfoGet: {
                        SaHpiSessionIdT       session_id;
                        SaHpiResourceIdT      resource_id;
                        SaHpiDimiNumT         dimi_num;
                        SaHpiDimiTestNumT     test_num;
                        SaHpiDimiTestT        test;
                
                        PVERBOSE1("%p Processing saHpiDimiTestInfoGet.", thrdid);
                
                        if (HpiDemarshalRequest4(request_mFlags & dMhEndianBit,
                                                 hm, pReq,
                                                 &session_id, &resource_id,
                                                 &dimi_num, &test_num) < 0)
                                return eResultError;
                
                        ret = saHpiDimiTestInfoGet(session_id, resource_id,
                                                   dimi_num, test_num,
                                                   &test);
                
                        thrdinst->header.m_len = HpiMarshalReply1(hm, pReq,
                                                                  &ret, &test);
                }
                break;

                case eFsaHpiDimiTestReadinessGet: {
                        SaHpiSessionIdT       session_id;
                        SaHpiResourceIdT      resource_id;
                        SaHpiDimiNumT         dimi_num;
                        SaHpiDimiTestNumT     test_num;
                        SaHpiDimiReadyT       ready;
                
                        PVERBOSE1("%p Processing saHpiDimiTestReadinessGet.", thrdid);
                
                        if (HpiDemarshalRequest4(request_mFlags & dMhEndianBit,
                                                 hm, pReq,
                                                 &session_id, &resource_id,
                                                 &dimi_num, &test_num) < 0)
                                return eResultError;
                
                        ret = saHpiDimiTestReadinessGet(session_id, resource_id,
                                                        dimi_num, test_num,
                                                        &ready);
                
                        thrdinst->header.m_len = HpiMarshalReply1(hm, pReq,
                                                                  &ret, &ready);
                }
                break;

                case eFsaHpiDimiTestStart: {
                        SaHpiSessionIdT       session_id;
                        SaHpiResourceIdT      resource_id;
                        SaHpiDimiNumT         dimi_num;
                        SaHpiDimiTestNumT     test_num;
                        SaHpiDimiTestVariableParamsListT params_list;
                
                        PVERBOSE1("%p Processing saHpiDimiTestStart.", thrdid);
                
                        if (HpiDemarshalRequest5(request_mFlags & dMhEndianBit,
                                                 hm, pReq,
                                                 &session_id, &resource_id,
                                                 &dimi_num, &test_num,
                                                 &params_list) < 0)
                                return eResultError;
                
                        ret = saHpiDimiTestStart(session_id, resource_id,
                                                 dimi_num, test_num,
                                                 params_list.NumberOfParams,
                                                 params_list.ParamsList);
                
                        free(params_list.ParamsList);
                        thrdinst->header.m_len = HpiMarshalReply0(hm, pReq,
                                                                  &ret);
                }
                break;

                case eFsaHpiDimiTestCancel: {
                        SaHpiSessionIdT       session_id;
                        SaHpiResourceIdT      resource_id;
                        SaHpiDimiNumT         dimi_num;
                        SaHpiDimiTestNumT     test_num;
                
                        PVERBOSE1("%p Processing saHpiDimiTestCancel.", thrdid);
                
                        if (HpiDemarshalRequest4(request_mFlags & dMhEndianBit,
                                                 hm, pReq,
                                                 &session_id, &resource_id,
                                                 &dimi_num, &test_num) < 0)
                                return eResultError;
                
                        ret = saHpiDimiTestCancel(session_id, resource_id,
                                                  dimi_num, test_num);
                
                        thrdinst->header.m_len = HpiMarshalReply0(hm, pReq,
                                                                  &ret);
                }
                break;

                case eFsaHpiDimiTestStatusGet: {
                        SaHpiSessionIdT       session_id;
                        SaHpiResourceIdT      resource_id;
                        SaHpiDimiNumT         dimi_num;
                        SaHpiDimiTestNumT     test_num;
                        SaHpiDimiTestPercentCompletedT percent;
                        SaHpiDimiTestRunStatusT status;
                
                        PVERBOSE1("%p Processing saHpiDimiTestStatusGet.", thrdid);
                
                        if (HpiDemarshalRequest4(request_mFlags & dMhEndianBit,
                                                 hm, pReq,
                                                 &session_id, &resource_id,
                                                 &dimi_num, &test_num) < 0)
                                return eResultError;
                
                        ret = saHpiDimiTestStatusGet(session_id, resource_id,
                                                     dimi_num, test_num,
                                                     &percent, &status);
                
                        thrdinst->header.m_len = HpiMarshalReply2(hm, pReq, &ret,
                                                                  &percent,
                                                                  &status);
                }
                break;

                case eFsaHpiDimiTestResultsGet: {
                        SaHpiSessionIdT       session_id;
                        SaHpiResourceIdT      resource_id;
                        SaHpiDimiNumT         dimi_num;
                        SaHpiDimiTestNumT     test_num;
                        SaHpiDimiTestResultsT results;
                
                        PVERBOSE1("%p Processing saHpiDimiTestResultsGet.", thrdid);
                
                        if (HpiDemarshalRequest4(request_mFlags & dMhEndianBit,
                                                 hm, pReq,
                                                 &session_id, &resource_id,
                                                 &dimi_num, &test_num) < 0)
                                return eResultError;
                
                        ret = saHpiDimiTestResultsGet(session_id, resource_id,
                                                      dimi_num, test_num,
                                                      &results);
                
                        thrdinst->header.m_len = HpiMarshalReply1(hm, pReq,
                                                                  &ret, &results);
                }
                break;

                case eFsaHpiFumiSpecInfoGet: {
                        SaHpiSessionIdT session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiFumiNumT fumi_num;
                        SaHpiFumiSpecInfoT spec_info;

                        PVERBOSE1("%p Processing saHpiFumiSpecInfoGet.", thrdid);
                        if (HpiDemarshalRequest3(request_mFlags & dMhEndianBit,
                                        hm, pReq,
                                        &session_id, &resource_id, &fumi_num) < 0)
                                return eResultError;

                        ret = saHpiFumiSpecInfoGet(session_id, resource_id,
                                fumi_num, &spec_info );

                        thrdinst->header.m_len = HpiMarshalReply1(hm, pReq, &ret, &spec_info);
                }
                break;

                case eFsaHpiFumiServiceImpactGet: {
                        SaHpiSessionIdT session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiFumiNumT fumi_num;
                        SaHpiFumiServiceImpactDataT service_impact;

                        PVERBOSE1("%p Processing saHpiFumiServiceImpactGet.", thrdid);
                        if (HpiDemarshalRequest3(request_mFlags & dMhEndianBit,
                                        hm, pReq,
                                        &session_id, &resource_id, &fumi_num) < 0)
                                return eResultError;

                        ret = saHpiFumiServiceImpactGet(session_id, resource_id,
                                fumi_num, &service_impact );

                        thrdinst->header.m_len = HpiMarshalReply1(hm, pReq, &ret, &service_impact);
                }
                break;

                case eFsaHpiFumiSourceSet: {
                        SaHpiSessionIdT session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiFumiNumT fumi_num;
			SaHpiBankNumT bank_num;
			SaHpiTextBufferT source_uri;

                        PVERBOSE1("%p Processing saHpiFumiSourceSet.", thrdid);
                        if (HpiDemarshalRequest5(request_mFlags & dMhEndianBit,
                                        hm, pReq,
                                        &session_id, &resource_id,
                                        &fumi_num, &bank_num, &source_uri) < 0)
                                return eResultError;

                        ret = saHpiFumiSourceSet(session_id, resource_id,
                                fumi_num, bank_num, &source_uri);

                        thrdinst->header.m_len = HpiMarshalReply0(hm, pReq, &ret);
                }
                break;

                case eFsaHpiFumiSourceInfoValidateStart: {
                        SaHpiSessionIdT session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiFumiNumT fumi_num;
			SaHpiBankNumT bank_num;

                        PVERBOSE1("%p Processing saHpiFumiSourceInfoValidateStart.", thrdid);
                        if (HpiDemarshalRequest4(request_mFlags & dMhEndianBit,
                                        hm, pReq,
                                        &session_id, &resource_id,
                                        &fumi_num, &bank_num) < 0)
                                return eResultError;

                        ret = saHpiFumiSourceInfoValidateStart(session_id, resource_id,
                                fumi_num, bank_num);

                        thrdinst->header.m_len = HpiMarshalReply0(hm, pReq, &ret);
                }
                break;

                case eFsaHpiFumiSourceInfoGet: {
                        SaHpiSessionIdT session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiFumiNumT fumi_num;
			SaHpiBankNumT bank_num;
			SaHpiFumiSourceInfoT source_info;

                        PVERBOSE1("%p Processing saHpiFumiSourceInfoGet.", thrdid);
                        if (HpiDemarshalRequest4(request_mFlags & dMhEndianBit,
                                        hm, pReq,
                                        &session_id, &resource_id,
                                        &fumi_num, &bank_num) < 0)
                                return eResultError;

                        ret = saHpiFumiSourceInfoGet(session_id, resource_id,
                                fumi_num, bank_num, &source_info);

                        thrdinst->header.m_len = HpiMarshalReply1(hm, pReq, &ret,
				&source_info);
                }
                break;

                case eFsaHpiFumiSourceComponentInfoGet: {
                        SaHpiSessionIdT session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiFumiNumT fumi_num;
                        SaHpiBankNumT bank_num;
                        SaHpiEntryIdT entry_id;
                        SaHpiEntryIdT next_entry_id = 0;
                        SaHpiFumiComponentInfoT component_info;

                        PVERBOSE1("%p Processing saHpiFumiSourceComponentInfoGet.", thrdid);
                        if (HpiDemarshalRequest5(request_mFlags & dMhEndianBit,
                                        hm, pReq,
                                        &session_id, &resource_id,
                                        &fumi_num, &bank_num,
                                        &entry_id ) < 0)
                                return eResultError;

                        ret = saHpiFumiSourceComponentInfoGet(session_id, resource_id,
                                fumi_num, bank_num, entry_id, &next_entry_id,
                                &component_info );

                        thrdinst->header.m_len = HpiMarshalReply2(hm, pReq, &ret,
                            &next_entry_id, &component_info);
                }
                break;

                case eFsaHpiFumiTargetInfoGet: {
                        SaHpiSessionIdT session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiFumiNumT fumi_num;
			SaHpiBankNumT bank_num;
			SaHpiFumiBankInfoT bank_info;

                        PVERBOSE1("%p Processing saHpiFumiTargetInfoGet.", thrdid);
                        if (HpiDemarshalRequest4(request_mFlags & dMhEndianBit,
                                        hm, pReq,
                                        &session_id, &resource_id,
                                        &fumi_num, &bank_num) < 0)
                                return eResultError;

                        ret = saHpiFumiTargetInfoGet(session_id, resource_id,
                                fumi_num, bank_num, &bank_info);

                        thrdinst->header.m_len = HpiMarshalReply1(hm, pReq, &ret,
				&bank_info);
                }
                break;

                case eFsaHpiFumiTargetComponentInfoGet: {
                        SaHpiSessionIdT session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiFumiNumT fumi_num;
                        SaHpiBankNumT bank_num;
                        SaHpiEntryIdT entry_id;
                        SaHpiEntryIdT next_entry_id = 0;
                        SaHpiFumiComponentInfoT component_info;

                        PVERBOSE1("%p Processing saHpiFumiTargetComponentInfoGet.", thrdid);
                        if (HpiDemarshalRequest5(request_mFlags & dMhEndianBit,
                                        hm, pReq,
                                        &session_id, &resource_id,
                                        &fumi_num, &bank_num,
                                        &entry_id ) < 0)
                                return eResultError;

                        ret = saHpiFumiTargetComponentInfoGet(session_id, resource_id,
                                fumi_num, bank_num, entry_id, &next_entry_id,
                                &component_info );

                        thrdinst->header.m_len = HpiMarshalReply2(hm, pReq, &ret,
                            &next_entry_id, &component_info);
                }
                break;

                case eFsaHpiFumiLogicalTargetInfoGet: {
                        SaHpiSessionIdT session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiFumiNumT fumi_num;
                        SaHpiFumiLogicalBankInfoT bank_info;

                        PVERBOSE1("%p Processing saHpiFumiLogicalTargetInfoGet.", thrdid);
                        if (HpiDemarshalRequest3(request_mFlags & dMhEndianBit,
                                        hm, pReq,
                                        &session_id, &resource_id,
                                        &fumi_num) < 0)
                                return eResultError;

                        ret = saHpiFumiLogicalTargetInfoGet(session_id, resource_id,
                                fumi_num, &bank_info);

                        thrdinst->header.m_len = HpiMarshalReply1(hm, pReq, &ret,
                            &bank_info);
                }
                break;

                case eFsaHpiFumiLogicalTargetComponentInfoGet: {
                        SaHpiSessionIdT session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiFumiNumT fumi_num;
                        SaHpiEntryIdT entry_id;
                        SaHpiEntryIdT next_entry_id = 0;
                        SaHpiFumiLogicalComponentInfoT component_info;

                        PVERBOSE1("%p Processing saHpiFumiLogicalTargetComponentInfoGet.", thrdid);
                        if (HpiDemarshalRequest4(request_mFlags & dMhEndianBit,
                                        hm, pReq,
                                        &session_id, &resource_id,
                                        &fumi_num, &entry_id ) < 0)
                                return eResultError;

                        ret = saHpiFumiLogicalTargetComponentInfoGet(session_id, resource_id,
                                fumi_num, entry_id, &next_entry_id,
                                &component_info );

                        thrdinst->header.m_len = HpiMarshalReply2(hm, pReq, &ret,
                            &next_entry_id, &component_info);
                }
                break;

                case eFsaHpiFumiBackupStart: {
                        SaHpiSessionIdT session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiFumiNumT fumi_num;

                        PVERBOSE1("%p Processing saHpiFumiBackupStart.", thrdid);
                        if (HpiDemarshalRequest3(request_mFlags & dMhEndianBit,
                                        hm, pReq,
                                        &session_id, &resource_id,
                                        &fumi_num) < 0)
                                return eResultError;

                        ret = saHpiFumiBackupStart(session_id, resource_id,
                                fumi_num);

                        thrdinst->header.m_len = HpiMarshalReply0(hm, pReq, &ret);
                }
                break;

                case eFsaHpiFumiBankBootOrderSet: {
                        SaHpiSessionIdT session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiFumiNumT fumi_num;
			SaHpiBankNumT bank_num;
			SaHpiUint32T position;

                        PVERBOSE1("%p Processing saHpiFumiBankBootOrderSet.", thrdid);
                        if (HpiDemarshalRequest5(request_mFlags & dMhEndianBit,
                                        hm, pReq,
                                        &session_id, &resource_id,
                                        &fumi_num, &bank_num, &position) < 0)
                                return eResultError;

                        ret = saHpiFumiBankBootOrderSet(session_id, resource_id,
                                fumi_num, bank_num, position);

                        thrdinst->header.m_len = HpiMarshalReply0(hm, pReq, &ret);
                }
                break;

                case eFsaHpiFumiBankCopyStart: {
                        SaHpiSessionIdT session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiFumiNumT fumi_num;
			SaHpiBankNumT source_bank;
			SaHpiBankNumT target_bank;

                        PVERBOSE1("%p Processing saHpiFumiBankCopyStart.", thrdid);
                        if (HpiDemarshalRequest5(request_mFlags & dMhEndianBit,
                                        hm, pReq,
                                        &session_id, &resource_id,
                                        &fumi_num, &source_bank, &target_bank) < 0)
                                return eResultError;

                        ret = saHpiFumiBankCopyStart(session_id, resource_id,
                                fumi_num, source_bank, target_bank);

                        thrdinst->header.m_len = HpiMarshalReply0(hm, pReq, &ret);
                }
                break;

                case eFsaHpiFumiInstallStart: {
                        SaHpiSessionIdT session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiFumiNumT fumi_num;
			SaHpiBankNumT bank_num;

                        PVERBOSE1("%p Processing saHpiFumiInstallStart.", thrdid);
                        if (HpiDemarshalRequest4(request_mFlags & dMhEndianBit,
                                        hm, pReq,
                                        &session_id, &resource_id,
                                        &fumi_num, &bank_num) < 0)
                                return eResultError;

                        ret = saHpiFumiInstallStart(session_id, resource_id,
                                fumi_num, bank_num);

                        thrdinst->header.m_len = HpiMarshalReply0(hm, pReq, &ret);
                }
                break;

                case eFsaHpiFumiUpgradeStatusGet: {
                        SaHpiSessionIdT session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiFumiNumT fumi_num;
			SaHpiBankNumT bank_num;
			SaHpiFumiUpgradeStatusT status;

                        PVERBOSE1("%p Processing saHpiFumiUpgradeStatusGet.", thrdid);
                        if (HpiDemarshalRequest4(request_mFlags & dMhEndianBit,
                                        hm, pReq,
                                        &session_id, &resource_id,
                                        &fumi_num, &bank_num) < 0)
                                return eResultError;

                        ret = saHpiFumiUpgradeStatusGet(session_id, resource_id,
                                fumi_num, bank_num, &status);

                        thrdinst->header.m_len = HpiMarshalReply1(hm, pReq, &ret,
				&status);
                }
                break;

                case eFsaHpiFumiTargetVerifyStart: {
                        SaHpiSessionIdT session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiFumiNumT fumi_num;
			SaHpiBankNumT bank_num;

                        PVERBOSE1("%p Processing saHpiFumiTargetVerifyStart.", thrdid);
                        if (HpiDemarshalRequest4(request_mFlags & dMhEndianBit,
                                        hm, pReq,
                                        &session_id, &resource_id,
                                        &fumi_num, &bank_num) < 0)
                                return eResultError;

                        ret = saHpiFumiTargetVerifyStart(session_id, resource_id,
                                fumi_num, bank_num);

                        thrdinst->header.m_len = HpiMarshalReply0(hm, pReq, &ret);
                }
                break;

                case eFsaHpiFumiTargetVerifyMainStart: {
                        SaHpiSessionIdT session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiFumiNumT fumi_num;

                        PVERBOSE1("%p Processing saHpiFumiTargetVerifyMainStart.", thrdid);
                        if (HpiDemarshalRequest3(request_mFlags & dMhEndianBit,
                                        hm, pReq,
                                        &session_id, &resource_id,
                                        &fumi_num) < 0)
                                return eResultError;

                        ret = saHpiFumiTargetVerifyMainStart(session_id, resource_id,
                                fumi_num);

                        thrdinst->header.m_len = HpiMarshalReply0(hm, pReq, &ret);
                }
                break;

                case eFsaHpiFumiUpgradeCancel: {
                        SaHpiSessionIdT session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiFumiNumT fumi_num;
			SaHpiBankNumT bank_num;

                        PVERBOSE1("%p Processing saHpiFumiUpgradeCancel.", thrdid);
                        if (HpiDemarshalRequest4(request_mFlags & dMhEndianBit,
                                        hm, pReq,
                                        &session_id, &resource_id,
                                        &fumi_num, &bank_num) < 0)
                                return eResultError;

                        ret = saHpiFumiUpgradeCancel(session_id, resource_id,
                                fumi_num, bank_num);

                        thrdinst->header.m_len = HpiMarshalReply0(hm, pReq, &ret);
                }
                break;

                case eFsaHpiFumiAutoRollbackDisableGet: {
                        SaHpiSessionIdT session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiFumiNumT fumi_num;
                        SaHpiBoolT disable = SAHPI_FALSE;

                        PVERBOSE1("%p Processing saHpiFumiAutoRollbackDisableGet.", thrdid);
                        if (HpiDemarshalRequest3(request_mFlags & dMhEndianBit,
                                        hm, pReq,
                                        &session_id, &resource_id,
                                        &fumi_num) < 0)
                                return eResultError;

                        ret = saHpiFumiAutoRollbackDisableGet(session_id, resource_id,
                                fumi_num, &disable);

                        thrdinst->header.m_len = HpiMarshalReply1(hm, pReq, &ret,
                            &disable);
                }
                break;

                case eFsaHpiFumiAutoRollbackDisableSet: {
                        SaHpiSessionIdT session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiFumiNumT fumi_num;
                        SaHpiBoolT disable;

                        PVERBOSE1("%p Processing saHpiFumiAutoRollbackDisableSet.", thrdid);
                        if (HpiDemarshalRequest4(request_mFlags & dMhEndianBit,
                                        hm, pReq,
                                        &session_id, &resource_id,
                                        &fumi_num, &disable) < 0)
                                return eResultError;

                        ret = saHpiFumiAutoRollbackDisableSet(session_id, resource_id,
                                fumi_num, disable);

                        thrdinst->header.m_len = HpiMarshalReply0(hm, pReq, &ret );
                }
                break;

                case eFsaHpiFumiRollbackStart: {
                        SaHpiSessionIdT session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiFumiNumT fumi_num;

                        PVERBOSE1("%p Processing saHpiFumiRollbackStart.", thrdid);
                        if (HpiDemarshalRequest3(request_mFlags & dMhEndianBit,
                                        hm, pReq,
                                        &session_id, &resource_id,
                                        &fumi_num) < 0)
                                return eResultError;

                        ret = saHpiFumiRollbackStart(session_id, resource_id,
                                fumi_num);

                        thrdinst->header.m_len = HpiMarshalReply0(hm, pReq, &ret);
                }
                break;

                case eFsaHpiFumiActivate: {
                        SaHpiSessionIdT session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiFumiNumT fumi_num;

                        PVERBOSE1("%p Processing saHpiFumiActivate.", thrdid);
                        if (HpiDemarshalRequest3(request_mFlags & dMhEndianBit,
                                        hm, pReq,
                                        &session_id, &resource_id,
                                        &fumi_num) < 0)
                                return eResultError;

                        ret = saHpiFumiActivate(session_id, resource_id,
                                fumi_num);

                        thrdinst->header.m_len = HpiMarshalReply0(hm, pReq, &ret);
                }
                break;

                case eFsaHpiFumiActivateStart: {
                        SaHpiSessionIdT session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiFumiNumT fumi_num;
                        SaHpiBoolT logical;

                        PVERBOSE1("%p Processing saHpiFumiActivateStart.", thrdid);
                        if (HpiDemarshalRequest4(request_mFlags & dMhEndianBit,
                                        hm, pReq,
                                        &session_id, &resource_id,
                                        &fumi_num, &logical) < 0)
                                return eResultError;

                        ret = saHpiFumiActivateStart(session_id, resource_id,
                                fumi_num, logical);

                        thrdinst->header.m_len = HpiMarshalReply0(hm, pReq, &ret);
                }
                break;

                case eFsaHpiFumiCleanup: {
                        SaHpiSessionIdT session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiFumiNumT fumi_num;
                        SaHpiBankNumT bank_num;

                        PVERBOSE1("%p Processing saHpiFumiCleanup.", thrdid);
                        if (HpiDemarshalRequest4(request_mFlags & dMhEndianBit,
                                        hm, pReq,
                                        &session_id, &resource_id,
                                        &fumi_num, &bank_num) < 0)
                                return eResultError;

                        ret = saHpiFumiCleanup(session_id, resource_id,
                                fumi_num, bank_num);

                        thrdinst->header.m_len = HpiMarshalReply0(hm, pReq, &ret);
                }
                break;
                
                case eFsaHpiHotSwapPolicyCancel: {
                        SaHpiSessionIdT  session_id;
                        SaHpiResourceIdT resource_id;
                
                        PVERBOSE1("%p Processing saHpiHotSwapPolicyCancel.", thrdid);
                
                        if ( HpiDemarshalRequest2( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id ) < 0 )
                                return eResultError;
                
                        ret = saHpiHotSwapPolicyCancel( session_id, resource_id );
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                }
                break;
                
                case eFsaHpiResourceActiveSet: {
                        SaHpiSessionIdT  session_id;
                        SaHpiResourceIdT resource_id;
                
                        PVERBOSE1("%p Processing saHpiResourceActiveSet.", thrdid);
                
                        if ( HpiDemarshalRequest2( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id ) < 0 )
                                return eResultError;
                
                        ret = saHpiResourceActiveSet( session_id, resource_id );
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                }
                break;
                
                case eFsaHpiResourceInactiveSet: {
                        SaHpiSessionIdT  session_id;
                        SaHpiResourceIdT resource_id;
                
                        PVERBOSE1("%p Processing saHpiResourceInactiveSet.", thrdid);
                
                        if ( HpiDemarshalRequest2( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id ) < 0 )
                                return eResultError;
                
                        ret = saHpiResourceInactiveSet( session_id, resource_id );
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                }
                break;
                
                case eFsaHpiAutoInsertTimeoutGet: {
                        SaHpiSessionIdT session_id;
                        SaHpiTimeoutT   timeout;
                
                        PVERBOSE1("%p Processing saHpiAutoInsertTimeoutGet.", thrdid);
                
                        if ( HpiDemarshalRequest1( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id ) < 0 )
                                return eResultError;
                
                        ret = saHpiAutoInsertTimeoutGet( session_id, &timeout );
                
                        thrdinst->header.m_len = HpiMarshalReply1( hm, pReq, &ret, &timeout );
                }
                break;
                
                case eFsaHpiAutoInsertTimeoutSet: {
                        SaHpiSessionIdT session_id;
                        SaHpiTimeoutT   timeout;
                
                        PVERBOSE1("%p Processing saHpiAutoInsertTimeoutSet.", thrdid);
                
                        if ( HpiDemarshalRequest2( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &timeout ) < 0 )
                                return eResultError;
                
                        ret = saHpiAutoInsertTimeoutSet( session_id, timeout );
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                }
                break;
                
                case eFsaHpiAutoExtractTimeoutGet: {
                        SaHpiSessionIdT  session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiTimeoutT    timeout;
                
                        PVERBOSE1("%p Processing saHpiAutoExtractTimeoutGet.", thrdid);
                
                        if ( HpiDemarshalRequest2( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id ) < 0 )
                                return eResultError;
                
                        ret = saHpiAutoExtractTimeoutGet( session_id, resource_id, &timeout );
                
                        thrdinst->header.m_len = HpiMarshalReply1( hm, pReq, &ret, &timeout );
                }
                break;
                
                case eFsaHpiAutoExtractTimeoutSet: {
                        SaHpiSessionIdT  session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiTimeoutT    timeout;
                
                        PVERBOSE1("%p Processing saHpiAutoExtractTimeoutSet.", thrdid);
                
                        if ( HpiDemarshalRequest3( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &timeout ) < 0 )
                                return eResultError;
                
                        ret = saHpiAutoExtractTimeoutSet( session_id, resource_id, timeout );
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                }
                break;
                
                case eFsaHpiHotSwapStateGet: {
                        SaHpiSessionIdT  session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiHsStateT    state;
                
                        PVERBOSE1("%p Processing saHpiHotSwapStateGet.", thrdid);
                
                        if ( HpiDemarshalRequest2( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id ) < 0 )
                                return eResultError;
                
                        ret = saHpiHotSwapStateGet( session_id, resource_id, &state );
                
                        thrdinst->header.m_len = HpiMarshalReply1( hm, pReq, &ret, &state );
                }
                break;
                
                case eFsaHpiHotSwapActionRequest: {
                        SaHpiSessionIdT  session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiHsActionT   action;
                
                        PVERBOSE1("%p Processing saHpiHotSwapActionRequest.", thrdid);
                
                        if ( HpiDemarshalRequest3( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &action ) < 0 )
                                return eResultError;
                
                        ret = saHpiHotSwapActionRequest( session_id, resource_id, action );
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                }
                break;
                
                case eFsaHpiHotSwapIndicatorStateGet: {
                        SaHpiSessionIdT        session_id;
                        SaHpiResourceIdT       resource_id;
                        SaHpiHsIndicatorStateT state;
                
                        PVERBOSE1("%p Processing saHpiHotSwapIndicatorStateGet.", thrdid);
                
                        if ( HpiDemarshalRequest2( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id ) < 0 )
                                return eResultError;
                
                        ret = saHpiHotSwapIndicatorStateGet( session_id, resource_id, &state );
                
                        thrdinst->header.m_len = HpiMarshalReply1( hm, pReq, &ret, &state );
                }
                break;
                
                case eFsaHpiHotSwapIndicatorStateSet: {
                        SaHpiSessionIdT        session_id;
                        SaHpiResourceIdT       resource_id;
                        SaHpiHsIndicatorStateT state;
                
                        PVERBOSE1("%p Processing saHpiHotSwapIndicatorStateSet.", thrdid);
                
                        if ( HpiDemarshalRequest3( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &state ) < 0 )
                                return eResultError;
                
                        ret = saHpiHotSwapIndicatorStateSet( session_id, resource_id, state );
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                }
                break;
                
                case eFsaHpiParmControl: {
                        SaHpiSessionIdT  session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiParmActionT action;
                
                        PVERBOSE1("%p Processing saHpiParmControl.", thrdid);
                
                        if ( HpiDemarshalRequest3( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &action ) < 0 )
                                return eResultError;
                
                        ret = saHpiParmControl( session_id, resource_id, action );
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                }
                break;

                case eFsaHpiResourceLoadIdGet: {
                        SaHpiSessionIdT  session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiLoadIdT load_id;
                
                        PVERBOSE1("%p Processing saHpiResourceLoadIdGet.", thrdid);
                
                        if ( HpiDemarshalRequest2( request_mFlags & dMhEndianBit,
                                                   hm, pReq, &session_id,
                                                   &resource_id ) < 0 )
                                return eResultError;
                
                        ret = saHpiResourceLoadIdGet( session_id, resource_id, &load_id );
                
                        thrdinst->header.m_len = HpiMarshalReply1( hm, pReq, &ret, &load_id );
                }
                break;

                case eFsaHpiResourceLoadIdSet: {
                        SaHpiSessionIdT  session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiLoadIdT load_id;
                
                        PVERBOSE1("%p Processing saHpiResourceLoadIdSet.", thrdid);
                
                        if ( HpiDemarshalRequest3( request_mFlags & dMhEndianBit,
                                                   hm, pReq, &session_id,
                                                   &resource_id, &load_id ) < 0 )
                                return eResultError;
                
                        ret = saHpiResourceLoadIdSet( session_id, resource_id, &load_id );
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                }
                break;
                
                case eFsaHpiResourceResetStateGet: {
                        SaHpiSessionIdT   session_id;
                        SaHpiResourceIdT  resource_id;
                        SaHpiResetActionT action;
                
                        PVERBOSE1("%p Processing saHpiResourceResetStateGet.", thrdid);
                
                        if ( HpiDemarshalRequest2( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id ) < 0 )
                                return eResultError;
                
                        ret = saHpiResourceResetStateGet( session_id, resource_id, &action );
                
                        thrdinst->header.m_len = HpiMarshalReply1( hm, pReq, &ret, &action );
                }
                break;
                
                case eFsaHpiResourceResetStateSet: {
                        SaHpiSessionIdT   session_id;
                        SaHpiResourceIdT  resource_id;
                        SaHpiResetActionT action;
                
                        PVERBOSE1("%p Processing saHpiResourceResetStateSet.", thrdid);
                
                        if ( HpiDemarshalRequest3( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &action ) < 0 )
                                return eResultError;
                
                        ret = saHpiResourceResetStateSet( session_id, resource_id, action );
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                }
                break;
                
                case eFsaHpiResourcePowerStateGet:
                        {
                        SaHpiSessionIdT  session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiPowerStateT state;
                
                        PVERBOSE1("%p Processing saHpiResourcePowerStateGet.", thrdid);
                
                        if ( HpiDemarshalRequest2( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id ) < 0 )
                                return eResultError;
                
                        ret = saHpiResourcePowerStateGet( session_id, resource_id, &state );
                
                        thrdinst->header.m_len = HpiMarshalReply1( hm, pReq, &ret, &state );
                }
                break;
                
                case eFsaHpiResourcePowerStateSet: {
                        SaHpiSessionIdT  session_id;
                        SaHpiResourceIdT resource_id;
                        SaHpiPowerStateT state;
                
                        PVERBOSE1("%p Processing saHpiResourcePowerStateGet.", thrdid);
                
                        if ( HpiDemarshalRequest3( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &session_id, &resource_id,
                                                        &state  ) < 0 )
                                return eResultError;
                
                        ret = saHpiResourcePowerStateSet( session_id, resource_id, state );
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                }
                break;

                case eFoHpiHandlerCreate: {
                        oHpiHandlerIdT id;
                        oHpiHandlerConfigT config;
                        GHashTable *config_table = g_hash_table_new_full(
                        	g_str_hash, g_str_equal,
                        	g_free, g_free
                        );

                        PVERBOSE1("%p Processing oHpiHandlerCreate.", thrdid);
                
                        if ( HpiDemarshalRequest1( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &config ) < 0 )
                                return eResultError;
                
                        for (int n = 0; n < config.NumberOfParams; n++) {
                        	g_hash_table_insert(config_table,
                        			    g_strdup((const gchar *)config.Params[n].Name),
                        			    g_strdup((const gchar *)config.Params[n].Value));
                        }
                        free(config.Params);
                        ret = oHpiHandlerCreate(config_table, &id);
                        g_hash_table_destroy(config_table);
                
                        thrdinst->header.m_len = HpiMarshalReply1( hm, pReq, &ret, &id );
                        result = eResultClose;
                }
                break;
                
                case eFoHpiHandlerDestroy: {
                        oHpiHandlerIdT id;
                
                        PVERBOSE1("%p Processing oHpiHandlerDestroy.", thrdid);
                
                        if ( HpiDemarshalRequest1( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &id ) < 0 )
                                return eResultError;
                
                        ret = oHpiHandlerDestroy(id);
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                        result = eResultClose;
                }
                break;
                
                case eFoHpiHandlerInfo: {
                        oHpiHandlerIdT id;
                        oHpiHandlerInfoT info;
                
                        PVERBOSE1("%p Processing oHpiHandlerInfo.", thrdid);
                
                        if ( HpiDemarshalRequest1( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &id ) < 0 )
                                return eResultError;
                
                        ret = oHpiHandlerInfo(id, &info);
                
                        thrdinst->header.m_len = HpiMarshalReply1( hm, pReq, &ret, &info );
                        result = eResultClose;
                }
                break;
                
                case eFoHpiHandlerGetNext: {
                        oHpiHandlerIdT id, next_id;
                
                        PVERBOSE1("%p Processing oHpiHandlerGetNext.", thrdid);
                
                        if ( HpiDemarshalRequest1( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &id ) < 0 )
                                return eResultError;
                
                        ret = oHpiHandlerGetNext(id, &next_id);
                
                        thrdinst->header.m_len = HpiMarshalReply1( hm, pReq, &ret, &next_id );
                        result = eResultClose;
                }
                break;

		case eFoHpiHandlerFind: {
                        SaHpiSessionIdT session_id;
			SaHpiResourceIdT rid;
			oHpiHandlerIdT hid;
                
                        PVERBOSE1("%p Processing oHpiHandlerFind.", thrdid);
                
                        if ( HpiDemarshalRequest2( request_mFlags & dMhEndianBit,
						   hm, pReq, &session_id, &rid ) < 0 )
                                return eResultError;
                
                        ret = oHpiHandlerFind(session_id, rid, &hid);
                
                        thrdinst->header.m_len = HpiMarshalReply1( hm, pReq, &ret, &hid );
                        result = eResultClose;
                }
                break;

		case eFoHpiHandlerRetry: {
			oHpiHandlerIdT hid;
                
                        PVERBOSE1("%p Processing oHpiHandlerRetry.", thrdid);
                
                        if ( HpiDemarshalRequest1( request_mFlags & dMhEndianBit,
						   hm, pReq, &hid ) < 0 )
                                return eResultError;
                
                        ret = oHpiHandlerRetry(hid);
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                        result = eResultClose;
                }
                break;
                
                case eFoHpiGlobalParamGet: {
                        oHpiGlobalParamT param;
                
                        PVERBOSE1("%p Processing oHpiGlobalParamGet.", thrdid);
                
                        if ( HpiDemarshalRequest1( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &param ) < 0 )
                                return eResultError;
                
                        ret = oHpiGlobalParamGet(&param);
                
                        thrdinst->header.m_len = HpiMarshalReply1( hm, pReq, &ret, &param );
                        result = eResultClose;
                }
                break;
                
                case eFoHpiGlobalParamSet: {
                        oHpiGlobalParamT param;
                
                        PVERBOSE1("%p Processing oHpiGlobalParamSet.", thrdid);
                
                        if ( HpiDemarshalRequest1( request_mFlags & dMhEndianBit,
                                                        hm, pReq, &param ) < 0 )
                                return eResultError;
                
                        ret = oHpiGlobalParamSet(&param);
                
                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );
                        result = eResultClose;
                }
                break;

                case eFoHpiInjectEvent: {
                        oHpiHandlerIdT  id = 0;
                        SaHpiEventT     event;
                        SaHpiRptEntryT  rpte;
                        SaHpiRdrT       rdr;

                        memset(&event, 0, sizeof(SaHpiEventT));
                        memset(&rpte,  0, sizeof(SaHpiRptEntryT));
                        memset(&rdr,   0, sizeof(SaHpiRdrT));

                
                        PVERBOSE1("%p Processing oHpiInjectEvent.\n", thrdid);
                
                        if ( HpiDemarshalRequest4( request_mFlags & dMhEndianBit,
                                                   hm, 
                                                   pReq, 
                                                   &id, 
                                                   &event,
                                                   &rpte, 
                                                   &rdr ) < 0 )
                                return eResultError;


                        ret = oHpiInjectEvent(id, &event, &rpte, &rdr);

                        thrdinst->header.m_len = HpiMarshalReply0( hm, pReq, &ret );

                        result = eResultClose;
                }
                break;
                
                default:
                        PVERBOSE2("%p Function not found", thrdid);
                        return eResultError;
       }

       // send the reply
       bool wrt_result = thrdinst->WriteMsg(pReq);
       if (wrt_result) {
               PVERBOSE2("%p Socket write failed.", thrdid);
               return eResultError;
       }

       PVERBOSE1("%p Return code = %d", thrdid, ret);

       return result;
}
