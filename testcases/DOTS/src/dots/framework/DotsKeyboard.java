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

/**
 * This class is keyboard listener.It reads keyboard input and acts accordingly.
 */
public class DotsKeyboard extends Thread{

	/**
	 * Constructor 
	 */
	public DotsKeyboard() {
		super();
	}

	/**
	 * Process the command line input,
	 *			STOP 		-stop the test case
	 *			HELP or ? 	-Print help message
	 */
	public void run() {

		String cmdInput;
		BufferedReader bufReader = new BufferedReader(new InputStreamReader(System.in));

		try {
			while (!DotsConfig.TERMINATION){
				System.out.print("DOTS> ");
				cmdInput = bufReader.readLine();
				/* Check keyboard input */
				if (cmdInput.equalsIgnoreCase("stop")) {
					System.out.println("Are you sure to quit DOTS?(Y=yes,N=no)");
					cmdInput = bufReader.readLine();
					if (cmdInput.equalsIgnoreCase("y")) {
						DotsLogging.logMessage("The execution of Test Case is stopped by user (pressing STOP).\n");
						System.out.println("DOTS is closing. Please wait ... ");
						DotsConfig.TERMINATION = true;
						break;
					}
				} else if (cmdInput.equalsIgnoreCase("help") || cmdInput.equalsIgnoreCase("?")) {
					System.out.println("\t\tSTOP		-Stop the execution of Test Case");
					System.out.println("\t\tHELP or ?	-Print this message");
				}
			}
		}
		catch (IOException e) {
			DotsLogging.logException("DotsKeyboard.run:" + e);
		}finally {
			DotsConfig.TERMINATION = true;
		}
	}
}