package URI::URL::file;
require URI::URL::_generic;
@ISA = qw(URI::URL::_generic);

require Carp;
require Config;

# First we try to determine what kind of system we run on
my $os = $Config::Config{'osname'};
OS: {
    $ostype = 'vms', last if $os eq 'VMS';
    $ostype = 'dos', last if $os =~ /^(?:os2|mswin32|msdos)$/i;
    $ostype = 'mac', last if $os eq "Mac";
    $ostype = 'unix';  # The default
}
# NOTE: If you add more types to this list, remember to add a xxx_path
# method as well.

# This is the BNF found in RFC 1738:
#
# fileurl        = "file://" [ host | "localhost" ] "/" fpath
# fpath          = fsegment *[ "/" fsegment ]
# fsegment       = *[ uchar | "?" | ":" | "@" | "&" | "=" ]
# Note that fsegment can contain '?' (query) but not ';' (param)

sub _parse {
    my($self, $init) = @_;
    # The file URL can't have query
    $self->URI::URL::_generic::_parse($init, qw(netloc path params frag));
}

# sub local_path { ... }
#
# Returns a path suitable for use on the local system (we just
# set up an alias (derived from $ostype) to one of the path methods
# defined below)
*local_path = \&{$ostype . "_path"};

*query  = \&URI::URL::bad_method;
*equery = \&URI::URL::bad_method;

# A U T O  L O A D E R
# Don't remove this comment, it keeps AutoSplit happy!!
# @ISA = qw(AutoLoader)
#
# These methods are autoloaded as needed
sub newlocal;
sub unix_path;
sub dos_path ;
sub mac_path ;
sub vms_path ;
1;
__END__

sub newlocal {
    my($class, $path) = @_;

    Carp::croak("Only implemented for Unix and OS/2 file systems")
      unless $ostype eq "unix" or $^O =~ /os2|mswin32/i;
    # XXX: Should implement the same thing for other systems

    my $url = new URI::URL "file:";
    unless (defined $path and
    	    ($path =~ m:^/: or 
	     ($^O eq 'os2' and Cwd::sys_is_absolute($path)) or
	     ($^O eq 'MSWin32' and $path =~ m<^[A-Za-z]:[\\/]|^[\\/]{2}>))) {
	require Cwd;
	my $cwd = Cwd::fastcwd();
	$cwd =~ s:/?$:/:; # force trailing slash on dir
	$path = (defined $path) ? $cwd . $path : $cwd;
    }
    $url->path($path);
    $url;
}

sub unix_path
{
    my $self = shift;
    my @p;
    for ($self->path_components) {
	Carp::croak("Path component contains '/' or '\0'") if m|[\0/]|;
	if (@p) {
	    next unless length $_;   # skip empty path segments
	    next if $_ eq '.';       # skip these too
	    if ($_ eq '..' && $p[-1] ne '..') {  # go up one level
		pop(@p) if $p[-1] ne '';
		next;
	    }
	}
	push(@p, $_);
    }
    shift(@p) if @p > 1 && $p[0] eq '.';   # './' rendundant if there is more
    return '/' if !@p || (@p == 1 && $p[0] eq '');
    join('/', @p);
}

sub dos_path
{
    my $self = shift;
    my @p;
    for ($self->path_components) {
	Carp::croak("Path component contains '/' or '\\'") if m|[/\\]|;
	push(@p, uc $_);
    }
    my $p = join("\\", @p);
    $p =~ s/^\\([A-Z]:)/$1/;  # Fix drive letter specification
    $p;
}

sub mac_path
{
    my $self = shift;
    my @p;
    for ($self->path_components) {
	Carp::croak("Path component contains ':'") if /:/;
	# XXX: Should probably want to do something about ".." and "."
	# path segments.  I don't know how these are represented in
	# the Machintosh file system.  If these are valid file names
	# then we should split the path ourself, as $u->path_components
	# loose the distinction between '.' and '%2E'.
	push(@p, $_);
    }
    if (@p && $p[0] eq '') {
	shift @p;
    } else {
	unshift(@p, '');
    }
    join(':', @p);
}

sub vms_path
{
    # ????? Can some VMS people please redo this function ??????

    # This is implemented based on what RFC1738 (sec 3.10) says in the
    # VMS file example:
    #
    #  DISK$USER:[MY.NOTES]NOTE123456.TXT
    #
    #      that might become
    #
    #  file:/disk$user/my/notes/note12345.txt
    #
    # BEWARE: I don't have a VMS machine myself so this is pure guesswork!!!

    my $self = shift;
    my @p = $self->path_components;
    my $abs = 0;
    if (@p && $p[0] eq '') {
	shift @p;
	$abs = 1;
    }
    # First I assume there must be a dollar in a disk spesification
    my $p = '';
    $p = uc(shift(@p)) . ":"  if @p && $p[0] =~ /\$/;
    my $file = pop(@p);
    $p .= "[" . join(".", map{uc($_)} @p) . "]" if @p;
    $p .= uc $file;
    # XXX: How is an absolute path different from a relative one??
    $p =~ s/\[/[./ unless $abs;
    # XXX: How is a directory denoted??
    $p;
}

1;
