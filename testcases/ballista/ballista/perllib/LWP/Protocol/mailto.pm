#
# $Id: mailto.pm,v 1.1 2004/10/18 17:57:56 mridge Exp $
#
# This module implements the mailto protocol.  It is just a simple
# frontend to the Unix sendmail program.  In the long run this module
# will built using the Mail::Send module.

package LWP::Protocol::mailto;

require LWP::Protocol;
require HTTP::Request;
require HTTP::Response;
require HTTP::Status;

use Carp;

@ISA = qw(LWP::Protocol);

$SENDMAIL = "/usr/lib/sendmail";


sub request
{
    my($self, $request, $proxy, $arg, $size) = @_;

    # check proxy
    if (defined $proxy)
    {
	return new HTTP::Response &HTTP::Status::RC_BAD_REQUEST,
				  'You can not proxy with mail';
    }

    # check method
    $method = $request->method;

    if ($method ne 'POST') {
	return new HTTP::Response &HTTP::Status::RC_BAD_REQUEST,
				  'Library does not allow method ' .
				  "$method for 'mailto:' URLs";
    }

    # check url
    my $url = $request->url;

    my $scheme = $url->scheme;
    if ($scheme ne 'mailto') {
	return new HTTP::Response &HTTP::Status::RC_INTERNAL_SERVER_ERROR,
				  "LWP::file::request called for '$scheme'";
    }
    unless (-x $SENDMAIL) {
	return new HTTP::Response &HTTP::Status::RC_INTERNAL_SERVER_ERROR,
				  "You don't have $SENDMAIL";
    }

    open(SENDMAIL, "| $SENDMAIL -oi -t") or
	return new HTTP::Response &HTTP::Status::RC_INTERNAL_SERVER_ERROR,
				  "Can't run $SENDMAIL: $!";

    my $addr = $url->encoded822addr;

    $request->header('To', $addr);
    print SENDMAIL $request->headers_as_string;
    print SENDMAIL "\n";
    my $content = $request->content;
    if (defined $content) {
	my $contRef = ref($content) ? $content : \$content;
	if (ref($contRef) eq 'SCALAR') {
	    print SENDMAIL $$contRef;
	} elsif (ref($contRef) eq 'CODE') {
	    # Callback provides data
	    my $d;
	    while (length($d = &$contRef)) {
		print SENDMAIL $d;
	    }
	}
    }
    close(SENDMAIL);

    my $response = new HTTP::Response &HTTP::Status::RC_ACCEPTED,
				     'Mail accepted by sendmail';
    $response->header('Content-Type', 'text/plain');
    $response->content("Mail sent to <$addr>\n");

    return $response;
}

1;
