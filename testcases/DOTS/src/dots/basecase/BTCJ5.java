/*
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

package dots.basecase;

import java.sql.*;
import dots.framework.*;

/**
 * This class use prepared statements to execute database operations 
 * such as insert, update, select and delete.
 */
public class BTCJ5 implements Runnable {
	/**Database connection*/
	private Connection conn;
	/**The integer identify this thread */ 
	private int identity = 0;
	/**SQL, insert one record to the table basic1*/
	private final String insert1 = 
		"INSERT INTO BASIC1(ID_1,RND_CHAR, RND_FLOAT) VALUES(?,?,?)"; 
	/**SQL, insert one record to the table basic2*/
	private final String insert2 = 
		"INSERT INTO BASIC2(ID_2,RND_INTEGER,RND_TIME,RND_DATE) VALUES(?,?,?,?)"; 
	/**SQL, insert one record to the table basic3*/
	private final String insert3 = 
		"INSERT INTO BASIC3(ID_1,ID_2,RND_TIMESTAMP,RND_INT) values(?,?,?,?)"; 
	/**SQL, query records from the table basic1,basic2,basic3. Use table's join. */
	private final String select = 
		"SELECT * FROM BASIC1,BASIC2,BASIC3 WHERE "
			+ "BASIC1.ID_1 = BASIC3.ID_1 AND BASIC2.ID_2 =BASIC3.ID_2 "
			+ " AND BASIC3.RND_INT>? ORDER BY BASIC3.RND_INT"; 
	/**SQL, update records in the table basic2*/
	private final String update = 
		"UPDATE BASIC2 SET RND_INTEGER =? WHERE " + " ID_2 LIKE ? "; 
	/**SQL, count records in the table basic1*/
	private final String countSql = "SELECT COUNT(*) FROM BASIC1";
	/**SQL, delete records from the table basic1*/
	private final String deleteSql1 = "DELETE FROM BASIC1 WHERE ID_1 LIKE ?";	
	/**SQL, delete records from the table basic2*/
	private final String deleteSql2 = "DELETE FROM BASIC2 WHERE ID_2 LIKE ?";	
	/**SQL, delete records from the table basic3*/
	private final String deleteSql3 = "DELETE FROM BASIC3 WHERE ID_1 LIKE ?";

/**
 * BTCJ5 constructor. Generate identity and receive database connection from 
 * framework.
 */
public BTCJ5(Connection con) {
	super();
	identity = DotsGenerator.mdInt(0, 1000);
	this.conn = con;
}



/**
 * Count the number of records in the table basic1.
 * @return int 
 */
public int countRecords() {
	PreparedStatement pstmt = null;
	ResultSet rset = null;
	int count = 0;

	try {
		pstmt = conn.prepareStatement(countSql);
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ5.countRecords(): " + e);
		return -2;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ5.countRecords() :" + t);
		return -2;
	}
	try{
		rset = pstmt.executeQuery();
		DotsConfig.QUERYCOUNT++;
		if (rset.next())
			count = rset.getInt(1);
		rset.close();
		rset = null;
		pstmt.close();
		pstmt = null;

		return count;
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ5.countRecords(): " + e);
		return -1;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ5.countRecords() :" + t);
		return -2;
	}
}



/**
 * run() method of the thread.
 */
public void run() {
	int succeed = 0;
	int count = countRecords();
	int index = 0;

	// If records in the table BASIC1 are less than 2000 and Dots is not terminated, 
	// insert records into table BASIC1,BAISC2 and BASIC3.
	while ((count < 2000) && (count != -1) && (!DotsConfig.TERMINATION)) {
		for (int i = 0; i < 100; i++) {
			succeed = populateTables();
			if (succeed == -2)	return;
		}
		count = countRecords();
		
	}

	do {
		//Count records in table BAISC1. If the records are less than 20000, do insert,
		//update and query orderly; otherwise, do insert,update,query and delete orderly.

		count = countRecords();
		if (count == -2)	return;
		if (count > 20000) {
			index = populateTables(DotsGenerator.mdInt(count, 2 * count));
			if (index == -2)	return;
			succeed = updateTables();
			if (succeed == -2)	return;
			succeed = queryTables();
			if (succeed == -2)	return;
			if (index > 0) {
				succeed = clearTables(index);
				if (succeed == -2)	return;
			}
		} else {
			succeed = populateTables();
			if (succeed == -2)	return;
			succeed = updateTables();
			if (succeed == -2)	return;
			succeed = queryTables();
			if (succeed == -2)	return;
		}

	} while (!DotsConfig.TERMINATION);
	
	try {
		conn.close();
		conn = null;
		System.gc();
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ5.run() :" + e);
	}
}



/**
 * Use PreparedStatement to insert one record to table basic1, basic2 and basic3. The key's value
 * is incremental.
 * @return int.
 */
public int populateTables() {
	PreparedStatement pstmt = null;
	ResultSet rset = null;
	int count = countRecords();
	if (count == -2) 	return -2;
	String id1 = "ID1:" + count + ":" + identity;
	String id2 = "ID2:" + count + ":" + identity;
	
	try {		
		pstmt = conn.prepareStatement(insert1);
		pstmt.setString(1, id1);
		pstmt.setString(2, DotsGenerator.mdString(10));
		pstmt.setFloat(3, DotsGenerator.mdPrice());
		pstmt.executeUpdate();
		DotsConfig.INSERTCOUNT++;
		pstmt.close();
		pstmt = null;
				
		pstmt = conn.prepareStatement(insert2);
		pstmt.setString(1, id2);
		pstmt.setInt(2, DotsGenerator.mdInt());
		pstmt.setTime(3, new Time(System.currentTimeMillis()));
		pstmt.setTimestamp(4, new Timestamp(System.currentTimeMillis()));
		pstmt.executeUpdate();
		DotsConfig.INSERTCOUNT++;
		pstmt.close();
		pstmt = null;
		
		pstmt = conn.prepareStatement(insert3);
		pstmt.setString(1, id1);
		pstmt.setString(2, id2);
		pstmt.setTimestamp(3, new Timestamp(System.currentTimeMillis()));
		pstmt.setInt(4, DotsGenerator.mdInt(0, 1000));
		pstmt.executeUpdate();
		DotsConfig.INSERTCOUNT++;
		
		pstmt.close();
		pstmt = null;
		return 0;
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ5.populateTables(): " + e);
		return -1;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ5.populateTables() :" + t);
		return -2;
	}
}

/**
 * Use Prepared Statement to insert one record to table basic1, basic2 and basic3. The key's value
 * is decided by index.
 * @param index int.
 * @return int.
 */
public int populateTables(int index) {
	PreparedStatement pstmt = null;
	String id1 = "ID1:" + index + ":" + identity;
	String id2 = "ID2:" + index + ":" + identity;
	
	try {		
		pstmt = conn.prepareStatement(insert1);
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ5.populateTable1(): " + e);
		return -2;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ5.populateTables() :" + t);
		return -2;
	}
	try{
		pstmt.setString(1, id1);
		pstmt.setString(2, DotsGenerator.mdString(10));
		pstmt.setFloat(3, DotsGenerator.mdPrice());
		pstmt.executeUpdate();
		pstmt.close();
		pstmt = null;
		DotsConfig.INSERTCOUNT++;
		
		pstmt = conn.prepareStatement(insert2);
		pstmt.setString(1, id2);
		pstmt.setInt(2, DotsGenerator.mdInt());
		pstmt.setTime(3, new Time(System.currentTimeMillis()));
		pstmt.setTimestamp(4, new Timestamp(System.currentTimeMillis()));
		pstmt.executeUpdate();
		pstmt.close();
		pstmt = null;
		DotsConfig.INSERTCOUNT++;
		
		pstmt = conn.prepareStatement(insert3);
		pstmt.setString(1, id1);
		pstmt.setString(2, id2);
		pstmt.setTimestamp(3, new Timestamp(System.currentTimeMillis()));
		pstmt.setInt(4, DotsGenerator.mdInt(0, 1000));
		pstmt.executeUpdate();
		pstmt.close();
		pstmt = null;
		DotsConfig.INSERTCOUNT++;
		
		return index;

	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ5.populateTables(): " + e);
		return -1;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ5.populateTables() :" + t);
		return -2;
	}
	
}

/**
 * Delete record from all related tables.
 * @param index int.
 */ 
public int clearTables(int index) {
	PreparedStatement pstmt = null;	
	String id1 = "ID1:" + index + ":%";
	String id2 = "ID2:" + index + ":%";
	
	try {
		pstmt = conn.prepareStatement(deleteSql1);
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ5.clearTables(): " + e);
		return -2;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ5.clearTables() :" + t);
		return -2;
	}
	
	try{
		pstmt.setString(1,id1);
		pstmt.executeUpdate();
		DotsConfig.DELETECOUNT++;
		pstmt.close();
		pstmt = null;
		
		pstmt = conn.prepareStatement(deleteSql2);
		pstmt.setString(1,id2);
		pstmt.executeUpdate();
		DotsConfig.DELETECOUNT++;
		pstmt.close();
		pstmt = null;
		
		pstmt = conn.prepareStatement(deleteSql3);
		pstmt.setString(1,id1);
		pstmt.executeUpdate();
		DotsConfig.DELETECOUNT++;
		
		pstmt.close();
		pstmt = null;
		return 0;
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ5.clearTables(): " + e);
		return -1;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ5.clearTables() :" + t);
		return -2;
	}
}

/**
 * Query table. Use join of table baisc1, basic2 and basic3.
 */
public int queryTables() {
	PreparedStatement pstmt = null;
	//ResultSet rset = null;
	String tmpString = null;
	int tmpInt;
	int columnCount = 0;

	try {
		pstmt = conn.prepareStatement(select);
	} catch (SQLException e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ5.queryTables() : " + e);
		return -2;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ5.queryTables() :" + t);
		return -2;
	}
	
	try{
		pstmt.setInt(1, DotsGenerator.mdInt(500, 1000));
		pstmt.executeQuery();
		DotsConfig.QUERYCOUNT++;
		
		if (pstmt != null) {
			pstmt.close();
			pstmt = null;
		}
		return 0;
	} catch (SQLException e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ5.queryTables() : " + e);
		return -1;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ5.queryTables() :" + t);
		return -2;
	}
}

/**
 * Update table basic2.
 */
 
public int updateTables() {
	
	PreparedStatement pstmt = null;
	
	try {
		pstmt = conn.prepareStatement(update);
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ5.updateTables(): " + e);
		return -2;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ5.updateTables() :" + t);
		return -2;
	}
	
	try{
		pstmt.setInt(1, DotsGenerator.mdInt());
		pstmt.setString(2, "%:" + DotsGenerator.mdInt(0, 20000) + ":%"); 
		pstmt.executeUpdate();
		pstmt.clearParameters();
		DotsConfig.UPDATECOUNT++;
		pstmt.close();
		pstmt = null;
		return 0;
		
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ5.updateTables(): " + e);
		return -1;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ5.updateTables() :" + t);
		return -2;
	}
	
}}