#
# $Id: http.pm,v 1.1 2004/10/18 17:57:56 mridge Exp $

package LWP::Protocol::http;

require LWP::Debug;
require HTTP::Response;
require HTTP::Status;
require IO::Socket;
require IO::Select;

require LWP::Protocol;
@ISA = qw(LWP::Protocol);

use strict;
my $CRLF         = "\015\012";     # how lines should be terminated;
				   # "\r\n" is not correct on all systems, for
				   # instance MacPerl defines it to "\012\015"

sub _new_socket
{
    my($self, $host, $port, $timeout) = @_;

    local($^W) = 0;  # IO::Socket::INET can be noisy
    my $sock = IO::Socket::INET->new(PeerAddr => $host,
				     PeerPort => $port,
				     Proto    => 'tcp',
				     Timeout  => $timeout,
				    );
    unless ($sock) {
	# IO::Socket::INET leaves additional error messages in $@
	$@ =~ s/^.*?: //;
	die "Can't connect to $host:$port ($@)";
    }
    $sock;
}


sub _check_sock
{
    #my($self, $req, $sock) = @_;
}

sub _get_sock_info
{
    my($self, $res, $sock) = @_;
    $res->header("Client-Peer" =>
		 $sock->peerhost . ":" . $sock->peerport);
}


sub request
{
    my($self, $request, $proxy, $arg, $size, $timeout) = @_;
    LWP::Debug::trace('()');

    $size ||= 4096;

    # check method
    my $method = $request->method;
    unless ($method =~ /^[A-Za-z0-9_!\#\$%&\'*+\-.^\`|~]+$/) {  # HTTP token
	return new HTTP::Response &HTTP::Status::RC_BAD_REQUEST,
				  'Library does not allow method ' .
				  "$method for 'http:' URLs";
    }

    my $url = $request->url;
    my($host, $port, $fullpath);

    # Check if we're proxy'ing
    if (defined $proxy) {
	# $proxy is an URL to an HTTP server which will proxy this request
	$host = $proxy->host;
	$port = $proxy->port;
	$fullpath = $url->as_string;
    }
    else {
	$host = $url->host;
	$port = $url->port;
	$fullpath = $url->full_path;
    }

    # connect to remote site
    my $socket = $self->_new_socket($host, $port, $timeout);
    $self->_check_sock($request, $socket);
	    
    my $sel = IO::Select->new($socket) if $timeout;

    my $request_line = "$method $fullpath HTTP/1.0$CRLF";

    my $h = $request->headers->clone;
    my $cont_ref = $request->content_ref;
    $cont_ref = $$cont_ref if ref($$cont_ref);
    my $ctype = ref($cont_ref);

    # If we're sending content we *have* to specify a content length
    # otherwise the server won't know a messagebody is coming.
    if ($ctype eq 'CODE') {
	die 'No Content-Length header for request with dynamic content'
	    unless defined($h->header('Content-Length')) ||
		   $h->content_type =~ /^multipart\//;
	# For HTTP/1.1 we could have used chunked transfer encoding...
    } else {
	$h->header('Content-Length' => length $$cont_ref)
	        if defined($$cont_ref) && length($$cont_ref);
    }
    
    # HTTP/1.1 will require us to send the 'Host' header, so we might
    # as well start now.
    my $hhost = $url->netloc;
    $hhost =~ s/^([^\@]*)\@//;  # get rid of potential "user:pass@"
    $h->header('Host' => $hhost) unless defined $h->header('Host');

    # add authorization header if we need them.  HTTP URLs do
    # not really support specification of user and password, but
    # we allow it.
    if (defined($1) && not $h->header('Authorization')) {
	$h->authorization_basic($url->user, $url->password);
    }

    my $buf = $request_line . $h->as_string($CRLF) . $CRLF;
    my $n;  # used for return value from syswrite/sysread

    die "write timeout" if $timeout && !$sel->can_write($timeout);
    $n = $socket->syswrite($buf, length($buf));
    die $! unless defined($n);
    die "short write" unless $n == length($buf);
    LWP::Debug::conns($buf);

    if ($ctype eq 'CODE') {
	while ( ($buf = &$cont_ref()), defined($buf) && length($buf)) {
	    die "write timeout" if $timeout && !$sel->can_write($timeout);
	    $n = $socket->syswrite($buf, length($buf));
	    die $! unless defined($n);
	    die "short write" unless $n == length($buf);
	    LWP::Debug::conns($buf);
	}
    } elsif (defined($$cont_ref) && length($$cont_ref)) {
	die "write timeout" if $timeout && !$sel->can_write($timeout);
	$n = $socket->syswrite($$cont_ref, length($$cont_ref));
	die $! unless defined($n);
	die "short write" unless $n == length($$cont_ref);
	LWP::Debug::conns($buf);
    }
    
    # read response line from server
    LWP::Debug::debug('reading response');

    my $response;
    $buf = '';

    # Inside this loop we will read the response line and all headers
    # found in the response.
    while (1) {
	{
	    die "read timeout" if $timeout && !$sel->can_read($timeout);
	    $n = $socket->sysread($buf, $size, length($buf));
	    die $! unless defined($n);
	    die "unexpected EOF before status line seen" unless $n;
	    LWP::Debug::conns($buf);
	}
	if ($buf =~ s/^(HTTP\/\d+\.\d+)[ \t]+(\d+)[ \t]*([^\012]*)\012//) {
	    # HTTP/1.0 response or better
	    my($ver,$code,$msg) = ($1, $2, $3);
	    $msg =~ s/\015$//;
	    LWP::Debug::debug("$ver $code $msg");
	    $response = HTTP::Response->new($code, $msg);
	    $response->protocol($ver);

	    # ensure that we have read all headers.  The headers will be
	    # terminated by two blank lines
	    until ($buf =~ /^\015?\012/ || $buf =~ /\015?\012\015?\012/) {
		# must read more if we can...
		LWP::Debug::debug("need more header data");
		die "read timeout" if $timeout && !$sel->can_read($timeout);
		$n = $socket->sysread($buf, $size, length($buf));
		die $! unless defined($n);
		die "unexpected EOF before all headers seen" unless $n;
		#LWP::Debug::conns($buf);
	    }

	    # now we start parsing the headers.  The strategy is to
	    # remove one line at a time from the beginning of the header
	    # buffer ($res).
	    my($key, $val);
	    while ($buf =~ s/([^\012]*)\012//) {
		my $line = $1;

		# if we need to restore as content when illegal headers
		# are found.
		my $save = "$line\012"; 

		$line =~ s/\015$//;
		last unless length $line;

		if ($line =~ /^([a-zA-Z0-9_\-]+)\s*:\s*(.*)/) {
		    $response->push_header($key, $val) if $key;
		    ($key, $val) = ($1, $2);
		} elsif ($line =~ /^\s+(.*)/) {
		    unless ($key) {
			$response->header("Client-Warning" =>
					 => "Illegal continuation header");
			$buf = "$save$buf";
			last;
		    }
		    $val .= " $1";
		} else {
		    $response->header("Client-Warning" =>
				      "Illegal header '$line'");
		    $buf = "$save$buf";
		    last;
		}
	    }
	    $response->push_header($key, $val) if $key;
	    last;

	} elsif ((length($buf) >= 5 and $buf !~ /^HTTP\//) or
		 $buf =~ /\012/ ) {
	    # HTTP/0.9 or worse
	    LWP::Debug::debug("HTTP/0.9 assume OK");
	    $response = HTTP::Response->new(&HTTP::Status::RC_OK, "OK");
	    $response->protocol('HTTP/0.9');
	    last;

	} else {
	    # need more data
	    LWP::Debug::debug("need more status line data");
	}
    };
    $response->request($request);
    $self->_get_sock_info($response, $socket);


    my $usebuf = length($buf) > 0;
    $response = $self->collect($arg, $response, sub {
        if ($usebuf) {
	    $usebuf = 0;
	    return \$buf;
	}
	die "read timeout" if $timeout && !$sel->can_read($timeout);
	my $n = $socket->sysread($buf, $size);
	die $! unless defined($n);
	#LWP::Debug::conns($buf);
	return \$buf;
	} );

    $socket->close;

    $response;
}

1;
