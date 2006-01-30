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

import java.util.*;

/**
 * This class is designed to write summary to summary file
 */

public class DotsSummary extends Thread {

	/** Exit flag. */
	public static boolean canExit = false;
	/** Testcase start time. */
	public static long startTime;
	/**
	 * Constructor
	 */
	public DotsSummary() {
		super();
	}
	/**
	 * Write summary to summary file.
	 */
	public static void writeSummaryFile() {
		StringBuffer bufString;
		long currentTime;
		int duration;
			
		currentTime = System.currentTimeMillis();
		duration = (int)((currentTime - startTime) / 60000);
		bufString = new StringBuffer();

		DotsPerf.writePerfSummaryString();
		if (!DotsConfig.TERMINATION)
			DotsPerf.resetIntervalData();
		/* Construct summary string and write this string to file */
		bufString.append("\n\t<Total Execution Time>                  " + duration / 60 + " hours " + duration % 60 + " minutes.\n");
		bufString.append("\n\t<Current Concurrent DB Connections>     " + DotsConfig.THRDGRP.activeCount() + ".\n");
		bufString.append("\n\t<JDBC APIs>   Total number of QUERY     " + DotsConfig.QUERYCOUNT + ".\n");
		bufString.append("\t              Total number of UPDATE    " + DotsConfig.UPDATECOUNT + ".\n");
		bufString.append("\t              Total number of INSERT    " + DotsConfig.INSERTCOUNT + ".\n");
		bufString.append("\t              Total number of DELETE    " + DotsConfig.DELETECOUNT + ".\n");
		bufString.append("\t              Total number of FAILED    " + DotsConfig.FAILEDCOUNT + ".\n\n");
		bufString.append(DotsConfig.PERF_SUMMARY);
		DotsLogging.logMessage("Writing summary to Summary File...");
		DotsLogging.logSummary(bufString.toString());

		return;
	}
	/**
	 * Timely write summary string to the summary file
	 */
	public void run() {
			
		while (!DotsConfig.TERMINATION) {
			try {
				/* Write summary per SUMMARY_INTERVAL */
				sleep(DotsConfig.SUMMARY_INTERVAL * 1000 * 60);
				writeSummaryFile();
			} catch (InterruptedException e) {
				DotsLogging.logException("DotsSummary.run: " + e);
			}
		}
		while (!canExit) {
			try {
				sleep(100);
			} catch (Exception e) {}
		}
	}
}