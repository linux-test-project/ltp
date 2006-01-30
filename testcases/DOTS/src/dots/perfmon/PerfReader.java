/*
 *   @(#)PerfReader.java        1.0 2001/07/18
 *
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
package dots.perfmon;

import java.io.*;
import java.util.*;

/**
 * This class is designed for the Performance Reader.
 * It reads performance data from /proc and save to PerfConfig.
 */
public class PerfReader extends Thread{

 	/** Stat file */
	private static File fileStat;
	/** Memory file */
	private static File fileMem;
	/** STATINFO file name */
	final private static String STATINFO = "/proc/stat";
	/** VERSION file name */
	final private static String VERSION = "/proc/version";
	/** MEMORY file name */
	final private static String MEMINFO = "/proc/meminfo";

	/** Store the current cpu time */
	private static long[][] cpuTime;
	/** Store the current diskIO */
	private static long	diskIO = 0;
	/** Store the current PageIn */
	private static long	pageIn = 0;
	/** Store the current PageOut */
	private static long	pageOut = 0;
	/** Store the cup count */
	public static int cpuCount = 0;

	/** added the following variables for number of columns in /proc/stat*/
	/** added on Feb 10th 2003 */
                BufferedReader myStatReader;
                String myStatString;
                StringTokenizer myStk;
                int gotColumnsCount = 0, columnsCount = 0;
	/** Feb 10th 2003 */

	
	/**
	 * Constructor
	 */
	public PerfReader() {

		try {
			/* Check stat file */
			fileStat = new File(STATINFO);
			if (!fileStat.canRead()) {
				System.out.println("The File " + STATINFO + " cannot be read!");
				System.exit(1);
			}
			/* Check meminfo file */
			fileMem = new File(MEMINFO);
			if (!fileMem.canRead()) {
				System.out.println("The File " + MEMINFO + " cannot be read!");
				System.exit(1);
			}

			/*  Get the number of columns for CPUs in /proc/stat */
			/*  Added on Feb 10th 2003 */
                        myStatReader = new BufferedReader(new FileReader(fileStat));
                        myStatString = myStatReader.readLine();
                        while ((myStatString != null) && (gotColumnsCount==0)) {
                                if (myStatString.startsWith("cpu ")) {
                                        /* Get average CPU usage */
                                        myStk = new StringTokenizer(myStatString);                                        gotColumnsCount=1;
                                        if (myStk.hasMoreTokens()) {
                                                myStatString = myStk.nextToken();
                                                columnsCount = 0;
                                                while (myStk.hasMoreTokens()) {
                                                        myStatString = myStk.nextToken();
                                                        columnsCount ++;
                                                        }
                                                }
                                        }
                                }	/* Feb 10th 2003 */
			gotColumnsCount = 0; /* Feb 10th 2003 */

			/* Get CPU count */
			BufferedReader bufReader = new BufferedReader(new FileReader(fileStat));
			String bufString = bufReader.readLine();
			while (bufString != null) {
				if (bufString.startsWith("cpu")) cpuCount++;
				bufString = bufReader.readLine();
			}
			if (cpuCount > 1) cpuCount --;
			/**cpuTime = new long[cpuCount + 1][4]; Feb 10th 2003 */
			cpuTime = new long[cpuCount + 1][columnsCount]; /* Feb  10th 2003 */
			bufReader.close();
		} catch (Exception e) {
			System.out.println("PerfMon.PerfReader: " + e);
			e.printStackTrace();
			System.exit(1);
		}

	}

	/**
	 * Get the linux version
	 */
	public static String getVersion() {
		File fileVersion = new File(VERSION);
		String verString = null;
		String version;
		
		try {
			if (!fileVersion.canRead()) {
				System.out.println("The File " + VERSION + " cannot be read");
				return " ";
			}
			/* Get linux kernel version */ 
			BufferedReader verReader = new BufferedReader(new FileReader(fileVersion));
			verString = verReader.readLine();
			if (verString.startsWith("Linux version")) {
				StringTokenizer stk = new StringTokenizer(verString);
				if (stk.hasMoreTokens()) {
					for (int i = 0; i < 3; i ++)
						verString = stk.nextToken();
				}
				return verString;
			}else {
				System.out.println("ERROR: Linux version information is not available.");
				return " ";
			}
		}catch (Exception e) {
			System.out.println("PerfReader.getVersion: " + e);
		}
		return " ";
	}
	/**
	 * Get the current memory info
	 */
	public static int getMemory() {
		String memString;
		long totalMemory,usedMemory;

		try {
			/* Get memory usage */
			BufferedReader memReader = new BufferedReader(new FileReader(MEMINFO));
			memString = memReader.readLine();
			while (memString != null) {
				if (memString.startsWith("Mem:")) {
					StringTokenizer stk = new StringTokenizer(memString);
					if (stk.hasMoreTokens()) {
						memString = stk.nextToken();
						totalMemory = Long.parseLong(stk.nextToken());
						usedMemory = Long.parseLong(stk.nextToken());
						usedMemory = usedMemory/(1024*1024);
						return (int)usedMemory;
					}
				}
				memString = memReader.readLine();
			}
			memReader.close();
		} catch (IOException e) {
			System.out.println("PerfMon.PerfReader:" + e);
		}
		return 0;
	}

	/**
	 * Get the cpu,diskIO,pagein,pageout from the /proc/stat file
	 */
	public void getStat() {
		BufferedReader statReader;
		String statString;
		StringTokenizer stk;
		int i = 1,j = 0;

		try {
			statReader = new BufferedReader(new FileReader(fileStat));
			statString = statReader.readLine();

			while (statString != null) {
				if (statString.startsWith("cpu ")) {
					/* Get average CPU usage */
					stk = new StringTokenizer(statString);
					if (stk.hasMoreTokens()) {
						statString = stk.nextToken();
						j = 0;
						while (stk.hasMoreTokens()) {
							statString = stk.nextToken();
							cpuTime[0][j] = Long.parseLong(statString);
							if (cpuCount <= 1)
								cpuTime[1][j] = cpuTime[0][j];
							j ++;
						}
					}
				}else if (statString.startsWith("cpu") && !statString.startsWith("cpu ") && (cpuCount > 1)) {
					/* Get CPU usage of per CPU */
					stk = new StringTokenizer(statString);
					if (stk.hasMoreTokens()) {
						statString = stk.nextToken();
						j = 0;
						while (stk.hasMoreTokens()) {
							statString = stk.nextToken();
							cpuTime[i][j++] = Long.parseLong(statString);
						}
						i ++;
					}
				}else if (statString.startsWith("disk ")) {
					/* Get diskIO of kernel 2.2 */
					stk = new StringTokenizer(statString);
					if (stk.hasMoreTokens()) {
						diskIO = 0;
						statString = stk.nextToken();
						while (stk.hasMoreTokens())
							diskIO += Long.parseLong(stk.nextToken());
					}
				} else if (statString.startsWith("page")) {
					/* Get Page In and Page Out */
					stk = new StringTokenizer(statString);
					if (stk.hasMoreTokens()) {
						statString = stk.nextToken();
						pageIn = Integer.parseInt(stk.nextToken());
						pageOut = Integer.parseInt(stk.nextToken());
					}
				}else if (statString.startsWith("disk_io:")) { //kernel >= 2.4
					/* Get diskIO */
					stk = new StringTokenizer(statString);
					diskIO = 0;
					if (stk.hasMoreTokens()) statString = stk.nextToken();
					while (stk.hasMoreTokens()) {
						diskIO += getDiskIO(stk.nextToken());
					}
				}
				statString = statReader.readLine();
			}
			statReader.close();
		}catch (IOException e) {
			System.out.println("PerfReader.getStat: " + e);
		}
		return;
	}

	/**
	 * Periodically generate the cpu,diskIO,Page,Mem info
	 */
	public void run() {
		long[][] origCpuTime = new long[cpuCount + 1][4];
		int[] deltaCpuTime = {0,0,0,0};
		int i,j,totalCpu,useCpu;
		int ioUsage,pageInUsage,pageOutUsage,memUsage;
		int[] cpuUsage = new int[cpuCount + 1];
		long origDiskIO = 0;
		long origPageIn = 0,origPageOut = 0;
		String statString;
		boolean firstRun = true;
			
		PerfConfig.VERSION = getVersion();
		while (true) {
			getStat();

			/* Calculate CPU utilization */	
			for (i = 0; i < cpuCount + 1; i ++){
				for (j = 0; j < 4; j ++) {
					deltaCpuTime[j] = (int)(cpuTime[i][j] - origCpuTime[i][j]);
					origCpuTime[i][j] = cpuTime[i][j];
				}
				totalCpu = deltaCpuTime[0] + deltaCpuTime[1] + deltaCpuTime[2] + deltaCpuTime[3];
				useCpu = totalCpu - deltaCpuTime[3];
				cpuUsage[i] = (int)(((float)useCpu) / ((float)totalCpu) * 100.0);
			}
			System.out.println();
			/* Calculate diskIO */
			ioUsage = (int)(diskIO - origDiskIO);
			origDiskIO = diskIO;

			/* Calculate Page In */
			pageInUsage = (int)(pageIn - origPageIn);
			origPageIn = pageIn;

			/* Calculate Page Out */
			pageOutUsage = (int)(pageOut - origPageOut);
			origPageOut = pageOut;

			/* Get memory usage */
			memUsage = getMemory();
			statString = "";
			
			if (firstRun) {
				/* reset performance data */
				firstRun = false;
				System.out.print("CPU=" + 0);
				statString += "0;";
				for (i = 1; i < cpuCount + 1; i ++) {
					System.out.print(" CPU" + i + "=" + 0);
					statString += "0;";
				}
				System.out.println();
				System.out.println(" DiskIO = " + 0 + " PageIN = " + 0 + " PageOut = " + 0 + " Memory = " + memUsage + "M");
				statString += memUsage + ";" + "0;0;0";
			} else {
				/* Disiplay performance data */
				System.out.print("CPU=" + cpuUsage[0]);
				statString += cpuUsage[0] + ";";
				for (i = 1; i < cpuCount + 1; i ++){
					System.out.print(" CPU" + i + "=" + cpuUsage[i]);
					statString += cpuUsage[i] + ";";
				}
				System.out.println();
				System.out.println("DiskIO = " + ioUsage + " PageIN = " + pageInUsage + " PageOut = " + pageOutUsage + " Memory = " + memUsage + "M");
				statString += memUsage + ";" + ioUsage + ";" + pageInUsage + ";" + pageOutUsage;
			}
			
			PerfConfig.STATE = statString;
			
			try {
				Thread.sleep(5000);
			}catch (Exception e) {
				System.out.println("PerfReader.run: " + e);
			}
		}
	}
	/**
	 * Parse the diskIO string,get the diskIO value
	 * @param diskString: diskIO string
	 */
	private long getDiskIO(String diskString) {
		StringTokenizer stk = new StringTokenizer(diskString,":");
		String dataString;

		/* Get diskIO of kernel 2.4 */
		if (!stk.hasMoreTokens()) return 0;
		dataString = stk.nextToken();
		while (stk.hasMoreTokens()) dataString = stk.nextToken();

		dataString = dataString.replace('(',' ');
		StringTokenizer stk2 = new StringTokenizer(dataString,",");
		if (stk2.hasMoreTokens()) {
			dataString = stk2.nextToken().trim();
			return Long.parseLong(dataString);
		} else {
			return 0;
		}
	}}
