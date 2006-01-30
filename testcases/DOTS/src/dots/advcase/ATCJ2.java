/*
 *   @(#)ATCJ2.java        
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

import java.util.*;
import java.sql.*;
import dots.framework.*;

/**
 * This class is designed for advanced case ATCJ2. This case simulates auction activities.
 */
public class ATCJ2 implements Runnable {

	/** Database connection */
	private java.sql.Connection conn;

	/** Row count of BID table */
	private static int bidRows = 0;

	/** Row count of ITEM table */
	private static int itemRows =0 ;

	/** Row count of REGISTRY table */
	private static int regRows = 0;

	/** If bidRows,itemRows and regRows are set */
	private static boolean ifRowsSet = false;
	
	/** SQL for adding new items to ITEM table */
	private final static String addNewItemSQL =
		"INSERT INTO ITEM (ITEMID,SELLERID,DESCRIPTION,BID_PRICE,START_TIME,END_TIME,BID_COUNT) VALUES (?, ?, ? ,?, ?, ?, ?)";	
	
	/** SQL for creating new bid for one item */
	private final static String createBidSQL =
		"INSERT INTO BID (ITEMID,BIDERID,BID_PRICE,BID_TIME) VALUES (?, ?, ?, ?)";	
		
	/** SQL for getting bid history */
	private final static String getBidHistorySQL =
		"SELECT BIDERID, BID_PRICE, BID_TIME FROM BID WHERE ITEMID =  " ;	
		
	/** SQL for get the bid related columns from ITEM table */
	private final static String getBidItemSQL =
		"SELECT BID_PRICE,BID_COUNT FROM ITEM WHERE ITEMID = ";	
		
	/** SQL for getting row count of BID table */
	private final static java.lang.String getBidRowsSQL = "SELECT COUNT(*) FROM BID";	
	
	/** SQL for getting row count of ITEM table */
	private final static java.lang.String getItemRowsSQL = "SELECT COUNT(*) FROM ITEM";	
	
	/** SQL for getting the details of one item */
	private final static String getItemSQL =
		"SELECT SELLERID,DESCRIPTION,BID_PRICE,START_TIME,END_TIME,BID_COUNT FROM ITEM WHERE ITEMID = ";	
	
	/** SQL for getting row count of REGISTRY table */
	private final static java.lang.String getRegRowsSQL = "SELECT COUNT(*) FROM REGISTRY";	
	
	/** SQL for updating bid related columns in ITEM table */
	private final static String updateBidItemSQL =
		"UPDATE ITEM SET BID_PRICE = ?,BID_COUNT=? WHERE ITEMID = ";	
	
	/** SQL for updating ITEM table */
	private final static String updateItemSQL =
		"UPDATE ITEM SET DESCRIPTION = ?,BID_PRICE = ?,START_TIME = ?,END_TIME = ? WHERE ITEMID = ";	

/**
 * ATCJ1 constructor
 */
public ATCJ2(Connection con) {

	super();
	this.conn = con;
	//auction = new Auction(con);

}

/**
 * run() method for the thread
 */
public void run() {
	int action;
	boolean succeed = true;
	String passwd = "password";
	String userID;
	try {
		if (!ifRowsSet) {
			regRows = getRows("registry");
			bidRows = getRows("bid");
			itemRows = getRows("item");
			if ((regRows == -1) || (bidRows == -1) || (itemRows == -1))
				return;
			if (regRows == 0) {
				ATCJ1 atcj1 = new ATCJ1(conn);
				for (int i = 1000; i > 0; i--) {
					regRows++;
					userID = "UID" + regRows;
					succeed = atcj1.createRegistry(userID, passwd, DotsGenerator.mdAddress(), DotsGenerator.mdEmail(), DotsGenerator.mdString(10)); 
					if (!succeed)
						return;
				}
				atcj1 = null;
			}
			if (itemRows == 0) {
				for (int i = 100; i > 0; i--) {
					itemRows++;
					succeed = doPut();
					if (!succeed)
						return;
				}
			}
			if (bidRows == 0) {
				for (int i = 100; i > 0; i--) {
					bidRows++;
					succeed = doBid();
					if (!succeed)
						return;
				}
			}
			ifRowsSet = true;
		}
		do {
			action = DotsGenerator.mdInt(1, 6);
			switch (action) {
				case 1 :
					if (bidRows++ < DotsConfig.MAX_ROWS)
						succeed = doBid();
					if (!succeed)
						return;
					break;
				case 2 :
					if (itemRows++ < DotsConfig.MAX_ROWS)
						succeed = doPut();
					if (!succeed)
						return;
					break;
				case 3 :
					succeed = doViewItem();
					if (!succeed)
						return;
					break;
				case 4 :
					succeed = doUpdateItem();
					if (!succeed)
						return;
					break;
				case 5 :
					succeed = getBidHistory(DotsGenerator.mdUserID("ITEM", 1, itemRows));
					if (!succeed)
						return;
					break;
			}
			System.gc();
		} while (!DotsConfig.TERMINATION);
		
	} catch (Throwable t) {
		DotsLogging.logException("ATCJ2.run() Exception: " + t);
		return;
	}
}

/**
 * Create bid data and bid
 */
public boolean doBid() {
	boolean reVal = true;

	String bidderID = DotsGenerator.mdUserID("UID",1,regRows);
	float bidPrice = DotsGenerator.mdPrice();
	java.sql.Date   bidTime = new java.sql.Date(System.currentTimeMillis());
	String itemID = DotsGenerator.mdUserID("ITEM",1,itemRows);
	float currentBidPrice;
	
	Statement stmt = null;
	PreparedStatement pstmt = null;
	ResultSet rs = null;
			
	int bidCount;
	int updateCount;

	try
	{
		stmt = conn.createStatement(ResultSet.TYPE_SCROLL_INSENSITIVE,ResultSet.CONCUR_READ_ONLY);
	} catch (Exception e){
		DotsLogging.logException("ATCJ1.createRegistry() Exception: " + e);
		reVal = false;
		DotsConfig.FAILEDCOUNT++;
		return reVal;
	}
	
	try{  
		rs = stmt.executeQuery(getBidItemSQL + "'" + itemID + "'");

		if (rs.next()) {
			currentBidPrice = rs.getFloat(1);
			bidCount = rs.getInt(2);
			rs.close();
			rs = null;
			stmt.close();
			stmt = null;
			if (currentBidPrice < bidPrice) {
				pstmt = conn.prepareStatement(updateBidItemSQL + "'" + itemID + "'");
				pstmt.setFloat(1,bidPrice);
				pstmt.setInt(2,bidCount++);
				updateCount = pstmt.executeUpdate();
				DotsConfig.UPDATECOUNT++;
				
				if (updateCount > 0) {

					pstmt = conn.prepareStatement(createBidSQL);
					pstmt.setString(1, itemID);
					pstmt.setString(2,bidderID);
					pstmt.setFloat(3,bidPrice);
					pstmt.setDate(4,bidTime);
					updateCount = pstmt.executeUpdate();

					DotsConfig.INSERTCOUNT++;
					pstmt.close();
					pstmt = null;

					if (updateCount > 0) {
						if(DotsConfig.DRIVER_CLASS_NAME.indexOf("mysql") == -1)
							conn.commit();
						//reVal = true;
					}
					else {
						if(DotsConfig.DRIVER_CLASS_NAME.indexOf("mysql") == -1)
							conn.rollback();
					}
				}
			}
		}


	}
	catch (Exception e)
	{
		DotsLogging.logException("Auction.bid() Exception: " + e);
		//reVal = false;

		DotsConfig.FAILEDCOUNT++;

		try {
			if(DotsConfig.DRIVER_CLASS_NAME.indexOf("mysql") == -1)
				conn.rollback();
		} catch (SQLException se) {
			DotsLogging.logException("Auction.bid() SQLException: " + se);
		}

	}
	

	return reVal;
}
	
/**
 * Create item data and add new item
 */
public boolean doPut() {
	String itemID = "ITEM" + itemRows;
	String sellerID = DotsGenerator.mdUserID("UID",1,regRows);
	String description = DotsGenerator.mdString(100);
	float  price = DotsGenerator.mdPrice();
	long  currentTime = System.currentTimeMillis();
	java.sql.Date   startDate = new java.sql.Date(currentTime);
	java.sql.Date   endDate = new java.sql.Date(currentTime + 10*24*60*60*1000);
	int bidCount = 0;

	//reVal = auction.addNewItem(itemID,sellerID,description,price,startDate,endDate,bidCount);
	PreparedStatement pstmt = null;
	boolean reVal = true;

	try
	{
		pstmt = conn.prepareStatement(addNewItemSQL);
	} catch (Exception e){
		DotsLogging.logException("ATCJ1.createRegistry() Exception: " + e);
		reVal = false;
		DotsConfig.FAILEDCOUNT++;
		return reVal;
	}
	try{
		pstmt.setString(1, itemID);
		pstmt.setString(2,sellerID);
		pstmt.setString(3, description);
		pstmt.setFloat(4, price);
		pstmt.setDate(5, startDate);
		pstmt.setDate(6, endDate);
		pstmt.setInt(7, bidCount);
		pstmt.executeUpdate();
		pstmt.close();
		pstmt = null;

		DotsConfig.INSERTCOUNT++;
		if(DotsConfig.DRIVER_CLASS_NAME.indexOf("mysql") == -1)
			conn.commit();

	}
	catch (Exception e)
	{
		DotsLogging.logException("Auction.addNewItem() Exception: " + e);
		DotsConfig.FAILEDCOUNT++;

	}
	

	return reVal;
}	
/**
 * Create new item data and update that item
 */	
public boolean doUpdateItem() {

	boolean reVal = true;
	String itemID = DotsGenerator.mdUserID("ITEM",1,itemRows);
	String newDescription = DotsGenerator.mdString(100);
	float newBidPrice = DotsGenerator.mdPrice();
	java.sql.Date   newStartTime = new java.sql.Date(System.currentTimeMillis());
	java.sql.Date   newEndTime = new java.sql.Date(System.currentTimeMillis() + 10*24*60*60*1000);
	//reVal = auction.updateItem(itemID,description,bidPrice,startDate,endDate);
	
	Statement stmt = null;
	ResultSet rs = null;
	
	java.sql.Date startTime;
	java.sql.Date endTime;
	java.sql.Date currentTime;
	float bidPrice;
	PreparedStatement pstmt = null;

	int count = 0;

	try
	{
		stmt = conn.createStatement(ResultSet.TYPE_SCROLL_INSENSITIVE,ResultSet.CONCUR_READ_ONLY);
	} catch (Exception e){
		DotsLogging.logException("ATCJ1.createRegistry() Exception: " + e);
		reVal = false;
		DotsConfig.FAILEDCOUNT++;
		return reVal;
	}
	try{  
		rs = stmt.executeQuery(getItemSQL + "'" + itemID + "'");

		DotsConfig.QUERYCOUNT++;

		if (rs.next()) {

			bidPrice = rs.getFloat(3);
			startTime = rs.getDate(4);
			endTime = rs.getDate(5);

			rs.close();
			rs = null;
			stmt.close();
			stmt = null;

			currentTime = new java.sql.Date(System.currentTimeMillis());
			if (startTime.after(currentTime)) {
				pstmt = conn.prepareStatement(updateItemSQL + "'" + itemID + "'");
				pstmt.setString(1,newDescription);
				pstmt.setFloat(2,newBidPrice);
				pstmt.setDate(3,newStartTime);
				pstmt.setDate(4,newEndTime);
				count = pstmt.executeUpdate();
				pstmt.close();
				pstmt = null;

				//if (count > 0)
				//	reVal = true;
				DotsConfig.UPDATECOUNT++;
			}
			else if (startTime.before(currentTime) && currentTime.before(endTime)) {
				pstmt = conn.prepareStatement(updateItemSQL + "'" + itemID + "'");
				pstmt.setString(1,newDescription);
				pstmt.setFloat(2,bidPrice);
				pstmt.setDate(3,startTime);
				if (newEndTime.after(currentTime))
					pstmt.setDate(4,newEndTime);
				else
					pstmt.setDate(4,endTime);

				count = pstmt.executeUpdate();
				pstmt.close();
				pstmt = null;

				//if (count > 0)
				//	reVal = true;
				DotsConfig.UPDATECOUNT++;
			}
			if(DotsConfig.DRIVER_CLASS_NAME.indexOf("mysql") == -1)
				conn.commit();
		}


	}
	catch (Exception e)
	{
		DotsLogging.logException("Auction.updateItem() Exception: " + e);
		//reVal = false;
		DotsConfig.FAILEDCOUNT++;
	}

	return reVal;
}	

/**
 * view the detail information of a item
 */
public boolean doViewItem() {

	boolean reVal = true;
	String itemID = DotsGenerator.mdUserID("ITEM",1,itemRows);
	//reVal = auction.browseItem(itemID);
	Statement stmt = null;
	ResultSet rs = null;
	
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
		rs = stmt.executeQuery(getItemSQL + "'" + itemID + "'");

		DotsConfig.QUERYCOUNT++;

		rs.close();
		rs = null;
		stmt.close();
		stmt = null;
	}
	catch (Exception e)
	{
		DotsLogging.logException("Auction.browseItem() Exception: " + e);
		reVal = false;
		DotsConfig.FAILEDCOUNT++;
	}
	
	return reVal;
}	

/**
 * Get bid history
 * @param itemID item ID
 */
public boolean getBidHistory(String itemID) {

	Statement stmt = null;
	boolean reVal = true;
	ResultSet rs = null;


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
		rs = stmt.executeQuery(getBidHistorySQL + "'" + itemID + "' ORDER BY BID_TIME DESC");

		DotsConfig.QUERYCOUNT++;

		rs.close();
		rs = null;
		stmt.close();
		stmt = null;

	}
	catch (Exception e)
	{
		DotsLogging.logException("Auction.getBidHistory() Exception: " + e);
		//reVal = false;
		DotsConfig.FAILEDCOUNT++;

	}
	
	return reVal;

}	

/**
 * Get row count of a table
 * @param tablename table name
 */
public int getRows(String tablename) {

	Statement stmt = null;
	ResultSet rs = null;
	int reVal = 0;
	String sql = "";

	if (tablename.equalsIgnoreCase("registry"))
		sql = getRegRowsSQL;
	else if (tablename.equalsIgnoreCase("bid"))
		sql = getBidRowsSQL;
	else if (tablename.equalsIgnoreCase("item"))
		sql = getItemRowsSQL;

	try {
		stmt = conn.createStatement();
	} catch (Exception e){
		DotsLogging.logException("ATCJ1.createRegistry() Exception: " + e);
		DotsConfig.FAILEDCOUNT++;
		return -1;
	}
	try{
		rs = stmt.executeQuery(sql);

		DotsConfig.QUERYCOUNT++;

		if (rs.next()) {
			reVal = rs.getInt(1);
		}
		rs.close();
		rs = null;
		stmt.close();
		stmt = null;

	}catch(Exception e) {
		DotsLogging.logException("ATCJ1.getRows() Exception: " +e);
		DotsConfig.FAILEDCOUNT++;

	}
	
	return reVal;

}}