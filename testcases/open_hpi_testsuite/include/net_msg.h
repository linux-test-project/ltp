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
 * Authors:  David Judkovics
 *     
 *     
 *     
 */

#ifndef __NET_MSG_H
#define __NET_MSG_H

#define READ_BUFFER_SIZE 4096

#define SERV_TCP_PORT   6543
#define SERV_HOST_ADDR  "127.0.0.1" /* host addr for server */
/* #define SERV_HOST_ADDR  "9.60.72.223" */ /* host addr for server */

/* externel prototypes */
int send_msg(char *message, int message_length, int sockfd);


enum Msg_Types {
        OPEN = 1,
        CLOSE,
        GET_EVENT,
        DISCOVER_RESOURCES, 
        GET_SEL_ID, 
        GET_SEL_INFO, 
        SET_SEL_TIME, 
        SET_SEL_STATE, 
        GET_SENSOR_DATA, 
        GET_SENSOR_THRESHOLDS, 
        SET_SENSOR_THRESHOLDS,  
        GET_SENSOR_EVENT_ENABLES, 
        SET_SENSOR_EVENT_ENABLES, 
        GET_CONTROL_INFO, 
        GET_CONTROL_STATE, 
        SET_CONTROL_STATE, 
        GET_INVENTORY_SIZE, 
        GET_INVENTORY_INFO,
        SET_INVENTORY_INFO, 
        GET_WATCHDOG_INFO, 
        SET_WATCHDOG_INFO, 
        RESET_WATCHDOG, 
        GET_HOTSWAP_STATE, 
        SET_HOTSWAP_STATE, 
        REQUEST_HOTSWAP_ACTION, 
        GET_POWER_STATE, 
        SET_POWER_STATE,
        GET_INDICATOR_STATE, 
        SET_INDICATOR_STATE, 
        CONTROL_PARM, 
        GET_RESET_STATE,  
        SET_RESET_STATE,
        UNKNOWN_MSG_TYPE
};


typedef struct {
        int msg_length;
        int msg_type;
} NETWORK_HDR_STR;

typedef struct {
        NETWORK_HDR_STR header;
        char text_message[32];
} OPEN_MSG_STR;

typedef struct {
        NETWORK_HDR_STR header;
} CLOSE_MSG_STR;

typedef struct {
        NETWORK_HDR_STR header;
} GET_EVENTMSG_STR;

typedef struct {
        NETWORK_HDR_STR header;
        int error;
        int num_resource_events;
	struct oh_event event;   
//	this is for your own good need to recreate indexing without use of this element
//        struct oh_event oh_event_arry[3];
} DISCOVER_RESOURCES_MSG_STR;

typedef struct {
        NETWORK_HDR_STR header;
} GET_SEL_IDMSG_STR;

typedef struct {
        NETWORK_HDR_STR header;
} GET_SEL_INFO_MSG_STR;

typedef struct {
        NETWORK_HDR_STR header;
} SET_SEL_TIME_MSG_STR;

typedef struct {
        NETWORK_HDR_STR header;
} SET_SEL_STATE_MSG_STR;

typedef struct {
        NETWORK_HDR_STR header;
} GET_SENSOR_DATA_MSG_STR;

typedef struct {
        NETWORK_HDR_STR header;
} GET_SENSOR_THRESHOLDS_MSG_STR;

typedef struct {
        NETWORK_HDR_STR header;
} SET_SENSOR_THRESHOLDS_MSG_STR;

typedef struct {
        NETWORK_HDR_STR header;
} GET_SENSOR_EVENT_ENABLES_MSG;

typedef struct {
        NETWORK_HDR_STR header;
} SET_SENSOR_EVENT_ENABLES_MSG_STR;

typedef struct {
        NETWORK_HDR_STR header;
} GET_CONTROL_INFO_MSG_STR;

typedef struct {
        NETWORK_HDR_STR header;
} GET_CONTROL_STATE_MSG_STR;

typedef struct {
        NETWORK_HDR_STR header;
} SET_CONTROL_STATE_MSG_STR;

typedef struct {
        NETWORK_HDR_STR header;
} GET_INVENTORY_SIZE_MSG_STR;

typedef struct {
        NETWORK_HDR_STR header;
} GET_INVENTORY_INFO_MSG_STR;

typedef struct {
        NETWORK_HDR_STR header;
} SET_INVENTORY_INFO_MSG_STR;

typedef struct {
        NETWORK_HDR_STR header;
} GET_WATCHDOG_INFO_MSG_STR;

typedef struct {
        NETWORK_HDR_STR header;
} SET_WATCHDOG_INFO_MSG_STR;

typedef struct {
        NETWORK_HDR_STR header;
} RESET_WATCHDOG_MSG_STR;

typedef struct {
        NETWORK_HDR_STR header;
} GET_HOTSWAP_STATE_MSG_STR;

typedef struct {
        NETWORK_HDR_STR header;
} SET_HOTSWAP_STATE_MSG_STR;

typedef struct {
        NETWORK_HDR_STR header;
} REQUEST_HOTSWAP_ACTION_MSG_STR;

typedef struct {
        NETWORK_HDR_STR header;
} GET_POWER_STATE_MSG_STR;

typedef struct {
        NETWORK_HDR_STR header;
} SET_POWER_STATE_MSG_STR;

typedef struct {
        NETWORK_HDR_STR header;
} GET_INDICATOR_STATE_MSG_STR;

typedef struct {
        NETWORK_HDR_STR header;
} SET_INDICATOR_STATE_MSG_STR;

typedef struct {
        NETWORK_HDR_STR header;
} CONTROL_PARM_MSG_STR;

typedef struct {
        NETWORK_HDR_STR header;
} GET_RESET_STATE_MSG_STR;

typedef struct {
        NETWORK_HDR_STR header;
} SET_RESET_STATE_MSG_STR;

typedef struct {
        NETWORK_HDR_STR header;
} UNKNOWN_MSG_TYPE_MSG_STR;

#endif/*__NET_MSG_H*/


