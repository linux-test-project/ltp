#
# $Id: https.pm,v 1.1 2004/10/18 17:57:56 mridge Exp $

use strict;

package LWP::Protocol::https;
require Net::SSL;  # from Crypt-SSLeay

use vars qw(@ISA);

require LWP::Protocol::http;
@ISA=qw(LWP::Protocol::http);

sub _new_socket
{
    my($self, $host, $port, $timeout) = @_;
    local($^W) = 0;  # IO::Socket::INET can be noisy
    my $sock = Net::SSL->new(PeerAddr => $host,
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
    my($self, $req, $sock) = @_;
    my $check = $req->header("If-SSL-Cert-Subject");
    if (defined $check) {
	my $cert = $sock->get_peer_certificate ||
	    die "Missing SSL certificate";
	my $subject = $cert->subject_name;
	die "Bad SSL certificate subject: '$subject' !~ /$check/"
	    unless $subject =~ /$check/;
	$req->remove_header("If-SSL-Cert-Subject");  # don't pass it on
    }
}

sub _get_sock_info
{
    my $self = shift;
    $self->SUPER::_get_sock_info(@_);
    my($res, $sock) = @_;
    $res->header("Client-SSL-Cipher" => $sock->get_cipher);
    my $cert = $sock->get_peer_certificate;
    if ($cert) {
	$res->header("Client-SSL-Cert-Subject" => $cert->subject_name);
	$res->header("Client-SSL-Cert-Issuer" => $cert->issuer_name);
    }
    $res->header("Client-SSL-Warning" => "Peer certificate not verified");
}

1;
