#
# $Id: data.pm,v 1.1 2004/10/18 17:57:56 mridge Exp $
#
# Implements access to data:-URLs as specified in
# draft-masinter-url-data-02.txt

package LWP::Protocol::data;

require HTTP::Response;
require HTTP::Status;

require LWP::Protocol;
@ISA = qw(LWP::Protocol);

use HTTP::Date qw(time2str);
require LWP;  # needs version number

sub request
{
    my($self, $request, $proxy, $arg, $size) = @_;

    # check proxy
    if (defined $proxy)
    {
	return new HTTP::Response &HTTP::Status::RC_BAD_REQUEST,
				  'You can not proxy with data';
    }

    # check method
    if ($request->method ne 'GET') {
	return new HTTP::Response &HTTP::Status::RC_BAD_REQUEST,
				  'Library does not allow method ' .
				  "$method for 'data:' URLs";
    }

    my $url = $request->url;
    my $response = new HTTP::Response &HTTP::Status::RC_OK, "Document follows";

    my($media_type, $params) = $url->media_type;
    $media_type .= ";$params" if $params;

    my $data = $url->data;
    $response->header('Content-Type'   => $media_type,
		      'Content-Length' => length($data),
		      'Date'           => time2str(time),
		      'Server'         => "libwww-perl-internal/$LWP::VERSION"
		     );
    $response->content($data);

    return $response;
}

1;
