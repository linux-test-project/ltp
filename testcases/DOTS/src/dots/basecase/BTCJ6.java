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
import java.io.*;
import dots.framework.*;

/**
 * This is a basic test case. This case use PreparedStatement to manipulate 
 * such datatype as CLOB. 
 */
public class BTCJ6 implements Runnable {
	/**Database connection*/
	private Connection conn = null;
	/**The integer identify this thread */ 
	private int identity = 0;
	/**Files' name. These file represent randomly generated CLOB*/
	private String clobFileNames[];
	/**SQL, count records in the table basic5*/
	private final String countSql = "SELECT COUNT(*) FROM BASIC5";
	/**SQL, insert one record to the table basic5, DOC's type is CLOB*/
	private final String insertSql1 = 
		"INSERT INTO BASIC5 (ID_5,NAME,AGE,SALARY,DOC) VALUES(?,?,?,?,?)";	
	/**SQL, used by my MySQL */
	private final String insertSql1_m = 
		"INSERT INTO BASIC5_TMP (ID_5,NAME,AGE,SALARY,DOC) VALUES(?,?,?,?,?)";	
	/**SQL, insert one record to the table basic5, DOC's value is selected from other record.*/
	private final String insertSql2 = 
		"INSERT INTO BASIC5 (ID_5,NAME,AGE,SALARY,DOC) VALUES(?,?,?,?,(SELECT DOC FROM BASIC5 WHERE ID_5 = ?))";
	/**SQL, used by my MySQL */
	private final String insertSql2_m = 
		"INSERT INTO BASIC5 (ID_5,NAME,AGE,SALARY,DOC) SELECT ?,?,?,?,DOC FROM BASIC5_TMP WHERE BASIC5_TMP.ID_5 = ?";
	/**SQL, select records from the table basic5*/
	private final String selectSql = 
		"SELECT * FROM BASIC5 WHERE ID_5 like ?";
	/**SQL, update records in the table basic5*/
	private final String updateSql = 
		"UPDATE BASIC5 SET DOC = (SELECT DOC FROM BASIC5 WHERE ID_5 = ?) WHERE SALARY = (SELECT MAX(SALARY) FROM BASIC5 WHERE AGE > ? )";

/**
 * BTCJ6 constructor. Generate identity, receive database connection and
 * receive file names from framework. Each file is a randomly generated CLOB.
 */
public BTCJ6(Connection con,String[] FileName) {
	super();
	identity = DotsGenerator.mdInt(0, 100);
	clobFileNames = FileName;
	this.conn = con;
}

/**
 * Count the number of records in the table BASIC5.
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
		DotsLogging.logException("BTCJ6.countRecords() :" + e);
		return -2;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ6.countRecords() :" + t);
		return -2;
	}
	try{
		rset = stmt.executeQuery(countSql);
		DotsConfig.QUERYCOUNT++;
		if (rset.next())
			count = rset.getInt(1);
		rset.close();
		rset = null;
		
		stmt.close();
		stmt = null;
		return count;
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ6.countRecords() :" + e);
		return -1;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ6.countRecords() :" + t);
		return -2;
	}
}



/**
 * run() method of the thread.
 */
public void run() {
	int count = countRecords();
	int succeed = 0;
	if (count == -2)	
		return;
	else if ((count >= 0) && (count<9)){
		setAutoCommitFalse();
		for(int i=count;i<10;i++) {
			populateTable(i);
		}
		setAutoCommitTrue();
	}
	succeed = populateTable2();
	if (succeed == -2)	return;
	do {
		setAutoCommitFalse();
		
		//Count records in table BAISC5. If the records are less than 10000, do insert
		//and query orderly; otherwise, do update and query orderly.
		count = countRecords();
		if (count == -2)	return;
		if (count > 10000) {
			switch (DotsGenerator.mdInt(1, 5)) {
			case 1 :
				succeed = getClob(count);
				if (succeed == -2)	return;				
				break;
			default :
				if(DotsConfig.DRIVER_CLASS_NAME.indexOf("mysql") == -1){
					succeed = updateClob();
					if (succeed == -2)	return;
				}
				break;
			}
		} else {
			switch (DotsGenerator.mdInt(1, 5)) {
			case 1 :				
				succeed = getClob(count);
				if (succeed == -2)	return;				
				break;
			default :
				succeed = populateTable2();
				if (succeed == -2)	return;
				break;
			}
		}

		setAutoCommitTrue();


	} while (!DotsConfig.TERMINATION);
	
	try {
		conn.close();
		conn = null;
		System.gc();
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ6.run() :" + e);
	}
}

/**
* Get Clob from the table basic5.
*/ 
public int getClob(int count) {
	PreparedStatement selectStmt = null;
	ResultSet rset = null;
	Clob doc = null;
	

	try{
		selectStmt = conn.prepareStatement(selectSql);
	}catch(Exception e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ6.getClob(): " + e);
	}catch(Throwable t) {
		DotsLogging.logException("BTCJ6.getClob() :" + t);
		return -2;
	}

	try{
		selectStmt.setString(1, "ID5:"+DotsGenerator.mdInt(0,count)+":%");
	
		rset = selectStmt.executeQuery();		
		if(DotsConfig.DRIVER_CLASS_NAME.indexOf("mysql") == -1){
			if (rset.next()) {
				doc = rset.getClob("DOC"); //MySQL JDBC driver org.gjt.mm.mysql.jdbc2.NotImplemented.
	//			System.out.println("doc: " + doc);
			}
		}
		doc = null;
		rset.close();
		rset = null;
		selectStmt.close();
		selectStmt = null;
		DotsConfig.QUERYCOUNT++;
		return 0;
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ6.getClob(): " + e);
		return -1;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ6.getClob() :" + t);
		return -2;
	}
}

/**
 * Insert one record to table basic5. The key's value is incremental.
 */
public int populateTable(int index) {
	PreparedStatement pstmt = null;
	String id = "ID5:" + index ;
	FileReader clobReader;
	
	try {
		if(DotsConfig.DRIVER_CLASS_NAME.indexOf("mysql") == -1){
			pstmt = conn.prepareStatement(insertSql1);
		} else {
			pstmt = conn.prepareStatement(insertSql1_m);
		}
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ6.populateTable(): " + e);
		return -2;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ6.populateTable() :" + t);
		return -2;
	}
	
	try{
		pstmt.setString(1, id.trim());
		pstmt.setString(2, DotsGenerator.mdString(20));
		pstmt.setInt(3, DotsGenerator.mdInt(20, 100));
		pstmt.setFloat(4, DotsGenerator.mdPrice());
		clobReader = new FileReader(clobFileNames[DotsGenerator.mdInt(0, 10)]); 
		pstmt.setCharacterStream(5, clobReader, 200);
		pstmt.executeUpdate();
		clobReader = null;
		DotsConfig.INSERTCOUNT++;
		
		pstmt.close();
		pstmt = null;
		return 0;
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ6.populateTable(): " + e);
		return -1;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ6.populateTable() :" + t);
		return -2;
	}
}

/**
 * Insert one record to table basic5. The key's value is incremental.
 */
public int populateTable2() {
	PreparedStatement pstmt = null;

	int count = countRecords();
	if ( count == -2) return count;
	String id = "ID5:" + count + ":" + identity;
	
	try {
		if(DotsConfig.DRIVER_CLASS_NAME.indexOf("mysql") == -1){
			pstmt = conn.prepareStatement(insertSql2);
		} else {
			pstmt = conn.prepareStatement(insertSql2_m);
		}
	} catch (Exception e) {		
		DotsLogging.logException("BTCJ6.populateTable(): " + e);
		DotsConfig.FAILEDCOUNT ++;return -2;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ6.populateTable() :" + t);
		return -2;
	}
	
	try{

		pstmt.setString(1, id);
		pstmt.setString(2, DotsGenerator.mdString(20));
		pstmt.setInt(3, DotsGenerator.mdInt(20, 100));
		pstmt.setFloat(4, DotsGenerator.mdPrice());
		pstmt.setString(5, "ID5:"+ DotsGenerator.mdInt(0,10));
		
		pstmt.executeUpdate();
		
		DotsConfig.INSERTCOUNT++;
		
		pstmt.close();
		pstmt = null;
		return 0;
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ6.populateTable(): " + e);
		return -1;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ6.populateTable() :" + t);
		return -2;
	}
}

/**
 * Update clob in the table basic5.
 */
public int updateClob() {
	PreparedStatement updateStmt = null;
	
	try {
		updateStmt = conn.prepareStatement(updateSql);
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT++;
		DotsLogging.logException("BTCJ6.updateClob(): " + e);
		return -2;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ6.updateClob() :" + t);
		return -2;
	}
	try{
		updateStmt.setString(1, "ID5:"+DotsGenerator.mdInt(0, 10));
		updateStmt.setInt(2, DotsGenerator.mdInt(20, 100));
		updateStmt.executeUpdate();
		DotsConfig.UPDATECOUNT++;
		updateStmt.close();
		updateStmt = null;
		return 0;
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT++;
		DotsLogging.logException("BTCJ6.updateClob(): " + e);
		return -1;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ6.updateClob() :" + t);
		return -2;
	}
}

public void setAutoCommitTrue()
{
	try {
		if(DotsConfig.DRIVER_CLASS_NAME.indexOf("postgresql") != -1)
			conn.setAutoCommit(true);
	} catch(Exception e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ6.setAutoCommitTrue() :" + e);
	}
}

public void setAutoCommitFalse()
{
	try {
		if(DotsConfig.DRIVER_CLASS_NAME.indexOf("postgresql") != -1)
			conn.setAutoCommit(false);
	} catch(Exception e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ6.setAutoCommitFalse() :" + e);
	}
}}
