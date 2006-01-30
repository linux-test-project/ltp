/*
 *   @(#)PerfConfig.java        1.0 2001/07/18
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

import java.lang.*;
/**
 * This class is designed to store performance data
 */

public class PerfConfig {

 	/** Database server linux kernel version and server CPU count string. */
	public static String VERSION;
	/** Database server stat string,include CPU,memory,diskIO,pagingin,pagingout. */
	public static String STATE;

	/** Socket port */
	public static int	 PORT;


}