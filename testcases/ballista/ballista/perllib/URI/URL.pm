package URI::URL;

$VERSION = "4.15";   # $Date: 2004/10/18 17:58:12 $
sub Version { $VERSION; }

require 5.004;

require Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(url);

require AutoLoader;
*AUTOLOAD = \&AutoLoader::AUTOLOAD;

use Carp ();

# Basic lexical elements, taken from RFC 1738:
#
#  safe         = "$" | "-" | "_" | "." | "+"
#  extra        = "!" | "*" | "'" | "(" | ")" | ","
#  national     = "{" | "}" | "|" | "\" | "^" | "~" | "[" | "]" | "`"
#  punctuation  = "<" | ">" | "#" | "%" | <">
#  reserved     = ";" | "/" | "?" | ":" | "@" | "&" | "="
#  escape       = "%" hex hex
#  unreserved   = alpha | digit | safe | extra
#  uchar        = unreserved | escape
#  xchar        = unreserved | reserved | escape

# draft-fielding-url-syntax-05.txt adds '+' to the reserved chars and
# takes '~' out

use strict;
use vars qw($reserved $reserved_no_slash $reserved_no_form $unsafe
	    $Debug $Strict_URL
	   );

$reserved          = ";\\/?:\\@&=+#%"; # RFC 1738 reserved pluss '#' and '%'
$reserved_no_slash = ";?:\\@&=+#%";    # used when escaping path
$reserved_no_form  = ";\\/?:\\@#%";    # used when escaping params and query

# This is the unsafe characters (excluding those reserved)
$unsafe   = "\x00-\x20{}|\\\\^\\[\\]`<>\"\x7F-\xFF";
#$unsafe .= "~";  # according to RFC1738 but not to common practice

$Debug         = 0;     # set to 1 to print URLs on creation
$Strict_URL    = 0;     # see new()

use overload ( '""' => 'as_string', 'fallback' => 1 );

my %Implementor = (); # mapping from scheme to implementation class


# Easy to use constructor
sub url ($;$)
{
    URI::URL->new(@_);
}


# URI::URL objects are implemented as blessed hashes:
#
# Each of the URL components (scheme, netloc, user, password, host,
# port, path, params, query, fragment) are stored under their
# name. The netloc, path, params and query is stored in quoted
# (escaped) form.  The others is stored unquoted (unescaped).
#
# Netloc is special since it is rendundant (same as
# "user:password@host:port") and must be kept in sync with those.
#
# The '_str' key stores a cached stringified version of the URL
# (by definition in quoted form).
# The '_base' key stores the optional base of a relative URL.
#
# The '_orig_url' is used while debugging is on.
#
# Subclasses may add their own keys but must take great care to
# avoid names which might be used in later verions of this module.

sub new
{
    my($class, $init, $base) = @_;

    my $self;
    if (ref $init) {
	$self = $init->clone;
	$self->base($base) if $base;
    } else {
	$init = "" unless defined $init;
	# RFC 1738 appendix suggest that we just ignore extra whitespace
	$init =~ s/\s+//g;
	# Also get rid of any <URL: > wrapper
	$init =~ s/^<(?:URL:)?(.*)>$/$1/;

	# We need a scheme to determine which class to use
	my($scheme) = $init =~ m/^([.+\-\w]+):/;
	if (!$scheme and $base){ # get scheme from base
	    if (ref $base){ # may be object or just a string
		$scheme = $base->scheme;
	    } else {
		$scheme = $1 if $base =~ m/^([.+\-\w]+):/;
	    }
	}
	unless($scheme){
	    Carp::croak("Unable to determine scheme for '$init'")
		if $Strict_URL;
	    $scheme = 'http';
	}
	my $impclass = URI::URL::implementor($scheme);
	unless ($impclass) {
	    Carp::croak("URI::URL scheme '$scheme' is not supported")
		if $Strict_URL;
	    # use generic as fallback
	    require URI::URL::_generic;
	    URI::URL::implementor($scheme, 'URI::URL::_generic');
	    $impclass = 'URI::URL::_generic';
	}

	# hand-off to scheme specific implementation sub-class
	$self->{'_orig_url'} = $init if $Debug;
	$self = $impclass->new($init, $base);
    }
    $self->print_on('STDERR') if $Debug;
    return $self;
}


sub clone
{
    my $self = shift;
    # this work as long as none of the components are references themselves
    bless { %$self }, ref $self;
}


sub implementor
{
    my($scheme, $impclass) = @_;
    unless (defined $scheme) {
	require URI::URL::_generic;
	return 'URI::URL::_generic';
    }

    $scheme = lc($scheme);
    if ($impclass) {
	$impclass->_init_implementor($scheme);
	my $old = $Implementor{$scheme};
	$Implementor{$scheme} = $impclass;
	return $old;
    }

    my $ic = $Implementor{$scheme};
    return $ic if $ic;

    # scheme not yet known, look for internal or
    # preloaded (with 'use') implementation
    $ic = "URI::URL::$scheme";  # default location
    no strict 'refs';
    # check we actually have one for the scheme:
    unless (defined @{"${ic}::ISA"}) {
	# Try to load it
	eval { require "URI/URL/$scheme.pm"; };
	die $@ if $@ && $@ !~ /Can\'t locate/;
	$ic = '' unless defined @{"${ic}::ISA"};
    }
    if ($ic) {
	$ic->_init_implementor($scheme);
	$Implementor{$scheme} = $ic;
    }
    $ic;
}


sub _init_implementor
{
    my($class, $scheme) = @_;
    # Remember that one implementor class may actually
    # serve to implement several URL schemes.

    if ($] < 5.003_17) {
	no strict qw(refs);
	# Setup overloading inheritace - experimental
	%{"${class}::OVERLOAD"} = %URI::URL::OVERLOAD
	    unless defined %{"${class}::OVERLOAD"};
    }
}


# This private method help us implement access to the elements in the
# URI::URL object hash (%$self).  You can set up access to an element
# with a routine similar to this one:
#
#  sub component { shift->_elem('component', @_); }

sub _elem
{
    my $self = shift;
    my $elem = shift;
    my $old = $self->{$elem};
    if (@_) {
	$self->{$elem} = shift;
	$self->{'_str'} = '';        # void cached string
    }
    $old;
}


# Make all standard methods available to all kinds of URLs.  This allow
# us to call these without to much worry when URI::URL::strict(0);

sub bad_method;

*netloc       = \&bad_method;
*user         = \&bad_method;
*password     = \&bad_method;
*host         = \&bad_method;
*port         = \&bad_method;
*default_port = \&bad_method;

*full_path    = \&bad_method;
*epath        = \&bad_method;
*eparams      = \&bad_method;
*equery       = \&bad_method;

*path         = \&bad_method;
*path_components = \&bad_method;
*params       = \&bad_method;
*query        = \&bad_method;
*frag         = \&bad_method;


#
# A U T O  L O A D I N G
#
# The rest of the methods are autoloaded because they should be less
# frequently used.  We define stubs here so that inheritance works as
# it should.
sub newlocal;
sub strict;
sub base;
sub scheme;
sub crack;
sub abs;
sub rel;
sub as_string;
sub eq;
sub print_on;

# Don't need DESTROY but avoid trying to AUTOLOAD it.
sub DESTROY { }

1;
__END__

sub newlocal
{
    require URI::URL::file;
    my $class = shift;
    URI::URL::file->newlocal(@_);  # pass it on the the file class
}

sub strict
{
    return $Strict_URL unless @_;
    my $old = $Strict_URL;
    $Strict_URL = $_[0];
    $old;
}

# Access some attributes of a URL object:
sub base {
    my $self = shift;
    my $base  = $self->{'_base'};

    if (@_) { # set
	my $new_base = shift;
	$new_base = $new_base->abs if ref($new_base);  # unsure absoluteness
	$self->{_base} = $new_base;
    }
    return unless defined wantarray;

    # The base attribute supports 'lazy' conversion from URL strings
    # to URL objects. Strings may be stored but when a string is
    # fetched it will automatically be converted to a URL object.
    # The main benefit is to make it much cheaper to say:
    #   new URI::URL $random_url_string, 'http:'
    if (defined($base) && !ref($base)) {
	$base = new URI::URL $base;
	$self->_elem('_base', $base); # set new object
    }
    $base;
}

sub scheme {
    my $self = shift;
    my $old = $self->{'scheme'};
    if (@_) {
	my $new_scheme = shift;
	if (defined($new_scheme) && length($new_scheme)) {
	    # reparse URL with new scheme
	    my $str = $self->as_string;
	    $str =~ s/^[\w+\-.]+://;
	    my $newself = new URI::URL "$new_scheme:$str";
	    %$self = %$newself;
	    bless $self, ref($newself);
	} else {
	    $self->{'scheme'} = undef;
	}
    }
    $old;
}

sub crack
{
    # should be overridden by subclasses
    my $self = shift;
    ($self->scheme,  # 0: scheme
     undef,          # 1: user
     undef,          # 2: passwd
     undef,          # 3: host
     undef,          # 4: port
     undef,          # 5: path
     undef,          # 6: params
     undef,          # 7: query
     undef           # 8: fragment
    )
}

# These are overridden by _generic (this is just a noop for those schemes that
# do not wish to be a subclass of URI::URL::_generic)
sub abs { shift->clone; }
sub rel { shift->clone; }

# This method should always be overridden in subclasses
sub as_string {
    "<URL>";
}

# Compare two URLs, subclasses will provide a more correct implementation
sub eq {
    my($self, $other) = @_;
    $other = URI::URL->new($other, $self) unless ref $other;
    ref($self) eq ref($other) &&
      $self->scheme eq $other->scheme &&
      $self->as_string eq $other->as_string;  # Case-sensitive
}

# This is set up as an alias for various methods
sub bad_method {
    my $self = shift;
    my $scheme = $self->scheme;
    Carp::croak("Illegal method called for $scheme: URL")
	if $Strict_URL;
    # Carp::carp("Illegal method called for $scheme: URL")
    #     if $^W;
    undef;
}

sub print_on
{
    no strict qw(refs);  # because we use strings as filehandles
    my $self = shift;
    my $fh = shift || 'STDERR';
    my($k, $v);
    print $fh "Dump of URI::URL $self...\n";
    foreach $k (sort keys %$self){
	$v = $self->{$k};
	$v = 'UNDEF' unless defined $v;
	print $fh "  $k\t'$v'\n";
    }
}

1;


#########################################################################
#### D O C U M E N T A T I O N
#########################################################################

=head1 NAME

URI::URL - Uniform Resource Locators (absolute and relative)

=head1 SYNOPSIS

 use URI::URL;

 # Constructors
 $url1 = new URI::URL 'http://www.perl.com/%7Euser/gisle.gif';
 $url2 = new URI::URL 'gisle.gif', 'http://www.com/%7Euser';
 $url3 = url 'http://www.sn.no/'; # handy constructor
 $url4 = $url2->abs;       # get absolute url using base
 $url5 = $url2->abs('http:/other/path');
 $url6 = newlocal URI::URL 'test';

 # Stringify URL
 $str1 = $url->as_string;  # complete escaped URL string
 $str2 = $url->full_path;  # escaped path+params+query
 $str3 = "$url";           # use operator overloading

 # Retrieving Generic-RL components:
 $scheme   = $url->scheme;
 $netloc   = $url->netloc; # see user,password,host,port below
 $path     = $url->path;
 $params   = $url->params;
 $query    = $url->query;
 $frag     = $url->frag;

 # Accessing elements in their escaped form
 $path     = $url->epath;
 $params   = $url->eparams;
 $query    = $url->equery;

 # Retrieving Network location (netloc) components:
 $user     = $url->user;
 $password = $url->password;
 $host     = $url->host;
 $port     = $url->port;   # returns default if not defined

 # Retrieve escaped path components as an array
 @path     = $url->path_components;

 # HTTP query-string access methods
 @keywords = $url->keywords;
 @form     = $url->query_form;

 # All methods above can set the field values, e.g:
 $url->scheme('http');
 $url->host('www.w3.org');
 $url->port($url->default_port);
 $url->base($url5);                      # use string or object
 $url->keywords(qw(dog bones));

 # File methods
 $url = new URI::URL "file:/foo/bar";
 open(F, $url->local_path) or die;

 # Compare URLs
 if ($url->eq("http://www.sn.no")) or die;

=head1 DESCRIPTION

This module implements the URI::URL class representing Uniform
Resource Locators (URL). URLs provide a compact string representation
for resources available via the Internet. Both absolute (RFC 1738) and
relative (RFC 1808) URLs are supported.

URI::URL objects are created by calling new(), which takes as argument
a string representation of the URL or an existing URL object reference
to be cloned. Specific individual elements can then be accessed via
the scheme(), user(), password(), host(), port(), path(), params(),
query() and frag() methods.  In addition escaped versions of the path,
params and query can be accessed with the epath(), eparams() and
equery() methods.  Note that some URL schemes will support all these
methods.

The object constructor new() must be able to determine the scheme for
the URL.  If a scheme is not specified in the URL itself, it will use
the scheme specified by the base URL. If no base URL scheme is defined
then new() will croak if URI::URL::strict(1) has been invoked,
otherwise I<http> is silently assumed.  Once the scheme has been
determined new() then uses the implementor() function to determine
which class implements that scheme.  If no implementor class is
defined for the scheme then new() will croak if URI::URL::strict(1)
has been invoked, otherwise the internal generic URL class is assumed.

Internally defined schemes are implemented by the
URI::URL::I<scheme_name> module.  The URI::URL::implementor() function
can be used to explicitly set the class used to implement a scheme if
you want to override this.


=head1 HOW AND WHEN TO ESCAPE


=over 3

=item This is an edited extract from a URI specification:

The printability requirement has been met by specifying a safe set of
characters, and a general escaping scheme for encoding "unsafe"
characters. This "safe" set is suitable, for example, for use in
electronic mail.  This is the canonical form of a URI.

There is a conflict between the need to be able to represent many
characters including spaces within a URI directly, and the need to be
able to use a URI in environments which have limited character sets
or in which certain characters are prone to corruption. This conflict
has been resolved by use of an hexadecimal escaping method which may
be applied to any characters forbidden in a given context. When URLs
are moved between contexts, the set of characters escaped may be
enlarged or reduced unambiguously.  The canonical form for URIs has
all white spaces encoded.

=item Notes:

A URL string I<must>, by definition, consist of escaped
components. Complete URLs are always escaped.

The components of a URL string must be I<individually> escaped.  Each
component of a URL may have a separate requirements regarding what
must be escaped, and those requirements are also dependent on the URL
scheme.

Never escape an already escaped component string.

=back

This implementation expects an escaped URL string to be passed to
new() and will return a fully escaped URL string from as_string()
and full_path().

Individual components can be manipulated in unescaped or escaped
form. The following methods return/accept unescaped strings:

    scheme                  path
    user                    params
    password                query
    host                    frag
    port

The following methods return/accept partial I<escaped> strings:

    netloc                  eparams
    epath                   equery

I<Partial escaped> means that only reserved characters
(i.e. ':', '@', '/', ';', '?', '=', '&' in addition to '%', '.' and '#')
needs to be escaped when they are to be treated as normal characters.
I<Fully escaped> means that all unsafe characters are escaped. Unsafe
characters are all all control characters (%00-%1F and %7F), all 8-bit
characters (%80-%FF) as well
as '{', '}', '|', '\', '^', '[', ']' '`', '"', '<' and '>'.
Note that the character '~' is B<not> considered
unsafe by this library as it is common practice to use it to reference
personal home pages, but it is still unsafe according to RFC 1738.

=head1 ADDING NEW URL SCHEMES

New URL schemes or alternative implementations for existing schemes
can be added to your own code. To create a new scheme class use code
like:

   package MYURL::foo;
   @ISA = (URI::URL::implementor());   # inherit from generic scheme

The 'URI::URL::implementor()' function call with no parameters returns
the name of the class which implements the generic URL scheme
behaviour (typically C<URI::URL::_generic>). All hierarchical schemes
should be derived from this class.

Your class can then define overriding methods (e.g., new(), _parse()
as required).

To register your new class as the implementor for a specific scheme
use code like:

   URI::URL::implementor('x-foo', 'MYURL::foo');

Any new URL created for scheme 'x-foo' will be implemented by your
C<MYURL::foo> class. Existing URLs will not be affected.


=head1 FUNCTIONS

=over 3

=item $url = URI::URL->new( $url_string [, $base_url] )

This is the object constructor.  It will create a new URI::URL object,
initialized from the URL string.

=item $url = URI::URL->newlocal($path);

Returns an URL object that denotes a path within the local filesystem.
Paths not starting with '/' are interpreted relative to the current
working directory.  This constructor always return an absolute 'file'
URL.

=item $url = url($url_string, [, $base_url])

Alternative constructor function.  The url() function is exported by
the URI::URL module and is easier both to type and read than calling
C<URI::URL->new> directly.  Useful for constructs like this:

   $h = url($str)->host;

This function is just a wrapper for URI::URL->new.

=item URI::URL::strict($bool)

If strict is true then we croak on errors.  The function returns the
previous value.

=item URI::URL::implementor([$scheme, [$class]])

Use this function to get or set implementor class for a scheme.
Returns '' if specified scheme is not supported.  Returns generic URL
class if no scheme specified.

=back

=head1 METHODS

This section describes the methods available for an URI::URL object.
Note that some URL schemes will disallow some of these methods and
will croak if they are used.  Some URL schemes add additional methods
that are described in the sections to follow.

Attribute access methods marked with (*) can take an optional argument
to set the value of the attribute, and they always return the old
value.

=over 3

=item $url->abs([$base])

The abs() method attempts to return a new absolute URI::URL object
for a given URL.  In order to convert a relative URL into an absolute
one, a I<base> URL is required. You can associate a default base with a
URL either by passing a I<base> to the new() constructor when a
URI::URL is created or using the base() method on the object later.
Alternatively you can specify a one-off base as a parameter to the
abs() method.

The rel() method will do the opposite transformation.

Some older parsers used to allow the scheme name to be present in the
relative URL if it was the same as the base URL scheme.  RFC1808 says
that this should be avoided, but you can enable this old behaviour by
setting the $URI::URL::ABS_ALLOW_RELATIVE_SCHEME variable to a TRUE
value.  The difference is demonstrated by the following examples:

  url("http:foo")->abs("http://host/a/b")     ==>  "http:foo"

  local $URI::URL::ABS_ALLOW_RELATIVE_SCHEME = 1;
  url("http:foo")->abs("http://host/a/b")     ==>  "http:/host/a/foo"

You can also have the abs() method ignore if there is too many ".."
segments in the relative URL by setting
$URI::URL::ABS_REMOTE_LEADING_DOTS to a TRUE value.  The difference is
demonstrated by the following examples:

  url("../../../foo")->abs("http://host/a/b")   ==> "http://host/../../foo"

  local $URI::URL::ABS_REMOTE_LEADING_DOTS = 1;
  url("../../../foo")->abs("http://host/a/b")   ==> "http://host/foo"


=item $url->as_string

Returns a string representing the URL in its canonical form.  All
unsafe characters will be escaped.  This method is overloaded as the
perl "stringify" operator, which means that URLs can be used as
strings in many contexts.

=item $url->base (*)

Get/set the base URL associated with the current URI::URL object.  The
base URL matters when you call the abs() method.

=item $url->clone

Returns a copy of the current URI::URL object.

=item $url->crack

Return a 9 element array with the following content:

   0: $url->scheme *)
   1: $url->user
   2: $url->password
   3: $url->host
   4: $url->port
   5: $url->epath
   6: $url->eparams
   7: $url->equery
   8: $url->frag

All elements except I<scheme> will be undefined if the corresponding
URL part is not available.

B<Note:> The scheme (first element) returned by crack will aways be
defined.  This is different from what the $url->scheme returns, since
it will return I<undef> for relative URLs.

=item $url->default_port

Returns the default port number for the URL scheme that the URI::URL
belongs too.

=item $url->eparams (*)

Get/set the URL parameters in escaped form.

=item $url->epath (*)

Get/set the URL path in escaped form.

=item $url->eq($other_url)

Compare two URLs to decide if they match or not.  The rules for how
comparison is made varies for different parts of the URLs; scheme and
netloc comparison is case-insensitive, and escaped chars match their
%XX encoding unless they are "reserved" or "unsafe".

=item $url->equery (*)

Get/set the URL query string in escaped form.

=item $url->full_path

Returns the string "/path;params?query".  This is the string that is
passed to a remote server in order to access the document.

=item $url->frag (*)

Get/set the fragment (unescaped)

=item $url->host (*)

Get/set the host (unescaped)

=item $url->netloc (*)

Get/set the network location in escaped form.  Setting the network
location will affect 'user', 'password', 'host' and 'port'.

=item $url->params (*)

Get/set the URL parameters (unescaped)

=item $url->password (*)

Get/set the password (unescaped)

=item $url->path (*)

Get/set the path (unescaped).  This method will croak if any of the
path components in the return value contain the "/" character.  You
should use the epath() method to be safe.

=item $url->path_components (*)

Get/set the path using a list of unescaped path components.  The
return value will loose the distinction beween '.' and '%2E'.  When
setting a value, a '.' is converted to be a literal '.' and is
therefore encoded as '%2E'.

=item $url->port (*)

Get/set the network port (unescaped)

=item $url->rel([$base])

Return a relative URL if possible.  This is the opposite of what the
abs() method does.  For instance:

   url("http://www.math.uio.no/doc/mail/top.html",
       "http://www.math.uio.no/doc/linux/")->rel

will return a relative URL with path set to "../mail/top.html" and
with the same base as the original URL.

If the original URL already is relative or the scheme or netloc does
not match the base, then a copy of the original URL is returned.


=item $url->print_on(*FILEHANDLE);

Prints a verbose presentation of the contents of the URL object to
the specified file handle (default STDERR).  Mainly useful for
debugging.

=item $url->scheme (*)

Get/set the scheme for the URL.

=item $url->query (*)

Get/set the query string (unescaped).  This method will croak if the
string returned contains both '+' and '%2B' or '=' together with '%3D'
or '%26'.  You should use the equery() method to be safe.

=item $url->user (*)

Get/set the URL user name (unescaped)

=back

=head1 HTTP METHODS

For I<http> URLs you may also access the query string using the
keywords() and the query_form() methods.  Both will croak if the query
is not of the correct format.  The encodings look like this:

  word1+word2+word3..        # keywords
  key1=val1&key2=val2...     # query_form

Note: These functions does not return the old value when they are used
to set a value of the query string.

=over 3

=item $url->keywords (*)

The keywords() method returns a list of unescaped strings.  The method
can also be used to set the query string by passing in the keywords as
individual arguments to the method.

=item $url->query_form (*)

The query_form() method return a list of unescaped key/value pairs.
If you assign the return value to a hash you might loose some values
if the key is repeated (which it is allowed to do).

This method can also be used to set the query sting of the URL like this:

  $url->query_form(foo => 'bar', foo => 'baz', equal => '=');

If the value part of a key/value pair is a reference to an array, then
it will be converted to separate key/value pairs for each value.  This
means that these two calls are equal:

  $url->query_form(foo => 'bar', foo => 'baz');
  $url->query_form(foo => ['bar', 'baz']);

=back

=head1 FILE METHODS

The I<file> URLs implement the local_path() method that returns a path
suitable for access to files within the current filesystem.  These
methods can B<not> be used to set the path of the URL.

=over 3

=item $url->local_path

This method is really just an alias for one of the methods below
depending on what system you run on.

=item $url->unix_path

Returns a path suitable for use on a Unix system.  This method will
croak if any of the path segments contains a "/" or a NULL character.

=item $url->dos_path

Returns a path suitable for use on a MS-DOS or MS-Windows system.

=item $url->mac_path

Returns a path suitable for use on a Macintosh system.

=item $url->vms_path

Returns a path suitable for use on a VMS system.  VMS is a trademark
of Digital.

=back

=head1 GOPHER METHODS

The methods access the parts that are specific for the gopher URLs.
These methods access different parts of the $url->path.

=over 3

=item $url->gtype (*)

=item $url->selector (*)

=item $url->search (*)

=item $url->string (*)

=back

=head1 NEWS METHODS

=over 3

=item $url->group (*)

=item $url->article (*)

=back

=head1 WAIS METHODS

The methods access the parts that are specific for the wais URLs.
These methods access different parts of the $url->path.

=over 3

=item $url->database (*)

=item $url->wtype (*)

=item $url->wpath (*)

=back

=head1 MAILTO METHODS

=over 3

=item $url->address (*)

The mail address can also be accessed with the netloc() method.

=back


=head1 WHAT A URL IS NOT

URL objects do not, and should not, know how to 'get' or 'put' the
resources they specify locations for, anymore than a postal address
'knows' anything about the postal system. The actual access/transfer
should be achieved by some form of transport agent class (see
L<LWP::UserAgent>). The agent class can use the URL class, but should
not be a subclass of it.

=head1 AUTHORS / ACKNOWLEDGMENTS

This module is (distantly) based on the C<wwwurl.pl> code in the
libwww-perl distribution developed by Roy Fielding
<fielding@ics.uci.edu>, as part of the Arcadia project at the
University of California, Irvine, with contributions from Brooks
Cutter.

Gisle Aas <aas@sn.no>, Tim Bunce <Tim.Bunce@ig.co.uk>, Roy Fielding
<fielding@ics.uci.edu> and Martijn Koster <m.koster@webcrawler.com>
(in English alphabetical order) have collaborated on the complete
rewrite for Perl 5, with input from other people on the libwww-perl
mailing list.

If you have any suggestions, bug reports, fixes, or enhancements, send
them to the libwww-perl mailing list at <libwww-perl@ics.uci.edu>.

=head1 COPYRIGHT

Copyright 1995-1997 Gisle Aas.
Copyright 1995 Martijn Koster.

This program is free software; you can redistribute it and/or modify
it under the same terms as Perl itself.

=cut
