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
 *  This class is designed to randomly generate some data type for database 
 * operation. 
 */
 
 public class DotsGenerator {

	/** Random generator. */
	private static Random myRandom = new Random();
 	/**
 	 * Constructor
 	 */
	public DotsGenerator() {
		super();
	}
	
	/**
	 * Randomly generate a char of('a'-'z','A'-'Z','0'-'9') 
	 */
	public static char mdChar() {
		int ram;

		ram = myRandom.nextInt(62); //62 is total char number,62 = 10 + 26 * 2

		if (ram < 10) return (char)(ram + 48);//'0'-'9',ASCII '0' is 48
		if (ram > 35) return (char)(ram + 61);//'a'-'z',ASCII 'a' is 97
		return (char)(ram + 55);//'A'-'Z',ASCII 'A' is 65
	}

	/**
	 * Randomly generate a string,the string length is len.
	 * @param len:	string length.
	 */
	public static String mdString(int len) {
		StringBuffer bufString = new StringBuffer();
		int i;

		for(i = 0; i < len; i ++ )
			bufString = bufString.append(mdChar());
		return bufString.toString();
	}

	/**
	 * Randomly generate a integer between min(inclusive) and max(exclusive)
	 * @param min:	the minimum
	 * @param max:	the maximum
	 */
	public static int mdInt(int min,int max) {
		return (myRandom.nextInt(max-min) + min);
	}

	/**
	 * Randomly generate a integer
	 */
	public static int mdInt(){
		return myRandom.nextInt();
	}

	/**
	 * Randomly generate a Uid,format is "string"+"number"
	 * @param name: user id title
	 * @param minNumber: minimum of the following number
	 * @param maxNumber: maximum of the following number 
	 */
	public static String mdUserID(String name,int minNumber,int maxNumber){
		return name + mdInt(minNumber,maxNumber);
	}

	/**
	 * Randomly generate a string between 40(inclusive) and 60(exclusive) to simulate address
	 */
	public static String mdAddress(){
		return mdString(mdInt(40,60));
	}

	/**
	 * Randomly generate a string to simulate email
	 */
	public static String mdEmail(){
		StringBuffer myEmail = new StringBuffer();
		int	i;

		myEmail = myEmail.append(mdString(mdInt(4,10)));
		myEmail = myEmail.append("@");
		for (i = 0; i < mdInt(2,4); i ++){
			myEmail = myEmail.append(mdString(mdInt(2,5)));
			myEmail = myEmail.append(".");
		}
		myEmail = myEmail.deleteCharAt(myEmail.length() - 1);

		return myEmail.toString();	
	}

	/**
	 * Randomly generate a float to simulate price
	 */
	public static float mdPrice(){
		return (float)Math.abs(myRandom.nextInt()) / 1000;
	}

	/**
	 * Randomly generate a Clob File
	 * @param length: the file size(K)
	 */
	public static String mdClob(int length) {
		int 	currentSize = 0;
		byte[]	bytes = new byte[1024];
		Calendar	currentTime = Calendar.getInstance();
		StringBuffer fileName = new StringBuffer();

		/* Clob file name */
		fileName = fileName.append("clob").append(currentTime.get(Calendar.MONTH)).
			append(currentTime.get(Calendar.DAY_OF_MONTH)).append(currentTime.get(Calendar.HOUR))
			.append(currentTime.get(Calendar.MINUTE)).append(currentTime.get(Calendar.SECOND))
			.append(currentTime.get(Calendar.MILLISECOND));
		try {
		
			BufferedWriter clobWriter = new BufferedWriter(new FileWriter(fileName.toString(),false));
			while(currentSize < length)
			{
				for(int i = 0; i<512; i++)
				{
					clobWriter.write(mdChar());
				}
				currentSize = currentSize +1;
			}	
			clobWriter.flush();
			clobWriter.close();
		}
		catch(IOException e)
		{
			DotsLogging.logException("DotsGenerator.mdClob: " + e);
		}

		return fileName.toString();
	}
	
	/**
	 * Randomly generate a Blob File
	 * @param length: the file size(K)
	 */
	public static String mdBlob(int length) {
		int 	currentSize = 0;
		byte[]	bytes = new byte[1024];
		Calendar	currentTime = Calendar.getInstance();
		StringBuffer fileName = new StringBuffer();

		/* Blob file name */
		fileName = fileName.append("blob").append(currentTime.get(Calendar.MONTH))
			.append(currentTime.get(Calendar.DAY_OF_MONTH)).append(currentTime.get(Calendar.HOUR))
			.append(currentTime.get(Calendar.MINUTE)).append(currentTime.get(Calendar.SECOND))
			.append(currentTime.get(Calendar.MILLISECOND));
		try {
			BufferedOutputStream blobWriter = new BufferedOutputStream(new FileOutputStream(fileName.toString(),false));
			while(currentSize < length)
			{
				myRandom.nextBytes(bytes);
				blobWriter.write(bytes,0,1024);
				currentSize = currentSize +1;
			}	
			blobWriter.flush();
			blobWriter.close();
		} catch (IOException e) {
			DotsLogging.logException("DotsGenerator.mdBlob: " + e);
		}
		return fileName.toString();
	 }
}