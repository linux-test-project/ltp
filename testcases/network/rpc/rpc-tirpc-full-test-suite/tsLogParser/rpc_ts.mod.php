<?php
/*
 * Copyright (c) 2007, Bull S.A..  All rights reserved.
 * Module created by: Cyril Lacabanne
 * Based on OPTS Module from Sebastien Decugis
 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
 */

 /* This module is for use with the Linux RPC & TI-RPC Test Suite.
   Based on code from earlier TSLogParser releases. */
   
   
class textUtils {
/*
  This function return a number corresponding to test name :
  
  test name          value returned
  basic				1
  stress			2
  limits			3
  mt				4
  scalability		5
  dataint			6
  perf				7
  complex			8
  
  if value return is 0 : testname is wrong
*/
	function testName_ToInt($testname)
	{		
		if ($testname == "basic")
		{
			return (1);
		}
		if ($testname == "stress")
		{
			return (2);
		}
		if ($testname == "limits")
		{
			return (3);
		}
		if ($testname == "mt")
		{
			return (4);
		}
		if ($testname == "scalability")
		{
			return (5);
		}
		if ($testname == "dataint")
		{
			return (6);
		}
		if ($testname == "perf")
		{
			return (7);
		}
		if ($testname == "complex")
		{
			return (8);
		}
		return (0);/**/
	}
}
   

/* The following class contains the routines for the XML parser used later */
class rpc_ts_xml_routines {
	
	var $assertion;
	var $xml_parser;
	var $debug=0;
	
	/* The functions below are used within the XML parser -- see PHP doc for more info */
	function startElement($parser, $name, $attrs)
	{
		if ($this->debug)
			echo "startElement($name)\n";
		if ($name == "ASSERTION")
		{
			$this->assertion["cur"]=$attrs["ID"];
			$this->assertion[$attrs["ID"]]="";
		}
	}
	
	function endElement($parser, $name)
	{
		if ($this->debug)
			echo "endElement($name)\n";
		if ($name == "ASSERTION")
			$this->assertion["cur"]=-1;
	}
	
	function characterData($parser, $data)
	{
		if ($this->debug)
			echo $data;
		if (($this->assertion["cur"] != -1) && (trim($data)))
			$this->assertion[$this->assertion["cur"]] .= $data."\n";
	}
	
	/* This function is called for each assertions.xml file */
	function parse_assertions($file)
	{
		/* Open the file for reading */
		if ($this->debug)
			echo "Opening <i>$file</i>\n";
		if (!($fp = fopen($file, "r"))) 
		{
		   die("could not open XML input");
		}
		
		/* Create the XML parser */
		$this->assertion = array("cur"=>-1);
		$this->xml_parser = xml_parser_create();
		xml_set_object($this->xml_parser, $this);
		xml_parser_set_option($this->xml_parser, XML_OPTION_CASE_FOLDING, true);
		xml_set_element_handler($this->xml_parser, "startElement", "endElement");
		xml_set_character_data_handler($this->xml_parser, "characterData");
		
		/* Parse the file */
		while ($data = fread($fp, 4096)) {
		   if ($this->debug)
			echo "Raw:<hr>".htmlentities($data)."<hr>\n";
		   if (!xml_parse($this->xml_parser, $data, feof($fp))) {
		       die(sprintf("XML error: %s at line %d<br>\nin ".$file,
				   xml_error_string(xml_get_error_code($this->xml_parser)),
				   xml_get_current_line_number($this->xml_parser)));
		   }
		}
		
		/* Clean up the XML parser */
		xml_parser_free($this->xml_parser);
		unset($this->assertion["cur"]);
		
		/* return */
		return $this->assertion;
	}
}

class rpc_ts {
/*
 module_info will return an HTML-formated text (enclosed in <p> and </p> tags)
  describing the module and the testsuite it supports.
  All information related to the module (version, known bugs, ...) are suitable 
  for this function (think of it as the only documentation for the module).
*/
	function module_info($what="")
	{
		$urlInfo = "http://"."/";
		$moduleVers = "0.3 BETA";
		$moduleDate = "2007-05-09";/**/
		
		$title = "<b>Linux RPC & TI-RPC</b> parser module for <b>TSLogParser</b>";
		
		$text = "<p>$title</p>\n";
		$text.= "<p>Release: <b>".$moduleVers."</b> ".$moduleDate."</p>\n";
		$text.= "<p>History: \n";
		$text.= "<ul>\n<li>This is first stable release of that module\n";
		$text.= "</li></ul></p>\n";
		$text.= "<p>See the <a href='".$urlInfo."'>homepage</a> for more information.</p>\n";
		
		if ($what == "title")
			return $title;

		return $text;
	}
	
/*	
 TS_parse will check for the directory TS_path and analyse its content.
  In case a correct testsuite structure is found, the testsuite is parsed
  and put into the database with name and description as provided.
  The return value is $true if success and $false otherwise.
*/	
	function TS_parse(&$parent, $TS_name, $TS_description, $path)
	{
		if ( $parent->debug )
			echo "rpc_ts->TS_parse($TS_name, $TS_description, $path)\n";
			
		$text_utils = new textUtils();
		
		echo "NAME : ".$TS_name."<br />";
		
		$xmlparser = new rpc_ts_xml_routines();
		
		$opts_tree=array();
		
		//$regexp_testcase="^([0-9]*)-(\w*)\.(bin|sh)$";
		$regexp_testcase="^([0-9]*)-(.*)\.(bin)$";
	
		/* Check the directory contains a coherent structure */
		if ((!is_dir($path)) || (!is_dir($path."/rpc_suite")))
		{
			$parent->last_error="Directory '$path' does not contain a valid source tree -- check your archive format.\n";
			return FALSE;
		}
		
		/* Open and browse the tree */
		$dh  = opendir($path."/rpc_suite/");
		if (!$dh)
		{
			$parent->last_error="Failed to open directory $path/rpc_suite/ for reading.\n";
			return FALSE;
		}
		
		while (($file = readdir($dh)) !== false) 
		{
			if (($file == ".") || ($file == "..") || ($file == "CVS"))
				continue;
		
			if (is_dir($path."/rpc_suite/".$file))
			{
				$dh2 =  opendir($path."/rpc_suite/".$file);
				if (!$dh2)
				{
					$parent->last_error= "Failed to open directory $path/rpc_suite/$file for reading.\n";
					return FALSE;
				}
				
				while (($file2 = readdir($dh2)) !== false) 
				{
					if (($file2 == ".") || ($file2 == "..") || ($file2 == "CVS"))
						continue;
					
					$file2array=array($file2);
					
					/* Special case: headers sys/mman.h etc... */
					if (($file == "definitions") && ($file2 == "sys"))
					{
						$dh2b = opendir($path."/rpc_suite/definitions/sys");
						if (!$dh2b)
						{
							$parent->last_error= "Failed to open dir $path/rpc_suite/definitions/sys for reading.\n";
							return FALSE;
						}
						$file2array=array();
						while (($file2b = readdir($dh2b)) !== false) 
						{
							if (($file2b == ".") || ($file2b == "..") || ($file2b == "CVS"))
								continue;
							$file2array[]="sys/".$file2b;
						}
						closedir($dh2b);
					}
			
					
					foreach ($file2array as $file2)
					{
						if (is_dir($path."/rpc_suite/".$file."/".$file2))
						{
							$dh3 =  opendir($path."/rpc_suite/".$file."/".$file2);
							if (!$dh3)
							{
								$parent->last_error= "Failed to open directory $path/rpc_suite/$file/$file2 for reading.\n";
								return FALSE;
							}
							
							$assertion_file = 0;
							
							while (($file3 = readdir($dh3)) !== false) 
							{
								if (($file3 == ".") || ($file3 == "..") || ($file3 == "CVS"))
									continue;
								
								/* We're looking for "assertions.xml" files */
								if ($file3 == "assertions.xml")
								{
									$assertion_file = 1;
									continue;
								}
								
								/* We also keep track of every testcase file */
								if (ereg($regexp_testcase, $file3, $regs))
								{
									$num = $text_utils->testName_ToInt($regs[2]);
									$opts_tree[$file][$file2]["testcase"][$regs[1]][$num]=$regs[1]."-".$regs[2].".".$regs[3];
								}
								
								/* Last but not least, we want the speculative tests in the database 
								if ($file3 == "speculative")
								{
									$dh4 = opendir($path."/rpc_suite/".$file."/".$file2."/".$file3);
									if (!$dh4)
									{
										$parent->last_error= "Failed to open directory $path/rpc_suite/$file/$file2/speculative for reading.\n";
										return FALSE;
									}
									while (($file4 = readdir($dh4)) !== false) 
									{
										if (($file4 == ".") || ($file4 == "..") || ($file4 == "CVS"))
											continue;
										if (ereg($regexp_testcase, $file4, $regs))
											$opts_tree[$file][$file2]["testcase"][$regs[1]][(int)$regs[2]]="speculative/".$regs[1]."-".$regs[2].".".$regs[3];
									}
									closedir($dh4);
								}*/
							}
							closedir($dh3);
							
							/* We now parse the assertions */
							if ($assertion_file)
							{
								$opts_tree[$file][$file2]["assertions"]=$xmlparser->parse_assertions($path."/rpc_suite/".$file."/".$file2."/assertions.xml");
							}
						}	
					}	
				}
				closedir ($dh2);
			}	
		}
		closedir($dh);
		
		/* We've parsed the whole tree */
		if ($parent->debug > 1)
			print_r($opts_tree);
		
		/* The database shall be initialized here */
		if (!is_db_init())
		{
			$parent->last_error="Database was not initialized\n";
			return FALSE;
		}
		
		/* Check no release with the same name already exist */
		$releases=query_version($TS_name, 1);
		if ($releases)
		{
			$parent->last_error= "The release '$TS_name' is already in the database \n".
				"<i>".stringFromDB($releases[$TS_name]["ver_comment"])."</i>\n";
			return FALSE;
		}
		
		/* Now, compare the $opts_tree with the $current_asserts and build up the list of assertions
		   to be added to the database.*/

   		$current_routines=query_routines();

		/* We start with looking for missing routines */
		$missing_routines=array();

		/* browse the new release assertions */
		foreach ($opts_tree as $domain)
		{
			foreach ($domain as $routine=>$asserts)
			{
				/* If the routine name is missing from opts_routines table, we'll add it */
				if (!isset($current_routines[$routine]))
					$missing_routines[]=$routine;
			}
		}
		
		if ($parent->debug > 1)
			print_r($missing_routines);
		
		/* If any routine is missing, it must be added previously to further processing */
		if ($missing_routines)
		{
			echo "New routines are being added to the database...\n";
			$counter=0;
			foreach ($missing_routines as $routine)
			{
/**** MODIF ****/
				$sql = "INSERT INTO opts_routines ( rou_name, rou_comment ) "
					."VALUES ( ".stringToDB($routine).","
					.stringToDB("Added on ".date("F j, Y")." with release '$TS_name'")." )";
				if ($parent->debug > 1)
					echo htmlentities($sql)."<br>\n";
				if (db_execute_insert($sql))
					$counter++;
				else
					echo "Failed to add $routine to the database...\n";
			}
			echo "Done. <b>$counter</b> routine have been added.\n\n";
			$current_routines=query_routines();
		}
		
		$current_asserts=query_all_asserts();
		$missing_assertions=array();
		
		
		/* browse the new release assertions */
		foreach ($opts_tree as $domain)
		{
			foreach ($domain as $routine=>$asserts)
			{
				/* Check if the routine is already in the database */
				if (!isset($current_asserts[$routine]))
				{
					if (!isset($current_routines[$routine]))
					{
						$parent->last_error="Internal script error: routine $routine was not added in 1st pass";
						return FALSE;
					}
					
					/* We now schedule addition of the assertions for this routine, as none was already defined */
					foreach ($asserts["assertions"] as $id => $assert)
					{
						
						$missing_assertions[]=array(
							"routine"=>$routine,
							"assert"=>$assert,
							"oldid"=>$id);
					}
				}
				else
				{
					foreach ($asserts["assertions"] as $id => $assert)
					{
						/* Check if this assertion text was already in the database */
						if(!in_array($assert, $current_asserts[$routine]))
							$missing_assertions[]=array(
								"routine"=>$routine,
								"assert"=>$assert,
								"oldid"=>$id);
					}
				}
			}
		}
		if ($parent->debug > 1)
			print_r($missing_assertions);
		
		/* If any assertion is missing, it must be added previously to further processing */
		if ($missing_assertions)
		{
			echo "New assertions are being added to the database...\n";
			$counter=0;
			foreach ($missing_assertions as $assertion)
			{
/**** MODIF ****/
				$sql = "INSERT INTO opts_assertions ( assert_routine, assert_text ) "
					."VALUES ( ".$current_routines[$assertion["routine"]]["routine_id"].","
					.stringToDB($assertion["assert"])." )";
				if ($parent->debug > 1)
					echo htmlentities($sql)."<br>\n";
				if (db_execute_insert($sql))
					$counter++;
				else
					echo "Failed to add assertion ".$assertion["oldid"]." of ".$assertion["routine"]." to the database...\n";
			}
			echo "Done. <b>$counter</b> assertions have been added.\n\n";
			$current_asserts=query_all_asserts();
		}
		
		/* OK, we can now create the new release of OPTS in the database */
/**** MODIF ****/
		$sql="INSERT INTO opts_versions (ver_name, ver_comment, ver_module) "
			. "VALUES ( ".stringToDB($TS_name).", "
			. stringToDB($TS_description).", "
			. "'rpc_ts' )";
		if ($parent->debug > 1)
			echo htmlentities($sql)."<br>\n";
		if (!db_execute_insert($sql))
		{
			$parent->last_error= "Failed to insert new version in the database\n";
			return FALSE;
		}
		
		/* We retrieve the new release uniqueID */
		$releases=query_version($TS_name, 1);
		
		if (!$releases)
		{
			$parent->last_error= "Internal error: the new OPTS version was not created\n";
			return FALSE;
		}
		if ($parent->debug > 1)
			print_r($current_asserts);
		
		/* We can create the full release description */
		$release_description = array();
		$missing_test=0;
		foreach ($opts_tree as $domain)
		{
			foreach ($domain as $routine=>$asserts)
			{
				if (!isset($current_asserts[$routine]) || !isset($current_routines[$routine]))
				{
					
					$parent->last_error= "Internal script error: routine $routine was not added in 1st pass";
					return FALSE;
				}
					
				/* We now schedule addition of the assertions for this routine, as none was already defined */
				foreach ($asserts["assertions"] as $id => $assert)
				{
					if (!isset($asserts["testcase"][$id]))
						$missing_test++;
					else
					{
						foreach ($asserts["testcase"][$id] as $number => $infos)
						{
							$release_description[]=array(
								"descr_assert" => array_search($assert, $current_asserts[$routine]),
								"descr_num_assert" => $id,
								"descr_num_test" => $number,
								"descr_info" => $infos);
						}
						unset($asserts["testcase"][$id]);
					}
				}
				if (isset($asserts["testcase"]))
					foreach($asserts["testcase"] as $id => $tcinfos)
						echo "<b>Warning</b>, $routine's test $id-* has no matching assertions and therefore will be ignored.\n";
			}
		}
		
		if ($missing_test)
			echo "\n<i>Info:</i> $missing_test assertions are not tested.\n\n";
		
		/* We've enough information now; we can create the release */
		reset($releases);
		$rlstmp=current($releases);
		$release_id=$rlstmp["ver_id"];
		
		$counter=0;
		
		foreach ($release_description as $testcase)
		{
/**** MODIF ****/			
			$sql = "INSERT INTO opts_version_descriptions "
				." (descr_version, descr_assert, descr_num_assert, descr_num_test, descr_info)"
				." VALUES (".$release_id.", "
					    .$testcase["descr_assert"].", "
					    .$testcase["descr_num_assert"].", "
					    .$testcase["descr_num_test"].", "
					    .stringToDB($testcase["descr_info"])." )";
			if ($parent->debug > 1)
				echo htmlentities($sql)."<br>\n";
			if (db_execute_insert($sql))
				$counter++;
			else
				echo "Failed to execute: ".htmlentities($sql)."\n";
		}
		
		echo "<b><i>$counter testcases have been added</i></b>\n\n";
		echo "Process terminated.\n"; 
		
		return TRUE;
	}
	
	
	function TS_delete(&$parent, $TS_id)
	{
		
		if ( $parent->debug )
			echo "opts->TS_delete($TS_id)\n";
		
		/* Check there is no run within this testsuite */
/**** MODIF ****/
		$sql = "SELECT * from opts_run_results, opts_version_descriptions"
			." WHERE res_testcase=descr_id"
			." AND descr_version=".$TS_id;
		if ($parent->debug > 1)
			echo htmlentities($sql)."<br>\n";
		$tmp = db_execute_select($sql);
		if ($tmp)
		{
			$parent->last_error="The testsuite contains runs -- cannot be deleted.\n Delete the runs first.\n";
			return FALSE;
		}
		
		/* Check the testsuite is an OPTS one */
/**** MODIF ****/
		$sql = "SELECT ver_module from opts_versions"
			." WHERE ver_id=".$TS_id;
		if ($parent->debug > 1)
			echo htmlentities($sql)."<br>\n";
		$tmp = db_execute_select($sql);
		if (!$tmp)
		{
			$parent->last_error="The testsuite cannot be found in the database.\n";
			return FALSE;
		}
		if ($tmp[0]["ver_module"] != "rpc_ts")
		{
			$parent->last_error="The testsuite is not an RPC_TS -- cannot be deleted within the current module.\n";
			return FALSE;
		}
		
		/* Now, delete the testsuite description */
/**** MODIF ****/
		$sql = "DELETE from opts_version_descriptions"
			." WHERE descr_version=".$TS_id;
		
		if ($parent->debug > 1)
			echo htmlentities($sql)."<br>\n";
		$tmp = db_execute_insert($sql);
/**** MODIF ****/
		echo "$tmp rows deleted from opts_version_descriptions<br>\n";
		
		/* and the testsuite name */
/**** MODIF ****/
		$sql = "DELETE from opts_versions"
			." WHERE ver_id=".$TS_id;
		
		if ($parent->debug > 1)
			echo htmlentities($sql)."<br>\n";
		$tmp = db_execute_insert($sql);
		if ($tmp == 0)
		{
			$parent->last_error="No row deleted in opts_version\n";
			return FALSE;
		}
		if ($parent->debug > 1)
			echo "$tmp rows deleted from opts_version<br>\n";
		
		return true;
	}
	
	
	function RUN_parse(&$parent, $RUN_name, $RUN_description, $TS_id, &$CONTENT)
	{
		if ( $parent->debug )
			echo "opts->RUN_parse($RUN_name, $RUN_description, $TS_id, ...".strlen($CONTENT)."c...)\n";
		
		/* Check this TS id first */
		$sql = "SELECT ver_id, ver_name, ver_comment, ver_module FROM opts_versions WHERE ver_id=$TS_id";
		if ($parent->debug > 1)
			echo htmlentities($sql)."<br>\n";		
		$release = db_execute_select($sql);
		if (!$release)
		{
			$parent->last_error="The provided testsuite ID was not found in database\n";
			return false;
		}
		if ($release[0]["ver_module"] != "rpc_ts")
		{
			$parent->last_error="This testsuite's ID is not of type Linux RPC & TIRPC Test Suite. Aborted.\n";
			return false;
		}
		
		
		/* Check that run name is free */
		$sql = "SELECT run_id, run_name, run_comments FROM opts_run WHERE run_name LIKE ".stringToDB($RUN_name);
		if ($parent->debug > 1)
			echo htmlentities($sql)."<br>\n";		
		$res = db_execute_select($sql);
		if ($res)
		{
			$elem=$res[0];
			$parent->last_error ="The test run '$RUN_name' is already in the database\n";
			$parent->last_error.="<i>".stringFromDB($elem["run_comments"])."</i>\n";
			return false;
		}
		
		/* The trick for parsing the logfile is matching with a perl regexp */
		$log_data=array();
		
		$regexp = "/rpc_suite\/"
			."\w+\/" /* definition, interface, ... */
			."((sys\/)?" /* special case for headers <sys/...> */
			."\w+)\/"  /* routine name matching */
			."(speculative\/)?" /* we also want speculative tests */
			."(\d*)-(.*):"     /* test name */
			."\s*(build|link|execution):" /* Status type */
			."\s*(FAILED|PASS|SKIP|UNSUPPORTED|UNTESTED|HUNG|INTERRUPTED|UNRESOLVED)" /* status */
			."\s*:*\s*/";
		$num_match = 7; /* This is the number of grouping directives in this regexp */
		
		/* Actually parse the logfile */
		$temp_array=preg_split($regexp, $CONTENT, -1, PREG_SPLIT_DELIM_CAPTURE);
		
		if ( $parent->debug > 4 )
			print_r($temp_array);
		
		if (count($temp_array) % ($num_match+1) != 1)
		{
			$parent->last_error="Regexp match error.\nInvalid logfile format -- expecting rpc_ts.";
			return false;
		}
		
		// Declare a new test utility Class
		$text_utils = new textUtils();
		
		/* See preg_split documentation for more information on the data here */
		for ($idx=1; isset($temp_array[$idx]); $idx+=($num_match+1))
		{
			$log_data[]=array(
				"routine"    => $temp_array[$idx+0],
				"assert_num" => $temp_array[$idx+3],
				"test_num"   => $text_utils->testName_ToInt($temp_array[$idx+4]),
				"status"     => $temp_array[$idx+5]." ".$temp_array[$idx+6],
				"log"        => $temp_array[$idx+7]
				);
		}
		/* free some resources */
		unset($CONTENT);
		unset($temp_array);
		if ( $parent->debug > 1 )
			print_r($log_data);
		/* We're done with the file parsing. */
		
		/* Next step is to eliminate duplicates and match testcases with database definition */ 
		
		/* We'll need the routine list */
		$routines=query_routines();
		if (!$routines)
		{
			$parent->last_error="Failed to get routines list from database";
			return false;
		}
		
		/* We also need this testsuite complete definition */
/**** MODIF ****/
		$sql = "SELECT descr_id, assert_routine, descr_num_assert, descr_num_test"
			." FROM opts_version_descriptions, opts_assertions"
			." WHERE opts_version_descriptions.descr_assert=opts_assertions.assert_id"
			." AND descr_version=$TS_id";
		if ($parent->debug > 1)
			echo htmlentities($sql)."<br>\n";		
		$opts_definition_tmp = db_execute_select($sql);
		if (!$opts_definition_tmp)
		{
			$parent->last_error="The RPC_TS release description was not found in database.\n";
			return false;
		}
		/* We hash the result for efficiency */
		$opts_definition = array();
		foreach($opts_definition_tmp as $record)
			$opts_definition
			 [$record["assert_routine"]]
			  [$record["descr_num_assert"]]
			   [$record["descr_num_test"]]
			    =$record["descr_id"];
		unset($opts_definition_tmp);
		//print_r($opts_definition);
		
		/* We're ready to proceed:
		 * -> walk through the log file (analyzed)
		 * -> foreach test, find the corresponding description ID
		 * -> save a record with the information: description ID, test status, test log.
		 * -> this will then be used to generate the database entries.
		 */
		$result = array();
		foreach ($log_data as $record)
		{
			if (!isset($opts_definition
				     [$routines[$record["routine"]]["routine_id"]]
				      [$record["assert_num"]]
				       [$record["test_num"]]))
				echo "The test ".$record["routine"]."/".$record["assert_num"]."-".$record["test_num"]." was not found in the database -- ignored\n";
			else
				$result[$opts_definition
					[$routines[$record["routine"]]["routine_id"]]
					 [$record["assert_num"]]
					  [$record["test_num"]]
				       ]=array(
					"status"=>$record["status"],
					"log"   =>$record["log"]);
		}
		/* We can trash everything else :) */
		unset ($routines);
		unset ($opts_definition);
		unset ($log_data);
		
		echo "\n<b>".count($result)."</b> test results can be inserted in the results database.\n\n";
		
		/* Now we've got to add the new run name in the database and get its ID */
/**** MODIF ****/
		$sql = "INSERT INTO opts_run ( run_name, run_comments )"
			." VALUES ( ".stringToDB($RUN_name).", ".stringToDB($RUN_description)." )";
		if ($parent->debug > 1)
			echo htmlentities($sql)."<br>\n";		
		$res = db_execute_insert($sql);
		if (!$res)
		{
			$parent->last_error="Failed to insert new run name";
			return false;
		}
		
/**** MODIF ****/
		$sql = "SELECT run_id, run_name, run_comments FROM opts_run WHERE run_name LIKE ".stringToDB($RUN_name);
		if ($parent->debug > 1)
			echo htmlentities($sql)."<br>\n";		
		$res = db_execute_select($sql);
		if (!$res)
		{
			$parent->last_error="Internal error: the run was inserted but disappeared\n";
			return false;
		}
		$run_id=$res[0]["run_id"];
		
		$counter=0;
		foreach($result as $desc_id => $testdata)
		{
/**** MODIF ****/
			$sql = "INSERT INTO opts_run_results ( res_run, res_testcase, res_status, res_log )"
				." VALUES ( $run_id, $desc_id, ".stringToDB($testdata["status"]).", "
				.stringToDB($testdata["log"])." )";
			if ($parent->debug > 1)
				echo htmlentities($sql)."<br>\n";		
			if (db_execute_insert($sql))
				$counter++;
			else
				echo "<b><i>Failed to execute the following instruction</i></b>; skipping.\n$sql\n";
		}
		echo "<b>$counter</b> records added to the database!\n";

		return true;
	}
	
	
	function RUN_delete(&$parent, $RUN_id)
	{
		if ( $parent->debug )
			echo "opts->RUN_delete($RUN_id)\n";
		
		/* Check this run belongs to an OPTS testsuite */
/**** MODIF ****/
		$sql = "SELECT ver_module FROM opts_versions, opts_version_descriptions, opts_run_results"
		      ." WHERE res_run=$RUN_id"
		      ." AND res_testcase=descr_id AND descr_version=ver_id"
		      ." GROUP BY ver_module";
		if ($parent->debug > 1)
			echo htmlentities($sql)."<br>\n";
		$tmp = db_execute_select($sql);
		if (!$tmp)
		{
			$parent->last_error="The run ID or corresponding testsuite cannot be found in the database.\n";
			return FALSE;
		}
		if ($tmp[0]["ver_module"] != "rpc_ts")
		{
			$parent->last_error="The testsuite is not an rpc_ts -- cannot be deleted within the current module.\n";
			return FALSE;
		}
		
		/* We can delete everything related to this run */
/**** MODIF ****/
		$sql = "DELETE from opts_run_results "
		      ."WHERE res_run=$RUN_id";
		if ($parent->debug > 1)
			echo htmlentities($sql)."<br>\n";
		$tmp = db_execute_insert($sql);
		if ($tmp == 0)
		{
			$parent->last_error="No row deleted in opts_run_results\n";
			return FALSE;
		}
		if ($parent->debug > 1)
			echo "$tmp rows deleted from opts_run_results<br>\n";
		
/**** MODIF ****/
		$sql = "DELETE from opts_run "
		      ."WHERE run_id=$RUN_id";
		if ($parent->debug > 1)
			echo htmlentities($sql)."<br>\n";
		$tmp = db_execute_insert($sql);
		if ($tmp == 0)
		{
			$parent->last_error="No row deleted in opts_run\n";
			return FALSE;
		}
		if ($parent->debug > 1)
			echo "$tmp row deleted from opts_run<br>\n";
		
		return true;
	}
}

/* Return the class name so it is added to the catalog */
return("rpc_ts");
?>
