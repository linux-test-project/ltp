#####################################################################
#
#       Internal pre-defined generic scheme support
#
# In this implementation all schemes are subclassed from
# URI::URL::_generic. This turns out to have reasonable mileage.
# See also draft-ietf-uri-relative-url-06.txt

package URI::URL::_generic;           # base support for generic-RL's
require URI::URL;
@ISA = qw(URI::URL);

use URI::Escape qw(uri_escape uri_unescape %escapes);


sub new {                               # inherited by subclasses
    my($class, $init, $base) = @_;
    my $url = bless { }, $class;        # create empty object
    $url->_parse($init);                # parse $init into components
    $url->base($base) if $base;
    $url;
}


# Generic-RL parser
# See draft-ietf-uri-relative-url-06.txt Section 2

sub _parse {
    my($self, $u, @comps) = @_;
    return unless defined $u;

    # Deside which components to parse (scheme & path is manatory)
    @comps = qw(netloc query params frag) unless (@comps);
    my %parse = map {$_ => 1} @comps;

    # This parsing code is based on
    #   draft-ietf-uri-relative-url-06.txt Section 2.4

    # 2.4.1
    $self->{'frag'} = uri_unescape($1)
      if $parse{'frag'} && $u =~ s/#(.*)$//;
    # 2.4.2
    $self->{'scheme'} = lc($1) if $u =~ s/^\s*([\w\+\.\-]+)://;
    # 2.4.3
    $self->netloc("$1")	# passing $1 directly fails if netloc is autoloaded
      if $parse{'netloc'} && $u =~ s!^//([^/]*)!!;
    # 2.4.4
    $self->{'query'} = $1
      if $parse{'query'} && $u =~ s/\?(.*)//;
    # 2.4.5
    $self->{'params'} = $1
      if $parse{'params'} && $u =~ s/;(.*)//;

    # 2.4.6
    #
    # RFC 1738 says:
    #
    #     Note that the "/" between the host (or port) and the
    #     url-path is NOT part of the url-path.
    #
    # however, RFC 1808, 2.4.6. says:
    #
    #    Even though the initial slash is not part of the URL path,
    #    the parser must remember whether or not it was present so
    #    that later processes can differentiate between relative
    #    and absolute paths.  Often this is done by simply storing
    #    he preceding slash along with the path.
    #
    # In version < 4.01 of URI::URL we used to strip the leading
    # "/" when asked for $self->path().  This created problems for
    # the consitency of the interface, so now we just consider the
    # slash to be part of the path and we also make an empty path
    # default to "/".

    # we don't test for $parse{path} becase it is mandatory
    $self->{'path'} = $u;
}


# Generic-RL stringify
#
sub as_string
{
    my $self = shift;
    return $self->{'_str'} if $self->{'_str'};

    my($scheme, $netloc, $frag) = @{$self}{qw(scheme netloc frag)};

    my $u = $self->full_path(1);  # path+params+query

    # rfc 1808 says:
    #    Note that the fragment identifier (and the "#" that precedes
    #    it) is not considered part of the URL.  However, since it is
    #    commonly used within the same string context as a URL, a parser
    #    must be able to recognize the fragment when it is present and
    #    set it aside as part of the parsing process.
    $u .= "#" . uri_escape($frag, $URI::URL::unsafe) if defined $frag;

    $u = "//$netloc$u" if defined $netloc;
    $u = "$scheme:$u" if $scheme;
    # Inline: uri_escape($u, $URI::URL::unsafe);
    $u =~ s/([$URI::URL::unsafe])/$escapes{$1}/go;
    $self->{'_str'} = $u;  # set cache and return
}

# Generic-RL stringify full path "path;params?query"
#
sub full_path
{
    my($self, $dont_escape)  = @_;
    my($path, $params, $query) = @{$self}{'path', 'params', 'query'};
    my $p = '';
    $p .= $path if defined $path;
    # see comment in _parse 2.4.6 about the next line
    $p = "/$p" if defined($self->{netloc}) && $p !~ m:^/:;
    $p .= ";$params" if defined $params;
    $p .= "?$query"  if defined $query;
    return $p if $dont_escape;
    # Inline: URI::Escape::uri_escape($p, $URI::URL::unsafe);
    $p =~ s/([$URI::URL::unsafe])/$escapes{$1}/go;
    $p;
}

# default_port()
#
# subclasses will usually want to override this
#
sub default_port { undef; }


#####################################################################
#
# Methods to handle URL's elements

# These methods always return the current value,
# so you can use $url->path to read the current value.
# If a new value is passed, e.g. $url->path('foo'),
# it also sets the new value, and returns the previous value.
# Use $url->path(undef) to set the value to undefined.

sub netloc {
    my $self = shift;
    my $old = $self->_elem('netloc', @_);
    return $old unless @_;

    # update fields derived from netloc
    my $nl = $self->{'netloc'} || '';
    if ($nl =~ s/^([^:@]*):?(.*?)@//){
	$self->{'user'}     = uri_unescape($1);
	$self->{'password'} = uri_unescape($2) if $2 ne '';
    }
    if ($nl =~ /^([^:]*):?(\d*)$/){
	my $port = $2;
	# Since this happes so frequently, we inline this call:
	#    my $host = uri_unescape($1);
	my $host = $1;
	$host =~ s/%([\dA-Fa-f]{2})/chr(hex($1))/eg;
	$self->{'host'} = $host;
	if ($port ne '') {
	    $self->{'port'} = $port;
	    if ($self->default_port == $port) {
		$self->{'netloc'} =~ s/:\d+//;
	    }
	} elsif (defined $self->{'netloc'}) {
	    $self->{'netloc'} =~ s/:$//;  # handle empty port spec
	}
    }
    $self->{'_str'} = '';
    $old;
}


# A U T O  L O A D E R
# Don't remove this comment, it keeps AutoSplit happy!!
# @ISA = qw(AutoLoader)
#
# The rest of the methods are only loaded on demand.  Stubs are neccesary
# for inheritance to work.

#sub netloc;  # because netloc is used by the _parse()
sub user;
sub password;
sub host;
sub port;
sub _netloc_elem;
sub epath;
sub path;
sub path_components;
sub eparams;
sub params;
sub equery;
sub query;
sub frag;
sub crack;
sub abs;
sub rel;
sub eq;

1;
__END__


# Fields derived from generic netloc:
sub user     { shift->_netloc_elem('user',    @_); }
sub password { shift->_netloc_elem('password',@_); }
sub host     { shift->_netloc_elem('host',    @_); }

sub port {
    my $self = shift;
    my $old = $self->_netloc_elem('port', @_);
    defined($old) ? $old : $self->default_port;
}

sub _netloc_elem {
    my($self, $elem, @val) = @_;
    my $old = $self->_elem($elem, @val);
    return $old unless @val;

    # update the 'netloc' element
    my $nl = '';
    my $host = $self->{'host'};
    if (defined $host) {  # can't be any netloc without any host
	my $user = $self->{'user'};
	$nl .= uri_escape($user, $URI::URL::reserved) if defined $user;
	$nl .= ":" . uri_escape($self->{'password'}, $URI::URL::reserved)
	  if defined($user) and defined($self->{'password'});
	$nl .= '@' if length $nl;
	$nl .= uri_escape($host, $URI::URL::reserved);
	my $port = $self->{'port'};
	$nl .= ":$port" if defined($port) && $port != $self->default_port;
    }
    $self->{'netloc'} = $nl;
    $self->{'_str'} = '';
    $old;
}

sub epath {
     my $self = shift;
     my $old = $self->_elem('path', @_);
     return '/' if !defined($old) || !length($old);
     return "/$old" if $old !~ m|^/| && defined $self->{'netloc'};
     $old;
}

sub path {
    my $self = shift;
    my $old = $self->_elem('path',
		      map uri_escape($_, $URI::URL::reserved_no_slash), @_);
    return unless defined wantarray;
    return '/' if !defined($old) || !length($old);
    Carp::croak("Path components contain '/' (you must call epath)")
	if $old =~ /%2[fF]/ and !@_;
    $old = "/$old" if $old !~ m|^/| && defined $self->{'netloc'};
    return uri_unescape($old);
}

sub path_components {
    my $self = shift;
    my $old = $self->{'path'};
    $old = '' unless defined $old;
    $old = "/$old" if $old !~ m|^/| && defined $self->{'netloc'};
    if (@_) {
	$self->_elem('path',
		     join("/", map {uri_escape($_, $URI::URL::reserved)} @_));
    }
    map { uri_unescape($_) } split("/", $old, -1);
}

sub eparams  { shift->_elem('params',  @_); }

sub params {
    my $self = shift;
    my $old = $self->_elem('params', map {uri_escape($_,$URI::URL::reserved_no_form)} @_);
    return uri_unescape($old) if defined $old;
    undef;
}

sub equery   { shift->_elem('query',   @_); }

sub query {
    my $self = shift;
    my $old = $self->_elem('query', map { uri_escape($_, $URI::URL::reserved_no_form) } @_);
    if (defined(wantarray) && defined($old)) {
	if ($old =~ /%(?:26|2[bB]|3[dD])/) {  # contains escaped '=' '&' or '+'
	    my $mess;
	    for ($old) {
		$mess = "Query contains both '+' and '%2B'"
		  if /\+/ && /%2[bB]/;
		$mess = "Form query contains escaped '=' or '&'"
		  if /=/  && /%(?:3[dD]|26)/;
	    }
	    if ($mess) {
		Carp::croak("$mess (you must call equery)");
	    }
	}
	# Now it should be safe to unescape the string without loosing
	# information
	return uri_unescape($old);
    }
    undef;

}

# No efrag method because the fragment is always stored unescaped
sub frag     { shift->_elem('frag', @_); }

sub crack
{
    my $self = shift;
    return $self unless wantarray;
    my @c = @{$self}{qw(scheme user password host port path params query frag)};
    if (!$c[0]) {
	# try to determine scheme
	my $base = $self->base;
	$c[0] = $base->scheme if $base;
	$c[0] ||= 'http';  # last resort, default in URI::URL::new
    }
    $c[4] ||= $self->default_port;
    @c;
}

# Generic-RL: Resolving Relative URL into an Absolute URL
#
# Based on RFC1808 section 4
#
sub abs
{
    my($self, $base, $allow_relative_scheme) = @_;
    $allow_relative_scheme = $URI::URL::ABS_ALLOW_RELATIVE_SCHEME if @_ < 3;
    my $embed = $self->clone;

    $base = $self->base unless $base;      # default to default base
    return $embed unless $base;            # we have no base (step1)

    $base = new URI::URL $base unless ref $base; # make obj if needed

    my($scheme, $host, $path, $params, $query, $frag) =
	@{$embed}{qw(scheme host path params query frag)};

    # just use base if we are empty             (2a)
    return $base->clone
      unless grep(defined($_) && $_ ne '',
		  $scheme,$host,$port,$path,$params,$query,$frag);

    # if we have a scheme we must already be absolute   (2b),
    #
    # but sec. 5.2 also says: Some older parsers allow the scheme name
    # to be present in a relative URL if it is the same as the base
    # URL scheme.  This is considered to be a loophole in prior
    # specifications of the partial URLs and should be avoided by
    # future parsers.
    #
    # The old behavoir can be enabled by passing a TRUE value to the
    # $allow_relative_scheme parameter.
    return $embed if $scheme &&
      (!$allow_relative_scheme || $scheme ne $base->{'scheme'});

    $embed->{'_str'} = '';                      # void cached string
    $embed->{'scheme'} = $base->{'scheme'};     # (2c)

    return $embed if $embed->{'netloc'};        # (3)
    $embed->netloc($base->{'netloc'});          # (3)

    return $embed if $path =~ m:^/:;            # (4)

    if ($path eq '') {                          # (5)
	$embed->{'path'} = $base->{'path'};     # (5)

	return $embed if defined $embed->{'params'}; # (5a)
	$embed->{'params'} = $base->{'params'};      # (5a)

	return $embed if defined $embed->{'query'};  # (5b)
	$embed->{'query'} = $base->{'query'};        # (5b)

	return $embed;
    }

    # (Step 6)  # draft 6 suggests stack based approach

    my $basepath = $base->{'path'};
    my $relpath  = $embed->{'path'};

    $basepath =~ s!^/!!;
    $basepath =~ s!/$!/.!;                # prevent empty segment
    my @path = split('/', $basepath);     # base path into segments
    pop(@path);                           # remove last segment

    $relpath =~ s!/$!/.!;                 # prevent empty segment

    push(@path, split('/', $relpath));    # append relative segments

    my @newpath = ();
    my $isdir = 0;
    my $segment;

    foreach $segment (@path) {            # left to right
	if ($segment eq '.') {            # ignore "same" directory
	    $isdir = 1;
	}
	elsif ($segment eq '..') {
	    $isdir = 1;
	    my $last = pop(@newpath);
	    if (!defined $last) {         # nothing to pop
		push(@newpath, $segment); # so must append
	    }
	    elsif ($last eq '..') {       # '..' cannot match '..'
		# so put back again, and append
		push(@newpath, $last, $segment);
	    }
	    #else
		# it was a component,
		# keep popped
	} else {
	    $isdir = 0;
	    push(@newpath, $segment);
	}
    }

    if ($URI::URL::ABS_REMOTE_LEADING_DOTS) {
	shift @newpath while @newpath && $newpath[0] =~ /^\.\.?$/;
    }

    $embed->{'path'} = '/' . join('/', @newpath) .
	($isdir && @newpath ? '/' : '');

    $embed;
}

# The oposite of $url->abs.  Return a URL as much relative as possible
sub rel {
    my($self, $base) = @_;
    my $rel = $self->clone;
    $base = $self->base unless $base;
    return $rel unless $base;
    $base = new URI::URL $base unless ref $base;
    $rel->base($base);

    my($scheme, $netloc, $path) = @{$rel}{qw(scheme netloc path)};
    if (!defined($scheme) && !defined($netloc)) {
	# it is already relative
	return $rel;
    }

    my($bscheme, $bnetloc, $bpath) = @{$base}{qw(scheme netloc path)};
    for ($bscheme, $bnetloc, $netloc) { $_ = '' unless defined }

    unless ($scheme eq $bscheme && $netloc eq $bnetloc) {
	# different location, can't make it relative
	return $rel;
    }

    for ($path, $bpath) {  $_ = "/$_" unless m,^/,; }

    # Make it relative by eliminating scheme and netloc
    $rel->{'scheme'} = undef;
    $rel->netloc(undef);

    # This loop is based on code from Nicolai Langfeldt <janl@ifi.uio.no>.
    # First we calculate common initial path components length ($li).
    my $li = 1;
    while (1) {
	my $i = index($path, '/', $li);
	last if $i < 0 ||
                $i != index($bpath, '/', $li) ||
	        substr($path,$li,$i-$li) ne substr($bpath,$li,$i-$li);
	$li=$i+1;
    }
    # then we nuke it from both paths
    substr($path, 0,$li) = '';
    substr($bpath,0,$li) = '';

    if ($path eq $bpath && defined($rel->frag) && !defined($rel->equery)) {
        $rel->epath('');
    } else {
        # Add one "../" for each path component left in the base path
        $path = ('../' x $bpath =~ tr|/|/|) . $path;
	$path = "./" if $path eq "";
        $rel->epath($path);
    }

    $rel;
}


# Compare two URLs
sub eq {
    my($self, $other) = @_;
    local($^W) = 0; # avoid warnings if we compare undef values
    $other = URI::URL->new($other, $self) unless ref $other;

    # Compare scheme and netloc
    return 0 if ref($self) ne ref($other);                # must be same class
    return 0 if $self->scheme ne $other->scheme;          # Always lower case
    return 0 if lc($self->netloc) ne lc($other->netloc);  # Case-insensitive

    # Compare full_path:
    # According to <draft-ietf-http-v11-spec-05>:
    # Characters other than those in the "reserved" and "unsafe" sets
    # are equivalent to their %XX encodings.
    my $fp1 = $self->full_path;
    my $fp2 = $other->full_path;
    for ($fp1, $fp2) {
	s,%([\dA-Fa-f]{2}),
	  my $x = $1;
	  my $c = chr(hex($x));
	  $c =~ /^[;\/?:\@&=+\"\#%<>\0-\040\177]/ ? "%\L$x" : $c;
	,eg;
    }
    return 0 if $fp1 ne $fp2;
    return 0 if $self->frag ne $other->frag;
    1;
}

1;
