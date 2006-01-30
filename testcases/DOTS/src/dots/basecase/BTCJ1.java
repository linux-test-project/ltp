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
import java.util.*;
import dots.framework.*;

/**
 * The main function of BTCJ1 is to get database meta data. 
 */
 
public class BTCJ1 implements Runnable {
	/**Driver*/
	private Driver d;
	/**Database connection*/
	private Connection conn = null;	
	/**SQL, count records in the table basic1*/
	private final String countSql = "SELECT COUNT(*) FROM BASIC1";	
	/**The integer identify this thread */ 
	private int identity = 0;

/**
 * BTCJ1 constructor. Generate identity and receive database connection from 
 * framework.
 */
public BTCJ1(Connection con) {
	super();
	identity = DotsGenerator.mdInt(0, 1000);
	this.conn = con;
}


/**
 * Deregister all loaded drivers
 * @param en Enumeration
 */ 
public void deRegisterDrivers(Enumeration en) {
	try {
		while (en.hasMoreElements()) {
			d = (Driver) en.nextElement();
			DriverManager.deregisterDriver(d);			
		}
		
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ1.deRegisterDriver():" + e);
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ1.deRegisterDriver() :" + t);
		
	}
}



/**
 * Get information about drivers.
 * @param en Enumeration
 */ 
public void getDriverInfo(Enumeration en) {
	int length;
	Properties prop = new Properties();
	String tmp;
	prop.setProperty("user", DotsConfig.DB_UID);
	prop.setProperty("password", DotsConfig.DB_PASSWD);
	try {
		while (en.hasMoreElements()) {
			d = (Driver) en.nextElement();
			if (d.acceptsURL(DotsConfig.URL)) {
				d.getMajorVersion();
				d.getMinorVersion();
				d.jdbcCompliant();
				String name = d.getClass().getName();
				length = d.getPropertyInfo(DotsConfig.URL, prop).length;
				DriverPropertyInfo[] driverInfo = new DriverPropertyInfo[length];
				driverInfo = d.getPropertyInfo(DotsConfig.URL, prop);
				for (int i = 0; i < length; i++) {
					tmp = driverInfo[i].description;
					tmp = driverInfo[i].name;
					tmp = driverInfo[i].value;
				}
				driverInfo = null;
			}
		}
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ1.getDriverInfo(): " + e);
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ1.getDriverInfo(): " + t);
	} 
}

/**
 * Get all registered dirvers.
 * @return Enumeration
 */
public Enumeration getDrivers() {
	Enumeration en = null;
	int timeout;
	StringBuffer sb = new StringBuffer();
	try {
		en = DriverManager.getDrivers();
		while (en.hasMoreElements()) {
			d = (Driver) en.nextElement();
			sb.append(d.getClass().getName());
		}
		timeout = DriverManager.getLoginTimeout();
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ1.getDrivers(): " + e);
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ1.getDrivers(): " + t);
	} 
	return en;
}



/**
 * run() method of thread.
 */ 
public void run() {
	Enumeration en = null;


	en = getDrivers();
	getDriverInfo(en);
	int succeed = -1;

	int count = countRecords();
	if (count == -2)	return;
	
	// If records in the table BASIC1 are less than 10000 and Dots is not terminated, 
	// insert records into table BASIC1,BAISC2 and BASIC3.
	while ((count < 10000) && (count != -1) && (!DotsConfig.TERMINATION)) {
		for (int i = 0; i < 100; i++) {
			succeed = populateTables();
			if (succeed == -2)	return;
		}
		count = countRecords();
		if (count == -2)	return;
	}

	// Get database meta data and retrieve data from a specific table circularly 
	// until Dots is terminated.
	
	do {
		succeed = getDBInfo(DotsGenerator.mdInt(1, 14));
		if (succeed == -2)	return;
		succeed = queryTableInfo("BASIC" + DotsGenerator.mdInt(1, 4));
		if (succeed == -2)	return;
		System.gc();
	} while (!DotsConfig.TERMINATION );
	
	try {
		if(conn != null){
			conn.close();
			conn = null;
			System.gc();
		}
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ1.run() :" + e);
	}
	deRegisterDrivers(en);
	en = null;	
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
		DotsLogging.logException("BTCJ1.countRecords() :" + e);
		return -2;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ1.countRecords() :" + t);
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
		DotsLogging.logException("BTCJ1.countRecords() :" + e);
		return -1;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ1.countRecords() :" + t);
		return -2;
	}
}

/**
 * Insert one record to table basic1, basic2 and basic3. The key's value
 * is incremental.
 * @return int.
 */ 
public int populateTables() {
	Statement stmt = null;
	ResultSet rset = null;
	int count = 0;

	try {
		stmt = conn.createStatement();
	}catch (Exception e){
		DotsConfig.FAILEDCOUNT++;
		DotsLogging.logException("BTCJ1.populateTables() :" + e);
		return -2;
	}catch (Throwable t) {
		DotsLogging.logException("BTCJ1.populateTables(): " + t);
		return -2;
	} 

	try{
		rset = stmt.executeQuery(countSql);
		DotsConfig.QUERYCOUNT++;

		if (rset.next())
			count = rset.getInt(1);
		rset.close();
		rset = null;

		String id1 = "ID1:" + count + ":" + identity;
		String insertSql1 = 
			"INSERT INTO BASIC1(ID_1,RND_CHAR, RND_FLOAT) VALUES('"
				+ id1
				+ "', '"
				+ DotsGenerator.mdString(10)
				+ "', "
				+ DotsGenerator.mdPrice()
				+ ")"; 

		stmt.executeUpdate(insertSql1);
		DotsConfig.INSERTCOUNT++;

		String id2 = "ID2:" + count + ":" + identity;
		String insertSql2 = 
			"INSERT INTO BASIC2(ID_2,RND_INTEGER,RND_TIME,RND_DATE) VALUES('"
				+ id2
				+ "', "
				+ DotsGenerator.mdInt()
				+ ", null, null)"; 
		stmt.executeUpdate(insertSql2);
		DotsConfig.INSERTCOUNT++;

		String insertSql3 = 
			"INSERT INTO BASIC3(ID_1,ID_2,RND_TIMESTAMP,RND_INT) VALUES('"
				+ id1
				+ "', '"
				+ id2
				+ "', null ,"
				+ DotsGenerator.mdInt(0, 1000)
				+ ")"; 
		stmt.executeUpdate(insertSql3);
		DotsConfig.INSERTCOUNT++;

		stmt.close();
		stmt = null;
		return 0;
		
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT++;
		DotsLogging.logException("BTCJ1.populateTables() :" + e);
		return -1;
	}catch (Throwable t) {
		DotsLogging.logException("BTCJ1.populateTables(): " + t);
		return -2;
	} 
}
/**
 * Get database's comprehensive information.
 * @param choice int
 */
public int getDBInfo(int choice) {
	DatabaseMetaData dbInfo = null;
	SQLWarning warning = null;
	
	try {
		if (conn == null)
			conn = DriverManager.getConnection(DotsConfig.URL, DotsConfig.DB_UID, DotsConfig.DB_PASSWD); 
	} catch (Exception e) {
		e.printStackTrace();
		DotsConfig.FAILEDCOUNT++;
		DotsLogging.logException("BTCJ1.getDBInfo(): " + e);
		return -2;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ1.getDBInfo(): " + t);
		return -2;
	} 
	
	try {
		dbInfo = conn.getMetaData();

		switch (choice) {
			case 1 :
				dbInfo.allProceduresAreCallable();
				dbInfo.allTablesAreSelectable();
				dbInfo.dataDefinitionCausesTransactionCommit();
				dbInfo.doesMaxRowSizeIncludeBlobs();
				dbInfo.getCatalogSeparator();
				dbInfo.getCatalogTerm();
				DotsConfig.QUERYCOUNT = DotsConfig.QUERYCOUNT + 6;
				break;
			case 2 :
				dbInfo.getDatabaseProductName();
				dbInfo.getDatabaseProductVersion();
				dbInfo.getDefaultTransactionIsolation();
				dbInfo.getDriverMajorVersion();
				dbInfo.getDriverMinorVersion();
				dbInfo.getDriverName();
				dbInfo.getDriverVersion();
				dbInfo.getExtraNameCharacters();
				dbInfo.getIdentifierQuoteString();
				DotsConfig.QUERYCOUNT = DotsConfig.QUERYCOUNT + 9;
				break;
			case 3 :
				dbInfo.getMaxBinaryLiteralLength();
				dbInfo.getMaxCatalogNameLength();
				dbInfo.getMaxCharLiteralLength();
				dbInfo.getMaxColumnNameLength();
				dbInfo.getMaxColumnsInGroupBy();
				dbInfo.getMaxColumnsInIndex();
				dbInfo.getMaxColumnsInOrderBy();
				dbInfo.getMaxColumnsInSelect();
				DotsConfig.QUERYCOUNT = DotsConfig.QUERYCOUNT + 8;
				break;
			case 4 :
				dbInfo.getMaxColumnsInTable();
				dbInfo.getMaxConnections();
				dbInfo.getMaxCursorNameLength();
				dbInfo.getMaxIndexLength();
				dbInfo.getMaxProcedureNameLength();
				dbInfo.getMaxRowSize();
				dbInfo.getMaxSchemaNameLength();
				dbInfo.getMaxStatementLength();
				dbInfo.getMaxStatements();
				dbInfo.getMaxTableNameLength();
				dbInfo.getMaxTablesInSelect();
				dbInfo.getMaxUserNameLength();
				DotsConfig.QUERYCOUNT = DotsConfig.QUERYCOUNT + 12;
				break;
			case 5 :
				dbInfo.getNumericFunctions();
				dbInfo.getProcedureTerm();
				dbInfo.getSQLKeywords();
				dbInfo.getSchemaTerm();
				dbInfo.getSearchStringEscape();
				dbInfo.getStringFunctions();
				dbInfo.getSystemFunctions();
				dbInfo.getTimeDateFunctions();
				dbInfo.getURL();
				dbInfo.getUserName();
				DotsConfig.QUERYCOUNT = DotsConfig.QUERYCOUNT + 10;
				break;
			case 6 :
				dbInfo.isCatalogAtStart();
				dbInfo.isReadOnly();
				dbInfo.nullPlusNonNullIsNull();
				dbInfo.nullsAreSortedAtEnd();
				dbInfo.nullsAreSortedAtStart();
				dbInfo.nullsAreSortedHigh();
				dbInfo.nullsAreSortedLow();
				dbInfo.storesLowerCaseIdentifiers();
				dbInfo.storesLowerCaseQuotedIdentifiers();
				dbInfo.storesMixedCaseIdentifiers();
				dbInfo.storesMixedCaseQuotedIdentifiers();
				dbInfo.storesUpperCaseIdentifiers();
				dbInfo.storesUpperCaseQuotedIdentifiers();
				DotsConfig.QUERYCOUNT = DotsConfig.QUERYCOUNT + 13;
				break;
			case 7 :
				dbInfo.supportsANSI92EntryLevelSQL();
				dbInfo.supportsANSI92FullSQL();
				dbInfo.supportsANSI92IntermediateSQL();
				dbInfo.supportsAlterTableWithAddColumn();
				dbInfo.supportsAlterTableWithDropColumn();
				dbInfo.supportsBatchUpdates();
				dbInfo.supportsCatalogsInDataManipulation();
				dbInfo.supportsCatalogsInIndexDefinitions();
				dbInfo.supportsCatalogsInPrivilegeDefinitions();
				dbInfo.supportsCatalogsInProcedureCalls();
				dbInfo.supportsCatalogsInTableDefinitions();
				dbInfo.supportsColumnAliasing();
				dbInfo.supportsConvert();
				dbInfo.supportsCoreSQLGrammar();
				dbInfo.supportsCorrelatedSubqueries();
				DotsConfig.QUERYCOUNT = DotsConfig.QUERYCOUNT + 15;
				break;
			case 8 :
				dbInfo.supportsDataDefinitionAndDataManipulationTransactions();
				dbInfo.supportsDataManipulationTransactionsOnly();
				dbInfo.supportsDifferentTableCorrelationNames();
				dbInfo.supportsExpressionsInOrderBy();
				dbInfo.supportsExtendedSQLGrammar();
				dbInfo.supportsFullOuterJoins();
				dbInfo.supportsGroupBy();
				dbInfo.supportsGroupByBeyondSelect();
				dbInfo.supportsGroupByUnrelated();
				dbInfo.supportsIntegrityEnhancementFacility();
				dbInfo.supportsLikeEscapeClause();
				DotsConfig.QUERYCOUNT = DotsConfig.QUERYCOUNT + 11;
				break;
			case 9 :
				dbInfo.supportsLimitedOuterJoins();
				dbInfo.supportsMinimumSQLGrammar();
				dbInfo.supportsMixedCaseIdentifiers();
				dbInfo.supportsMixedCaseQuotedIdentifiers();
				dbInfo.supportsMultipleResultSets();
				dbInfo.supportsMultipleTransactions();
				dbInfo.supportsNonNullableColumns();
				dbInfo.supportsOpenCursorsAcrossCommit();
				dbInfo.supportsOpenCursorsAcrossRollback();
				dbInfo.supportsOpenStatementsAcrossCommit();
				dbInfo.supportsOpenStatementsAcrossRollback();
				dbInfo.supportsOrderByUnrelated();
				DotsConfig.QUERYCOUNT = DotsConfig.QUERYCOUNT + 12;
				break;
			case 10 :
				dbInfo.supportsOuterJoins();
				dbInfo.supportsPositionedDelete();
				dbInfo.supportsPositionedUpdate();
				dbInfo.supportsSchemasInDataManipulation();
				dbInfo.supportsSchemasInIndexDefinitions();
				dbInfo.supportsSchemasInPrivilegeDefinitions();
				dbInfo.supportsSchemasInProcedureCalls();
				dbInfo.supportsSchemasInTableDefinitions();
				dbInfo.supportsSelectForUpdate();
				dbInfo.supportsStoredProcedures();
				dbInfo.supportsSubqueriesInComparisons();
				DotsConfig.QUERYCOUNT = DotsConfig.QUERYCOUNT + 11;
				break;
			case 11 :
				dbInfo.supportsSubqueriesInExists();
				dbInfo.supportsSubqueriesInIns();
				dbInfo.supportsSubqueriesInQuantifieds();
				dbInfo.supportsTableCorrelationNames();
				dbInfo.supportsTransactions();
				dbInfo.supportsUnion();
				dbInfo.supportsUnionAll();
				dbInfo.usesLocalFilePerTable();
				dbInfo.usesLocalFiles();
				DotsConfig.QUERYCOUNT = DotsConfig.QUERYCOUNT + 9;
				break;
			case 12 :
				dbInfo.getCatalogs();
				dbInfo.getSchemas();
				dbInfo.getTypeInfo();
				dbInfo.getTableTypes();
				dbInfo.getProcedures(null, null, null);
				DotsConfig.QUERYCOUNT = DotsConfig.QUERYCOUNT + 5;
				break;
			case 13 :
				warning = conn.getWarnings();
				while (warning != null) {
					warning = warning.getNextWarning();
				}
				DotsConfig.QUERYCOUNT = DotsConfig.QUERYCOUNT + 1;
		}
		dbInfo = null;
		conn.close();
		conn = null;
		return 0;
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT++;
		DotsLogging.logException("BTCJ1.getDBInfo(): 3" + e);
		return -1;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ1.getDBInfo(): 4" + t);
		return -2;
	} 
}

/**
 * Get table's information.
 * @param tableName String
 */ 
public int queryTableInfo(String tableName) {
	
	ResultSet rset = null;
	Statement stmt = null;
	DatabaseMetaData dbInfo = null;
	SQLWarning warning = null;
	String columnName = null;
	
	try {
		if (conn==null)
			conn = DriverManager.getConnection(DotsConfig.URL, DotsConfig.DB_UID, DotsConfig.DB_PASSWD);
		dbInfo = conn.getMetaData();
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ1.queryTableInfo(): " + e);
		return -2;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ1.queryTableInfo(): " + t);
		return -2;
	}
	
	try{
		
		dbInfo.getTables(null, null, tableName, null);
		dbInfo.getPrimaryKeys(null, null, tableName);
		dbInfo.getImportedKeys(null, null, tableName);
		dbInfo.getExportedKeys(null, null, tableName);
		rset = dbInfo.getColumns(null, null, tableName, null);	//Get columns from specific tables.
		//rset = dbInfo.getColumnPrivileges(null, null, tableName, null);
		DotsConfig.QUERYCOUNT = DotsConfig.QUERYCOUNT + 7;
		dbInfo = null;
		//Query data from specific table.
		if (rset.next()) {
			columnName = rset.getString("COLUMN_NAME");
			rset.close();
			rset = null;
			stmt = conn.createStatement();
			stmt.executeQuery("SELECT * FROM " + tableName + "  ORDER BY " + columnName);
			DotsConfig.QUERYCOUNT++;
			stmt.close();
			stmt = null;
		}
			
		conn.close();
		conn = null;
		System.gc();
		return 0;
		
	} catch (Exception e) {
		DotsConfig.FAILEDCOUNT ++;
		DotsLogging.logException("BTCJ1.queryTableInfo(): " + e);
		return -1;
	} catch (Throwable t) {
		DotsLogging.logException("BTCJ1.queryTableInfo(): " + t);
		return -2;
	} 
}}
