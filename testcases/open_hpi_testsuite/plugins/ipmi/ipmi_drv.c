/*      -*- linux-c -*-
 *
 * Copyright (c) 2004 by Intel Corp.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
 * file and program are licensed under a BSD style license.  See
 * the Copying file included with the OpenHPI distribution for
 * full licensing terms.
 *
 * These routines access the OpenIPMI driver directly, rather
 * than using OpenIPMI library calls. 
 *
 * Authors:
 *     Andy Cress <andrew.r.cress@intel.com>
 * Changes:
 *     12/01/04 ARCress - created from ipmiutil/ipmimv.c
 */
#include "ipmi.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#ifdef SCO_UW
#include <sys/ioccom.h>
#endif
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define uchar    unsigned char
#define IPMI_MAX_ADDR_SIZE 32
#define IPMI_BMC_CHANNEL  0xf
#define IPMI_IOC_MAGIC 'i'

#ifdef TV_PORT
/* use this to define timeval if it is a portability issue */
struct timeval {
        long int     tv_sec;         /* (time_t) seconds */
        long int     tv_usec;        /* (suseconds_t) microseconds */
};
#endif


struct ipmi_req
{
        unsigned char *addr; /* Address to send the message to. */
        unsigned int  addr_len;
        long    msgid; /* The sequence number for the message.  */
        struct ipmi_msg msg;
};
 
struct ipmi_recv
{
        int     recv_type;  	/* Is this a command, response, etc. */
        unsigned char *addr;    /* Address the message was from */
	int  addr_len;  	/* The size of the address buffer. */
        long    msgid;  	/* The sequence number from the request */
        struct ipmi_msg msg; 	/* The data field must point to a buffer. */
};


#define IPMICTL_RECEIVE_MSG         _IOWR(IPMI_IOC_MAGIC, 12, struct ipmi_recv)
#define IPMICTL_RECEIVE_MSG_TRUNC   _IOWR(IPMI_IOC_MAGIC, 11, struct ipmi_recv)
#define IPMICTL_SEND_COMMAND        _IOR(IPMI_IOC_MAGIC,  13, struct ipmi_req)
#define IPMICTL_SET_GETS_EVENTS_CMD _IOR(IPMI_IOC_MAGIC,  16, int)


#if 0
FILE *fperr = NULL;    /* if NULL, no messages */
static int ipmi_timeout_mv = 10;   /* 10 seconds, was 5 sec */
static int ipmi_fd = -1;
static int curr_seq = 0;
#endif

//int ipmi_open_mv(void);
//int ipmi_close_mv(void);
int ipmicmd_mv(struct ohoi_handler *ipmi_handler,
           uchar cmd, uchar netfn, uchar lun, uchar *pdata, uchar sdata, 
	   uchar *presp, int sresp, int *rlen);
int ipmicmd_send(ipmi_domain_t *domain,
                 uchar netfn, uchar cmd, uchar lun, uchar chan,
		 uchar *pdata, uchar sdata,
		 ipmi_addr_response_handler_t handler,
		 void *handler_data);
int ipmicmd_mc_send(ipmi_mc_t *mc,
                 uchar netfn, uchar cmd, uchar lun,
		 uchar *pdata, uchar sdata,
		 ipmi_mc_response_handler_t handler,
		 void *handler_data);

#if 0
int ipmi_open_mv(void)
{
    if (ipmi_fd != -1) return(0);
    ipmi_fd = open("/dev/ipmi/0", O_RDWR);
    if (ipmi_fd == -1) 
        ipmi_fd = open("/dev/ipmi0", O_RDWR);
    if (ipmi_fd == -1)
        ipmi_fd = open("/dev/ipmidev0", O_RDWR);
    if (ipmi_fd == -1) 
        ipmi_fd = open("/dev/ipmidev/0", O_RDWR);
    if (ipmi_fd == -1) return(-1);
    return(0);
}

int ipmi_close_mv(void)
{
    int rc = 0;
    if (ipmi_fd != -1) { 
	rc = close(ipmi_fd);
	ipmi_fd = -1; 
    }
    return(rc);
}

int ipmicmd_mv(uchar cmd, uchar netfn, uchar lun, uchar *pdata, uchar sdata, 
		uchar *presp, int sresp, int *rlen)
{
    fd_set readfds;
    struct timeval tv;
    struct ipmi_recv      rsp;
    struct ipmi_addr      addr;
    struct ipmi_req                   req;
    struct ipmi_system_interface_addr bmc_addr;
    int    i;
    int    rv;

    rv = ipmi_open_mv();
    if (rv != 0) return(rv);

    i = 1;
    rv = ioctl(ipmi_fd, IPMICTL_SET_GETS_EVENTS_CMD, &i);
    if (rv) { return(errno); }

    FD_ZERO(&readfds);
    // FD_SET(0, &readfds);  /* dont watch stdin */
    FD_SET(ipmi_fd, &readfds);  /* only watch ipmi_fd for input */

    /* Send the IPMI command */ 
    bmc_addr.addr_type = IPMI_SYSTEM_INTERFACE_ADDR_TYPE;
    bmc_addr.channel = IPMI_BMC_CHANNEL;
    bmc_addr.lun = lun;       // BMC_LUN = 0
    req.addr = (unsigned char *) &bmc_addr;
    req.addr_len = sizeof(bmc_addr);
    req.msg.cmd = cmd;
    req.msg.netfn = netfn;   
    req.msgid = curr_seq;
    req.msg.data = pdata;
    req.msg.data_len = sdata;
    rv = ioctl(ipmi_fd, IPMICTL_SEND_COMMAND, &req);
    curr_seq++;
    if (rv == -1) { rv = errno; }

    if (rv == 0) {
	tv.tv_sec=ipmi_timeout_mv;
	tv.tv_usec=0;
	rv = select(ipmi_fd+1, &readfds, NULL, NULL, &tv);
	/* expect select rv = 1 here */
	if (rv <= 0) { /* no data within 5 seconds */
	   if (fperr != NULL)
             fprintf(fperr,"drv select timeout, fd = %d, isset = %d, rv = %d, errno = %d\n",
		  ipmi_fd,FD_ISSET(ipmi_fd, &readfds),rv,errno);
	   if (rv == 0) rv = -3;
	   else rv = errno;
	} else {
	   /* receive the IPMI response */
	   rsp.addr = (unsigned char *) &addr;
	   rsp.addr_len = sizeof(addr);
	   rsp.msg.data = presp;
	   rsp.msg.data_len = sresp;
	   rv = ioctl(ipmi_fd, IPMICTL_RECEIVE_MSG_TRUNC, &rsp);
	   if (rv == -1) { 
	      if ((errno == EMSGSIZE) && (rsp.msg.data_len == sresp))
		 rv = 0;   /* errno 90 is ok */
	      else { 
		 rv = errno; 
		 if (fperr != NULL)
                   fprintf(fperr,"drv rcv_trunc errno = %d, len = %d\n",
			errno, rsp.msg.data_len);
	      }
	   } else rv = 0;
	   *rlen = rsp.msg.data_len;
	}
    }

    /* ipmi_close_mv();  * rely on the app calling ipmi_close */
    return(rv);
}

#endif


int ipmicmd_send(ipmi_domain_t *domain,
                 uchar netfn, uchar cmd, uchar lun, uchar chan,
		 uchar *pdata, uchar sdata,
		 ipmi_addr_response_handler_t handler,
		 void *handler_data)
{
    struct ipmi_system_interface_addr si;
    struct ipmi_msg                   msg;


    /* Send the IPMI command */ 
    si.addr_type = IPMI_SYSTEM_INTERFACE_ADDR_TYPE;
    si.channel = chan;
    si.lun = lun;       // BMC_LUN = 0
    
    msg.netfn = netfn;
    msg.cmd = cmd;
    msg.data = pdata;
    msg.data_len = sdata;
    
    return ipmi_send_command_addr(domain,
			(ipmi_addr_t *) &si, sizeof(si),
			&msg,
			handler, handler_data, NULL);
}


int ipmicmd_mc_send(ipmi_mc_t *mc,
                 uchar netfn, uchar cmd, uchar lun,
		 uchar *pdata, uchar sdata,
		 ipmi_mc_response_handler_t handler,
		 void *handler_data)
{

    struct ipmi_msg                   msg;
    
    msg.netfn = netfn;
    msg.cmd = cmd;
    msg.data = pdata;
    msg.data_len = sdata;

    return ipmi_mc_send_command(mc, lun, &msg, handler, handler_data);
}



typedef struct {
	uchar cmd;
	uchar netfn;
	uchar lun;
	uchar *pdata;
	uchar sdata;
	uchar *presp;
	int sresp;
	int *rlen;
	SaErrorT rv;
	int done;
} ipmicmd_mv_arg_t;



static int ipmicmd_mv_handler(
			ipmi_domain_t *domain,
			ipmi_msgi_t   *rspi)
{
	ipmicmd_mv_arg_t *info = rspi->data1;
	ipmi_msg_t *msg = &rspi->msg;
	
	
	if (domain == NULL) {
		err("domain == NULL");
		info->rv = SA_ERR_HPI_INVALID_PARAMS;
		info->done = 1;
		return IPMI_MSG_ITEM_NOT_USED;
	}
	if (info->sresp < msg->data_len) {
		err("info->sresp(%d) < msg->data_len(%d)",
			info->sresp, msg->data_len);
		info->done = 1;
		info->rv = SA_ERR_HPI_OUT_OF_SPACE;
		return IPMI_MSG_ITEM_NOT_USED;
	}
	memcpy(info->presp, msg->data, msg->data_len);
	*info->rlen = msg->data_len;
	info->done = 1;
	return IPMI_MSG_ITEM_NOT_USED;
}


static void ipmicmd_mv_cb(ipmi_domain_t *domain, void *cb_data)
{
	ipmicmd_mv_arg_t *info = cb_data;
	int rv;
	rv = ipmicmd_send(domain, info->netfn, info->cmd, info->lun,
		0, info->pdata, info->sdata,
		ipmicmd_mv_handler, cb_data);
	if (rv != 0) {
		err("ipmicmd_send = %d", rv);
		OHOI_MAP_ERROR(info->rv, rv);
		info->done = 1;
	}
}

int ipmicmd_mv(struct ohoi_handler *ipmi_handler,
               uchar cmd,
	       uchar netfn,
	       uchar lun,
	       uchar *pdata,
	       uchar sdata, 
	       uchar *presp,
	       int sresp,
	       int *rlen)
{
	ipmicmd_mv_arg_t info;
	int rv;
	
	info.cmd = cmd;
	info.netfn = netfn;
	info.lun = lun;
	info.pdata = pdata;
	info.sdata = sdata;
	info.presp = presp;
	info.sresp = sresp;
	info.rlen = rlen;
	info.rv = 0;
	info.done = 0;
	
	rv = ipmi_domain_pointer_cb(ipmi_handler->domain_id,
					ipmicmd_mv_cb, &info);
	if (rv != 0) {
		err("ipmi_domain_pointer_cb = %d", rv);
		return SA_ERR_HPI_BUSY;
	}
	rv = ohoi_loop(&info.done, ipmi_handler);
	if (rv != SA_OK) {
		err("ohoi_loop = %d", rv);
		return rv;
	}
	return info.rv;
}
	
	

#ifdef TEST
int
main(int argc, char *argv[])
{
    fd_set readfds;
    struct timeval tv;
    char   data[40];
    int    i, j;
    int    err;
    int    rlen;

    err = ipmicmd_mv(0x01, 0x06, 0, NULL, 0, data, sizeof(data), &rlen);
    printf("ipmicmd_mv ret=%d, cc=%02x\n",err,(uchar)data[0]);
    printf(" ** Return Code: %2.2X\n", data[0]);
    printf(" ** Data[%d]: ",rlen);
    for (i=1; i < rlen; i++)
	    printf("%2.2X ", (uchar)data[i]);
    printf("\n");

    printf("\n");
    ipmi_close_mv();
    return 0;
}
#endif
