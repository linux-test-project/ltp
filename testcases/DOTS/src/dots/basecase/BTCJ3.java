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
 * This is a basic test case. This case use Statement to execute 
 * database operations such as insert, update, select and delete.
 * This case is something like BTCJ2, but send multiple SQL statements 
 * to the database as a unit, or a batch.  
 */ 
public class BTCJ3 implements Runnable {
	/**Database connection*/
	private Connection conn = null;
	/**The integer identify this thread */ 
	private int identity = 0;
	/**SQL, count records in the table basic1*/
	private final String countSql = "SELECT COUNT(*) FROM BASIC1";


/**
 * BTCJ3 constructor. Generate identity and receive database connection from 
 * framework.
 */
public BTCJ3(Connection con) {
	super();
	identity = DotsGenerator.mdInt(0, 1000);
	this.conn = con;
}



/**
 * Count the number of records in the table BASIC1.
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
		DotsLogging.logException("BTCJ3.countRecords() :" + e);
		return -2;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ3.countRecords() :" + t);
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
		DotsLogging.logException("BTCJ3.countRecords() :" + e);
		return -1;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ3.countRecords() :" + t);
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
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT++;
		DotsLogging.logException("BTCJ3.run() :" + e);
	}
}



/**
 * Insert one record to table basic1, basic2 and basic3 in a batch. The key's value
 * is incremental.
 * @return int.
 */
public int populateTables() {
	Statement stmt = null;
	ResultSet rset = null;
	int count = 0;

	try {
		conn.setAutoCommit(false);
		stmt = conn.createStatement();
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT++;
		DotsLogging.logException("BTCJ3.populateTables() " + e);
		return -2;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ3.populateTables() " + t);
		return -2;
	}
	
	try{
		conn.commit();
		rset = stmt.executeQuery(countSql);
		DotsConfig.QUERYCOUNT++;

		if (rset.next())
			count = rset.getInt(1);
		rset.close();
		rset = null;
			
		String id1 = "ID1:" + count + ":" + identity;
		String id2 = "ID2:" + count + ":" + identity;
		String insertSql1 = 
			"INSERT INTO BASIC1(ID_1,RND_CHAR, RND_FLOAT) VALUES('"
				+ id1
				+ "', '"
				+ DotsGenerator.mdString(10)
				+ "', "
				+ DotsGenerator.mdPrice()
				+ ")"; 
		String insertSql2 = 
			"INSERT INTO BASIC2(ID_2,RND_INTEGER,RND_TIME,RND_DATE) VALUES('"
				+ id2
				+ "', "
				+ DotsGenerator.mdInt()
				+ ", null, null)"; 

		String insertSql3 = 
			"INSERT INTO BASIC3(ID_1,ID_2,RND_TIMESTAMP,RND_INT) VALUES('"
				+ id1
				+ "', '"
				+ id2
				+ "', null, "
				+ DotsGenerator.mdInt(0, 1000)
				+ ")"; 

		stmt.addBatch(insertSql1);
		stmt.addBatch(insertSql2);
		stmt.addBatch(insertSql3);
		int[] updateCounts = stmt.executeBatch();
		DotsConfig.INSERTCOUNT = DotsConfig.INSERTCOUNT + updateCounts.length;

		conn.commit();
		stmt.clearBatch();
		conn.setAutoCommit(true);
		stmt.close();
		stmt = null;
		return 0;
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT++;
		DotsLogging.logException("BTCJ3.populateTables() " + e);
		return -1;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ3.populateTables() " + t);
		return -2;
	}
}

/**
 * Insert one record to table basic1, basic2 and basic3 in a batch. The key's value
 * is decided by index.
 * @param index int.
 * @return int.
 */
public int populateTables(int index) {
	Statement stmt = null;
		
	try {
		conn.setAutoCommit(false);
		stmt = conn.createStatement();
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT++;
		DotsLogging.logException("BTCJ3.populateTables() " + e);
		return -2;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ3.populateTables() :" + t);
		return -2;
	}
	try{	
		String id1 = "ID1:" + index + ":" + identity;
		String id2 = "ID2:" + index + ":" + identity;
		String insertSql1 = 
			"INSERT INTO BASIC1(ID_1,RND_CHAR, RND_FLOAT) VALUES('"
				+ id1
				+ "', '"
				+ DotsGenerator.mdString(10)
				+ "', "
				+ DotsGenerator.mdPrice()
				+ ")"; 
		String insertSql2 = 
			"INSERT INTO BASIC2(ID_2,RND_INTEGER,RND_TIME,RND_DATE) VALUES('"
				+ id2
				+ "', "
				+ DotsGenerator.mdInt()
				+ ", null, null)"; 

		String insertSql3 = 
			"INSERT INTO BASIC3(ID_1,ID_2,RND_TIMESTAMP,RND_INT) VALUES('"
				+ id1
				+ "', '"
				+ id2
				+ "', null, "
				+ DotsGenerator.mdInt(0, 1000)
				+ ")"; 

		stmt.addBatch(insertSql1);
		stmt.addBatch(insertSql2);
		stmt.addBatch(insertSql3);
		int[] updateCounts = stmt.executeBatch();
		DotsConfig.INSERTCOUNT = DotsConfig.INSERTCOUNT + updateCounts.length;

		conn.commit();
		stmt.clearBatch();
		conn.setAutoCommit(true);
		stmt.close();
		stmt = null;
		return index;
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT++;
		DotsLogging.logException("BTCJ3.populateTables() " + e);
		return -1;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ3.populateTables() :" + t);
		return -2;
	}
}
/**
 * Delete record from all related tables in a batch.
 * @param index int.
 */
public int clearTables(int index) {
	Statement stmt = null;
	
	String id1 = "ID1:" + index + ":";
	String id2 = "ID2:" + index + ":";
	String deleteSql1 = 
		"delete from BASIC1 where ID_1 like '" + id1 + "%'"; 
	String deleteSql2 = 
		"delete from BASIC2 where ID_2 like '" + id2 + "%'"; 
	String deleteSql3 = 
		"delete from BASIC3 where (ID_1 like '" + id1 + "%') and (ID_2 like '" + id2 + "%')"; 

	try {
		conn.setAutoCommit(false);
		stmt = conn.createStatement();
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ3.clearTables() :" + e);
		return -2;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ3.clearTables() :" + t);
		return -2;
	}
	
	try{
		stmt.addBatch(deleteSql1);
		stmt.addBatch(deleteSql2);
		stmt.addBatch(deleteSql3);
		int[] updateCounts = stmt.executeBatch();
		DotsConfig.DELETECOUNT = DotsConfig.DELETECOUNT+ updateCounts.length;
		
		conn.commit();
		stmt.clearBatch();
		stmt.close();
		stmt = null;
		conn.setAutoCommit(true);
		return 0;
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ3.clearTables() :" + e);
		return -1;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ3.clearTables() :" + t);
		return -2;
	}
}

/**
 * Query table. Use join of table baisc1, basic2 and basic3.
 * At the same time, get result set meta data.
 */
public int queryTables() {
	Statement stmt = null;
	ResultSet rset = null;
	ResultSetMetaData rsmd = null;
	String selectSql = 
		"SELECT * FROM BASIC1,BASIC2,BASIC3 WHERE "
			+ "BASIC1.ID_1 = BASIC3.ID_1 AND BASIC2.ID_2 =BASIC3.ID_2 "
			+ " AND BASIC3.RND_INT>"
			+ DotsGenerator.mdInt(500, 1000)
			+ " ORDER BY BASIC3.RND_INT"; 
	int columnCount = 0;
	try {
		stmt = conn.createStatement();
		rset = stmt.executeQuery(selectSql);
		DotsConfig.QUERYCOUNT++;
		rsmd = rset.getMetaData();
		//columnCount = rsmd.getColumnCount();
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT++;
		DotsLogging.logException("BTCJ3.queryTables() : " + e);
		return -2;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ3.queryTables() :" + t);
		return -2;
	}
	try {
		
		//rsmd.getCatalogName(1);	Because sybase haven't finished it.
		if(DotsConfig.DRIVER_CLASS_NAME.indexOf("postgresql") == -1)
			rsmd.getColumnClassName(1);
		rsmd.getColumnDisplaySize(1);
		rsmd.getColumnLabel(1);
		rsmd.getColumnName(1);
		rsmd.getColumnType(1);
		rsmd.getColumnTypeName(1);
		rsmd.getPrecision(1);
		rsmd.getScale(1);
		//rsmd.getSchemaName(1);
		//rsmd.getTableName(1);
		rsmd.isCaseSensitive(1);
		rsmd.isCurrency(1);
		rsmd.isDefinitelyWritable(1);
		rsmd.isNullable(1);
		rsmd.isReadOnly(1);
		rsmd.isSearchable(1);
		rsmd.isSigned(1);
		rsmd.isWritable(1);
		columnCount = rsmd.getColumnCount();
		
		if (rset.next()) {
			for (int i = 1; i <= columnCount; i++) {
				switch (rsmd.getColumnType(i)) {
					case Types.CHAR :
						rset.getString(i);
						break;
					case Types.VARCHAR :
						rset.getString(i);
						break;
					case Types.LONGVARCHAR :
						rset.getString(i);
						break;
					case Types.NUMERIC :
						rset.getBigDecimal(i);
						break;
					case Types.DECIMAL :
						rset.getBigDecimal(i);
						break;
					case Types.SMALLINT :
						rset.getShort(i);
						break;
					case Types.INTEGER :
						rset.getInt(i);
						break;
					case Types.REAL :
						rset.getFloat(i);
						break;
					case Types.FLOAT :
						rset.getDouble(i);
						break;
					case Types.DOUBLE :
						rset.getDouble(i);
						break;
					case Types.DATE :
						rset.getDate(i);
						break;
					case Types.TIME :
						rset.getTime(i);
						break;
					case Types.TIMESTAMP :
						rset.getTimestamp(i);
						break;
					default :
						rset.getString(i);
				}
			}
		}
		rset.close();
		rset = null;
		stmt.close();
		stmt = null;
		return 0;
	} catch (SQLException e) {
		DotsConfig.FAILEDCOUNT++;
		DotsLogging.logException("BTCJ3.queryTables() :" + e);
		return -1;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ3.queryTables() :" + t);
		return -2;
	}
}

/**
 * Update table.
 */ 
public int updateTables() {
	Statement stmt = null;
	
	String updateSql1 = "";
	if(DotsConfig.DRIVER_CLASS_NAME.indexOf("mysql") == -1){
		updateSql1 = "UPDATE BASIC1 SET RND_FLOAT = "
			+ DotsGenerator.mdPrice()
			+ "  WHERE ID_1 in (SELECT ID_1 FROM BASIC3 WHERE RND_INT<"
			+ DotsGenerator.mdInt(0, 10)
			+ ")";
	}else{
		updateSql1 = "UPDATE BASIC1 SET BASIC1.RND_FLOAT ="
			+ DotsGenerator.mdPrice()
			+ " WHERE ID_1 LIKE 'ID1:"
			+ DotsGenerator.mdInt(0, 2000)
			+ ":'";
	}
	
	try {
		conn.setAutoCommit(false);
		stmt = conn.createStatement();
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT++;
		DotsLogging.logException("BTCJ3.updateTables() " + e);
		return -1;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ3.updateTables() :" + t);
		return -2;
	}
	
	try{
		stmt.addBatch(updateSql1);
		int[] updateCounts = stmt.executeBatch();
		DotsConfig.UPDATECOUNT = DotsConfig.UPDATECOUNT + updateCounts.length;
		
		conn.commit();
		stmt.clearBatch();
		conn.setAutoCommit(true);
		stmt.close();
		stmt = null;
		return 0;
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT++;
		DotsLogging.logException("BTCJ3.updateTables() " + e);
		return -1;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ3.updateTables() :" + t);
		return -2;
	}
}}
