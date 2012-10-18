#!/usr/bin/perl

#
# reconsile.cgi - reconsile two or more scanner files
#

use CGI qw(:standard);

chdir("/usr/tests/ltp/results/");

# Get the list of results to compare.
@results = param("results");

print header("text/html");
print start_html, "<pre>\n";

# Give a warning if the suites do not match
($a, $b, $lastsuite) = split(/\./, $results[0]);
for ($i = 1; $i <= $#results; $i++) {
	($a, $b, $thissuite) = split(/\./, $results[$i]);
	if ($lastsuite ne $thissuite) {
		print "Warning: Suites do not match!\n";
		last;
	}
}

# check that each requested result exists.  If one does not exist,
# print a warning and continue.  If the number of available results
# is less than two, halt with an error
@result_filenames = ();
foreach $a_result (@results) {
	if (-f "$a_result.scanner") {
		push(@result_filenames, "$a_result.scanner");
	} else {
		print "Could not find a scanner file for $a_result\n";
	}
}
if ($#result_filenames < 1) {
	print "Not enough result files to compare\n";
	die;
}

# for each result file read in and store the header information in
# an associative array.  Take the rest of the input file and store
# it as a list.
@result_details = ();
@result_testcases = ();
$i = 0;
foreach $result_filename (@result_filenames) {
	unless (open(F, $result_filename)) {
		print "failed openning $result_filename\n";
		next;
	}
	# advance past the header then read in the rest
	$result_testcases->[$i] = ();
	$result_details->[$i] = {};
	($host, $datestr, $suite, $ext) = split(/\./, $result_filename);
	$result_details->[$i]->{HOST} = $host;
	$result_details->[$i]->{DATESTR} = $datestr;
	$result_details->[$i]->{SUITE} = $suite;
	while ($line = <F>) {
		# check for the end of the header
		if ($line =~ /^-+/) {
			# we've reached the top of the scanner output
			# grab the rest and stop the while loop;
			@rest = <F>;
			close(F);
			last;
		}
		# grab information from the header
		if ($line =~ /^UNAME/) {
			$line =~ s/UNAME *//;
			$result_details->[$i]->{UNAME} = $line;
			next;
		}
	}
	# convert the results to records and add them to the list
	foreach $line (@rest) {
		($tag, $tcid, $tc, $status, $contact) = split(/\s+/, $line);
		# fix some of the fields so they sort properly
		$tcid = '{' if ($tcid eq '*');
		$tcid = '}' if ($tcid eq '-');
		$tc = '{' if ($tc eq '*');
		$tc = '}' if ($tc eq '-');
		$rec = ();
		$rec->{TAG} = $tag;
		$rec->{TCID} = $tcid;
		$rec->{TC} = $tc;
		$rec->{STATUS} = $status;
		$rec->{CONTACT} = $contact;
		push(@{$result_testcases[$i]}, $rec);
	}
	$i++;
}

# sort each set of results.
# This is the most important step since walking the data depends on
# correctly sorting the data.  Some substitutions are made to keep
# the test cases in each test tag in the proper order.  i.e.
# s/\*/{/
#$i = 0;
foreach $rtcs (@result_testcases) {
	@$rtcs = sort { $a->{TAG} cmp $b->{TAG}
					|| $a->{TCID} cmp $b->{TCID}
					|| $a->{TC} <=> $b->{TC}
					|| $a->{TC} cmp $b->{TC}
					|| $a->{STATUS} cmp $b->{STATUS}} @$rtcs;
	#print "sorted file $i\n";
	#print "=" x 50 . "\n";
	#foreach (@$rtcs) {
	#	print "$_->{TAG}:$_->{TCID}:$_->{TC}:$_->{STATUS}\n";
	#}
	#print "=" x 50 . "\n";
	#$i++;
}

# here is the loop that prints the data into a multi-column table with the test
# tags grouped together.

print "</pre>";
print "<table border=1>\n";

print "<tr><td>";
for($i=0; $i <= $#result_testcases; $i++) {
	print "<th colspan=3>$result_details->[$i]->{HOST}.$result_details->[$i]->{DATESTR}.$result_details->[$i]->{SUITE}";
}
print "</tr>\n";

print "<tr><th>Test Tag";
for($i=0; $i <= $#result_testcases; $i++) {
	print "<th>TCID<th>Test Case<th>Status";
}
print "<th>Contact</tr>\n";

# while the result lists still have test cases
# 	Find the smallest record from the top of the lists
#   remove matching records from the lists and output them
$last_tag = "";
while (1) {

	# if there wasn't anything left, leave
	$somethingleft = 0;
	foreach $rtcs (@result_testcases) {
		if ($#$rtcs > -1) {
			$somethingleft = 1;
			last;
		}
	}
	unless ($somethingleft) { last; }

	# find the Lowest Common Record
	@tops = ();
	foreach $rtcs (@result_testcases) {
		if (@$rtcs[0]) {
			push(@tops, copy_record(@$rtcs[0]));
		}
	}
	@tops = sort { $a->{TAG} cmp $b->{TAG}
				|| $a->{TCID} cmp $b->{TCID}
				|| $a->{TC} <=> $b->{TC}
				|| $a->{TC} cmp $b->{TC}
				|| $a->{STATUS} cmp $b->{STATUS}} @tops;

	$LCR = $tops[0];

	# check to see if everyone matches
	$matches = 0;
	foreach $rtcs (@result_testcases) {
		if (! @$rtcs[0]) { next; }
		if (@$rtcs[0]->{TAG} eq $LCR->{TAG}
			&& @$rtcs[0]->{TCID} eq $LCR->{TCID}
			&& @$rtcs[0]->{TC} eq $LCR->{TC}
			&& @$rtcs[0]->{STATUS} eq $LCR->{STATUS}) {

			$matches++;
		}
	}
	# if everyone does match (status included) shift them
	# and move on.
	if ($matches == ($#result_testcases+1)) {
		foreach $rtcs (@result_testcases) { shift(@$rtcs); }
		next;
	}

	# if we've already output stuff related to this test tag,
	# skip that column, otherwise print the tag
	if ($LCR->{TAG} eq $lasttag) {
		print "<tr><td>";
	} else {
		print "<tr><td>$LCR->{TAG}";
		$lasttag = $LCR->{TAG};
	}

	# walk through the lists again outputting as we match
	$column = 0;
	foreach $rtcs (@result_testcases) {
		if (! @$rtcs[0]) {
			print "<td><td><td>";
			$column++;
			next;
		} elsif (@$rtcs[0]->{TAG} eq $LCR->{TAG}
			&& @$rtcs[0]->{TCID} eq $LCR->{TCID}
			&& @$rtcs[0]->{TC} eq $LCR->{TC}) {

			$match = shift(@$rtcs);
			$match->{TCID} = '*' if ($match->{TCID} eq '{');
			$match->{TCID} = '-' if ($match->{TCID} eq '}');
			$match->{TC} = '*' if ($match->{TC} eq '{');
			$match->{TC} = '-' if ($match->{TC} eq '}');
			print "<td>";
			$rd = $result_details->[$column];
			print "<a href=\"results.cgi?get_df=$rd->{HOST}.$rd->{DATESTR}.$rd->{SUITE}.driver&zoom_tag=$match->{TAG}\">";
			print "$match->{TCID}</a>";
			print "<td>$match->{TC}";
			print "<td>";
			if ($match->{STATUS} =~ /PASS/) {
				print "<font color=green>";
			} elsif ($match->{STATUS} =~ /FAIL/) {
				print "<font color=red>";
			} elsif ($match->{STATUS} =~ /CONF/) {
				print "<font color=yello>";
			} elsif ($match->{STATUS} =~ /BROK/) {
				print "<font color=blue>";
			} else {
				print "<font color=black>";
			}
			print "$match->{STATUS}</font>";
		} else {
			print "<td><td><td>";
		}
		$column++;
	}
	print "<td>$LCR->{CONTACT}</tr>\n";
}
print "</table>";

print end_html;


sub copy_record {
	my $copy, $rec = shift;

	$copy->{TAG} = $rec->{TAG};
	$copy->{TCID} = $rec->{TCID};
	$copy->{TC} = $rec->{TC};
	$copy->{STATUS} = $rec->{STATUS};
	$copy->{CONTACT} = $rec->{CONTACT};
	return $copy;

}
