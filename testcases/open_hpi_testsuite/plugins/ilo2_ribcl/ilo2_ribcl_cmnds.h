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
 *     Shuah Khan <shuah.khan@hp.com>
 *     Richard White <richard.white@hp.com>
 */
#ifndef _INC_ILO2_RIBCL_CMNDS_H_
#define _INC_ILO2_RIBCL_CMNDS_H_

/***************
 * This header file contains all of the iLO2 RIBLC command denfinitions.
***************/
/* 
 * XML header definition. iLO2 requires that this  header be  sent at the
 * start of every RIBCL transation. The XML header ensures the connection
 * is an XML connection, not an HTTP connection.
 */
#define ILO2_RIBCL_XML_HDR      "<?xml version=\"1.0\"?>\r\n"

/* Here are all the RIBCL xml templates for the commands. For each command,
 * we have a format specification string that allows us to insert the
 * login and password strings for individual systems. Also, each command
 * has an assocoated macro value that gives its unique index for use in
 * command arrays. When you add a template, make sure to increment
 * IR_NUM_COMMANDS and add code to ir_xml_build_cmdbufs() in file
 * ilo2_ribcl_xml.c.
 */

#define IR_NUM_COMMANDS	22

/*
 * GET_SERVER_DATA command.
 * This is a combination of the RIBCL GET_HOST_DATA, GET_EMBEDDED_HEALTH,
 * and GET_FW_VERSION commands.
 * These three commands are sent in one single transation to get
 * complete server information including fans, temp sensors, cpus, dimms,
 * power supplies, VRMs, and I/O slots. The following define combines and
 * embedds the GET_HOST_DATA, GET_EMBEDDED_HEALTH and GET_FW_VERSION commands
 * within a single LOGIN block.
 */
#define IR_CMD_GET_SERVER_DATA 0
#define ILO2_RIBCL_GET_SERVER_DATA	"<LOCFG version=\"2.21\"/> <RIBCL version=\"2.22\"> <LOGIN USER_LOGIN=\"%s\" PASSWORD=\"%s\"> <SERVER_INFO MODE=\"READ\" > <GET_HOST_DATA /> <GET_EMBEDDED_HEALTH /> </SERVER_INFO> <RIB_INFO MODE=\"read\"> <GET_FW_VERSION/> </RIB_INFO> </LOGIN> </RIBCL>\r\n"

/*
 * GET_HOST_POWER_STATUS command
*/
#define IR_CMD_GET_HOST_POWER_STATUS 1
#define ILO2_RIBCL_GET_HOST_POWER_STATUS	"<LOCFG version=\"2.21\"/> <RIBCL VERSION=\"2.0\"> <LOGIN USER_LOGIN=\"%s\" PASSWORD=\"%s\"> <SERVER_INFO MODE=\"read\"> <GET_HOST_POWER_STATUS/> </SERVER_INFO> </LOGIN> </RIBCL>\r\n"

/*
 * SET_HOST_POWER commands
*/
#define IR_CMD_SET_HOST_POWER_ON 2
#define ILO2_RIBCL_SET_HOST_POWER_ON	"<LOCFG version=\"2.21\"/> <RIBCL VERSION=\"2.0\"> <LOGIN USER_LOGIN=\"%s\" PASSWORD=\"%s\"> <SERVER_INFO MODE=\"write\"> <SET_HOST_POWER HOST_POWER=\"Yes\"/> </SERVER_INFO> </LOGIN> </RIBCL>\r\n"

#define IR_CMD_SET_HOST_POWER_OFF 3
#define ILO2_RIBCL_SET_HOST_POWER_OFF	"<LOCFG version=\"2.21\"/> <RIBCL VERSION=\"2.0\"> <LOGIN USER_LOGIN=\"%s\" PASSWORD=\"%s\"> <SERVER_INFO MODE=\"write\"> <SET_HOST_POWER HOST_POWER=\"No\"/> </SERVER_INFO> </LOGIN> </RIBCL>\r\n"

/*
 * RESET_SERVER command
*/
#define IR_CMD_RESET_SERVER 4
#define ILO2_RIBCL_RESET_SERVER		"<LOCFG version=\"2.21\"/> <RIBCL VERSION=\"2.0\"> <LOGIN USER_LOGIN=\"%s\" PASSWORD=\"%s\"> <SERVER_INFO MODE=\"write\"> <RESET_SERVER/> </SERVER_INFO> </LOGIN> </RIBCL>\r\n"

/*
 * COLD_BOOT_SERVER command
*/
#define IR_CMD_COLD_BOOT_SERVER 5
#define ILO2_RIBCL_COLD_BOOT_SERVER		"<LOCFG version=\"2.21\"/> <RIBCL VERSION=\"2.0\"> <LOGIN USER_LOGIN=\"%s\" PASSWORD=\"%s\"> <SERVER_INFO MODE=\"write\"> <COLD_BOOT_SERVER/> </SERVER_INFO> </LOGIN> </RIBCL>\r\n"

/*
 * GET_UID_STATUS command
*/
#define IR_CMD_GET_UID_STATUS 6
#define ILO2_RIBCL_GET_UID_STATUS		"<LOCFG version=\"2.21\"/> <RIBCL VERSION=\"2.0\"> <LOGIN USER_LOGIN=\"%s\" PASSWORD=\"%s\"> <SERVER_INFO MODE=\"read\"> <GET_UID_STATUS/> </SERVER_INFO> </LOGIN> </RIBCL>\r\n"

/*
 * UID_CONTROL commands	
*/
#define IR_CMD_UID_CONTROL_OFF 7
#define ILO2_RIBCL_UID_CONTROL_OFF		"<LOCFG version=\"2.21\"/> <RIBCL VERSION=\"2.0\"> <LOGIN USER_LOGIN=\"%s\" PASSWORD=\"%s\"> <SERVER_INFO MODE=\"write\"> <UID_CONTROL UID=\"No\"/> </SERVER_INFO> </LOGIN> </RIBCL>\r\n"

#define IR_CMD_UID_CONTROL_ON 8
#define ILO2_RIBCL_UID_CONTROL_ON		"<LOCFG version=\"2.21\"/> <RIBCL VERSION=\"2.0\"> <LOGIN USER_LOGIN=\"%s\" PASSWORD=\"%s\"> <SERVER_INFO MODE=\"write\"> <UID_CONTROL UID=\"Yes\"/> </SERVER_INFO> </LOGIN> </RIBCL>\r\n"

/*
 * GET_HOST_POWER_SAVER_STATUS command
*/
#define IR_CMD_GET_HOST_POWER_SAVER_STATUS 9
#define ILO2_RIBCL_GET_HOST_POWER_SAVER_STATUS		"<LOCFG version=\"2.21\"/> <RIBCL VERSION=\"2.0\"> <LOGIN USER_LOGIN=\"%s\" PASSWORD=\"%s\"> <SERVER_INFO MODE=\"read\"> <GET_HOST_POWER_SAVER_STATUS/> </SERVER_INFO> </LOGIN> </RIBCL>\r\n"

/*
 * SET_HOST_POWER_SAVER commands
*/
#define IR_CMD_SET_HOST_POWER_SAVER_1 10
#define ILO2_RIBCL_SET_HOST_POWER_SAVER_1		"<LOCFG version=\"2.21\"/> <RIBCL VERSION=\"2.0\"> <LOGIN USER_LOGIN=\"%s\" PASSWORD=\"%s\"> <SERVER_INFO MODE=\"write\"> <SET_HOST_POWER_SAVER HOST_POWER_SAVER=\"1\"/> </SERVER_INFO> </LOGIN> </RIBCL>\r\n"

#define IR_CMD_SET_HOST_POWER_SAVER_2 11
#define ILO2_RIBCL_SET_HOST_POWER_SAVER_2		"<LOCFG version=\"2.21\"/> <RIBCL VERSION=\"2.0\"> <LOGIN USER_LOGIN=\"%s\" PASSWORD=\"%s\"> <SERVER_INFO MODE=\"write\"> <SET_HOST_POWER_SAVER HOST_POWER_SAVER=\"2\"/> </SERVER_INFO> </LOGIN> </RIBCL>\r\n"

#define IR_CMD_SET_HOST_POWER_SAVER_3 12
#define ILO2_RIBCL_SET_HOST_POWER_SAVER_3		"<LOCFG version=\"2.21\"/> <RIBCL VERSION=\"2.0\"> <LOGIN USER_LOGIN=\"%s\" PASSWORD=\"%s\"> <SERVER_INFO MODE=\"write\"> <SET_HOST_POWER_SAVER HOST_POWER_SAVER=\"3\"/> </SERVER_INFO> </LOGIN> </RIBCL>\r\n"

#define IR_CMD_SET_HOST_POWER_SAVER_4 13
#define ILO2_RIBCL_SET_HOST_POWER_SAVER_4		"<LOCFG version=\"2.21\"/> <RIBCL VERSION=\"2.0\"> <LOGIN USER_LOGIN=\"%s\" PASSWORD=\"%s\"> <SERVER_INFO MODE=\"write\"> <SET_HOST_POWER_SAVER HOST_POWER_SAVER=\"4\"/> </SERVER_INFO> </LOGIN> </RIBCL>\r\n"

#define IR_CMD_GET_SERVER_AUTO_PWR 14
#define ILO2_RIBCL_GET_SERVER_AUTO_PWR		"<LOCFG version=\"2.21\"/> <RIBCL VERSION=\"2.0\"> <LOGIN USER_LOGIN=\"%s\" PASSWORD=\"%s\"> <SERVER_INFO MODE=\"read\"> <GET_SERVER_AUTO_PWR/> </SERVER_INFO> </LOGIN> </RIBCL>\r\n"

/* Enable automatic power on with a minimum delay. */
#define IR_CMD_SERVER_AUTO_PWR_YES 15
#define ILO2_RIBCL_SERVER_AUTO_PWR_YES		"<LOCFG version=\"2.21\"/> <RIBCL VERSION=\"2.0\"> <LOGIN USER_LOGIN=\"%s\" PASSWORD=\"%s\"> <SERVER_INFO MODE=\"write\"> <SERVER_AUTO_PWR VALUE=\"Yes\"/> </SERVER_INFO> </LOGIN> </RIBCL>\r\n"

/*  Disable automatic power on. */
#define IR_CMD_SERVER_AUTO_PWR_NO 16
#define ILO2_RIBCL_SERVER_AUTO_PWR_NO		"<LOCFG version=\"2.21\"/> <RIBCL VERSION=\"2.0\"> <LOGIN USER_LOGIN=\"%s\" PASSWORD=\"%s\"> <SERVER_INFO MODE=\"write\"> <SERVER_AUTO_PWR VALUE=\"No\"/> </SERVER_INFO> </LOGIN> </RIBCL>\r\n"

/* Enable automatic power on with 15 seconds delay. */
#define IR_CMD_SERVER_AUTO_PWR_15 17
#define ILO2_RIBCL_SERVER_AUTO_PWR_15		"<LOCFG version=\"2.21\"/> <RIBCL VERSION=\"2.0\"> <LOGIN USER_LOGIN=\"%s\" PASSWORD=\"%s\"> <SERVER_INFO MODE=\"write\"> <SERVER_AUTO_PWR VALUE=\"15\"/> </SERVER_INFO> </LOGIN> </RIBCL>\r\n"

/* Enable automatic power on with 30 seconds delay. */
#define IR_CMD_SERVER_AUTO_PWR_30 18
#define ILO2_RIBCL_SERVER_AUTO_PWR_30		"<LOCFG version=\"2.21\"/> <RIBCL VERSION=\"2.0\"> <LOGIN USER_LOGIN=\"%s\" PASSWORD=\"%s\"> <SERVER_INFO MODE=\"write\"> <SERVER_AUTO_PWR VALUE=\"30\"/> </SERVER_INFO> </LOGIN> </RIBCL>\r\n"

/* Enable automatic power on with 45 seconds delay. */
#define IR_CMD_SERVER_AUTO_PWR_45 19
#define ILO2_RIBCL_SERVER_AUTO_PWR_45		"<LOCFG version=\"2.21\"/> <RIBCL VERSION=\"2.0\"> <LOGIN USER_LOGIN=\"%s\" PASSWORD=\"%s\"> <SERVER_INFO MODE=\"write\"> <SERVER_AUTO_PWR VALUE=\"45\"/> </SERVER_INFO> </LOGIN> </RIBCL>\r\n"

/* Enable automatic power on with 60 seconds delay. */
#define IR_CMD_SERVER_AUTO_PWR_60 20
#define ILO2_RIBCL_SERVER_AUTO_PWR_60		"<LOCFG version=\"2.21\"/> <RIBCL VERSION=\"2.0\"> <LOGIN USER_LOGIN=\"%s\" PASSWORD=\"%s\"> <SERVER_INFO MODE=\"write\"> <SERVER_AUTO_PWR VALUE=\"60\"/> </SERVER_INFO> </LOGIN> </RIBCL>\r\n"

/* Enable automatic power on with random delay up to 60 seconds. */
#define IR_CMD_SERVER_AUTO_PWR_RANDOM 21
#define ILO2_RIBCL_SERVER_AUTO_PWR_RANDOM		"<LOCFG version=\"2.21\"/> <RIBCL VERSION=\"2.0\"> <LOGIN USER_LOGIN=\"%s\" PASSWORD=\"%s\"> <SERVER_INFO MODE=\"write\"> <SERVER_AUTO_PWR VALUE=\"Random\"/> </SERVER_INFO> </LOGIN> </RIBCL>\r\n"

#endif /*_INC_ILO2_RIBCL_CMNDS_H_*/
