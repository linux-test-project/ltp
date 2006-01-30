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
 * This is a basic test case. This case use PreparedStatement to execute 
 * database operations such as insert, update, select and delete.
 */

public class BTCJ4 implements Runnable {
	/**Database connection*/
	private Connection conn = null;
	/**The integer identify this thread */ 
	private int identity = 0;
	/**Prepared Statement*/
	private PreparedStatement pstmt = null;
	/**Int used to calculate ms of a year*/
	private int calculator = 365*24*60*60*1000;
	/**SQL, count records in the table basic4*/
	private final String countSql = "SELECT COUNT(*) FROM BASIC4";
	/**SQL, insert one record to the table basic4*/
	private final String insertSql = 
		"INSERT INTO BASIC4(ID_4,NAME,SALARY,AGE,HIREDATE,DEPTNO) VALUES(?,?,?,?,?,?)"; 
	/**SQL, select records from the table basic4, use 'GROUP BY'*/
	private final String selectSql1 = 
		"SELECT SUM(SALARY), AVG(AGE) FROM BASIC4 WHERE DEPTNO = ? GROUP BY HIREDATE "; 
	/**SQL, select records from the table basic4, use 'ORDER BY'*/
	private final String selectSql2 = 
		"SELECT * FROM BASIC4 WHERE HIREDATE > ? AND DEPTNO = ? ORDER BY NAME, SALARY, AGE, DEPTNO"; 
	/**SQL, select records from the table basic4, use 'UNION'*/
	private final String selectSql3 = 
		"SELECT * FROM BASIC4 WHERE AGE > ? UNION SELECT * FROM BASIC4 WHERE HIREDATE < ? "; 
	/**SQL, update records in the table basic4*/
	private final String updateSql = 
		"UPDATE BASIC4 SET SALARY = (SELECT MAX(SALARY) FROM BASIC4 WHERE AGE IN (?,?)) WHERE ID_4 LIKE ?"; 
	/**SQL, used by MySQl for MySQL dosen't support above SQL*/
	private final String updateSql1 = 
		"UPDATE BASIC4 SET SALARY = ? WHERE ID_4 LIKE ?"; 
	/**SQL, delete records from the table basic4*/
	private final String deleteSql = "DELETE FROM BASIC4 WHERE ID_4 LIKE ? ";
	
/**
 * BTCJ4 constructor. Generate identity and receive database connection from 
 * framework.
 */
public BTCJ4(Connection con) {
	super();
	identity = DotsGenerator.mdInt(0, 1000);
	this.conn = con;
}


	
/**
 * Count the number of records in the table basic4.
 * @return int 
 */
public int countRecords() {
	Statement stmt = null;
	ResultSet rset = null;
	int count = 0;
	try {
		stmt = conn.createStatement();
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ4.countRecords() :" + e);
		return -2;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ4.countRecords() :" + t);
		return -2;
	}
	
	try{
		rset = stmt.executeQuery(countSql);
		DotsConfig.QUERYCOUNT ++;
		
		if (rset.next())
			count = rset.getInt(1);
		rset.close();
		rset = null;
		
		stmt.close();
		stmt = null;
		return count;
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ4.countRecords() :" + e);
		return -1;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ4.countRecords() :" + t);
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

	// If records in the table BASIC4 are less than 2000 and Dots is not terminated, 
	// insert records into table BASIC4.
	while ((count < 2000) && (count != -1) && (!DotsConfig.TERMINATION)) {
		for (int i = 0; i < 100; i++) {
			succeed = populateTable();
			if (succeed == -2)	return;
		}
		count = countRecords();
		
	}

	do {
		//Count records in table BAISC4. If the records are less than DotsConfig.MAX_ROWS, do insert,
		//update and query orderly; otherwise, do update and query orderly.

		count = countRecords();
		if (count == -2)	return;
		if (count >= DotsConfig.MAX_ROWS) {			
			switch(DotsGenerator.mdInt(1, 3)) {
				case 1:
					succeed = updateTable(count);
					if (succeed == -2) 	return;
					break;
				case 2:
					succeed = queryTable();
					if (succeed == -2)	return;
			}
		} else {
			switch(DotsGenerator.mdInt(1, 4)) {
				case 1:
					succeed = populateTable();
					if (succeed == -2)	return;
					break;
				case 2:
					succeed = updateTable(count);
					if (succeed == -2)	return;
					break;
				case 3:
					succeed = queryTable();
					if (succeed == -2)	return;
			}
		}

	} while (!DotsConfig.TERMINATION );

	try {
		conn.close();
		conn = null;
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT++;
		DotsLogging.logException("BTCJ4.run() :" + e);
	}
}



/**
 * Insert one record to table basic4. The key's value is incremental.
 * @return int.
 */
public int populateTable() {
	PreparedStatement pstmt = null;
	ResultSet rset = null;
	int count = countRecords();
	if(count == -2) 	return count;
	String id = "ID4:" + count + ":" + identity;;
	
	try {		
		pstmt = conn.prepareStatement(insertSql);
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ4.populateTable() :" + e);
		return -2;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ4.populateTable() :" + t);
		return -2;
	}
	try{
		pstmt.setString(1,id);
		pstmt.setString(2,DotsGenerator.mdString(25));
		pstmt.setFloat(3,DotsGenerator.mdPrice());
		pstmt.setInt(4,DotsGenerator.mdInt(20,100));
		pstmt.setDate(5,new Date(System.currentTimeMillis()-DotsGenerator.mdInt(0,50)*calculator));
		pstmt.setInt(6,DotsGenerator.mdInt(1,20));
		pstmt.executeUpdate();
		DotsConfig.INSERTCOUNT ++;
		
		pstmt.close();
		pstmt = null;
		return 0;
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ4.populateTable() :" + e);
		return -1;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ4.populateTable() :" + t);
		return -2;
	}
}

/**
 * Insert one record to table basic4. The key's value is decided by index.
 * @param index int.
 * @return int.
 */
public int populateTable(int index) {
	PreparedStatement pstmt = null;
	String id = "ID4:" + index + ":" + identity;
	
	try {
		
		pstmt = conn.prepareStatement(insertSql);
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ4.populateTable() :" + e);
		return -2;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ4.populateTable() :" + t);
		return -2;
	}
	
	try{
		pstmt.setString(1,id);
		pstmt.setString(2,DotsGenerator.mdString(25));
		pstmt.setFloat(3,DotsGenerator.mdPrice());
		pstmt.setInt(4,DotsGenerator.mdInt(20,100));
		pstmt.setDate(5,new Date(System.currentTimeMillis()-DotsGenerator.mdInt(0,50)*calculator));
		pstmt.setInt(6,DotsGenerator.mdInt(1,20));
		pstmt.executeUpdate();
		DotsConfig.INSERTCOUNT ++;
		
		pstmt.close();
		pstmt = null;
		return index;
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ4.populateTable() :" + e);
		return -1;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ4.populateTable() :" + t);
		return -2;
	}
}

/**
 * Query table. Randomly use one kind of query SQL.
 */
public int queryTable() {

	ResultSet rset = null;
	String tmp = null;
	int choice = 1;
	if(DotsConfig.DRIVER_CLASS_NAME.indexOf("mysql") == -1){
		choice = DotsGenerator.mdInt(1, 4);
	} else {
		choice = DotsGenerator.mdInt(1, 3);
	}

	try{
		switch (choice) {
			case 1 :
				pstmt = conn.prepareStatement(selectSql1);
				pstmt.setInt(1, DotsGenerator.mdInt(1, 20));
				break;
			case 2 :	
				pstmt = conn.prepareStatement(selectSql2);	
				pstmt.setDate(1, new Date(System.currentTimeMillis()-DotsGenerator.mdInt(0,50)*calculator));
				pstmt.setInt(2, DotsGenerator.mdInt(1, 20));
				break;  
			case 3 :		
				pstmt = conn.prepareStatement(selectSql3);
				pstmt.setInt(1, DotsGenerator.mdInt(20, 100));
				pstmt.setDate(2, new Date(System.currentTimeMillis()-DotsGenerator.mdInt(0,50)*calculator));
				break;				
		}
	} catch (SQLException e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ4.queryTable() :" + e);
		return -2;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ4.queryTable() :" + t);
		return -2;
	}

	try{
		rset = pstmt.executeQuery();
		if (rset.next())
			tmp = rset.getString(1);
		DotsConfig.QUERYCOUNT ++;
		
		rset.close();
		rset = null;
		pstmt.close();
		pstmt = null;
		return 0;
	} catch (SQLException e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ4.queryTables() :" + e);
		return -1;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ4.queryTable() :" + t);
		return -2;
	}
}

/**
 * Use prepared Statement to update table.
 */
public int updateTable(int index) {

	int age1 = DotsGenerator.mdInt(20, 100);
	int age2 = DotsGenerator.mdInt(20, 100);
	Math math = null;

	try {
		if(DotsConfig.DRIVER_CLASS_NAME.indexOf("mysql") == -1){
			pstmt = conn.prepareStatement(updateSql);
		} else {
			pstmt = conn.prepareStatement(updateSql1);
		}
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT++;
		DotsLogging.logException("BTCJ4.updateTable() : " + e);
		return -2;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ4.updateTable() :" + t);
		return -2;
	}
	try{
		if(DotsConfig.DRIVER_CLASS_NAME.indexOf("mysql") == -1){
			pstmt.setInt(1, math.min(age1, age2));
			pstmt.setInt(2, math.min(age1, age2));
			pstmt.setString(3, "ID4:"+DotsGenerator.mdInt(1, index)+"%");
		} else {
			pstmt.setFloat(1, DotsGenerator.mdPrice());
			pstmt.setString(2, "ID4:"+DotsGenerator.mdInt(1, index)+"%");
		}
		
		pstmt.executeUpdate();
		DotsConfig.UPDATECOUNT++;

		pstmt.close();
		pstmt = null;
		return 0;
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT++;
		DotsLogging.logException("BTCJ4.updateTable() : " + e);
		return -1;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ4.updateTable() :" + t);
		return -2;
	}
}}
