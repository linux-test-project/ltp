package LWP::Authen::Digest;
use strict;

require MD5;

sub authenticate
{
    my($class, $ua, $proxy, $auth_param, $response,
       $request, $arg, $size) = @_;

    my($user, $pass) = $ua->get_basic_credentials($auth_param->{realm},
                                                  $request->url, $proxy);
    return $response unless defined $user and defined $pass;

    my $md5 = new MD5;

    my(@digest);
    $md5->add(join(":", $user, $auth_param->{realm}, $pass));
    push(@digest, $md5->hexdigest);
    $md5->reset;

    push(@digest, $auth_param->{nonce});

    $md5->add(join(":", $request->method, $request->url->path));
    push(@digest, $md5->hexdigest);
    $md5->reset;

    $md5->add(join(":", @digest));
    my($digest) = $md5->hexdigest;
    $md5->reset;

    my %resp = map { $_ => $auth_param->{$_} } qw(realm nonce opaque);
    @resp{qw(username uri response)} = ($user, $request->url->path, $digest);

    my(@order) = qw(username realm nonce uri response);
    if($request->method =~ /^(?:POST|PUT)$/) {
	$md5->add($request->content);
	my $content = $md5->hexdigest;
	$md5->reset;
	$md5->add(join(":", @digest[0..1], $content));
	$md5->reset;
	$resp{"message-digest"} = $md5->hexdigest;
	push(@order, "message-digest");
    }
    push(@order, "opaque");
    my @pairs;
    for (@order) {
	next unless defined $resp{$_};
	push(@pairs, "$_=" . qq("$resp{$_}"));
    }

    my $auth_header = $proxy ? "Proxy-Authorization" : "Authorization";
    my $auth_value  = "Digest " . join(", ", @pairs);
    
    # Need to check this isn't a repeated fail!
    my $r = $response;
    while ($r) {
	my $auth = $r->request->header($auth_header);
	if ($auth && $auth eq $auth_value) {
	    # here we know this failed before
	    $response->header("Client-Warning" =>
			      "Credentials for '$user' failed before");
	    return $response;
	}
	$r = $r->previous;
    }

    my $referral = $request->clone;
    $referral->header($auth_header => $auth_value);
    return $ua->request($referral, $arg, $size, $response);
}

1;
