#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2004						      ##
##									      ##
## This program is free software;  you can redistribute it and/or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or	      ##
## (at your option) any later version.					      ##
##									      ##
## This program is distributed in the hope that it will be useful, but	      ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.							      ##
##									      ##
## You should have received a copy of the GNU General Public License	      ##
## along with this program;  if not, write to the Free Software		      ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##									      ##
################################################################################
#
# File :	mod_perl.sh
#
# Description:	Check that apache can serve up a Perl web page.
#
# Author:	Robert Paulsen, rpaulsen@us.ibm.com
#
# History:	Jul 09 2004 - Created. Robert Paulsen. rpaulsen@us.ibm.com
#		Jul 16 2004 - Added various use statements to look for
#				perl modules required by TSMC. This is a 
#				temp hack since other customers may not
#				have these modules. Will straighten out
#				later. (rcp)
#
###############################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# global variables
################################################################################

apachever="1"
[ "$1" ] && apachever=$1

#
# tc_local_setup
#
function tc_local_setup()
{
	apache_was_running=""

	tc_root_or_break || return

	httpd_server="/srv/www"

	# for apache 1
	[ "$apachever" == "1" ] && {
		httpd_etc="/etc/httpd"
		apacheinit="/etc/init.d/apache"
		perl_lib="/var/www/perllib"
	}

	# for apache 2
	[ "$apachever" == "2" ] && {
		httpd_etc="/etc/apache2"
		apacheinit="/etc/init.d/apache2"
		perl_conf="$httpd_etc/conf.d/mod_perl.conf"
		perl_mod="LoadModule perl_module                    /usr/lib/apache2/mod_perl.so"
		perl_lib="$httpd_server/perl-lib"
	}

	perl_cgi="$httpd_server/cgi-bin"
	httpd_conf="$httpd_etc/httpd.conf"
	mypage_cgi="$perl_cgi/perltest.pl"
	mypage_lib="$perl_lib/MyApache/MyTest.pm"
}

#
# tc_local_cleanup		cleanup unique to this testcase
#
function tc_local_cleanup()
{
	[ -f $TCTMP/httpd.conf ] && mv -f $TCTMP/httpd.conf $httpd_conf
	[ -f $mypage_cgi ] && rm $mypage_cgi &>/dev/null
	[ -f $mypage_lib ] && rm $mypage_lib &>/dev/null

	if [ "$apache_was_running" = "no" ] ; then
		[ -x "$apacheinit" ] && { 
			tc_info "stopping apache server"
			$apacheinit stop &>/dev/null
		}
	else
		[ -x "$apacheinit" ] && {
			tc_info "(re)starting the apache server"
			$apacheinit restart &>/dev/null
		}
	fi
}

################################################################################
# the testcase functions
################################################################################

#
# test01	installation check
#
function test01()
{
	tc_register "installation check"

	# apache must be installed
	tc_executes $apacheinit
	tc_fail_if_bad $? "not properly installed" || return

	# httpd.conf file must exist
	tc_exists $httpd_conf
	tc_pass_or_fail $? "not properly installed"
}

#
# test02	configure apache for mod_perl
#
function test02()
{
	tc_register "configure and start apache $apachever.x with mod_perl"

	# save original httpd.conf file
	cp -ax $httpd_conf $TCTMP/

	mkdir -p $perl_lib/MyApache
	pkg=MyTest

	# configure apache1 for mod_perl handler
	[ "$apachever" == "1" ] && {
		cat <<-EOF >> $httpd_conf	# config perl handler
			PerlModule MyApache::MyTest
			<Location /perltest>
				SetHandler  perl-script
				PerlHandler MyApache::MyTest
			</Location>
		EOF

		# Place handler page 
		cat <<-EOF > $mypage_lib
			package MyApache::$pkg;
			use strict;
			use warnings;
			use Apache::Constants qw(:common);

			# these for various perl modules used by TSMC
			use CGI;
			use Compress::Zlib;
			use Date::Manip;
			use DB_File;
			use Digest::MD5;
			use HTML::Parser;
			use HTML::Tagset;
			use Bundle::LWP;	# libwww-perl package
			use HTTP::Cookies;	# libwww-perl package
			use Net::HTTP;		# libwww-perl package
			use WWW::RobotRules;	# libwww-perl package
			use File::Listing;	# libwww-perl package
			use HTML::Form;		# libwww-perl package
			use LWP;		# libwww-perl package
			use Data::Grove;		# libxml-perl package
			use XML::Handler::Sample;	# libxml-perl package
			use MIME::Base64;
			use NDBM_File;
			use Parse::Yapp::Parse;
			use Parse::Yapp::Driver;
			use URI;
			use XML::DOM;
			use XML::Parser;
			use XML::RegExp;
			use XML::Simple;
			use XML::XQL;

			sub handler {
				if (\$ENV{MOD_PERL}) {
					my \$a="LIFE IS ";
					my \$b="GOOD";
					my \$r = shift;
					\$r->content_type('text/html');
					print "
						<html><body>
						<h1>Hello World from perl handler!</h1>
						<p>
					";
					print map { "\$_ = \$ENV{\$_}<br>\n" } sort (keys (%ENV));
					print "
						<p>\$a\$b
						</body></html>
					";
				} else {
					print "Content-type: text/plain\n\n";
					print "BAD NEWS!";
				}
				return Apache::OK;
			}
			1;
		EOF
	}

	# configure apache2 for mod_perl and mod_perl handler
	[ "$apachever" == "2" ] && {
		echo $perl_mod >> $httpd_conf	# install mod_perl
		cat $perl_conf >> $httpd_conf	# activate mod_perl
		cat <<-EOF >> $httpd_conf	# config perl handler
			<Location /perltest>
				SetHandler  perl-script
				PerlResponseHandler MyApache::MyTest
			</Location>
		EOF

		# Place handler page 
		cat <<-EOF > $mypage_lib
			package MyApache::$pkg;
			use strict;
			use warnings;
			use Apache::RequestRec ();
			use Apache::RequestIO ();
			use Apache::Const -compile => qw(OK);
			sub handler {
				if (\$ENV{MOD_PERL}) {
					my \$a="LIFE IS ";
					my \$b="GOOD";
					my \$r = shift;
					\$r->content_type('text/html');
					print "
						<html><body>
						<h1>Hello World from perl handler!</h1>
						<p>
					";
					print map { "\$_ = \$ENV{\$_}<br>\n" } sort (keys (%ENV));
					print "
						<p>\$a\$b
						</body></html>
					";
				} else {
					print "Content-type: text/plain\n\n";
					print "BAD NEWS!";
				}
				return Apache::OK;
			}
			1;
		EOF
	}

	chmod +x $mypage_lib

	# (re)start apache
	apache_was_running="no"
	$apacheinit status >/dev/null && apache_was_running="yes"
	$apacheinit restart &>$stdout
	tc_pass_or_fail $? "apache server would not start" || return
	sleep 2		# be sure server is ready to respond
}

#
# test03	fetch perl cgi page
#
function test03()
{
	tc_register "get Perl cgi page from server"
	tc_exec_or_break cat grep || return

	# Place web page 
	# Construct perl web page in a way that the returned value will only
	# match the expected if the page has been procesed by Perl.
	cat <<-EOF > $mypage_cgi
		#!/usr/bin/perl
		my \$a="LIFE IS ";
		my \$b="GOOD";
		print "Content-Type: text/html\n\n";
		print "<html><head><title>Script Environment</title>
		</head><body>\n";
		print "<h1>Hello World from perl registry app!</h1>
		";
		print map { "\$_ = \$ENV{\$_}<br>\n" } sort (keys (%ENV));
		print "
		<p>\$a\$b
		</body></html>\n";
	EOF
	chmod +x $mypage_cgi

	local expected="LIFE IS GOOD";

	# get the page from apache sever via http
	local port=80
	fivget http localhost $port perl/perltest.pl >$stdout 2>$stderr
	tc_fail_if_bad $? "failed to get perltest.pl from server" || return

	# compare for expected content
	grep -q "$expected" $stdout 2>$stderr
	tc_pass_or_fail $? "" "expected to see \"$expected\" in stdout" 
}

#
# test04	fetch perl handler page
#
function test04()
{
	tc_register "get Perl handler page from server"
	tc_exec_or_break cat grep || return

	local expected="LIFE IS GOOD";

	# get the page from apache sever via http
	local port=80
	fivget http localhost $port perltest >$stdout 2>$stderr
	tc_fail_if_bad $? "failed to get perltest.pm from server" || return

	# compare for expected content
	grep -q "$expected" $stdout 2>$stderr
	tc_pass_or_fail $? "" "expected to see \"$expected\" in stdout" 
}

################################################################################
# main
################################################################################

TST_TOTAL=3

tc_setup

test01 &&
test02 &&
test03 &&
test04
