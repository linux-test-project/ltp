#!/usr/bin/perl

#       $Id: generate_index.pl,v 1.10 2009/12/03 15:31:01 subrata_modak Exp $
 
#  (C) Copyright IBM Corp. 2004
 
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  This
#  file and program are licensed under a BSD style license.  See
#  the Copying file included with the OpenHPI distribution for
#  full licensing terms.
 
#  Authors:
#      Sean Dague <http://dague.net/sean>

use strict;
use HTML::Entities;

my $dir = shift;
opendir(IN,"$dir");
my @files = ();
while(my $file = readdir(IN)) {
    push @files, $file;
}
closedir(IN);

my $html = make_html_head();
$html .= make_html_body($dir, @files);
$html .= make_html_tail();

print $html;

sub make_html_head {
    return <<END;
<html>
<head><title>Test Coverage Analysis</title>
<link rel="stylesheet" href="/openhpi.css" type="text/css">
</head>
<body>
<div id="banner"><div><h1>The OpenHPI Project</h1><small>Open Hardware Platform Interface</small></div></div>
<table>
<tr>
<!--#include virtual="/sidebar.html" -->
<td id="maincolumn"><div class="mainsegment">
<h3>Test Coverage Analysis</h3>
<div>
<table>
END
}

sub make_html_tail {
    return <<END;
</table></div>
</div></td></tr></table>
</body>
</html>
END
}

# ok, here is a set of assumptions, which should hold true
#
# 1. .c.summary.html files will exist for everything we care about
# 2. a .c.gcov.html file will exist for each
# 3. my regex skills are honed enough to pull up the table from .c.summary.html file


sub make_html_body {
    my $dir = shift;
    my @files = @_;
    my $html;
    foreach my $file (sort @files) {
        if($file =~ /\.c\.summary\.html$/) {
            next if(-z "$dir/$file");
            my $name = $file;
            $name =~ s/.summary.html//;
            $html .= slurp_summary_table($name, "$dir/$file");
            $html .= "<a href='$file'>Full Coverage Report</a><br>\n";
            $html .= "<a href='$name.gcov.html'>Detailed Execution Report</a><br>\n";
        }
    }
    return $html;
}

sub slurp_summary_table {
    my ($name, $file) = @_;
    open(IN,"$file");
    local $/ = undef;
    my $content = <IN>;
    close(IN);

    my $snip = "";
    if($content =~ m{(<h4 class='file'>.*?</h4>\s*<table.*?</table>)}igs) {
        $snip = $1;
    }
    return $snip;
}
    
