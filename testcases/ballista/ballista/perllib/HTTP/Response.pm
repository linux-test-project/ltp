#
# $Id: Response.pm,v 1.1 2004/10/18 17:58:15 mridge Exp $

package HTTP::Response;


=head1 NAME

HTTP::Response - Class encapsulating HTTP Responses

=head1 SYNOPSIS

 require HTTP::Response;

=head1 DESCRIPTION

The C<HTTP::Response> class encapsulate HTTP style responses.  A
response consist of a response line, some headers, and a (potential
empty) content. Note that the LWP library will use HTTP style
responses also for non-HTTP protocol schemes.

Instances of this class are usually created and returned by the
C<request()> method of an C<LWP::UserAgent> object:

 #...
 $response = $ua->request($request)
 if ($response->is_success) {
     print $response->content;
 } else {
     print $response->error_as_HTML;
 }

C<HTTP::Response> is a subclass of C<HTTP::Message> and therefore
inherits its methods.  The inherited methods often used are header(),
push_header(), remove_header(), headers_as_string(), and content().
The header convenience methods are also available.  See
L<HTTP::Message> for details.

The following additional methods are available:

=over 4

=cut


require HTTP::Message;
@ISA = qw(HTTP::Message);
$VERSION = sprintf("%d.%02d", q$Revision: 1.1 $ =~ /(\d+)\.(\d+)/);

use HTTP::Status ();
use URI::URL ();
use strict;


=item $r = HTTP::Response->new($rc, [$msg, [$header, [$content]]])

Constructs a new C<HTTP::Response> object describing a response with
response code C<$rc> and optional message C<$msg>.  The message is a
short human readable single line string that explains the response
code.

=cut

sub new
{
    my($class, $rc, $msg, $header, $content) = @_;
    my $self = $class->SUPER::new($header, $content);
    $self->code($rc);
    $self->message($msg);
    $self;
}


sub clone
{
    my $self = shift;
    my $clone = bless $self->SUPER::clone, ref($self);
    $clone->code($self->code);
    $clone->message($self->message);
    $clone->request($self->request->clone) if $self->request;
    # we don't clone previous
    $clone;
}

=item $r->code([$code])

=item $r->message([$message])

=item $r->request([$request])

=item $r->previous([$previousResponse])

These methods provide public access to the member variables.  The
first two containing respectively the response code and the message
of the response.

The request attribute is a reference the request that gave this
response.  It does not have to be the same request as passed to the
$ua->request() method, because there might have been redirects and
authorization retries in between.

The previous attribute is used to link together chains of responses.
You get chains of responses if the first response is redirect or
unauthorized.

=cut

sub code      { shift->_elem('_rc',      @_); }
sub message   { shift->_elem('_msg',     @_); }
sub previous  { shift->_elem('_previous',@_); }
sub request   { shift->_elem('_request', @_); }

=item $r->status_line

Returns the string "E<lt>code> E<lt>message>".  If the message attribute
is not set then the official name of E<lt>code> (see L<HTTP::Status>)
is substituted.

=cut

sub status_line
{
    my $self = shift;
    my $code = $self->{'_rc'}  || "000";
    my $mess = $self->{'_msg'} || HTTP::Status::status_message($code) || "?";
    return "$code $mess";
}

=item $r->base

Returns the base URL for this response.  The return value will be a
reference to a URI::URL object.

The base URL is obtained from one the following sources (in priority
order):

=over 4

=item 1.

Embedded in the document content, for instance <BASE HREF="...">
in HTML documents.

=item 2.

A "Content-Base:" or a "Content-Location:" header in the response.

For backwards compatability with older HTTP implementations we will
also look for the "Base:" header.


=item 3.

The URL used to request this response. This might not be the original
URL that was passed to $ua->request() method, because we might have
received some redirect responses first.

=back

When the LWP protocol modules produce the HTTP::Response object, then
any base URL embedded in the document (step 1) will already have
initialized the "Content-Base:" header. This means that this method
only perform the last 2 steps (the content is not always available
either).

=cut

sub base
{
    my $self = shift;
    my $base = $self->header('Content-Base')     ||  # HTTP/1.1
               $self->header('Content-Location') ||  # HTTP/1.1
               $self->header('Base')             ||  # backwards compatability HTTP/1.0
               $self->request->url;
    $base = URI::URL->new($base) unless ref $base;
    $base;
}


=item $r->as_string

Method returning a textual representation of the response.  Mainly
useful for debugging purposes. It takes no arguments.

=cut

sub as_string
{
    require HTTP::Status;
    my $self = shift;
    my @result;
    #push(@result, "---- $self ----");
    my $code = $self->code;
    my $status_message = HTTP::Status::status_message($code) || "Unknown code";
    my $message = $self->message || "";

    my $status_line = "$code";
    my $proto = $self->protocol;
    $status_line = "$proto $status_line" if $proto;
    $status_line .= " ($status_message)" if $status_message ne $message;
    $status_line .= " $message";
    push(@result, $status_line);
    push(@result, $self->headers_as_string);
    my $content = $self->content;
    if (defined $content) {
	push(@result, $content);
    }
    #push(@result, ("-" x 40));
    join("\n", @result, "");
}

=item $r->is_info

=item $r->is_success

=item $r->is_redirect

=item $r->is_error

These methods indicate if the response was informational, sucessful, a
redirection, or an error.

=cut

sub is_info     { HTTP::Status::is_info     (shift->{'_rc'}); }
sub is_success  { HTTP::Status::is_success  (shift->{'_rc'}); }
sub is_redirect { HTTP::Status::is_redirect (shift->{'_rc'}); }
sub is_error    { HTTP::Status::is_error    (shift->{'_rc'}); }


=item $r->error_as_HTML()

Return a string containing a complete HTML document indicating what
error occurred.  This method should only be called when $r->is_error
is TRUE.

=cut

sub error_as_HTML
{
    my $self = shift;
    my $title = 'An Error Occurred';
    my $body  = $self->status_line;
    return <<EOM;
<HTML>
<HEAD><TITLE>$title</TITLE></HEAD>
<BODY>
<H1>$title</h1>
$body
</BODY>
</HTML>
EOM
}


=item $r->current_age

This function will calculate the "current age" of the response as
specified by E<lt>draft-ietf-http-v11-spec-07> section 13.2.3.  The
age of a response is the time since it was sent by the origin server.
The returned value is a number representing the age in seconds.

=cut

sub current_age
{
    my $self = shift;
    # Implementation of <draft-ietf-http-v11-spec-07> section 13.2.3
    # (age calculations)
    my $response_time = $self->client_date;
    my $date = $self->date;

    my $age = 0;
    if ($response_time && $date) {
	$age = $response_time - $date;  # apparent_age
	$age = 0 if $age < 0;
    }

    my $age_v = $self->header('Age');
    if ($age_v && $age_v > $age) {
	$age = $age_v;   # corrected_received_age
    }

    my $request = $self->request;
    if ($request) {
	my $request_time = $request->date;
	if ($request_time) {
	    # Add response_delay to age to get 'corrected_initial_age'
	    $age += $response_time - $request_time;
	}
    }
    if ($response_time) {
	$age += time - $response_time;
    }
    return $age;
}


=item $r->freshness_lifetime

This function will calculate the "freshness lifetime" of the response
as specified by E<lt>draft-ietf-http-v11-spec-07> section 13.2.4.  The
"freshness lifetime" is the length of time between the generation of a
response and its expiration time.  The returned value is a number
representing the freshness lifetime in seconds.

If the response does not contain an "Expires" or a "Cache-Control"
header, then this function will apply some simple heuristic based on
'Last-Modified' to determine a suitable lifetime.

=cut

sub freshness_lifetime
{
    my $self = shift;

    # First look for the Cache-Control: max-age=n header
    my @cc = $self->header('Cache-Control');
    if (@cc) {
	my $cc;
	for $cc (@cc) {
	    my $cc_dir;
	    for $cc_dir (split(/\s*,\s*/, $cc)) {
		if ($cc_dir =~ /max-age\s*=\s*(\d+)/i) {
		    return $1;
		}
	    }
	}
    }

    # Next possibility is to look at the "Expires" header
    my $date = $self->date || $self->client_date || time;      
    my $expires = $self->expires;
    unless ($expires) {
	# Must apply heuristic expiration
	my $last_modified = $self->last_modified;
	if ($last_modified) {
	    my $h_exp = ($date - $last_modified) * 0.10;  # 10% since last-mod
	    if ($h_exp < 60) {
		return 60;  # minimum
	    } elsif ($h_exp > 24 * 3600) {
		# Should give a warning if more than 24 hours according to
		# <draft-ietf-http-v11-spec-07> section 13.2.4, but I don't
		# know how to do it from this function interface, so I just
		# make this the maximum value.
		return 24 * 3600;
	    }
	    return $h_exp;
	} else {
	    return 3600;  # 1 hour is fallback when all else fails
	}
    }
    return $expires - $date;
}


=item $r->is_fresh

Returns TRUE if the response is fresh, based on the values of
freshness_lifetime() and current_age().  If the response is not longer
fresh, then it has to be refetched or revalidated by the origin
server.

=cut

sub is_fresh
{
    my $self = shift;
    $self->freshness_lifetime > $self->current_age;
}


=item $r->fresh_until

Returns the time when this entiy is no longer fresh.

=cut

sub fresh_until
{
    my $self = shift;
    return $self->freshness_lifetime - $self->current_age + time;
}

1;

=back 

=head1 COPYRIGHT

Copyright 1995-1997 Gisle Aas.

This library is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.

=cut
