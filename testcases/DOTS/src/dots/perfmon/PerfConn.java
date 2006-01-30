/*
 *   @(#)PerfConn.java        1.0 2001/07/18
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

import java.net.*;
import java.io.*;

/**
 * This class is designed for the Performance Connection. 
 * It response client request and send performance data to client.
 */
public class PerfConn extends Thread{

 	/** Connect socket */
	private static Socket socket;
	/** Socket Input Stream */
	private static InputStream in;
	/** Socket Output Stream */
	private static OutputStream out;
	
	/**
	 * Constructor
	 * @param socket1: The server socket
	 */
	public PerfConn(Socket socket1) {
		socket = socket1;
		try {
			in = socket.getInputStream();
			out = socket.getOutputStream();
		} catch (IOException e) {
			System.out.println("PerfConn.PerfConn: " + e);
			e.printStackTrace();
			System.exit(1);
		}
	}
	/**
	 * Act according the socket client input 
	 */
	public void run() {
		BufferedReader reader = new BufferedReader(new InputStreamReader(in));
		PrintWriter writer = new PrintWriter(new OutputStreamWriter(out),true);
		String clientRequest;

		while (true) {
			try {
				/* Handle client request */
				clientRequest = reader.readLine();
				if (clientRequest == null) break;
				if (clientRequest.startsWith("STATE")) {
					writer.println(PerfConfig.STATE);
				}
				else if (clientRequest.startsWith("INIT"))
					writer.println(PerfConfig.VERSION + ";" + PerfReader.cpuCount);
			} catch (IOException e) {
				System.out.println("PerfConn.run: " + e);
			}
		}
	}
}