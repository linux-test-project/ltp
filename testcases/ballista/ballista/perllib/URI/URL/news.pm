package URI::URL::news;
require URI::URL;
@ISA = qw(URI::URL);

use URI::Escape;

sub new {
    my($class, $init, $base) = @_;
    my $self = bless { }, $class;
    $self->{'scheme'}  = lc($1) if $init =~ s/^\s*([\w\+\.\-]+)://;
    $self->groupart(uri_unescape($init));
    $self->base($base) if $base;
    $self;
}

sub groupart {
    my $self = shift;
    my $old = $self->{'path'};
    if (@_) {
	my $p = shift;
	if (defined $p && $p =~ /\@/) {
	    # it is an message id
	    $p =~ s/^<(.*)>$/$1/;  # "<" and ">" should not be part of it
	}
	$self->{'path'} = $p;
    }
    $old;
}

*path = \&groupart;

sub article   {
    my $self = shift;
    Carp::croak("Illegal article id name (does not contain '\@')")
      if @_ && $_[0] !~ /\@/;
    my $old = $self->groupart(@_);
    return undef if $old !~ /\@/;
    $old;
}


sub group {
    my $self = shift;
    Carp::croak("Illegal group name (contains '\@')")
      if @_ && $_[0] =~ /\@/;
    my $old = $self->groupart(@_);
    return undef if $old =~ /\@/;
    $old;
}

sub crack
{
    my $self = shift;
    ('news',             # scheme
     undef,              # user
     undef,              # passwd
     undef,              # host
     undef,              # port
     $self->{'path'},    # path
     undef,              # params
     undef,              # query
     undef               # fragment
    )
}


sub as_string {
    my $self = shift;
    my $scheme = $self->{'scheme'} || "news";
    "$scheme:" . uri_escape($self->{'path'});
}

1;
