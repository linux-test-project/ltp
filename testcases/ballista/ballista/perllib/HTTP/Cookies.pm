package HTTP::Cookies;

# Based on draft-ietf-http-state-man-mec-08.txt and
# http://www.netscape.com/newsref/std/cookie_spec.html

use strict;
use HTTP::Date qw(str2time time2str);
use HTTP::Headers::Util qw(split_header_words join_header_words);
use LWP::Debug ();

use vars qw($VERSION);
$VERSION = sprintf("%d.%02d", q$Revision: 1.1 $ =~ /(\d+)\.(\d+)/);

=head1 NAME

HTTP::Cookies - Cookie storage and management

=head1 SYNOPSIS

 use HTTP::Cookies;
 $cookie_jar = HTTP::Cookies->new;

 $cookie_jar->add_cookie_header($request);
 $cookie_jar->extract_cookies($response);

=head1 DESCRIPTION

Cookies are a general mechanism which server side connections can use
to both store and retrieve information on the client side of the
connection.  For more information about cookies referrer to
<URL:http://www.netscape.com/newsref/std/cookie_spec.html> and
<URL:http://www.cookiecentral.com/>.  This module also implements the
new style cookies as described in I<draft-ietf-http-state-man-mec-08.txt>.
The two variants of cookies is supposed to be able to coexist happily.

Instances of the class I<HTTP::Cookies> are able to store a collection
of Set-Cookie2: and Set-Cookie:-headers and is able to use this
information to initialize Cookie-headers in I<HTTP::Request> objects.
The state of the I<HTTP::Cookies> can be saved and restored from
files.

=head1 METHODS

The following methods are provided:

=over 4

=cut

# A HTTP::Cookies object is a hash.  The main attribute is the
# COOKIES 3 level hash:  $self->{COOKIES}{$domain}{$path}{$key}.


=item $cookie_jar = HTTP::Cookies->new;

The constructor.  Takes hash style parameters.  The following
parameters are recognized:

  file:            name of the file to restore and save cookies to
  autosave:        should we save during destruction (bool)
  ignore_discard:  save even cookies that are requested to be discarded (bool)

Future parameters might include (not yet implemented):

  max_cookies               300
  max_cookies_per_domain    20
  max_cookie_size           4096

  no_cookies   list of domain names that we never return cookies to

=cut

sub new
{
    my $class = shift;
    my $self = bless {
	COOKIES => {},
    }, $class;
    my %cnf = @_;
    for (keys %cnf) {
	$self->{lc($_)} = $cnf{$_};
    }
    $self->load;
    $self;
}


=item $cookie_jar->add_cookie_header($request);

The add_cookie_header() method will set the appropriate Cookie:-header
for the I<HTTP::Request> object given as argument.  The $request must
have a valid url() attribute before this method is called.

=cut

sub add_cookie_header
{
    my $self = shift;
    my $request = shift || return;
    my $url = $request->url;
    my $domain = $url->host;
    $domain = "$domain.local" unless $domain =~ /\./;
    my $secure_request = ($url->scheme eq "https");
    my $req_path = $url->epath;
    my $req_port = $url->port;
    my $now = time();
    $self->_normalize_path($req_path) if $req_path =~ /%/;

    my @cval;    # cookie values for the "Cookie" header
    my $set_ver;

    while (($domain =~ tr/././) >= 2 || # must be at least 2 dots
           $domain =~ /\.local$/)
    {

        LWP::Debug::debug("Checking $domain for cookies");
	my $cookies = $self->{COOKIES}{$domain};
	next unless $cookies;

	# Want to add cookies corresponding to the most specific paths
	# first (i.e. longest path first)
	my $path;
	for $path (sort {length($b) <=> length($a) } keys %$cookies) {
            LWP::Debug::debug("- checking cookie path=$path");
	    if (index($req_path, $path) != 0) {
	        LWP::Debug::debug("  path $path:$req_path does not fit");
		next;
	    }

	    my($key,$array);
	    while (($key,$array) = each %{$cookies->{$path}}) {
		my($version,$val,$port,$path_spec,$secure,$expires) = @$array;
	        LWP::Debug::debug(" - checking cookie $key=$val");
		if ($secure && !$secure_request) {
		    LWP::Debug::debug("   not a secure requests");
		    next;
		}
		if ($expires && $expires < $now) {
		    LWP::Debug::debug("   expired");
		    next;
		}
		if ($port) {
		    my $found;
		    if ($port =~ s/^_//) {
			# The correponding Set-Cookie attribute was empty
			$found++ if $port eq $req_port;
			$port = "";
		    } else {
			my $p;
			for $p (split(/,/, $port)) {
			    $found++, last if $p eq $req_port;
			}
		    }
		    unless ($found) {
		        LWP::Debug::debug("   port $port:$req_port does not fit");
			next;
		    }
		}
	        LWP::Debug::debug("   it's a match");

		# set version number of cookie header.
	        # XXX: What should it be if multiple matching
                #      Set-Cookie headers have different versions themselves
		if (!$set_ver++) {
		    if ($version >= 1) {
			push(@cval, "\$Version=$version");
		    } else {
			$request->header(Cookie2 => "\$Version=1");
		    }
		}

		# do we need to quote the value
		if ($val =~ /\W/ && $version) {
		    $val =~ s/([\\\"])/\\$1/g;
		    $val = qq("$val");
		}

		# and finally remember this cookie
		push(@cval, "$key=$val");
		if ($version >= 1) {
		    push(@cval, qq(\$Path="$path"))     if $path_spec;
		    push(@cval, qq(\$Domain="$domain")) if $domain =~ /^\./;
		    if (defined $port) {
			my $p = '$Port';
			$p .= qq(="$port") if length $port;
			push(@cval, $p);
		    }
		}

	    }
        }

    } continue {
	# Try with a more general domain:  www.sol.no ==> .sol.no
	$domain =~ s/^\.?[^.]*//;
    }

    $request->header(Cookie => join("; ", @cval)) if @cval;

    $request;
}


=item $cookie_jar->extract_cookies($response);

The extract_cookies() method will look for Set-Cookie: and
Set-Cookie2:-headers in the I<HTTP::Response> object passed as
argument.  If some of these headers are found they are used to update
the state of the $cookie_jar.

=cut

sub extract_cookies
{
    my $self = shift;
    my $response = shift || return;
    my @set = split_header_words($response->_header("Set-Cookie2"));
    my $netscape_cookies;
    unless (@set) {
	@set = $response->_header("Set-Cookie");
	return $response unless @set;
	$netscape_cookies++;
    }

    my $url = $response->request->url;
    my $req_host = $url->host;
    $req_host = "$req_host.local" unless $req_host =~ /\./;
    my $req_port = $url->port;
    my $req_path = $url->epath;
    $self->_normalize_path($req_path) if $req_path =~ /%/;
    
    if ($netscape_cookies) {
	# The old Netscape cookie format for Set-Cookie
        # http://www.netscape.com/newsref/std/cookie_spec.html
	# can for instance contain an unquoted "," in the expires
	# field, so we have to use this ad-hoc parser.
	my $now = time();
	my @old = @set;
	@set = ();
	my $set;
	for $set (@old) {
	    my @cur;
	    my $param;
	    my $expires;
	    for $param (split(/\s*;\s*/, $set)) {
		my($k,$v) = split(/\s*=\s*/, $param, 2);
		#print "$k => $v\n";
		my $lc = lc($k);
		if ($lc eq "expires") {
		    push(@cur, "Max-Age" => str2time($v) - $now);
		    $expires++;
		} else {
		    push(@cur, $k => $v);
		}
	    }
	    push(@cur, "Port" => $req_port);
	    push(@cur, "Discard" => undef) unless $expires;
	    push(@cur, "Version" => 0);
	    push(@set, \@cur);
	}
    }

  SET_COOKIE:
    for my $set (@set) {
	next unless @$set >= 2;

	my $key = shift @$set;
	my $val = shift @$set;

        LWP::Debug::debug("Set cookie $key => $val");

	my %hash;
	while (@$set) {
	    my $k = shift @$set;
	    my $v = shift @$set;
	    my $lc = lc($k);
	    # don't loose case distinction for unknown fields
	    $k = $lc if $lc =~ /^(?:discard|domain|max-age|
                                    path|port|secure|version)$/x;
	    if ($k eq "discard" || $k eq "secure") {
		$v = 1 unless defined $v;
	    }
	    next if exists $hash{$k};  # only first value is signigicant
	    $hash{$k} = $v;
	};

	my %orig_hash = %hash;
	my $version   = delete $hash{version};
	$version = 1 unless defined($version);
	my $discard   = delete $hash{discard};
	my $secure    = delete $hash{secure};
	my $maxage    = delete $hash{'max-age'};

	# Check domain
	my $domain  = delete $hash{domain};
	if (defined($domain) && $domain ne $req_host) {
	    if ($domain !~ /\./ && $domain ne "local") {
	        LWP::Debug::debug("Domain $domain contains no dot");
		next SET_COOKIE;
	    }
	    $domain = ".$domain" unless $domain =~ /^\./;
	    if ($domain =~ /\.\d+$/) {
	        LWP::Debug::debug("IP-address $domain illeagal as domain");
		next SET_COOKIE;
	    }
	    my $len = length($domain);
	    unless (substr($req_host, -$len) eq $domain) {
	        LWP::Debug::debug("Domain $domain does not match host $req_host");
		next SET_COOKIE;
	    }
	    my $hostpre = substr($req_host, 0, length($req_host) - $len);
	    if ($hostpre =~ /\./ && !$netscape_cookies) {
	        LWP::Debug::debug("Host prefix contain a dot: $hostpre => $domain");
		next SET_COOKIE;
	    }
	} else {
	    $domain = $req_host;
	}

	my $path = delete $hash{path};
	my $path_spec;
	if (defined $path) {
	    $path_spec++;
	    $self->_normalize_path($path) if $path =~ /%/;
	    if (!$netscape_cookies &&
                substr($req_path, 0, length($path)) ne $path) {
	        LWP::Debug::debug("Path $path is not a prefix of $req_path");
		next SET_COOKIE;
	    }
	} else {
	    $path = $req_path;
	    $path =~ s,/[^/]*$,,;
	    $path = "/" unless length($path);
	}

	my $port;
	if (exists $hash{port}) {
	    $port = delete $hash{port};
	    if (defined $port) {
		$port =~ s/\s+//g;
		my $found;
		for my $p (split(/,/, $port)) {
		    unless ($p =~ /^\d+$/) {
		      LWP::Debug::debug("Bad port $port (not numeric)");
			next SET_COOKIE;
		    }
		    $found++ if $p eq $req_port;
		}
		unless ($found) {
		    LWP::Debug::debug("Request port ($req_port) not found in $port");
		    next SET_COOKIE;
		}
	    } else {
		$port = "_$req_port";
	    }
	}
	$self->set_cookie($version,$key,$val,$path,$domain,$port,$path_spec,$secure,$maxage,$discard, \%hash)
	    if $self->set_cookie_ok(\%orig_hash);
    }

    $response;
}

sub set_cookie_ok { 1 };

=item $cookie_jar->set_cookie($version, $key, $val, $path, $domain, $port, $path_spec, $secure, $maxage, $discard, \%rest)

The set_cookie() method updates the state of the $cookie_jar.  The
$key, $val, $domain, $port and $path arguments are strings.  The
$path_spec, $secure, $discard arguments are boolean values. The $maxage
value is a number indicating number of seconds that this cookie will
live.  A value <= 0 will delete this cookie.  The %rest are a place
for various other attributes like "Comment" and "CommentURL".

=cut

sub set_cookie
{
    my $self = shift;
    my($version,
       $key, $val, $path, $domain, $port,
       $path_spec, $secure, $maxage, $discard, $rest) = @_;

    # there must always be at least 2 dots in a domain
    return $self if ($domain =~ tr/././) < 2 &&
                     $domain !~ /\.local$/;

    # path and key can not be empty (key can't start with '$')
    return $self if !defined($path) || $path !~ m,^/, ||
	            !defined($key)  || $key  !~ m,[^\$],;

    # ensure legal port
    if (defined $port) {
	return $self unless $port =~ /^_?\d+(?:,\d+)*$/;
    }

    my $expires;
    if (defined $maxage) {
	if ($maxage <= 0) {
	    delete $self->{COOKIES}{$domain}{$path}{$key};
	    return $self;
	}
	$expires = time() + $maxage;
    }
    $version = 0 unless defined $version;

    my @array = ($version, $val,$port,
		 $path_spec,
		 $secure, $expires, $discard);
    push(@array, {%$rest}) if defined($rest) && %$rest;
    # trim off undefined values at end
    pop(@array) while !defined $array[-1];

    $self->{COOKIES}{$domain}{$path}{$key} = \@array;
    $self;
}

=item $cookie_jar->save( [$file] );

Calling this method file save the state of the $cookie_jar to a file.
The state can then be restored later using the load() method.  If a
filename is not specified we will use the name specified during
construction.  If the attribute I<ignore_discared> is set, then we
will even save cookies that are marked to be discarded.

The default is to save a sequence of "Set-Cookie3" lines.  The
"Set-Cookie3" is a proprietary LWP format, not known to be compatible
with any other browser.  The I<HTTP::Cookies::Netscape> sub-class can
be used to save in a format compatible with Netscape.

=cut

sub save
{
    my $self = shift;
    my $file = shift || $self->{'file'} || return;
    local(*FILE);
    open(FILE, ">$file") or die "Can't open $file: $!";
    print FILE "#LWP-Cookies-1.0\n";
    print FILE $self->as_string(!$self->{ignore_discard});
    close(FILE);
    1;
}

=item $cookie_jar->load( [$file] );

This method will read the cookies from the file and add them to the
$cookie_jar.  The file must be in the format written by the save()
method.

=cut

sub load
{
    my $self = shift;
    my $file = shift || $self->{'file'} || return;
    local(*FILE, $_);
    open(FILE, $file) or return;
    my $magic = <FILE>;
    unless ($magic =~ /^\#LWP-Cookies-(\d+\.\d+)/) {
	warn "$file does not seem to contain cookies";
	return;
    }
    while (<FILE>) {
	next unless s/^Set-Cookie3:\s*//;
	chomp;
	my $cookie;
	for $cookie (split_header_words($_)) {
	    my($key,$val) = splice(@$cookie, 0, 2);
	    my %hash;
	    while (@$cookie) {
		my $k = shift @$cookie;
		my $v = shift @$cookie;
		$hash{$k} = $v;
	    }
	    my $version   = delete $hash{version};
	    my $path      = delete $hash{path};
	    my $domain    = delete $hash{domain};
	    my $port      = delete $hash{port};
	    my $expires   = str2time(delete $hash{expires});

	    my $path_spec = exists $hash{path_spec}; delete $hash{path_spec};
	    my $secure    = exists $hash{secure};    delete $hash{secure};
	    my $discard   = exists $hash{discard};   delete $hash{discard};

	    my @array =	($version,$val,$port,
			 $path_spec,$secure,$expires,$discard);
	    push(@array, \%hash) if %hash;
	    $self->{COOKIES}{$domain}{$path}{$key} = \@array;
	}
    }
    close(FILE);
    1;
}

=item $cookie_jar->revert;

Will revert to the state of last save.

=cut

sub revert
{
    my $self = shift;
    $self->clear->load;
    $self;
}

=item $cookie_jar->clear( [$domain, [$path, [$key] ] ]);

Invoking this method without arguments will empty the whole
$cookie_jar.  If given a single argument only cookies belonging to
that domain will be removed.  If given two arguments, cookies
belonging to the specified path within that domain is removed.  If
given three arguments, then the cookie with the specified key, path
and domain is removed.

=cut

sub clear
{
    my $self = shift;
    if (@_ == 0) {
	$self->{COOKIES} = {};
    } elsif (@_ == 1) {
	delete $self->{COOKIES}{$_[0]};
    } elsif (@_ == 2) {
	delete $self->{COOKIES}{$_[0]}{$_[1]};
    } elsif (@_ == 3) {
	delete $self->{COOKIES}{$_[0]}{$_[1]}{$_[2]};
    } else {
	require Carp;
        Carp::carp('Usage: $c->clear([domain [,path [,key]]])');
    }
    $self;
}

sub DESTROY
{
    my $self = shift;
    $self->save if $self->{'autosave'};
}


=item $cookie_jar->scan( \&callback );

The argument is a subroutine that will be invoked for each cookie
stored within the $cookie_jar.  The subroutine will be invoked with
the following arguments:

  0  version
  1  key
  2  val
  3  path
  4  domain
  5  port
  6  path_spec
  7  secure
  8  expires
  9  discard
 10  hash

=cut

sub scan
{
    my($self, $cb) = @_;
    my($domain,$path,$key);
    for $domain (sort keys %{$self->{COOKIES}}) {
	for $path (sort keys %{$self->{COOKIES}{$domain}}) {
	    for $key (sort keys %{$self->{COOKIES}{$domain}{$path}}) {
		my($version,$val,$port,$path_spec,
		   $secure,$expires,$discard,$rest) =
		       @{$self->{COOKIES}{$domain}{$path}{$key}};
		$rest = {} unless defined($rest);
		&$cb($version,$key,$val,$path,$domain,$port,
		     $path_spec,$secure,$expires,$discard,$rest);
	    }
	}
    }
}

=item $cookie_jar->as_string( [$skip_discard] );

The as_string() method will return the state of the $cookie_jar
represented as a sequence of "Set-Cookie3" header lines separated by
"\n".  If given a argument that is TRUE, it will not return lines for
cookies with the I<Discard> attribute.

=cut

sub as_string
{
    my($self, $skip_discard) = @_;
    my @res;
    $self->scan(sub {
	my($version,$key,$val,$path,$domain,$port,
	   $path_spec,$secure,$expires,$discard,$rest) = @_;
	return if $discard && $skip_discard;
	my @h = ($key, $val);
	push(@h, "path", $path);
	push(@h, "domain" => $domain);
	push(@h, "port" => $port) if defined $port;
	push(@h, "path_spec" => undef) if $path_spec;
	push(@h, "secure" => undef) if $secure;
	push(@h, "expires" => HTTP::Date::time2isoz($expires)) if $expires;
	push(@h, "discard" => undef) if $discard;
	my $k;
	for $k (sort keys %$rest) {
	    push(@h, $k, $rest->{$k});
	}
	push(@h, "version" => $version);
	push(@res, "Set-Cookie3: " . join_header_words(\@h));
    });
    join("\n", @res, "");
}


sub _normalize_path  # so that plain string compare can be used
{
    shift;  # $self
    my $x;
    $_[0] =~ s/%([0-9a-fA-F][0-9a-fA-F])/
	         $x = uc($1);
                 $x eq "2F" || $x eq "25" ? "%$x" :
                                            pack("c", hex($x));
              /eg;
    $_[0] =~ s/([\0-\x20\x7f-\xff])/sprintf("%%%02X",ord($1))/eg;
}



=back

=head1 SUB CLASSES

We also provide a subclass called I<HTTP::Cookies::Netscape> which make
cookie loading and saving compatible with Netscape cookie files.  You
should be able to have LWP share Netscape's cookies by constructing
your $cookie_jar like this:

 $cookie_jar = HTTP::Cookies::Netscape->new(
                   File     => "$ENV{HOME}/.netscape/cookies",
                   AutoSave => 1,
               );

Please note that the Netscape cookie file format is not able to store
all the information available in the Set-Cookie2 headers, so you will
probably loose some information if you save using this format.

=cut

package HTTP::Cookies::Netscape;

use vars qw(@ISA);
@ISA=qw(HTTP::Cookies);

sub load
{
    my($self, $file) = @_;
    $file ||= $self->{'file'} || return;
    local(*FILE, $_);
    my @cookies;
    open(FILE, $file) || return;
    my $magic = <FILE>;
    unless ($magic =~ /^\# Netscape HTTP Cookie File/) {
	warn "$file does not look like a netscape cookies file" if $^W;
	close(FILE);
	return;
    }
    my $now = time();
    while (<FILE>) {
	next if /^\s*\#/;
	next if /^\s*$/;
	chomp;
	my($domain,$bool1,$path,$secure, $expires,$key,$val) = split(/\t/, $_);
	$secure = ($secure eq "TRUE");
	$self->set_cookie(undef,$key,$val,$path,$domain,undef,
			  0,$secure,$expires-$now, 0);
    }
    close(FILE);
    1;
}

sub save
{
    my($self, $file) = @_;
    $file ||= $self->{'file'} || return;
    local(*FILE, $_);
    open(FILE, ">$file") || return;

    print FILE <<EOT;
# Netscape HTTP Cookie File
# http://www.netscape.com/newsref/std/cookie_spec.html
# This is a generated file!  Do not edit.

EOT

    my $now = time;
    $self->scan(sub {
	my($version,$key,$val,$path,$domain,$port,
	   $path_spec,$secure,$expires,$discard,$rest) = @_;
	return if $discard && !$self->{ignore_discard};
	$expires ||= 0;
	return if $now > $expires;
	$secure = $secure ? "TRUE" : "FALSE";
	my $bool = $domain =~ /^\./ ? "TRUE" : "FALSE";
	print FILE join("\t", $domain, $bool, $path, $secure, $expires, $key, $val), "\n";
    });
    close(FILE);
    1;
}

1;

__END__

=head1 COPYRIGHT

Copyright 1997, Gisle Aas

This library is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.

=cut
