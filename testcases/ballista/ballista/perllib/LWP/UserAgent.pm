# $Id: UserAgent.pm,v 1.1 2004/10/18 17:57:54 mridge Exp $

package LWP::UserAgent;
use strict;

=head1 NAME

LWP::UserAgent - A WWW UserAgent class

=head1 SYNOPSIS

 require LWP::UserAgent;
 $ua = new LWP::UserAgent;

 $request = new HTTP::Request('GET', 'file://localhost/etc/motd');

 $response = $ua->request($request); # or
 $response = $ua->request($request, '/tmp/sss'); # or
 $response = $ua->request($request, \&callback, 4096);

 sub callback { my($data, $response, $protocol) = @_; .... }

=head1 DESCRIPTION

The C<LWP::UserAgent> is a class implementing a simple World-Wide Web
user agent in Perl. It brings together the HTTP::Request,
HTTP::Response and the LWP::Protocol classes that form the rest of the
core of libwww-perl library. For simple uses this class can be used
directly to dispatch WWW requests, alternatively it can be subclassed
for application-specific behaviour.

In normal usage the application creates a UserAgent object, and then
configures it with values for timeouts proxies, name, etc. The next
step is to create an instance of C<HTTP::Request> for the request that
needs to be performed. This request is then passed to the UserAgent
request() method, which dispatches it using the relevant protocol,
and returns a C<HTTP::Response> object.

The basic approach of the library is to use HTTP style communication
for all protocol schemes, i.e. you will receive an C<HTTP::Response>
object also for gopher or ftp requests.  In order to achieve even more
similarities with HTTP style communications, gopher menus and file
directories will be converted to HTML documents.

The request() method can process the content of the response in one of
three ways: in core, into a file, or into repeated calls of a
subroutine.  You choose which one by the kind of value passed as the
second argument to request().

The in core variant simply returns the content in a scalar attribute
called content() of the response object, and is suitable for small
HTML replies that might need further parsing.  This variant is used if
the second argument is missing (or is undef).

The filename variant requires a scalar containing a filename as the
second argument to request(), and is suitable for large WWW objects
which need to be written directly to the file, without requiring large
amounts of memory. In this case the response object returned from
request() will have empty content().  If the request fails, then the
content() might not be empty, and the file will be untouched.

The subroutine variant requires a reference to callback routine as the
second argument to request() and it can also take an optional chuck
size as third argument.  This variant can be used to construct
"pipe-lined" processing, where processing of received chuncks can
begin before the complete data has arrived.  The callback function is
called with 3 arguments: the data received this time, a reference to
the response object and a reference to the protocol object.  The
response object returned from request() will have empty content().  If
the request fails, then the the callback routine will not have been
called, and the response->content() might not be empty.

The request can be aborted by calling die() within the callback
routine.  The die message will be available as the "X-Died" special
response header field.

The library also accepts that you put a subroutine reference as
content in the request object.  This subroutine should return the
content (possibly in pieces) when called.  It should return an empty
string when there is no more content.

=head1 METHODS

The following methods are available:

=over 4

=cut


use vars qw(@ISA $VERSION);

require LWP::MemberMixin;
@ISA = qw(LWP::MemberMixin);
$VERSION = sprintf("%d.%02d", q$Revision: 1.1 $ =~ /(\d+)\.(\d+)/);


require URI::URL;
require HTTP::Request;
require HTTP::Response;

use HTTP::Date ();

use LWP ();
use LWP::Debug ();
use LWP::Protocol ();

use Carp ();

use AutoLoader ();
*AUTOLOAD = \&AutoLoader::AUTOLOAD;  # import the AUTOLOAD method


=item $ua = new LWP::UserAgent;

Constructor for the UserAgent.  Returns a reference to a
LWP::UserAgent object.

=cut

sub new
{
    my($class, $init) = @_;
    LWP::Debug::trace('()');

    my $self;
    if (ref $init) {
	$self = $init->clone;
    } else {
	$self = bless {
		'agent'       => "libwww-perl/$LWP::VERSION",
		'from'        => undef,
		'timeout'     => 3*60,
		'proxy'       => undef,
		'cookie_jar'  => undef,
		'use_eval'    => 1,
                'parse_head'  => 1,
                'max_size'    => undef,
		'no_proxy'    => [],
	}, $class;
    }
}


=item $ua->simple_request($request, [$arg [, $size]])

This method dispatches a single WWW request on behalf of a user, and
returns the response received.  The C<$request> should be a reference
to a C<HTTP::Request> object with values defined for at least the
method() and url() attributes.

If C<$arg> is a scalar it is taken as a filename where the content of
the response is stored.

If C<$arg> is a reference to a subroutine, then this routine is called
as chunks of the content is received.  An optional C<$size> argument
is taken as a hint for an appropriate chunk size.

If C<$arg> is omitted, then the content is stored in the response
object itself.

=cut

sub simple_request
{
    my($self, $request, $arg, $size) = @_;
    local($SIG{__DIE__});  # protect agains user defined die handlers

    my($method, $url) = ($request->method, $request->url);

    # Check that we have a METHOD and a URL first
    return HTTP::Response->new(&HTTP::Status::RC_BAD_REQUEST, "Method missing")
	unless $method;
    return HTTP::Response->new(&HTTP::Status::RC_BAD_REQUEST, "URL missing")
	unless $url;

    LWP::Debug::trace("$method $url");

    # Locate protocol to use
    my $scheme = '';
    my $proxy = $self->_need_proxy($url);
    if (defined $proxy) {
	$scheme = $proxy->scheme;
    } else {
	$scheme = $url->scheme;
    }
    my $protocol;
    eval {
	$protocol = LWP::Protocol::create($scheme);
    };
    if ($@) {
	$@ =~ s/\s+at\s+\S+\s+line\s+\d+\.?\s*//;  # remove file/line number
	return HTTP::Response->new(&HTTP::Status::RC_NOT_IMPLEMENTED, $@)
    }

    # Extract fields that will be used below
    my ($agent, $from, $timeout, $cookie_jar,
        $use_eval, $parse_head, $max_size) =
      @{$self}{qw(agent from timeout cookie_jar
                  use_eval parse_head max_size)};

    # Set User-Agent and From headers if they are defined
    $request->header('User-Agent' => $agent) if $agent;
    $request->header('From' => $from) if $from;
    $cookie_jar->add_cookie_header($request) if $cookie_jar;

    # Transfer some attributes to the protocol object
    $protocol->parse_head($parse_head);
    $protocol->max_size($max_size);
    
    my $response;
    if ($use_eval) {
	# we eval, and turn dies into responses below
	eval {
	    $response = $protocol->request($request, $proxy,
					   $arg, $size, $timeout);
	};
	if ($@) {
	    $@ =~ s/\s+at\s+\S+\s+line\s+\d+\.?\s*//;
	    $response =
	      HTTP::Response->new(&HTTP::Status::RC_INTERNAL_SERVER_ERROR,
				  $@);
	}
    } else {
	$response = $protocol->request($request, $proxy,
				       $arg, $size, $timeout);
	# XXX: Should we die unless $response->is_success ???
    }

    $response->request($request);  # record request for reference
    $cookie_jar->extract_cookies($response) if $cookie_jar;
    $response->header("Client-Date" => HTTP::Date::time2str(time));
    return $response;
}


=item $ua->request($request, $arg [, $size])

Process a request, including redirects and security.  This method may
actually send several different simple reqeusts.

The arguments are the same as for C<simple_request()>.

=cut

sub request
{
    my($self, $request, $arg, $size, $previous) = @_;

    LWP::Debug::trace('()');

    my $response = $self->simple_request($request, $arg, $size);

    my $code = $response->code;
    $response->previous($previous) if defined $previous;

    LWP::Debug::debug('Simple result: ' . HTTP::Status::status_message($code));

    if ($code == &HTTP::Status::RC_MOVED_PERMANENTLY or
	$code == &HTTP::Status::RC_MOVED_TEMPORARILY) {

	# Make a copy of the request and initialize it with the new URI
	my $referral = $request->clone;

	# And then we update the URL based on the Location:-header.
	# Some servers erroneously return a relative URL for redirects,
	# so make it absolute if it not already is.
	my $referral_uri = (URI::URL->new($response->header('Location'),
					  $response->base))->abs(undef,1);

	$referral->url($referral_uri);

	return $response unless $self->redirect_ok($referral);

	# Check for loop in the redirects
	my $count = 0;
	my $r = $response;
	while ($r) {
	    if (++$count > 13 ||
                $r->request->url->as_string eq $referral_uri->as_string) {
		$response->header("Client-Warning" =>
				  "Redirect loop detected");
		return $response;
	    }
	    $r = $r->previous;
	}

	return $self->request($referral, $arg, $size, $response);

    } elsif ($code == &HTTP::Status::RC_UNAUTHORIZED ||
	     $code == &HTTP::Status::RC_PROXY_AUTHENTICATION_REQUIRED
	    )
    {
	my $proxy = ($code == &HTTP::Status::RC_PROXY_AUTHENTICATION_REQUIRED);
	my $ch_header = $proxy ?  "Proxy-Authenticate" : "WWW-Authenticate";
	my $challenge = $response->header($ch_header);
	unless (defined $challenge) {
	    $response->header("Client-Warning" => 
			      "Missing Authenticate header");
	    return $response;
	}

	require HTTP::Headers::Util;
	$challenge =~ tr/,/;/;  # "," is used to separate auth-params!!
	($challenge) = HTTP::Headers::Util::split_header_words($challenge);
	my $scheme = lc(shift(@$challenge));
	shift(@$challenge); # no value
	$challenge = { @$challenge };  # make rest into a hash
	for (keys %$challenge) {       # make sure all keys are lower case
	    $challenge->{lc $_} = delete $challenge->{$_};
	}

	unless ($scheme =~ /^([a-z]+(?:-[a-z]+)*)$/) {
	    $response->header("Client-Warning" => 
			      "Bad authentication scheme '$scheme'");
	    return $response;
	}
	$scheme = $1;  # untainted now
	my $class = "LWP::Authen::\u$scheme";
	$class =~ s/-/_/g;
	
	no strict 'refs';
	unless (defined %{"$class\::"}) {
	    # try to load it
	    eval "require $class";
	    if ($@) {
		if ($@ =~ /^Can\'t locate/) {
		    $response->header("Client-Warning" =>
				      "Unsupport authentication scheme '$scheme'");
		} else {
		    $response->header("Client-Warning" => $@);
		}
		return $response;
	    }
	}
	return $class->authenticate($self, $proxy, $challenge, $response,
				    $request, $arg, $size);
    }
    return $response;
}


=item $ua->redirect_ok

This method is called by request() before it tries to do any
redirects.  It should return a true value if the redirect is allowed
to be performed. Subclasses might want to override this.

The default implementation will return FALSE for POST request and TRUE
for all others.

=cut

sub redirect_ok
{
    # draft-ietf-http-v10-spec-02.ps from www.ics.uci.edu, specify:
    #
    # If the 30[12] status code is received in response to a request using
    # the POST method, the user agent must not automatically redirect the
    # request unless it can be confirmed by the user, since this might change
    # the conditions under which the request was issued.

    my($self, $request) = @_;
    return 0 if $request->method eq "POST";
    1;
}


=item $ua->credentials($netloc, $realm, $uname, $pass)

Set the user name and password to be used for a realm.  It is often more
useful to specialize the get_basic_credentials() method instead.

=cut

sub credentials
{
    my($self, $netloc, $realm, $uid, $pass) = @_;
    @{ $self->{'basic_authentication'}{$netloc}{$realm} } = ($uid, $pass);
}


=item $ua->get_basic_credentials($realm, $uri, [$proxy])

This is called by request() to retrieve credentials for a Realm
protected by Basic Authentication or Digest Authentication.

Should return username and password in a list.  Return undef to abort
the authentication resolution atempts.

This implementation simply checks a set of pre-stored member
variables. Subclasses can override this method to e.g. ask the user
for a username/password.  An example of this can be found in
C<lwp-request> program distributed with this library.

=cut

sub get_basic_credentials
{
    my($self, $realm, $uri, $proxy) = @_;
    return if $proxy;

    my $netloc = $uri->netloc;
    if (exists $self->{'basic_authentication'}{$netloc}{$realm}) {
	return @{ $self->{'basic_authentication'}{$netloc}{$realm} };
    }

    return (undef, undef);
}


=item $ua->agent([$product_id])

Get/set the product token that is used to identify the user agent on
the network.  The agent value is sent as the "User-Agent" header in
the requests. The default agent name is "libwww-perl/#.##", where
"#.##" is substitued with the version numer of this library.

The user agent string should be one or more simple product identifiers
with an optional version number separated by the "/" character.
Examples are:

  $ua->agent('Checkbot/0.4 ' . $ua->agent);
  $ua->agent('Mozilla/5.0');

=item $ua->from([$email_address])

Get/set the Internet e-mail address for the human user who controls
the requesting user agent.  The address should be machine-usable, as
defined in RFC 822.  The from value is send as the "From" header in
the requests.  There is no default.  Example:

  $ua->from('aas@sn.no');

=item $ua->timeout([$secs])

Get/set the timeout value in seconds. The default timeout() value is
180 seconds, i.e. 3 minutes.

=item $ua->cookie_jar([$cookies])

Get/set the I<HTTP::Cookies> object to use.  The default is to have no
cookie_jar, i.e. never automatically add "Cookie" headers to the
requests.

=item $ua->parse_head([$boolean])

Get/set a value indicating wether we should initialize response
headers from the E<lt>head> section of HTML documents. The default is
TRUE.  Do not turn this off, unless you know what you are doing.

=item $ua->max_size([$bytes])

Get/set the size limit for response content.  The default is undef,
which means that there is not limit.  If the returned response content
is only partial, because the size limit was exceeded, then a
"X-Content-Range" header will be added to the response.

=cut

sub timeout    { shift->_elem('timeout',   @_); }
sub agent      { shift->_elem('agent',     @_); }
sub from       { shift->_elem('from',      @_); }
sub cookie_jar { shift->_elem('cookie_jar',@_); }
sub parse_head { shift->_elem('parse_head',@_); }
sub max_size   { shift->_elem('max_size',  @_); }

# depreciated
sub use_eval   { shift->_elem('use_eval',  @_); }
sub use_alarm
{
    Carp::carp("LWP::UserAgent->use_alarm(BOOL) is a no-op")
	if @_ > 1 && $^W;
    "";
}


# Declarations of AutoLoaded methods
sub clone;
sub is_protocol_supported;
sub mirror;
sub proxy;
sub env_proxy;
sub no_proxy;
sub _need_proxy;


1;
__END__


=item $ua->clone;

Returns a copy of the LWP::UserAgent object

=cut


sub clone
{
    my $self = shift;
    my $copy = bless { %$self }, ref $self;  # copy most fields

    # elements that are references must be handled in a special way
    $copy->{'no_proxy'} = [ @{$self->{'no_proxy'}} ];  # copy array

    $copy;
}


=item $ua->is_protocol_supported($scheme)

You can use this method to query if the library currently support the
specified C<scheme>.  The C<scheme> might be a string (like 'http' or
'ftp') or it might be an URI::URL object reference.

=cut

sub is_protocol_supported
{
    my($self, $scheme) = @_;
    if (ref $scheme) {
	# assume we got a reference to an URI::URL object
	$scheme = $scheme->abs->scheme;
    } else {
	Carp::croak("Illeal scheme '$scheme' passed to is_protocol_supported")
	    if $scheme =~ /\W/;
	$scheme = lc $scheme;
    }
    return LWP::Protocol::implementor($scheme);
}


=item $ua->mirror($url, $file)

Get and store a document identified by a URL, using If-Modified-Since,
and checking of the Content-Length.  Returns a reference to the
response object.

=cut

sub mirror
{
    my($self, $url, $file) = @_;

    LWP::Debug::trace('()');
    my $request = new HTTP::Request('GET', $url);

    if (-e $file) {
	my($mtime) = (stat($file))[9];
	if($mtime) {
	    $request->header('If-Modified-Since' =>
			     HTTP::Date::time2str($mtime));
	}
    }
    my $tmpfile = "$file-$$";

    my $response = $self->request($request, $tmpfile);
    if ($response->is_success) {

	my $file_length = (stat($tmpfile))[7];
	my($content_length) = $response->header('Content-length');

	if (defined $content_length and $file_length < $content_length) {
	    unlink($tmpfile);
	    die "Transfer truncated: " .
		"only $file_length out of $content_length bytes received\n";
	} elsif (defined $content_length and $file_length > $content_length) {
	    unlink($tmpfile);
	    die "Content-length mismatch: " .
		"expected $content_length bytes, got $file_length\n";
	} else {
	    # OK
	    if (-e $file) {
		# Some dosish systems fail to rename if the target exists
		chmod 0777, $file;
		unlink $file;
	    }
	    rename($tmpfile, $file) or
		die "Cannot rename '$tmpfile' to '$file': $!\n";

	    if (my $lm = $response->last_modified) {
		# make sure the file has the same last modification time
		utime $lm, $lm, $file;
	    }
	}
    } else {
	unlink($tmpfile);
    }
    return $response;
}

=item $ua->proxy(...)

Set/retrieve proxy URL for a scheme:

 $ua->proxy(['http', 'ftp'], 'http://proxy.sn.no:8001/');
 $ua->proxy('gopher', 'http://proxy.sn.no:8001/');

The first form specifies that the URL is to be used for proxying of
access methods listed in the list in the first method argument,
i.e. 'http' and 'ftp'.

The second form shows a shorthand form for specifying
proxy URL for a single access scheme.

=cut

sub proxy
{
    my($self, $key, $proxy) = @_;

    LWP::Debug::trace("$key, $proxy");

    if (!ref($key)) {   # single scalar passed
	my $old = $self->{'proxy'}{$key};
	$self->{'proxy'}{$key} = $proxy;
	return $old;
    } elsif (ref($key) eq 'ARRAY') {
	for(@$key) {    # array passed
	    $self->{'proxy'}{$_} = $proxy;
	}
    }
    return undef;
}

=item $ua->env_proxy()

Load proxy settings from *_proxy environment variables.  You might
specify proxies like this (sh-syntax):

  gopher_proxy=http://proxy.my.place/
  wais_proxy=http://proxy.my.place/
  no_proxy="my.place"
  export gopher_proxy wais_proxy no_proxy

Csh or tcsh users should use the C<setenv> command to define these
envirionment variables.

=cut

sub env_proxy {
    my ($self) = @_;
    my($k,$v);
    while(($k, $v) = each %ENV) {
	$k = lc($k);
	next unless $k =~ /^(.*)_proxy$/;
	$k = $1;
	if ($k eq 'no') {
	    $self->no_proxy(split(/\s*,\s*/, $v));
	}
	else {
	    $self->proxy($k, $v);
	}
    }
}

=item $ua->no_proxy($domain,...)

Do not proxy requests to the given domains.  Calling no_proxy without
any domains clears the list of domains. Eg:

 $ua->no_proxy('localhost', 'no', ...);

=cut

sub no_proxy {
    my($self, @no) = @_;
    if (@no) {
	push(@{ $self->{'no_proxy'} }, @no);
    }
    else {
	$self->{'no_proxy'} = [];
    }
}


# Private method which returns the URL of the Proxy configured for this
# URL, or undefined if none is configured.
sub _need_proxy
{
    my($self, $url) = @_;

    $url = new URI::URL($url) unless ref $url;

    LWP::Debug::trace("($url)");

    # check the list of noproxies

    if (@{ $self->{'no_proxy'} }) {
	my $host = $url->host;
	return undef unless defined $host;
	my $domain;
	for $domain (@{ $self->{'no_proxy'} }) {
	    if ($host =~ /$domain$/) {
		LWP::Debug::trace("no_proxy configured");
		return undef;
	    }
	}
    }

    # Currently configured per scheme.
    # Eventually want finer granularity

    my $scheme = $url->scheme;
    if (exists $self->{'proxy'}{$scheme}) {

	LWP::Debug::debug('Proxied');
	return new URI::URL($self->{'proxy'}{$scheme});
    }

    LWP::Debug::debug('Not proxied');
    undef;
}

1;

=back

=head1 SEE ALSO

See L<LWP> for a complete overview of libwww-perl5.  See F<lwp-request> and
F<lwp-mirror> for examples of usage.

=head1 COPYRIGHT

Copyright 1995-1997 Gisle Aas.

This library is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.

=cut
