/*
 *   Copyright (c) International Business Machines  Corp., 2001
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

package dots.framework;

import java.io.*;
import java.util.*;

/**
 * This class is designed to stores static global variables which are use 
 * by many other classes. 
 */
public class DotsConfig {

	/** Specify how many minutes a test case in DOTS is going to run. */
	public static int	DURATION;
	/** Database access thread group. */
	public static ThreadGroup	THRDGRP;
	/** Log file directory. */
	public static String 	LOG_DIR;
	/** Database Server IP. */
	public static String 	SERVER_IP;
	/** Performance monitor socket port. */
	public static String 	PERF_SVR_PORT;
	/** Write test summary time interval. */
	public static int	SUMMARY_INTERVAL;
	/** If automatically run test case to meet cpu target. */
	public static boolean 	AUTO_MODE;
	/** Termination flag. */
	public static boolean 	TERMINATION;
	/** Least thread number. */
	public static int	CONNECTIONS;
	/** CPU target. */
	public static int		CPU_TARGET;
	/** Jdbc driver class name. */
	public static String 	DRIVER_CLASS_NAME;
	/** Connection url. */
	public static String 	URL;
	/** Database user ID. */ 
	public static String 	DB_UID;
	/** Database user password. */
	public static String 	DB_PASSWD;
	/** Performance data from server. */
	public static String	PERF_SUMMARY;
	/** Server current cpu usage. */
	public static int		CPU_USAGE;
	/** Table max row number. */
	public static int	MAX_ROWS;
	/** Query database API access count. */
	public static long QUERYCOUNT;
	/** Update database API access count. */
	public static long UPDATECOUNT;
	/** Failed database access count. */
	public static long FAILEDCOUNT;
	/** Insert database API access count. */
	public static long INSERTCOUNT;
	/** Delete database API access count. */
	public static long DELETECOUNT;
	/** Maximum bytes of log file */
	public static int MAX_LOGFILESIZE;
	/** Create thread interval */
	public static int INTERVAL;
	/** If RUN_AUTO is true, Keyboard thread is disabled and the DOTS can run backgroud. */
	public static boolean RUN_AUTO;

}