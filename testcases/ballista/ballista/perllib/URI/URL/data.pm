package URI::URL::data;

# Implements data:-URLs as specified in draft-masinter-url-data-02.txt
#
# Abstract
#
# A new URL scheme, "data", is defined. It allows inclusion of small
# data items as "immediate" data, as if it had been included externally.
#
# Description
#
# Some applications that use URLs also have a need to embed (small)
# media type data directly inline. This document defines a new URL
# scheme that would work like 'immediate addressing'. The URLs are of
# the form:
#
#        data:[<mediatype>][;base64],<data>
#

require URI::URL;
@ISA = qw(URI::URL);

use URI::Escape;
use MIME::Base64 ();

# as defined in rfc1522.txt
my $tspecial  = qq(()<>@,;:/[]?.=);
my $tokenchar = qq([^\\s\000-\037\177-\377\Q$tspecial\E]);


sub new {
    my($class, $init, $base) = @_;

    my $self = bless { }, $class;

    $self->{'scheme'} = lc($1) if $init =~ s/^\s*([\w\+\.\-]+)://;
    $self->{'frag'} = $1 if $init =~ s/\#(.*)$//;

    my $type = "";
    if ($init =~ s/^($tokenchar+\/$tokenchar+)//o) {
	$type = $1;
    }
    while ($init =~ s/^;($tokenchar+)=([^,;]*)//o) {
	# XXX should we unescape the parst
	$type .= ";$1=$2";
    }

    $self->{'media_type'} = $type;

    my $base64;
    if ($init =~ s/^;base64//i) {
	$base64 = 1;
    }
    
    $init =~ s/^,//;
    if ($base64) {
	$self->{'base64'} = uri_unescape($init);
    } else {
	$self->{'data'} = uri_unescape($init);
    }
    $self->base($base) if $base;
    $self;
}

sub media_type
{
    my $self = shift;
    my $old = $self->{'media_type'};
    if (@_) {
	$self->{'media_type'} = shift || "";
	delete $self->{'_str'};
    }
    my($type, $param) = split(/;/, $old, 2);
    if ($type) {
	$type = lc $type;
    } else {
	$type = "text/plain";
	$param = "charset=US-ASCII" unless $param;
    }
    if (wantarray) {
	return ($type, $param);
    } else {
	return $type;
    }
}

sub data
{
    my $self = shift;
    my $old_data;
    my $old_base64;
    $old_data = $self->{'data'};
    $old_base64 = $self->{'base64'};
    if (@_) {
	if ($_[1]) { # base64 flag
	    $self->{'base64'} = $_[0];
	    delete $self->{'data'};
	} else {
	    $self->{'data'} = $_[0];
	    delete $self->{'base64'};
	}
	delete $self->{'_str'};
    }
    unless (defined $old_data) {
	$old_data = MIME::Base64::decode($old_base64);
	$self->{'data'} = $old_data unless @_;
    }
    $old_data;
}


sub crack
{
    my $self = shift;
    ($self->{'scheme'}
     || 'data',          # scheme
     undef,              # user
     undef,              # passwd
     undef,              # host
     undef,              # port
     $self->data,        # path
     $self->{'media_type'},  # params
     undef,              # query
     $self->{'frag'}     # fragment
    )
}

sub as_string {
    my $self = shift;
    return $self->{'_str'} if $self->{'_str'};
    my $str = ($self->{'scheme'} || 'data') . ":";
    $str .= $self->{'media_type'};
    if (defined $self->{'base64'}) {
	$str .= ";base64,$self->{'base64'}";
    } else {
	my $urlenc = uri_escape($self->{'data'});
	my $base64 = MIME::Base64::encode($self->{'data'});
	if (length($base64) + 7 < length($urlenc)) {
	    $str .= ";base64,$base64";
	    $self->{'base64'} = $base64;
	} else {
	    $str .= ",$urlenc";
	}
    }
    $self->{'_str'} = $str;
}

sub eq {
    my($self, $other) = @_;
    return 0 if ref($self) ne ref($other);
    return 0 if $self->scheme ne $other->scheme;

    my $mt1 = join(";", $self->media_type);
    my $mt2 = join(";", $other->media_type);
    return 0 if $mt1 ne $mt2;

    $self->data eq $other->data;
}

1;
