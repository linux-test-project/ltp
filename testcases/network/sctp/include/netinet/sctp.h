/* SCTP kernel reference Implementation: User API extensions.
 *
 * sctp.h
 *
 * Distributed under the terms of the LGPL v2.1 as described in
 *    http://www.gnu.org/copyleft/lesser.txt
 *
 * This file is part of the user library that offers support for the
 * SCTP kernel reference Implementation. The main purpose of this
 * code is to provide the SCTP Socket API mappings for user
 * application to interface with SCTP in kernel.
 *
 * This header represents the structures and constants needed to support
 * the SCTP Extension to the Sockets API.
 *
 * (C) Copyright IBM Corp. 2001, 2004
 * Copyright (c) 1999-2000 Cisco, Inc.
 * Copyright (c) 1999-2001 Motorola, Inc.
 *
 * Written or modified by:
 *    La Monte H.P. Yarroll    <piggy@acm.org>
 *    R. Stewart               <randall@sctp.chicago.il.us>
 *    K. Morneau               <kmorneau@cisco.com>
 *    Q. Xie                   <qxie1@email.mot.com>
 *    Karl Knutson             <karl@athena.chicago.il.us>
 *    Jon Grimm                <jgrimm@austin.ibm.com>
 *    Daisy Chang              <daisyc@us.ibm.com>
 *    Inaky Perez-Gonzalez     <inaky.gonzalez@intel.com>
 *    Sridhar Samudrala        <sri@us.ibm.com>
 */

#ifndef __linux_sctp_h__
#define __linux_sctp_h__

#include <stdint.h>
#include <linux/types.h>
#include <sys/socket.h>

__BEGIN_DECLS

typedef __s32 sctp_assoc_t;

/* Socket option layer for SCTP */
#ifndef SOL_SCTP
#define SOL_SCTP	132
#endif

#ifndef IPPROTO_SCTP
#define IPPROTO_SCTP    132
#endif

/* 9. Preprocessor constants */
#define HAVE_SCTP
#define HAVE_KERNEL_SCTP
#define HAVE_SCTP_MULTIBUF
#define HAVE_SCTP_NOCONNECT
#define HAVE_SCTP_PRSCTP
#define HAVE_SCTP_ADDIP
#define HAVE_SCTP_CANSET_PRIMARY

/* The following symbols come from the Sockets API Extensions for
 * SCTP <draft-ietf-tsvwg-sctpsocket-07.txt>.
 */
enum sctp_optname {
	SCTP_RTOINFO,
#define SCTP_RTOINFO SCTP_RTOINFO
	SCTP_ASSOCINFO,
#define SCTP_ASSOCINFO SCTP_ASSOCINFO
	SCTP_INITMSG,
#define SCTP_INITMSG SCTP_INITMSG
	SCTP_NODELAY, 	/* Get/set nodelay option. */
#define SCTP_NODELAY	SCTP_NODELAY
	SCTP_AUTOCLOSE,
#define SCTP_AUTOCLOSE SCTP_AUTOCLOSE
	SCTP_SET_PEER_PRIMARY_ADDR,
#define SCTP_SET_PEER_PRIMARY_ADDR SCTP_SET_PEER_PRIMARY_ADDR
	SCTP_PRIMARY_ADDR,
#define SCTP_PRIMARY_ADDR SCTP_PRIMARY_ADDR
	SCTP_ADAPTION_LAYER,
#define SCTP_ADAPTION_LAYER SCTP_ADAPTION_LAYER
	SCTP_DISABLE_FRAGMENTS,
#define SCTP_DISABLE_FRAGMENTS SCTP_DISABLE_FRAGMENTS
	SCTP_PEER_ADDR_PARAMS,
#define SCTP_PEER_ADDR_PARAMS SCTP_PEER_ADDR_PARAMS
	SCTP_DEFAULT_SEND_PARAM,
#define SCTP_DEFAULT_SEND_PARAM SCTP_DEFAULT_SEND_PARAM
	SCTP_EVENTS,
#define SCTP_EVENTS SCTP_EVENTS
	SCTP_I_WANT_MAPPED_V4_ADDR,  /* Turn on/off mapped v4 addresses  */
#define SCTP_I_WANT_MAPPED_V4_ADDR SCTP_I_WANT_MAPPED_V4_ADDR
	SCTP_MAXSEG, 	/* Get/set maximum fragment. */
#define SCTP_MAXSEG 	SCTP_MAXSEG
	SCTP_STATUS,
#define SCTP_STATUS SCTP_STATUS
	SCTP_GET_PEER_ADDR_INFO,
#define SCTP_GET_PEER_ADDR_INFO SCTP_GET_PEER_ADDR_INFO

	/* Internal Socket Options. Some of the sctp library functions are
	 * implemented using these socket options.
	 */
	SCTP_SOCKOPT_BINDX_ADD = 100,/* BINDX requests for adding addresses. */
#define SCTP_SOCKOPT_BINDX_ADD	SCTP_SOCKOPT_BINDX_ADD
	SCTP_SOCKOPT_BINDX_REM, /* BINDX requests for removing addresses. */
#define SCTP_SOCKOPT_BINDX_REM	SCTP_SOCKOPT_BINDX_REM
	SCTP_SOCKOPT_PEELOFF, 	/* peel off association. */
#define SCTP_SOCKOPT_PEELOFF	SCTP_SOCKOPT_PEELOFF
	SCTP_GET_PEER_ADDRS_NUM_OLD, 	/* Get number of peer addresss. */
#define SCTP_GET_PEER_ADDRS_NUM_OLD	SCTP_GET_PEER_ADDRS_NUM_OLD
	SCTP_GET_PEER_ADDRS_OLD, 	/* Get all peer addresss. */
#define SCTP_GET_PEER_ADDRS_OLD	SCTP_GET_PEER_ADDRS_OLD
	SCTP_GET_LOCAL_ADDRS_NUM_OLD, 	/* Get number of local addresss. */
#define SCTP_GET_LOCAL_ADDRS_NUM_OLD	SCTP_GET_LOCAL_ADDRS_NUM_OLD
	SCTP_GET_LOCAL_ADDRS_OLD, 	/* Get all local addresss. */
#define SCTP_GET_LOCAL_ADDRS_OLD	SCTP_GET_LOCAL_ADDRS_OLD
	SCTP_SOCKOPT_CONNECTX, /* CONNECTX requests. */
#define SCTP_SOCKOPT_CONNECTX	SCTP_SOCKOPT_CONNECTX
	SCTP_GET_PEER_ADDRS,    /* Get all peer addresss. */
#define SCTP_GET_PEER_ADDRS     SCTP_GET_PEER_ADDRS
	SCTP_GET_LOCAL_ADDRS,   /* Get all local addresss. */
#define SCTP_GET_LOCAL_ADDRS    SCTP_GET_LOCAL_ADDRS
};

/*
 * 5.2.1 SCTP Initiation Structure (SCTP_INIT)
 *
 *   This cmsghdr structure provides information for initializing new
 *   SCTP associations with sendmsg().  The SCTP_INITMSG socket option
 *   uses this same data structure.  This structure is not used for
 *   recvmsg().
 *
 *   cmsg_level    cmsg_type      cmsg_data[]
 *   ------------  ------------   ----------------------
 *   IPPROTO_SCTP  SCTP_INIT      struct sctp_initmsg
 *
 */
struct sctp_initmsg {
	__u16 sinit_num_ostreams;
	__u16 sinit_max_instreams;
	__u16 sinit_max_attempts;
	__u16 sinit_max_init_timeo;
};

/*
 * 5.2.2 SCTP Header Information Structure (SCTP_SNDRCV)
 *
 *   This cmsghdr structure specifies SCTP options for sendmsg() and
 *   describes SCTP header information about a received message through
 *   recvmsg().
 *
 *   cmsg_level    cmsg_type      cmsg_data[]
 *   ------------  ------------   ----------------------
 *   IPPROTO_SCTP  SCTP_SNDRCV    struct sctp_sndrcvinfo
 *
 */
struct sctp_sndrcvinfo {
	__u16 sinfo_stream;
	__u16 sinfo_ssn;
	__u16 sinfo_flags;
	__u32 sinfo_ppid;
	__u32 sinfo_context;
	__u32 sinfo_timetolive;
	__u32 sinfo_tsn;
	__u32 sinfo_cumtsn;
	sctp_assoc_t sinfo_assoc_id;
};

/*
 *  sinfo_flags: 16 bits (unsigned integer)
 *
 *   This field may contain any of the following flags and is composed of
 *   a bitwise OR of these values.
 */

enum sctp_sinfo_flags {
	SCTP_UNORDERED = 1,  /* Send/receive message unordered. */
	SCTP_ADDR_OVER = 2,  /* Override the primary destination. */
	SCTP_ABORT=4,        /* Send an ABORT message to the peer. */
	SCTP_EOF=MSG_FIN,    /* Initiate graceful shutdown process. */
};


typedef union {
	__u8   			raw;
	struct sctp_initmsg	init;
	struct sctp_sndrcvinfo	sndrcv;
} sctp_cmsg_data_t;

/* These are cmsg_types.  */
typedef enum sctp_cmsg_type {
	SCTP_INIT,              /* 5.2.1 SCTP Initiation Structure */
	SCTP_SNDRCV,            /* 5.2.2 SCTP Header Information Structure */
} sctp_cmsg_t;


/*
 * 5.3.1.1 SCTP_ASSOC_CHANGE
 *
 *   Communication notifications inform the ULP that an SCTP association
 *   has either begun or ended. The identifier for a new association is
 *   provided by this notificaion. The notification information has the
 *   following format:
 *
 */
struct sctp_assoc_change {
	__u16 sac_type;
	__u16 sac_flags;
	__u32 sac_length;
	__u16 sac_state;
	__u16 sac_error;
	__u16 sac_outbound_streams;
	__u16 sac_inbound_streams;
	sctp_assoc_t sac_assoc_id;
};

/*
 *   sac_state: 32 bits (signed integer)
 *
 *   This field holds one of a number of values that communicate the
 *   event that happened to the association.  They include:
 *
 *   Note:  The following state names deviate from the API draft as
 *   the names clash too easily with other kernel symbols.
 */
enum sctp_sac_state {
	SCTP_COMM_UP,
	SCTP_COMM_LOST,
	SCTP_RESTART,
	SCTP_SHUTDOWN_COMP,
	SCTP_CANT_STR_ASSOC,
};

/*
 * 5.3.1.2 SCTP_PEER_ADDR_CHANGE
 *
 *   When a destination address on a multi-homed peer encounters a change
 *   an interface details event is sent.  The information has the
 *   following structure:
 */
struct sctp_paddr_change {
	__u16 spc_type;
	__u16 spc_flags;
	__u32 spc_length;
	struct sockaddr_storage spc_aaddr;
	int spc_state;
	int spc_error;
	sctp_assoc_t spc_assoc_id;
} __attribute__((packed, aligned(4)));

/*
 *    spc_state:  32 bits (signed integer)
 *
 *   This field holds one of a number of values that communicate the
 *   event that happened to the address.  They include:
 */
enum sctp_spc_state {
	SCTP_ADDR_AVAILABLE,
	SCTP_ADDR_UNREACHABLE,
	SCTP_ADDR_REMOVED,
	SCTP_ADDR_ADDED,
	SCTP_ADDR_MADE_PRIM,
};


/*
 * 5.3.1.3 SCTP_REMOTE_ERROR
 *
 *   A remote peer may send an Operational Error message to its peer.
 *   This message indicates a variety of error conditions on an
 *   association. The entire error TLV as it appears on the wire is
 *   included in a SCTP_REMOTE_ERROR event.  Please refer to the SCTP
 *   specification [SCTP] and any extensions for a list of possible
 *   error formats. SCTP error TLVs have the format:
 */
struct sctp_remote_error {
	__u16 sre_type;
	__u16 sre_flags;
	__u32 sre_length;
	__u16 sre_error;
	sctp_assoc_t sre_assoc_id;
	__u8 sre_data[0];
};


/*
 * 5.3.1.4 SCTP_SEND_FAILED
 *
 *   If SCTP cannot deliver a message it may return the message as a
 *   notification.
 */
struct sctp_send_failed {
	__u16 ssf_type;
	__u16 ssf_flags;
	__u32 ssf_length;
	__u32 ssf_error;
	struct sctp_sndrcvinfo ssf_info;
	sctp_assoc_t ssf_assoc_id;
	__u8 ssf_data[0];
};

/*
 *   ssf_flags: 16 bits (unsigned integer)
 *
 *   The flag value will take one of the following values
 *
 *   SCTP_DATA_UNSENT  - Indicates that the data was never put on
 *                       the wire.
 *
 *   SCTP_DATA_SENT    - Indicates that the data was put on the wire.
 *                       Note that this does not necessarily mean that the
 *                       data was (or was not) successfully delivered.
 */
enum sctp_ssf_flags {
	SCTP_DATA_UNSENT,
	SCTP_DATA_SENT,
};

/*
 * 5.3.1.5 SCTP_SHUTDOWN_EVENT
 *
 *   When a peer sends a SHUTDOWN, SCTP delivers this notification to
 *   inform the application that it should cease sending data.
 */
struct sctp_shutdown_event {
	__u16 sse_type;
	__u16 sse_flags;
	__u32 sse_length;
	sctp_assoc_t sse_assoc_id;
};

/*
 * 5.3.1.6 SCTP_ADAPTION_INDICATION
 *
 *   When a peer sends a Adaption Layer Indication parameter , SCTP
 *   delivers this notification to inform the application
 *   that of the peers requested adaption layer.
 */
struct sctp_adaption_event {
	__u16 sai_type;
	__u16 sai_flags;
	__u32 sai_length;
	__u32 sai_adaption_ind;
	sctp_assoc_t sai_assoc_id;
};

/*
 * 5.3.1.7 SCTP_PARTIAL_DELIVERY_EVENT
 *
 *   When a reciever is engaged in a partial delivery of a
 *   message this notification will be used to inidicate
 *   various events.
 */
struct sctp_pdapi_event {
	__u16 pdapi_type;
	__u16 pdapi_flags;
	__u32 pdapi_length;
	__u32 pdapi_indication;
	sctp_assoc_t pdapi_assoc_id;
};

enum { SCTP_PARTIAL_DELIVERY_ABORTED=0, };

/*
 * Described in Section 7.3
 *   Ancillary Data and Notification Interest Options
 */
struct sctp_event_subscribe {
	__u8 sctp_data_io_event;
	__u8 sctp_association_event;
	__u8 sctp_address_event;
	__u8 sctp_send_failure_event;
	__u8 sctp_peer_error_event;
	__u8 sctp_shutdown_event;
	__u8 sctp_partial_delivery_event;
	__u8 sctp_adaption_layer_event;
};

/*
 * 5.3.1 SCTP Notification Structure
 *
 *   The notification structure is defined as the union of all
 *   notification types.
 *
 */
union sctp_notification {
	struct {
		__u16 sn_type;             /* Notification type. */
		__u16 sn_flags;
		__u32 sn_length;
	} sn_header;
	struct sctp_assoc_change sn_assoc_change;
	struct sctp_paddr_change sn_paddr_change;
	struct sctp_remote_error sn_remote_error;
	struct sctp_send_failed sn_send_failed;
	struct sctp_shutdown_event sn_shutdown_event;
	struct sctp_adaption_event sn_adaption_event;
	struct sctp_pdapi_event sn_pdapi_event;
};

/* Section 5.3.1
 * All standard values for sn_type flags are greater than 2^15.
 * Values from 2^15 and down are reserved.
 */

enum sctp_sn_type {
	SCTP_SN_TYPE_BASE     = (1<<15),
	SCTP_ASSOC_CHANGE,
	SCTP_PEER_ADDR_CHANGE,
	SCTP_SEND_FAILED,
	SCTP_REMOTE_ERROR,
	SCTP_SHUTDOWN_EVENT,
	SCTP_PARTIAL_DELIVERY_EVENT,
	SCTP_ADAPTION_INDICATION,
};

/* Notification error codes used to fill up the error fields in some
 * notifications.
 * SCTP_PEER_ADDRESS_CHAGE 	: spc_error
 * SCTP_ASSOC_CHANGE		: sac_error
 * These names should be potentially included in the draft 04 of the SCTP
 * sockets API specification.
 */
typedef enum sctp_sn_error {
	SCTP_FAILED_THRESHOLD,
	SCTP_RECEIVED_SACK,
	SCTP_HEARTBEAT_SUCCESS,
	SCTP_RESPONSE_TO_USER_REQ,
	SCTP_INTERNAL_ERROR,
	SCTP_SHUTDOWN_GUARD_EXPIRES,
	SCTP_PEER_FAULTY,
} sctp_sn_error_t;

/*
 * 7.1.1 Retransmission Timeout Parameters (SCTP_RTOINFO)
 *
 *   The protocol parameters used to initialize and bound retransmission
 *   timeout (RTO) are tunable.  See [SCTP] for more information on how
 *   these parameters are used in RTO calculation.
 */
struct sctp_rtoinfo {
	sctp_assoc_t	srto_assoc_id;
	__u32		srto_initial;
	__u32		srto_max;
	__u32		srto_min;
};

/*
 * 7.1.2 Association Parameters (SCTP_ASSOCINFO)
 *
 *   This option is used to both examine and set various association and
 *   endpoint parameters.
 */
struct sctp_assocparams {
	sctp_assoc_t	sasoc_assoc_id;
	__u16		sasoc_asocmaxrxt;
	__u16		sasoc_number_peer_destinations;
	__u32		sasoc_peer_rwnd;
	__u32		sasoc_local_rwnd;
	__u32		sasoc_cookie_life;
};

/*
 * 7.1.9 Set Peer Primary Address (SCTP_SET_PEER_PRIMARY_ADDR)
 *
 *  Requests that the peer mark the enclosed address as the association
 *  primary. The enclosed address must be one of the association's
 *  locally bound addresses. The following structure is used to make a
 *   set primary request:
 */
struct sctp_setpeerprim {
	sctp_assoc_t            sspp_assoc_id;
	struct sockaddr_storage sspp_addr;
} __attribute__((packed, aligned(4)));

/*
 * 7.1.10 Set Primary Address (SCTP_PRIMARY_ADDR)
 *
 *  Requests that the local SCTP stack use the enclosed peer address as
 *  the association primary. The enclosed address must be one of the
 *  association peer's addresses. The following structure is used to
 *  make a set peer primary request:
 */
struct sctp_prim {
	sctp_assoc_t            ssp_assoc_id;
	struct sockaddr_storage ssp_addr;
} __attribute__((packed, aligned(4)));

/*
 * 7.1.11 Set Adaption Layer Indicator (SCTP_ADAPTION_LAYER)
 *
 * Requests that the local endpoint set the specified Adaption Layer
 * Indication parameter for all future INIT and INIT-ACK exchanges.
 */
struct sctp_setadaption {
	__u32	ssb_adaption_ind;
};

/*
 * 7.1.13 Peer Address Parameters  (SCTP_PEER_ADDR_PARAMS)
 *
 *   Applications can enable or disable heartbeats for any peer address
 *   of an association, modify an address's heartbeat interval, force a
 *   heartbeat to be sent immediately, and adjust the address's maximum
 *   number of retransmissions sent before an address is considered
 *   unreachable. The following structure is used to access and modify an
 *   address's parameters:
 */
struct sctp_paddrparams {
	sctp_assoc_t		spp_assoc_id;
	struct sockaddr_storage	spp_address;
	__u32			spp_hbinterval;
	__u16			spp_pathmaxrxt;
} __attribute__((packed, aligned(4)));

/*
 * 7.2.2 Peer Address Information
 *
 *   Applications can retrieve information about a specific peer address
 *   of an association, including its reachability state, congestion
 *   window, and retransmission timer values.  This information is
 *   read-only. The following structure is used to access this
 *   information:
 */
struct sctp_paddrinfo {
	sctp_assoc_t		spinfo_assoc_id;
	struct sockaddr_storage	spinfo_address;
	__s32			spinfo_state;
	__u32			spinfo_cwnd;
	__u32			spinfo_srtt;
	__u32			spinfo_rto;
	__u32			spinfo_mtu;
} __attribute__((packed, aligned(4)));

/* Peer addresses's state. */
enum sctp_spinfo_state {
	SCTP_INACTIVE,
	SCTP_ACTIVE,
};

/*
 * 7.2.1 Association Status (SCTP_STATUS)
 *
 *   Applications can retrieve current status information about an
 *   association, including association state, peer receiver window size,
 *   number of unacked data chunks, and number of data chunks pending
 *   receipt.  This information is read-only.  The following structure is
 *   used to access this information:
 */
struct sctp_status {
	sctp_assoc_t		sstat_assoc_id;
	__s32			sstat_state;
	__u32			sstat_rwnd;
	__u16			sstat_unackdata;
	__u16			sstat_penddata;
	__u16			sstat_instrms;
	__u16			sstat_outstrms;
	__u32			sstat_fragmentation_point;
	struct sctp_paddrinfo	sstat_primary;
};

/* Association states.  */
enum sctp_sstat_state {
	SCTP_EMPTY                = 0,
	SCTP_CLOSED               = 1,
	SCTP_COOKIE_WAIT          = 2,
	SCTP_COOKIE_ECHOED        = 3,
	SCTP_ESTABLISHED          = 4,
	SCTP_SHUTDOWN_PENDING     = 5,
	SCTP_SHUTDOWN_SENT        = 6,
	SCTP_SHUTDOWN_RECEIVED    = 7,
	SCTP_SHUTDOWN_ACK_SENT    = 8,
};

/*
 * 8.3, 8.5 get all peer/local addresses in an association.
 * This parameter struct is used by SCTP_GET_PEER_ADDRS and
 * SCTP_GET_LOCAL_ADDRS socket options used internally to implement
 * sctp_getpaddrs() and sctp_getladdrs() API.
 */
struct sctp_getaddrs_old {
	sctp_assoc_t            assoc_id;
	int			addr_num;
	struct sockaddr		*addrs;
};
struct sctp_getaddrs {
	sctp_assoc_t            assoc_id; /*input*/
	__u32                   addr_num; /*output*/
	__u8                    addrs[0]; /*output, variable size*/
};

/* These are bit fields for msghdr->msg_flags.  See section 5.1.  */
/* On user space Linux, these live in <bits/socket.h> as an enum.  */
enum sctp_msg_flags {
	MSG_NOTIFICATION = 0x8000,
#define MSG_NOTIFICATION MSG_NOTIFICATION
};

/*
 * 8.1 sctp_bindx()
 *
 * The flags parameter is formed from the bitwise OR of zero or more of the
 * following currently defined flags:
 */
#define SCTP_BINDX_ADD_ADDR 0x01
#define SCTP_BINDX_REM_ADDR 0x02

/* This is the structure that is passed as an argument(optval) to
 * getsockopt(SCTP_SOCKOPT_PEELOFF).
 */
typedef struct {
	sctp_assoc_t associd;
	int sd;
} sctp_peeloff_arg_t;


int sctp_bindx(int sd, struct sockaddr *addrs, int addrcnt, int flags);

int sctp_connectx(int sd, struct sockaddr *addrs, int addrcnt);

int sctp_peeloff(int sd, sctp_assoc_t assoc_id);

/* Prototype for the library function sctp_opt_info defined in
 * API 7. Socket Options.
 */
int sctp_opt_info(int sd, sctp_assoc_t id, int opt, void *arg, socklen_t *size);

/* Get all peer address on a socket.  This is a new SCTP API
 * described in the section 8.3 of the Sockets API Extensions for SCTP.
 * This is implemented using the getsockopt() interface.
 */
int sctp_getpaddrs(int sd, sctp_assoc_t id, struct sockaddr **addrs);

/* Frees all resources allocated by sctp_getpaddrs().  This is a new SCTP API
 * described in the section 8.4 of the Sockets API Extensions for SCTP.
 */
int sctp_freepaddrs(struct sockaddr *addrs);

/* Get all locally bound address on a socket.  This is a new SCTP API
 * described in the section 8.5 of the Sockets API Extensions for SCTP.
 * This is implemented using the getsockopt() interface.
 */
int sctp_getladdrs(int sd, sctp_assoc_t id, struct sockaddr **addrs);

/* Frees all resources allocated by sctp_getladdrs().  This is a new SCTP API
 * described in the section 8.6 of the Sockets API Extensions for SCTP.
 */
int sctp_freeladdrs(struct sockaddr *addrs);

/* This library function assists the user with the advanced features
 * of SCTP.  This is a new SCTP API described in the section 8.7 of the
 * Sockets API Extensions for SCTP. This is implemented using the
 * sendmsg() interface.
 */
int sctp_sendmsg(int s, const void *msg, size_t len, struct sockaddr *to,
		 socklen_t tolen, uint32_t ppid, uint32_t flags,
		 uint16_t stream_no, uint32_t timetolive, uint32_t context);

/* This library function assist the user with sending a message without
 * dealing directly with the CMSG header.
 */
int sctp_send(int s, const void *msg, size_t len,
              const struct sctp_sndrcvinfo *sinfo, int flags);

/* This library function assists the user with the advanced features
 * of SCTP.  This is a new SCTP API described in the section 8.8 of the
 * Sockets API Extensions for SCTP. This is implemented using the
 * recvmsg() interface.
 */
int sctp_recvmsg(int s, void *msg, size_t len, struct sockaddr *from,
		 socklen_t *fromlen, struct sctp_sndrcvinfo *sinfo,
		 int *msg_flags);

__END_DECLS

#endif /* __linux_sctp_h__ */
