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
import java.net.*;
import java.util.*;

/**
 * This class is Performance Client.
 * It firstly establishes a connection with Performance Monitor via socket.
 * Then it requests performance data from Performance Monitor,
 * and save the performance data to DotsConfig member PERF_SUMMARY and CPU_USAGE.
 */
public class DotsPerf extends Thread{

	/** Client Socket. */
	private static Socket client;
	/** Socket input stream. */
	private static InputStream in;
	/** Socket output stream. */
	private static OutputStream out;
	/** Server CPU Count. */
	public static int cpuCount = 1;
	/** Exit flag. */
	public static boolean canExit = false;
	
	/** Timeslice count of the summary interval. */
	private static int timeslice = 0;

	/** Total timeslice count. */
	private static int totalTimeslice = 0;

	/** Peak cpu utilization. */
	private static int[] peakCpu;
	/** Peak cpu utilization within the summary interval. */
	private static int[] intervalPeakCpu;
	/** Summation of total cpu utilizaton. */
	private static int[] totalCpu;
	/** Summation of interval cpu utilization. */
	private static int[] intervalTotalCpu;

	/** Peak memory usage. */
	private static int peakMem = 0;
	/** Peak memory usage within the summary interval. */
	private static int intervalPeakMem = 0;
	/** Summation of total memory utilization. */
	private static long totalMem = 0;
	/** Summation of interval memory utilization. */
	private static int intervalTotalMem = 0;

	/** Peak diskIO count. */
	private static int peakIO = 0;
	/** Peak diskIO count within the summary interval. */
	private static int intervalPeakIO = 0;
	/** Summation of total diskIO count. */
	private static long totalIO = 0;
	/** Summation of interval diskIO count. */
	private static int intervalTotalIO = 0;

	/** Peak PageIn count. */
	private static int peakPageIn = 0;
	/** Peak PageIn count within the summary interval. */
	private static int intervalPeakPageIn = 0;
	/** Summation of total PageIn count. */
	private static long totalPageIn = 0;
	/** Summation of interval PageOut count. */
	private static int intervalTotalPageIn = 0;

	/** Peak PageOut count. */
	private static int peakPageOut = 0;
	/** Peak PageOut count within the summary interval. */
	private static int intervalPeakPageOut = 0;
	/** Summation of total PageOut count. */
	private static long totalPageOut = 0;
	/** Summation of interval PageOut count. */
	private static int intervalTotalPageOut = 0;

 	/**
 	 * Constructor
 	 */
	public DotsPerf() {

		try {
			client = new Socket(DotsConfig.SERVER_IP,Integer.parseInt(DotsConfig.PERF_SVR_PORT));

			in = client.getInputStream();
			out = client.getOutputStream();

			DotsLogging.logMessage("Starting Performance Monitor client ...OK ");
			DotsLogging.logMessage("Client Socket: " + client);

		} catch (Exception e) {
			DotsLogging.logException("DotsPerf.DotsPerf: " + e);
			e.printStackTrace();
			DotsLogging.logMessage("Failed to start Performance Monitor client.");
			System.out.println("Performance Monitor client failed to start. Please check if Performance Monitor server is started. Refer to error log for detail.");
			System.exit(1);
		}

	}
	/**
	 * Reset the performance data value
	 */
	public static void resetIntervalData() {
		timeslice = 0;

		for (int i = 0; i < cpuCount + 1; i ++) {
			intervalPeakCpu[i] = 0;
			intervalTotalCpu[i] = 0;
		}

		intervalPeakMem = 0;
		intervalTotalMem = 0;

		intervalPeakIO = 0;
		intervalTotalIO = 0;

		intervalPeakPageIn = 0;
		intervalTotalPageIn = 0;

		intervalPeakPageOut = 0;
		intervalTotalPageOut = 0;

		return;
	}
	/**
	 * Handle the performance data
	 * @param response: the server performance data string
	 */
	private static void handlePerfData(String response) {
		String tmpString;
		int tmpInteger;
		StringTokenizer stk = new StringTokenizer(response,";");

		timeslice ++;
		totalTimeslice ++;

		/* Average CPU Usage */
		if (stk.hasMoreTokens()) {
			tmpString = stk.nextToken();
			tmpInteger = Integer.parseInt(tmpString);
			DotsConfig.CPU_USAGE = tmpInteger;
			if (tmpInteger > intervalPeakCpu[0])	intervalPeakCpu[0] = tmpInteger;
			if (tmpInteger > peakCpu[0])	peakCpu[0] = tmpInteger;
			intervalTotalCpu[0] += tmpInteger;
			totalCpu[0] += tmpInteger;
		}

		/* CPU Usage of per CPU */
		for (int i = 1; i < cpuCount + 1; i ++) {
			if (stk.hasMoreTokens()) {
				tmpString = stk.nextToken();
				tmpInteger = Integer.parseInt(tmpString);
				if (tmpInteger > intervalPeakCpu[i])	intervalPeakCpu[i] = tmpInteger;
				if (tmpInteger > peakCpu[i])	peakCpu[i] = tmpInteger;
				intervalTotalCpu[i] += tmpInteger;
				totalCpu[i] += tmpInteger;
			}
		}
		/* Memory Usage */
		if (stk.hasMoreTokens()) {
			tmpString = stk.nextToken();
			tmpInteger = Integer.parseInt(tmpString);
			if (tmpInteger > intervalPeakMem)	intervalPeakMem = tmpInteger;;
			if (tmpInteger > peakMem)	peakMem = tmpInteger;
			intervalTotalMem += tmpInteger;
			totalMem += tmpInteger;
		}
		/* Disk IO */
		if (stk.hasMoreTokens()) {
			tmpString = stk.nextToken();
			tmpInteger = Integer.parseInt(tmpString);
			if (tmpInteger > intervalPeakIO)	intervalPeakIO = tmpInteger;;
			if (tmpInteger > peakIO)	peakIO = tmpInteger;
			intervalTotalIO += tmpInteger;
			totalIO += tmpInteger;
		}
		/* Page In */
		if (stk.hasMoreTokens()) {
			tmpString = stk.nextToken();
			tmpInteger = Integer.parseInt(tmpString);
			if (tmpInteger > intervalPeakPageIn)	intervalPeakPageIn = tmpInteger;
			if (tmpInteger > peakPageIn)	peakPageIn = tmpInteger;
			intervalTotalPageIn += tmpInteger;
			totalPageIn += tmpInteger;
		}
		/* Page Out */
		if (stk.hasMoreTokens()) {
			tmpString = stk.nextToken();
			tmpInteger = Integer.parseInt(tmpString);
			if (tmpInteger > intervalPeakPageOut)	intervalPeakPageOut = tmpInteger;
			if (tmpInteger > peakPageOut)	peakPageOut = tmpInteger;
			intervalTotalPageOut += tmpInteger;
			totalPageOut += tmpInteger;
		}
		return;
	}

	/**
	 * Calculate the summary data and merge the data into the PERF_SUMMARY string
	 */
	public static void writePerfSummaryString() {
		if (timeslice <= 0) {
			DotsConfig.PERF_SUMMARY = "";
			return;
		}
		/* Construct summary string */
		StringBuffer	summary = new StringBuffer();
		summary.append("\t<Average CPU> Peak of this Interval:    " + intervalPeakCpu[0] + "%\n");
		summary.append("\t              Average of this Interval: " + intervalTotalCpu[0] / timeslice + "%\n");
		summary.append("\t              Peak of all:              " + peakCpu[0] + "%\n");
		summary.append("\t              Average of all:           " + totalCpu[0] / totalTimeslice + "%\n\n");
		for (int i = 1; i < cpuCount + 1; i ++) {
			summary.append("\t<CPU" + (i - 1) + ">        Peak of this Interval:    " + intervalPeakCpu[i] + "%\n");
			summary.append("\t              Average of this Interval: " + intervalTotalCpu[i] / timeslice + "%\n");
			summary.append("\t              Peak of all:              " + peakCpu[i] + "%\n");
			summary.append("\t              Average of all:           " + totalCpu[i] / totalTimeslice + "%\n\n");
		}
		summary.append("\t<Memory>      Peak of this Interval:    " + intervalPeakMem + "M\n");
		summary.append("\t              Average of this Interval: " + intervalTotalMem / timeslice + "M\n");
		summary.append("\t              Peak of all:              " + peakMem + "M\n");
		summary.append("\t              Average of all:           " + totalMem / totalTimeslice + "M\n\n");
		summary.append("\t<Disk IO>     Peak of this Interval:    " + intervalPeakIO / 5 + "/s\n");
		summary.append("\t              Average of this Interval: " + intervalTotalIO / (timeslice * 5) + "/s\n");
		summary.append("\t              Peak of all:              " + peakIO / 5 + "/s\n");
		summary.append("\t              Average of all:           " + totalIO / (totalTimeslice * 5) + "/s\n\n");
		summary.append("\t<Page In>     Peak of this Interval:    " + intervalPeakPageIn / 5 + "/s\n");
		summary.append("\t              Average of this Interval: " + intervalTotalPageIn / (timeslice * 5) + "/s\n");
		summary.append("\t              Peak of all:              " + peakPageIn / 5 + "/s\n");
		summary.append("\t              Average of all:           " + totalPageIn / (totalTimeslice * 5) + "/s\n\n");
		summary.append("\t<Page Out>    Peak of this Interval:    " + intervalPeakPageOut / 5 + "/s\n");
		summary.append("\t              Average of this Interval: " + intervalTotalPageOut / (timeslice * 5) + "/s\n");
		summary.append("\t              Peak of all:              " + peakPageOut / 5 + "/s\n");
		summary.append("\t              Average of all:           " + totalPageOut / (totalTimeslice * 5) + "/s\n\n");

		DotsConfig.PERF_SUMMARY = summary.toString();
		return;
	}

	/**
	 * It requests performance data from Performance Monitor and
	 * save the performance data to DotsConfig member PERF_SUMMARY and CPU_USAGE
	 */
	public void run(){
		BufferedReader reader;
		PrintWriter writer;
		String	response;

		try {
			reader = new BufferedReader(new InputStreamReader(in));
			writer = new PrintWriter(new OutputStreamWriter(out),true);

			/* Init and get DB server information */
			writer.println("INIT");
			response = reader.readLine();
			StringTokenizer stk = new StringTokenizer(response,";");
			if (stk.hasMoreTokens()) {
				DotsLogging.logSummary("The Linux Kernel Version is " + stk.nextToken());
				cpuCount = Integer.parseInt(stk.nextToken());
			}
			DotsLogging.logSummary("The Database Server's CPU Count is " + cpuCount);
			/* Create CPU usage array */
			intervalPeakCpu = new int[cpuCount + 1];
			intervalTotalCpu = new int[cpuCount + 1];
			peakCpu = new int[cpuCount + 1];
			totalCpu = new int[cpuCount + 1];

			/* Get performance data per 5 second */
			while (!DotsConfig.TERMINATION) {
				writer.println("STATE");
				response = reader.readLine();
				handlePerfData(response);
				if (DotsConfig.TERMINATION) break;
				Thread.sleep(5000);
			}
			DotsLogging.logMessage("The Client Socket is closed!");
			client.close();
			while (!canExit){
				Thread.sleep(100);
			}
		}catch (IOException e) {
			DotsLogging.logException("DotsPerf.run: " + e);
			System.out.println("Performance Monitor error! Refer to error log for detail.");
		}catch (Exception e) {
			DotsLogging.logException("DotsPerf.run: " + e);
			System.out.println("Performance Monitor error! Refer to error log for detail.");
		}finally {
			DotsConfig.TERMINATION = true;
		}
	}
}