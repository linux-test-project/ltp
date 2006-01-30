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
 * such datatype as BLOB. 
 */
public class BTCJ7 implements Runnable {
	/**Database connection*/
	private Connection conn = null;
	/**The integer identify this thread */ 
	private int identity = 0;
	/**Files' name. These file represent randomly generated BLOB*/
	private String blobFileNames[];
	/**SQL, count records in the table basic6*/
	private final String countSql = "SELECT COUNT(*) FROM BASIC6";
	/**SQL, insert one record to the table basic6, PHOTO's type is BLOB*/
	private final String insertSql1 = 
		"INSERT INTO BASIC6 (ID_6,NAME,AGE,SALARY,PHOTO) VALUES(?,?,?,?,?)";
	/**SQL, used by MySQL*/
	private final String insertSql1_m = 
		"INSERT INTO BASIC6_TMP (ID_6,NAME,AGE,SALARY,PHOTO) VALUES(?,?,?,?,?)";	
	/**SQL, insert one record to the table basic6, PHOTO's value is selected from other record*/
	private final String insertSql2 = 
		"INSERT INTO BASIC6 (ID_6,NAME,AGE,SALARY,PHOTO) VALUES(?,?,?,?,(SELECT PHOTO FROM BASIC6 WHERE ID_6 = ?))";
	/**SQL, used by MySQL*/
	private final String insertSql2_m = 
		"INSERT INTO BASIC6 (ID_6,NAME,AGE,SALARY,PHOTO) SELECT ?,?,?,?,PHOTO FROM BASIC6_TMP WHERE BASIC6_TMP.ID_6 = ?";		
	/**SQL, select records from the table basic6*/
	private final String selectSql = 
		"SELECT * FROM BASIC6 WHERE ID_6 LIKE ?";
	/**SQL, update records in the table basic6*/
	private final String updateSql = 
		"UPDATE BASIC6 SET PHOTO = (SELECT PHOTO FROM BASIC6 WHERE ID_6 = ?) WHERE SALARY = (SELECT MAX(SALARY) FROM BASIC6 WHERE AGE > ?) ";

/**
 * BTCJ7 constructor. Generate identity, receive database connection and
 * receive file names from framework. Each file is a randomly generated BLOB.
 */
public BTCJ7(Connection con, String[] FileNames) {
	super();
	identity = DotsGenerator.mdInt(0, 100);
	blobFileNames = FileNames;
	this.conn = con;
}

/**
 * Count the number of records in the table BASIC6.
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
		DotsLogging.logException("BTCJ7.countRecords() :" + e);
		return -2;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ7.countRecords() :" + t);
		return -2;
	}
	
	try{
		rset = stmt.executeQuery(countSql);
		
		if (rset.next())
			count = rset.getInt(1);
		rset.close();
		rset = null;		
		stmt.close();
		stmt = null;
		DotsConfig.QUERYCOUNT++;
		return count;
		
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ7.countRecords() :" + e);
		return -1;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ7.countRecords() :" + t);
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
		//Count records in table BAISC6. If the records are less than 10000, do insert
		//and query orderly; otherwise, do update and query orderly.
		count = countRecords();
		if (count == -2)
			return;
		if (count > 10000) {
			switch (DotsGenerator.mdInt(1, 3)) {
			case 1 :
				succeed = getBlob(count);
				if (succeed == -2)	return;
				break;
			default :
				if(DotsConfig.DRIVER_CLASS_NAME.indexOf("mysql") == -1){
					succeed = updateBlob();	
					if (succeed == -2)	return;
				}
				break;
			}
		} else if (count >1) {
			switch (DotsGenerator.mdInt(1, 3)) {
			case 1 :
				succeed = getBlob(count);
				if (succeed == -2)	return;
				break;
			default :
				succeed = populateTable2();
				if (succeed == -2)	return;
				break;
			}
		}

		setAutoCommitTrue();

		System.gc();
	} while (!DotsConfig.TERMINATION );
	
	try {
		if (conn != null){
			conn.close();
			conn = null;
		}
		System.gc();
		
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ7.run() :" + e);
	}
}


/**
* Get blob from the table basic6.
*/
public int getBlob(int count) {
	PreparedStatement selectStmt = null;
	ResultSet rset = null;
	Blob photo = null;
	
	try {		
		selectStmt = conn.prepareStatement(selectSql);
	} catch (SQLException e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ7.getBlob(): " + e);
		return -2;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ7.getBlob() :" + t);
		return -2;
	}
	try{
		selectStmt.setString(1, "ID6:"+DotsGenerator.mdInt(0,count)+":%");
		
		rset = selectStmt.executeQuery();
		if(DotsConfig.DRIVER_CLASS_NAME.indexOf("mysql") == -1){
			if (rset.next()) {
				photo = rset.getBlob("PHOTO");
				// System.out.println("PHOTO: " + photo);
			}
		}
		photo = null;
		rset.close();
		rset = null;
		selectStmt.close();
		selectStmt = null;
		DotsConfig.QUERYCOUNT++;
		return 0;
	} catch (SQLException e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ7.getBlob(): " + e);
		return -1;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ7.getBlob() :" + t);
		return -2;
	}
}	

/**
 * Insert one record to table basic4. The key's value is incremental.
 */
public int populateTable(int index) {
	PreparedStatement pstmt = null;
	String id = "ID6:" + index ;
	FileInputStream blobReader;
	
	try {					
		if(DotsConfig.DRIVER_CLASS_NAME.indexOf("mysql") == -1){
			pstmt = conn.prepareStatement(insertSql1);
		} else {
			pstmt = conn.prepareStatement(insertSql1_m);
		}
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ7.populateTable(): " + e);
		return -2;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ7.populateTable() :" + t);
		return -2;
	}
		
	try{
		pstmt.setString(1, id.trim());
		pstmt.setString(2, DotsGenerator.mdString(20));
		pstmt.setInt(3, DotsGenerator.mdInt(20, 100));
		pstmt.setFloat(4, DotsGenerator.mdPrice());
		blobReader = new FileInputStream(blobFileNames[DotsGenerator.mdInt(0, 10)]); 
		pstmt.setBinaryStream(5, blobReader, 200);

		blobReader = null;
		pstmt.executeUpdate();
		
		pstmt.close();
		pstmt = null;
		DotsConfig.INSERTCOUNT++;
		System.gc();
		return 0;
		
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ7.populateTable(): " + e);
		return -1;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ7.populateTable() :" + t);
		return -2;
	}
}	

/**
 * Insert one record to table basic6. The key's value is incremental.
 */
public int populateTable2() {
	
	int count = countRecords();
	PreparedStatement pstmt = null;
	if ( count == -2) return count;
	String id = "ID6:" + count + ":" + identity;
	
	try {		
		if(DotsConfig.DRIVER_CLASS_NAME.indexOf("mysql") == -1){
			pstmt = conn.prepareStatement(insertSql2);
		} else {
			pstmt = conn.prepareStatement(insertSql2_m);
		}
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ7.populateTable2(): " + e);
		return -2;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ7.populateTable2() :" + t);
		return -2;
	}
	try{
		pstmt.setString(1, id);
		pstmt.setString(2, DotsGenerator.mdString(20));
		pstmt.setInt(3, DotsGenerator.mdInt(20, 100));
		pstmt.setFloat(4, DotsGenerator.mdPrice());
		pstmt.setString(5, "ID6:"+ DotsGenerator.mdInt(0,10));
		
		pstmt.executeUpdate();
		
		pstmt.close();
		pstmt = null;
		DotsConfig.INSERTCOUNT++;
		System.gc();			
		return 0;
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ7.populateTable2(): " + e);
		return -1;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ7.populateTable2() :" + t);
		return -2;
	}
}
	
/**
 * Update blob in the table basic6.
 */
public int updateBlob() {
	PreparedStatement updateStmt = null;
	
	try {
		updateStmt = conn.prepareStatement(updateSql);
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT++;
		DotsLogging.logException("BTCJ7.updateBlob(): " + e);
		return -2;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ7.updateBlob() :" + t);
		return -2;
	}
	try{	
		updateStmt.setString(1, "ID6:"+ DotsGenerator.mdInt(0,10));
		updateStmt.setInt(2, DotsGenerator.mdInt(50, 100));
		updateStmt.executeUpdate();
		updateStmt.close();
		updateStmt = null;

		DotsConfig.UPDATECOUNT++;
		return 0;

	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT++;
		DotsLogging.logException("BTCJ7.updateBlob(): " + e);
		return -1;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ7.updateBlob() :" + t);
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
		DotsLogging.logException("BTCJ7.setAutoCommitTrue() :" + e);
	}
}

public void setAutoCommitFalse()
{
	try {
		if(DotsConfig.DRIVER_CLASS_NAME.indexOf("postgresql") != -1)
			conn.setAutoCommit(false);
	} catch(Exception e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ7.setAutoCommitFalse() :" + e);
	}
}}
