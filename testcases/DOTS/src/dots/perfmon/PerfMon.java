/*
 *   @(#)PerfMon.java        1.0 2001/07/23
 *
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

package dots.perfmon;

import java.io.*;
import java.util.*;
import java.net.*;

/**
 * This class is the main class of Performance Monitor. It creates sockets to interact 
 * with client,and creates a thread to get performance data periodically.
 */
public class PerfMon {
 
	/**
	 * Constructor
	 */
	public PerfMon() {
		super();
	}

	/**
	 * Main function of Performance moniter.
	 * It read command line input and create server socket; 
	 * create a thread of PerfReader; creates a thread to interact with client.
	 */
	public static void main(String[] args) {

		/* Get command line input */
		if (args.length > 0) {
			for (int i = 0; i < args.length;) {
				String option = args[i++];

				if (option.equals("-port")) {
					if (i < args.length) {
						try {
							PerfConfig.PORT = (new Integer(args[i++])).intValue();
						} catch (Exception e) {
							System.out.println("Socket port number is not corrent!");
							System.exit(0);
						}
						break;
					}
				} 
				if (option.equals("-?") || option.equals("-help")) {
					System.out.println("Usage: PerfMon [-option]");
					System.out.println("\t\t-port			Socket port number");
					System.out.println("\t\t-help			Print this message");
					System.exit(0);
				}

				System.out.println("Excepted Option Is Not Found!");
				System.out.println("Usage: PerfMon -port [Socket port number]");
				System.exit(0);
			}
		} else {
			System.out.println("Excepted Option Is Not Found!");
			System.out.println("Usage: PerfMon -port [Socket port number]");
			System.exit(0);
		}

		/* Create PerfReader thread */
		PerfReader perf = new PerfReader();
		perf.start();

		/* Create server socket */
		ServerSocket server = null;
		Socket	socket = null;
		try {
			server = new ServerSocket(PerfConfig.PORT);
		} catch (Exception e) {
			System.out.println("PerfMon.Main: " + e);
			System.out.println("Cannot create server socket!");
			System.exit(1);
		}
		
		/* Wait client connection */
		while (true) {
			try {
				if (server != null)
					socket = server.accept();
				PerfConn conn = new PerfConn(socket);
				conn.start();
			} catch (Exception e) {
				System.out.println("PerfMon.Main: " + e);
			}
		}

	}
}