/*
 *   Copyright (c) International Business Machines  Corp., 2000
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
 * This class writes all the log, error and summary information to the correspondent files.
 */
public class DotsLogging {

 	/** Error file. */
	private static File	errorFile;
	/** Log file. */
	private static File logFile;
	/** Summary file. */
	private static File summaryFile;

	/** Log file writer. */
	private static PrintWriter writeLog;
	/** Error file writer. */
	private static PrintWriter writeError;
	/** Summary file writer. */
	private static PrintWriter writeSummary;

	/**
	 * Constructor.
	 * Create the corresponding file and file writer.
	 */
	public DotsLogging(String testcase) {
		String errorFileName;
		String logFileName;
		String summaryFileName;
		StringBuffer bufString = new StringBuffer();
		String	timeStamp;
		String	filePath = DotsConfig.LOG_DIR;
		Calendar	startTime = Calendar.getInstance();
		File	dir;

		/* Create log file */
		bufString.append(startTime.get(Calendar.YEAR)).append("-").
				append(startTime.get(Calendar.MONTH)+1).append("-").
				append(startTime.get(Calendar.DAY_OF_MONTH)).append("-").
				append(startTime.get(Calendar.HOUR_OF_DAY)).append("-").
				append(startTime.get(Calendar.MINUTE)).append("-").
				append(startTime.get(Calendar.SECOND)).append("-").
				append(startTime.get(Calendar.MILLISECOND));
		timeStamp = bufString.toString();

		if (filePath.charAt(filePath.length() - 1) != File.separatorChar)
			filePath = filePath + File.separator;
			
		dir = new File(filePath);
		try {
			if (!dir.exists())
				dir.mkdirs();				
		} catch (Exception e) {
			System.out.println("Exception: " + e);
			System.exit(1);
		}
		errorFileName = filePath + testcase + "-" + timeStamp + ".err";
		logFileName = filePath + testcase + "-" + timeStamp + ".log";
		summaryFileName = filePath + testcase + "-" + timeStamp + ".sum";
	
		errorFile = new File(errorFileName);
		logFile = new File(logFileName);
		summaryFile = new File(summaryFileName);

		/* Create file writer */
		try {
			writeLog = new PrintWriter(new FileWriter(logFile),true);
			writeError = new PrintWriter(new FileWriter(errorFile),true);
			writeSummary = new PrintWriter(new FileWriter(summaryFile),true);
		}catch (IOException e) {
			System.out.println("Create Log Files Error: " + e);
			e.printStackTrace();
			System.exit(1);
		}
	}
	
	/**
	 * Get timestamp.
	 */
	private static String getTimeStamp() {
		Calendar time = Calendar.getInstance();
		return  time.getTime().toString();	
	}
	/**
	 * Write message to Log file
	 * @message:	message content
	 */
	public static void logMessage(String message) {
		try {
			if (logFile.length() < DotsConfig.MAX_LOGFILESIZE)
				writeLog.println("[" + getTimeStamp() + "] "+ message);
			else {
				System.out.println("Log File Size Out Of Range!");
				DotsConfig.TERMINATION = true;
			}
		} catch (Exception e) {
			System.out.println("Some error occured when access Log File!");
			DotsConfig.TERMINATION = true;
		}
		return;
	}

	/**
	 * Writer exception to Error file
	 * @exception:	exception content
	 */
	public static void logException(String exception) {
		try {
			if (errorFile.length() < DotsConfig.MAX_LOGFILESIZE)
				writeError.println("[" + getTimeStamp() + "] " + exception);
			else {
				System.out.println("Error File Size Out Of Range!");
				DotsConfig.TERMINATION = true;
			}
		} catch (Exception e) {
			System.out.println("Some error occured when access Error File!");
			DotsConfig.TERMINATION = true;
		}
		return;
	}

	/**
	 * Write summary to Summary file.
	 * @summary:	summary content
	 */
	public static void logSummary(String summary) {
		try {
			if (summaryFile.length() < DotsConfig.MAX_LOGFILESIZE )
				writeSummary.println("[" + getTimeStamp() + "] " + summary);
			else {
				System.out.println("Summary File Size Out of Range!");
				DotsConfig.TERMINATION = true;
			}
		} catch (Exception e) {
			System.out.println("Some error occured when access Summary File!");
			DotsConfig.TERMINATION = true;
		}
		return;
	}

	/**
	 * Close the corresponding file writer and file.
	 */
	public static void close() {

		writeSummary.close();
		writeError.close();
		writeLog.close();

		errorFile = null;
		logFile = null;
		summaryFile = null;
	}
}