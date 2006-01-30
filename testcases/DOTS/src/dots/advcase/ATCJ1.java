/*
 *   @(#)ATCJ1.java      
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

package dots.advcase;

import java.sql.*;
import dots.framework.*;

/**
 * This class is designed for advanced case ATCJ1. This case simulates new user registration,
 * updating registration information and user authentication.
 */

public class ATCJ1 implements Runnable {

	/** Database connection */
	private Connection conn;

	/** SQL for creating registry */
	private static final String createRegistrySQL =
		"INSERT INTO REGISTRY (USERID,PASSWD,ADDRESS,EMAIL,PHONE) VALUES (?, ?, ?, ?, ?)";

	/** SQL for login authentication */
	private static final String loginSQL =
		"SELECT PASSWD FROM REGISTRY WHERE USERID = ";

	/** SQL for getting row count of table REGISTRY */
	private static final String getRegRowsSQL = "SELECT COUNT(*) FROM REGISTRY";

	/** SQL for updating REGISTRY */
	private static final String updateRegistrySQL =
		"UPDATE REGISTRY SET PASSWD = ?,ADDRESS = ?,EMAIL = ?, PHONE = ? WHERE USERID = ";

	/** Row count of table REGISTRY */
	private static int regRows = 0;

/**
 * ATCJ1 constructor
 */
public ATCJ1(Connection con) {
	super();
	this.conn = con;
}

/**
 * Create new registry(Insert one record into table REGISRTY)
 * @param userID user ID
 * @param passwd password
 * @param address address
 * @param email email
 * @param phone phone number
 */
	 
public boolean createRegistry(String userID, String passwd, String address, String email, String phone) {
	
	PreparedStatement pstmt = null;
	boolean reVal = true;
	
	try {
		pstmt = conn.prepareStatement(createRegistrySQL);
	} catch (Exception e) {
		//If prepareStatement can not be created, this method will return false. 
		DotsLogging.logException("ATCJ1.createRegistry() Exception: " + e);
		reVal = false;
		DotsConfig.FAILEDCOUNT++;
		return reVal;
	} 
	
	try {
		pstmt.setString(1, userID);
		pstmt.setString(2, passwd);
		pstmt.setString(3, address);
		pstmt.setString(4, email);
		pstmt.setString(5, phone);
		pstmt.executeUpdate();
		pstmt.close();
		pstmt = null;
		if(DotsConfig.DRIVER_CLASS_NAME.indexOf("mysql") == -1)
			conn.commit();
		DotsConfig.INSERTCOUNT++;
	} catch (Exception e) {
		DotsLogging.logException("ATCJ1.createRegistry() Exception: " + e);
		DotsConfig.FAILEDCOUNT++;
	} 
	
	return reVal;
}

/**
 * User authentication
 * @param userID user ID
 * @param passwd password
 */
public boolean login(
	String userID,
	String passwd) {

	Statement stmt = null;
	ResultSet rs = null;
	boolean reVal = true;

	try
	{
		stmt = conn.createStatement();
	} catch (Exception e){
		DotsLogging.logException("ATCJ1.createRegistry() Exception: " + e);
		reVal = false;
		DotsConfig.FAILEDCOUNT++;
		return reVal;
	}
	try{
		rs = stmt.executeQuery(loginSQL + "'" + userID + "'");

		DotsConfig.QUERYCOUNT++;

		rs.close();
		rs = null;
		stmt.close();
		stmt = null;

	}
	catch (Exception e)
	{
		DotsLogging.logException("ATCJ1.login() Exception: " + e);
		DotsConfig.FAILEDCOUNT++;
	}
	

	return reVal;
}

/**
 * Update registry
 * @param userID user ID
 * @param passwd password
 * @param address address
 * @param email email
 * @param phone phone number
 */
public boolean updateRegistry(
	String userID,
	String passwd,
	String address,
	String email,
	String phone) {

	PreparedStatement pstmt = null;
	boolean reVal = true;
	ResultSet rs = null;
	int count =0;

	try
	{
		pstmt = conn.prepareStatement(updateRegistrySQL+ "'" + userID + "'");
	} catch (Exception e){
		DotsLogging.logException("ATCJ1.createRegistry() Exception: " + e);
		reVal = false;
		DotsConfig.FAILEDCOUNT++;
		return reVal;
	}
	try{
		pstmt.setString(1,passwd);
		pstmt.setString(2,address);
		pstmt.setString(3,email);
		pstmt.setString(4,phone);
		count = pstmt.executeUpdate();
		pstmt.close();
		pstmt = null;
		if(DotsConfig.DRIVER_CLASS_NAME.indexOf("mysql") == -1)
			conn.commit();

		DotsConfig.UPDATECOUNT++;


	}
	catch (Exception e)
	{
		DotsLogging.logException("ATCJ1.updateRegistry() Exception: " + e);
		DotsConfig.FAILEDCOUNT++;
	}

	return reVal;
}


/**
 * Set row count of table REGISTRY
 */
public boolean setRegRows() {

	Statement stmt = null;
	ResultSet rs = null;
	boolean reVal = true;

	try {
		stmt = conn.createStatement(ResultSet.TYPE_SCROLL_INSENSITIVE,ResultSet.CONCUR_READ_ONLY);
	} catch (Exception e){
		DotsLogging.logException("ATCJ1.createRegistry() Exception: " + e);
		reVal = false;
		DotsConfig.FAILEDCOUNT++;
		return reVal;
	}
	try{
		rs = stmt.executeQuery(getRegRowsSQL);

		DotsConfig.QUERYCOUNT++;

		if (rs.next()) {
			regRows = rs.getInt(1);
			//reVal = true;
		}else {
			DotsLogging.logException("Table registry is empty");
		}
		rs.close();
		rs = null;
		stmt.close();
		stmt = null;

	}catch(Exception e) {
		DotsLogging.logException("ATCJ1.setRegRows() Exception: " +e);
		DotsConfig.FAILEDCOUNT++;

	}
	
	return reVal;

}


/**
 * run() method of the thread
 */
public void run() {
	int action;
	boolean succeed = true;
	String userID;
	String passwd = "password";
	
	try {
		if (regRows == 0) {
			succeed = setRegRows();
			if (!succeed)
				return;
			DotsLogging.logMessage("Registry table has " + regRows + " rows");
			if (regRows == 0)
				for (int i = 100; i > 0; i--) {
					regRows++;
					userID = "UID" + regRows;
					succeed = createRegistry(userID, passwd, DotsGenerator.mdAddress(), DotsGenerator.mdEmail(), DotsGenerator.mdString(10)); 
					if (!succeed)
						return;
				}
		}
		do {
			action = DotsGenerator.mdInt(1, 4);
			switch (action) {
				case 1 :
					//If rows in the table REGISTRY are less than DotsConfig.MAX_ROWS,
					//insert a new record to REGISTRY; otherwise do nothing.
					if (regRows++ < DotsConfig.MAX_ROWS) {
						userID = "UID" + regRows;
						succeed = createRegistry(userID, passwd, DotsGenerator.mdAddress(),DotsGenerator.mdEmail(), DotsGenerator.mdString(10)); 
						if (!succeed)
							return;
					}
					break;
				case 2 :
					//Update record in table REGISTRY according to userID.
					userID = DotsGenerator.mdUserID("UID", 1, regRows);
					succeed = updateRegistry(userID, passwd, DotsGenerator.mdAddress(), DotsGenerator.mdEmail(), DotsGenerator.mdString(10)); 
					if (!succeed)
						return;
					break;
				case 3 :
					userID = DotsGenerator.mdUserID("UID", 1, regRows);
					succeed = login(userID, passwd);
					if (!succeed)
						return;
					break;
			}
		} while (!DotsConfig.TERMINATION);
		
	} catch (Throwable t) {
		DotsLogging.logException("ATCJ1.run() Exception: " + t);
		return;
	}
}


}