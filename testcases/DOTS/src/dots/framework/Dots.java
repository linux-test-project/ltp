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

import dots.advcase.*;
import dots.basecase.*;

import java.sql.*;
import java.util.*;
import java.io.*;

/**
 * Main class of DOTS
 */
 
public class Dots {

	/** Test case name */
	private	static String TESTCASENAME = "";

	private static int INTERVAL;
	
	/**
	 * Dots constructor 
	 */
	public Dots() {
		super();
	}
	
	/**
	 * Read command line input and load config file,
	 * set config value to the instance of DotsConfig.
	 */
	public static void loadConfig(String[] args) {
		int		i;
		String	configFileName = "./config.ini";
		String	tmpString;
		String	value;
		File	configFile = null;
		StringTokenizer stk = null;
		Properties	props = new Properties();
		String	propString;

		/* Read command line input */
		for (i = 0; i < args.length;) {
			String option = args[i++];

			if (option.equals("-config")) {
				configFileName = args[i++];
			}else if (option.equals("-case")) {
				TESTCASENAME = args[i++];
			}else if (option.equals("-?") || option.equals("-help")) {
				System.out.println("Usage:Dots [-option]");
				System.out.println("\t\t-config				Config file name");
				System.out.println("\t\t-case				Test case name");
				System.out.println("\t\t-help				Print this message");
				System.exit(0);
			}
			else {
				System.out.println("Expected Option Is Not Found!");
				System.out.println("Usage:	Dots -config [config file] -case [case name]");
				System.exit(0);
			}
		}

		TESTCASENAME = TESTCASENAME.toUpperCase();

		/* Check test case name */
		if (TESTCASENAME.equals("")) {
			System.out.println("Testcase Name Must Be Specified!");
			System.out.println("Usage:	Dots -config [config file] -case [case name]");
			System.exit(0);
		}
		if (!(TESTCASENAME.equals("BTCJ1") || TESTCASENAME.equals("BTCJ2") || TESTCASENAME.equals("BTCJ3")
			|| TESTCASENAME.equals("BTCJ4") || TESTCASENAME.equals("BTCJ5") || TESTCASENAME.equals("BTCJ6")
			|| TESTCASENAME.equals("BTCJ7") || TESTCASENAME.equals("BTCJ8") || TESTCASENAME.equals("ATCJ1")
			|| TESTCASENAME.equals("ATCJ2"))) {
			System.out.println("Testcase Name Is Not Correct!");
			System.exit(0);
		}

		/* Load config file and set DotsConfig value */
		try {
			props.load(new FileInputStream(configFileName));

			if ((propString = props.getProperty("DURATION")) != null) {
				if (propString.trim().length() <= 0) {
					System.out.println("Expected Config File Option DURATION Is Not Found!");
					System.exit(1);
				}
				stk = new StringTokenizer(propString,":");
				if (stk.countTokens() > 1) {
					value = stk.nextToken().trim();
					tmpString = stk.nextToken().trim();
					DotsConfig.DURATION = Integer.parseInt(value) * 60 + Integer.parseInt(tmpString);
				}else
					DotsConfig.DURATION = Integer.parseInt(propString.trim()) * 60;
				if (DotsConfig.DURATION <= 1) {
					System.out.println("DURATION value is too small(<=1)!");
					System.exit(1);
				}
			} else {
				System.out.println("Expected Config File Option DURATION Is Not Found!");
				System.exit(1);
			}

			if ((propString = props.getProperty("CONCURRENT_CONNECTIONS")) != null) {
				if (propString.trim().length() <= 0) {
					System.out.println("Expected Config File Option CONCURRENT_CONNECTIONS Is Not Found!");
					System.exit(1);
				}
				DotsConfig.CONNECTIONS = Integer.parseInt(propString.trim());
				if (DotsConfig.CONNECTIONS < 1) {
					System.out.println("CONCURRENT_CONNECTIONS value is too small(<1)!");
					System.exit(1);
				}
			} else {
				System.out.println("Expected Config File Option CONCURRENT_CONNECTIONS Is Not Found!");
				System.exit(1);
			}
			
			if ((propString = props.getProperty("CPU_TARGET")) != null) {
				if (propString.trim().length() <= 0) {
					System.out.println("Expected Config File Option CPU_TARGET Is Not Found!");
					System.exit(1);
				}
				DotsConfig.CPU_TARGET = Integer.parseInt(propString.trim());
				if (DotsConfig.CPU_TARGET <= 10) {
					System.out.println("CPU_TARGET value is too small(<10)!");
					System.exit(1);
				}
				if (DotsConfig.CPU_TARGET > 100) {
					System.out.println("CPU_TARGET value is too large(>100)!");
					System.exit(1);
				}
			} else {
				System.out.println("Expected Config File Option CPU_TARGET Is Not Found!");
				System.exit(1);
			}
			
			if ((propString = props.getProperty("LOG_DIR")) != null) {
				if (propString.trim().length() <= 0) {
					System.out.println("Expected Config File Option LOG_DIR Is Not Found!");
					System.exit(1);
				}
				DotsConfig.LOG_DIR = propString.trim();
			} else {
				System.out.println("Expected Config File Option LOG_DIR Is Not Found!");
				System.exit(1);
			}

			if ((propString = props.getProperty("AUTO_MODE")) != null) {
				if (propString.trim().equalsIgnoreCase("yes"))
					DotsConfig.AUTO_MODE = true;
				else if (propString.trim().equalsIgnoreCase("no"))
					DotsConfig.AUTO_MODE = false;
				else {
					System.out.println("AUTO_MODE format is not correct!");
					System.exit(1);
				}
			} else {
				System.out.println("Expected Config File Option AUTO_MODE Is Not Found!");
				System.exit(1);
			}

			if ((propString = props.getProperty("SUMMARY_INTERVAL")) != null) {
				if (propString.trim().length() <= 0) {
					System.out.println("Expected Config File Option SUMMARY_INTERVAL Is Not Found!");
					System.exit(1);
				}
				DotsConfig.SUMMARY_INTERVAL = Integer.parseInt(propString.trim());
				if (DotsConfig.SUMMARY_INTERVAL < 1) {
					System.out.println("SUMMARY_INTERVAL value is too small(<1)!");
					System.exit(1);
				}
			} else {
				System.out.println("Expected Config File Option SUMMARY_INTERVAL Is Not Found!");
				System.exit(1);
			}

			if ((propString = props.getProperty("UserID")) != null) {
				if (propString.trim().length() <= 0) {
					System.out.println("Expected Config File Option UserID Is Not Found!");
					System.exit(1);
				}
				DotsConfig.DB_UID = propString.trim();
			} else {
				System.out.println("Expected Config File Option UserID Is Not Found!");
				System.exit(1);
			}

			if ((propString = props.getProperty("Password")) != null) {
				if (propString.trim().length() <= 0) {
					System.out.println("Expected Config File Option Password Is Not Found!");
					System.exit(1);
				}
				DotsConfig.DB_PASSWD = propString.trim();
			} else {
				System.out.println("Expected Config File Option Password Is Not Found!");
				System.exit(1);
			}

			if ((propString = props.getProperty("DriverClass")) != null) {
				if (propString.trim().length() <= 0) {
					System.out.println("Expected Config File Option DriverClass Is Not Found!");
					System.exit(1);
				}
				DotsConfig.DRIVER_CLASS_NAME = propString.trim();
			} else {
				System.out.println("Expected Config File Option DriverClass Is Not Found!");
				System.exit(1);
			}

			if ((propString = props.getProperty("URL")) != null) {
				if (propString.trim().length() <= 0) {
					System.out.println("Expected Config File Option URL Is Not Found!");
					System.exit(1);
				}
				DotsConfig.URL = propString.trim();
			} else {
				System.out.println("Expected Config File Option URL Is Not Found!");
				System.exit(1);
			}

			if ((propString = props.getProperty("SERVER_IP")) != null) {
				if (propString.trim().length() <= 0) {
					System.out.println("Expected Config File Option SERVER_IP Is Not Found!");
					System.exit(1);
				}
				DotsConfig.SERVER_IP = propString.trim();
			} else {
				System.out.println("Expected Config File Option SERVER_IP Is Not Found!");
				System.exit(1);
			}

			if ((propString = props.getProperty("SERVER_PORT")) != null) {
				if (propString.trim().length() <= 0) {
					System.out.println("Expected Config File Option SERVER_PORT Is Not Found!");
					System.exit(1);
				}
				DotsConfig.PERF_SVR_PORT = propString.trim();
			} else {
				System.out.println("Expected Config File Option SERVER_PORT Is Not Found!");
				System.exit(1);
			}

			if ((propString = props.getProperty("MAX_ROWS")) != null) {
				if (propString.trim().length() <= 0) {
					System.out.println("Expected Config File Option MAX_ROWS Is Not Found!");
					System.exit(1);
				}
				DotsConfig.MAX_ROWS = Integer.parseInt(propString.trim());
				if (DotsConfig.MAX_ROWS <= 1) {
					System.out.println("MAX_ROWS value is too small(<=1)!");
					System.exit(1);
				}
			} else {
				System.out.println("Expected Config File Option MAX_ROWS Is Not Found!");
				System.exit(1);
			}

			if ((propString = props.getProperty("MAX_LOGFILESIZE")) != null) {
				if (propString.trim().length() <= 0) {
					System.out.println("Excepted Config File Option MAX_LOGFILESIZE Is Not Found!");
					System.exit(1);
				}
				DotsConfig.MAX_LOGFILESIZE = Integer.parseInt(propString.trim());
				if (DotsConfig.MAX_LOGFILESIZE <= 102400) {
					System.out.println("MAX_LOGFILESIZE value is too small(<=100K)");
					System.exit(1);
				}
				if (DotsConfig.MAX_LOGFILESIZE > 1073741824) { // 1G
					System.out.println("MAX_LOGFILESIZE value is too large(>1G)");
					System.exit(1);
				}
			} else {
				System.out.println("Excepted Config File Option MAX_LOGFILESIZE Is Not Found!");
				System.exit(1);
			}

			if ((propString = props.getProperty("CREATIONINTERVAL")) != null ) {
				DotsConfig.INTERVAL = Integer.parseInt(propString.trim());
				if (DotsConfig.INTERVAL > 5) {
					System.out.println("CREATIONINTERVAL value is too large(>5min)");
					System.exit(1);
				}
				if (DotsConfig.INTERVAL < 1) {
					System.out.println("CREATIONPINTERVAL value is too small(<1min");
					System.exit(1);
				}
			} else {
				System.out.println("Excepted Config File Option CREATIONINTERVAL Is Not Found!");
				System.exit(1);
			}
			
			if ((propString = props.getProperty("RUN_AUTO")) != null ) {
				if (propString.trim().equalsIgnoreCase("yes"))
					DotsConfig.RUN_AUTO = true;
				else if (propString.trim().equalsIgnoreCase("no"))
					DotsConfig.RUN_AUTO = false;
				else {
					System.out.println("RUN_AUTO format is not correct!");
					System.exit(1);
				}
			} else {
				DotsConfig.RUN_AUTO = false;
			}
			
			DotsConfig.THRDGRP = new ThreadGroup("DBAccess");
			DotsConfig.CPU_USAGE = 0;
			DotsConfig.QUERYCOUNT = 0;
			DotsConfig.INSERTCOUNT = 0;
			DotsConfig.DELETECOUNT = 0;
			DotsConfig.UPDATECOUNT = 0;
			DotsConfig.FAILEDCOUNT = 0;
		}catch (NumberFormatException e) {
			System.out.println("Config File Number Format Error!");
			e.printStackTrace();
			System.exit(1);
		}catch (IOException e) {
			System.out.println("Read Config File Error!");
			e.printStackTrace();
			System.exit(1);
		}
	}

	/**
	 * Check if database connection can be established
	 * using the values from config file.
	 */
	public static void checkDBConn() {
		try {
			Connection	con = null;
			Statement	stmt = null;

			Class.forName(DotsConfig.DRIVER_CLASS_NAME).newInstance();
			con = DriverManager.getConnection(DotsConfig.URL,DotsConfig.DB_UID,DotsConfig.DB_PASSWD);
			stmt = con.createStatement();
			stmt.close();
			con.close();
		}catch (Exception e){
			DotsLogging.logException("Dots.checkDBConn: " + e.getMessage());
			System.out.println("DataBase Connect failed!");
			e.printStackTrace();
			System.exit(1);
		}
	}

	/**
	 * Create the performance client thread 
	 */
	public static void startPerfCtl() {
		DotsPerf perf = new DotsPerf();
		perf.start();
	}

	/**
	 * Create the keyboard listener thread 
	 */
	public static void startKeyboard() {
		DotsKeyboard keyboard = new DotsKeyboard();
		keyboard.start();
	}

	/**
	 * Create a connection to database
	 * @param driverClassName: Database access driver class
	 * @param url: Database url
	 * @param dbUid: Database user id
	 * @param dbPasswd: Database user password
	 */
	 public static Connection createConnection(String driverClassName,String url,String dbUid,String dbPasswd) {
		 Connection con = null;
		 try {
			 Class.forName(driverClassName).newInstance();
			 con = DriverManager.getConnection(url,dbUid,dbPasswd);
		 } catch (Exception e) {
			 DotsLogging.logException("Dots.createConnection: " + e);
		 }
		 return con;
	 }

	 /**
	 * Main function of the DOTS.
	 * Read the config file,create dotslogging,start performance client,
	 * check db connection,create keyboard thread.
	 * Create new testcase according the cpu_usage and connections,
	 * Write test summary and exception
	 */
	public static void main(String[] args) {

		int sleepInterval = 1;
		int waitInterval = 0;
		int averageUsage = 0;
		boolean reached = false;
		Calendar currentTime;

		/* Load DotsConfig value */
		loadConfig(args);
		/* Create the instance of DotsLogging */
		DotsLogging logging = new DotsLogging(TESTCASENAME);

		DotsLogging.logMessage("Database Opensource Test Suite V1.0");
		DotsLogging.logMessage("Start to run JDBC API Test Case - " + TESTCASENAME);
		DotsLogging.logSummary("Start to run JDBC API Test Case - " + TESTCASENAME);
		if(!DotsConfig.RUN_AUTO){
			System.out.println("\nDatabase Opensource Test Suite V1.0");
			System.out.println("\nStart to run JDBC API test case - " + TESTCASENAME);
		}

		DotsLogging.logMessage("Initialization started");
		if(!DotsConfig.RUN_AUTO){
			System.out.println("\nTo stop running the test case, type STOP then press Enter\n");
		}
		/* Start performance monitor thread */
		startPerfCtl();

		/* Check database connection */
		checkDBConn();

		DotsLogging.logMessage("Testing Database Connections ...OK");

		/* Start summary writer thread */
		DotsSummary summary = new DotsSummary();
		summary.start();

		DotsLogging.logMessage("Starting Summary Writer ... OK ");

		/* Set begin and end time */
		Calendar	endTime = Calendar.getInstance();
		DotsSummary.startTime = System.currentTimeMillis();
		endTime.add(Calendar.SECOND,DotsConfig.DURATION * 60);

		/* Start keyboard thread */
		if(!DotsConfig.RUN_AUTO){
			startKeyboard();
			DotsLogging.logMessage("Starting Keyboard Thread ... OK");
		}

		boolean firstRun = true; //used for LOB manipulation
		String lobFileName[] = new String[10]; //used for LOB manipulation

		averageUsage = DotsConfig.CPU_USAGE;
		int targetCpu = DotsConfig.CPU_TARGET;
		try {
			while (true) {
				currentTime = Calendar.getInstance();
				if (currentTime.after(endTime))	break;
				if (DotsConfig.TERMINATION)	break;
				/* Check if need create a new database access thread*/
				if ((DotsConfig.AUTO_MODE && (averageUsage < targetCpu))
					|| (!DotsConfig.AUTO_MODE && (DotsConfig.THRDGRP.activeCount() < DotsConfig.CONNECTIONS))) {
					Connection conn = createConnection(DotsConfig.DRIVER_CLASS_NAME, DotsConfig.URL, DotsConfig.DB_UID, DotsConfig.DB_PASSWD); 
					if (conn != null) {
						/* 5 second is a unit*/
						sleepInterval = DotsConfig.INTERVAL * 12;
						/* Create DB access thread */
						switch (TESTCASENAME.charAt(4)) {
							case '1' :
								if (TESTCASENAME.equals("BTCJ1")) {
									BTCJ1 btcj1 = new BTCJ1(conn);
									Thread thrdb1 = new Thread(DotsConfig.THRDGRP, btcj1);
									thrdb1.start();
								}
								else {												    
								        ATCJ1 atcj1 = new ATCJ1(conn);						
								        Thread thrda1 = new Thread(DotsConfig.THRDGRP,atcj1);					
								        thrda1.start();			
								}
								break;
							case '2' :
								if (TESTCASENAME.equals("BTCJ2")) {
									BTCJ2 btcj2 = new BTCJ2(conn);
									Thread thrdb2 = new Thread(DotsConfig.THRDGRP, btcj2);
									thrdb2.start();
								}
								else {
								        ATCJ2 atcj2 = new ATCJ2(conn);					
								        Thread thrda2 = new Thread(DotsConfig.THRDGRP,atcj2);						
								        thrda2.start();
								}
								break;
							case '3' :

								BTCJ3 btcj3 = new BTCJ3(conn);
								Thread thrdb3 = new Thread(DotsConfig.THRDGRP, btcj3);
								thrdb3.start();

								break;
							case '4' :

								BTCJ4 btcj4 = new BTCJ4(conn);
								Thread thrdb4 = new Thread(DotsConfig.THRDGRP, btcj4);
								thrdb4.start();

								break;
							case '5' :
								BTCJ5 btcj5 = new BTCJ5(conn);
								Thread thrdb5 = new Thread(DotsConfig.THRDGRP, btcj5);
								thrdb5.start();
								break;
							case '6' :
								if (firstRun) {
									for (int i = 0; i < 10; i++) {
										lobFileName[i] = DotsGenerator.mdClob(DotsGenerator.mdInt(10, 100));
									}
									firstRun = false;
								}
								BTCJ6 btcj6 = new BTCJ6(conn, lobFileName);
								Thread thrdb6 = new Thread(DotsConfig.THRDGRP, btcj6);
								thrdb6.start();
								break;
							case '7' :
								if (firstRun) {
									for (int i = 0; i < 10; i++) {
										lobFileName[i] = DotsGenerator.mdBlob(DotsGenerator.mdInt(10, 100));
									}
									firstRun = false;
								}
								BTCJ7 btcj7 = new BTCJ7(conn, lobFileName);
								Thread thrdb7 = new Thread(DotsConfig.THRDGRP, btcj7);
								thrdb7.start();
								break;
							case '8' :
								BTCJ8 btcj8 = new BTCJ8(conn);
								Thread thrdb8 = new Thread(DotsConfig.THRDGRP, btcj8);
								thrdb8.start();
								break;
							}
						}else {
							/* If createConnection failed,sleepInterval will double,
							   but the maximum of sleepInterval is 1 hour */
							sleepInterval = sleepInterval * 2;
							if (sleepInterval > 720 ) sleepInterval = 720;
						}
						try {
							averageUsage = 0;
							/* Wait 5*sleepInterval seconds,check TERMINATION per 5 second */
							for (int j = 0; j < sleepInterval; j ++){
								averageUsage += DotsConfig.CPU_USAGE;
								Thread.sleep(5000);
								if ((j % 12) == 0)
									DotsLogging.logMessage("Active Threads = " + DotsConfig.THRDGRP.activeCount() + " Average CPU Usage = " + DotsConfig.CPU_USAGE + "%");
								if ( DotsConfig.TERMINATION ) break;
								currentTime = Calendar.getInstance();
								if (currentTime.after(endTime)) {
									DotsConfig.TERMINATION = true;
									break;
								}
							}
							/* Calculate the average usage of this sleepInterval */
							averageUsage = averageUsage / sleepInterval;
							DotsLogging.logMessage(sleepInterval/12 + " Minutes Average CPU Usage = " + averageUsage + "%");
						} catch (Exception e) {
							DotsLogging.logException("Dots.main: " + e);
						}


				} else {
					/* When CPU_TARGET is reached,DOTS will not create new thread until 
					   average usage is less than CPU_TARGET-10 */
					if (DotsConfig.AUTO_MODE && !reached) {
						reached = true;
						targetCpu = DotsConfig.CPU_TARGET - 10;
						DotsLogging.logMessage("CPU Target " + DotsConfig.CPU_TARGET + "% is achieved now.");
					}
					/* Wait 5 minutes then check create connection condition */
					try {
						averageUsage = 0;
						for (int i = 0; i < 60; i ++) {
							averageUsage += DotsConfig.CPU_USAGE;
							Thread.sleep(5000);
							if ((i % 12) == 0)
								DotsLogging.logMessage("Active Threads = " + DotsConfig.THRDGRP.activeCount()  + " Average CPU Usage = " + DotsConfig.CPU_USAGE + "%");
							if ( DotsConfig.TERMINATION ) break;
							currentTime = Calendar.getInstance();
							if (currentTime.after(endTime)) {
								DotsConfig.TERMINATION = true;
								break;
							}
						}
						averageUsage = averageUsage / 60;
						DotsLogging.logMessage("5 Minutes Average CPU Usage = " + averageUsage + "%");
					} catch (Exception e) {
						DotsLogging.logException("Dots.main: " + e);
					}
				}
			}
		}catch (Throwable t) {
			DotsLogging.logException("Dots.main: " + t);
			System.out.println("Dots.main:" + t);
		}
		
		/* DURATION is reached or TERMINATION is set by other threads */	
		DotsConfig.TERMINATION = true;
		DotsLogging.logMessage("Dots is Terminating ...");
		/* Clear temporary file used by testcase 6&7 */
		if ((TESTCASENAME.charAt(4) == '6') || (TESTCASENAME.charAt(4) == '7')) {
			DotsLogging.logMessage("Cleaning temporary file ...");
			
			if(!DotsConfig.RUN_AUTO){
				System.out.println("Cleaning temporary file ...");
			}
			try {
				for (int j = 0; j < 10; j ++) {
					File tmpFile = new File(lobFileName[j]);
					if (tmpFile.exists()) tmpFile.delete();
				}
			} catch (Exception e) {
				DotsLogging.logException("Clean temporary file error.");
			}
		}
		/* Write summary file and wait DB thread to exit */
		try {
			if(!DotsConfig.RUN_AUTO){
				System.out.println("\nWriting summary file ...");			
			}
			summary.writeSummaryFile();
			summary.canExit = true;
			DotsPerf.canExit = true;
			DotsLogging.logSummary("Waiting for the active threads to exit ......");
			if(!DotsConfig.RUN_AUTO){
				System.out.println("\nWaiting for the active threads to exit ......");
			}
			while (DotsConfig.THRDGRP.activeCount() > 0) {
				waitInterval ++;
				Thread.sleep(1000);
				if(!DotsConfig.RUN_AUTO){
					System.out.println("Current active thread number is " + DotsConfig.THRDGRP.activeCount());
				}
				if (waitInterval > 60) break;  //Max wait time is 1 minute
			}
		}catch (InterruptedException e) {
			DotsLogging.logException("Dots.main:" + e);
		}
		DotsLogging.logSummary("All Database Access Threads exit.");
		DotsLogging.close();
		if(!DotsConfig.RUN_AUTO){
			System.out.println("DOTS closed successfully.");
		}
		System.exit(0);
	}
}
