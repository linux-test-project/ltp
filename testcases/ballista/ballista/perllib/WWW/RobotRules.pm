# $Id: RobotRules.pm,v 1.1 2004/10/18 17:58:13 mridge Exp $

package WWW::RobotRules;

=head1 NAME

WWW::RobotsRules - Parse robots.txt files

=head1 SYNOPSIS

 require WWW::RobotRules;
 my $robotsrules = new WWW::RobotRules 'MOMspider/1.0';

 use LWP::Simple qw(get);

 $url = "http://some.place/robots.txt";
 my $robots_txt = get $url;
 $robotsrules->parse($url, $robots_txt);

 $url = "http://some.other.place/robots.txt";
 my $robots_txt = get $url;
 $robotsrules->parse($url, $robots_txt);

 # Now we are able to check if a URL is valid for those servers that
 # we have obtained and parsed "robots.txt" files for.
 if($robotsrules->allowed($url)) {
     $c = get $url;
     ...
 }

=head1 DESCRIPTION

This module parses a F<robots.txt> file as specified in
"A Standard for Robot Exclusion", described in
<URL:http://info.webcrawler.com/mak/projects/robots/norobots.html>
Webmasters can use the F<robots.txt> file to disallow conforming
robots access to parts of their WWW server.

The parsed file is kept in the WWW::RobotRules object, and this object
provide methods to check if access to a given URL is prohibited.  The
same WWW::RobotRules object can parse multiple F<robots.txt> files.

The following methods are provided:

=over 4

=cut

$VERSION = sprintf("%d.%02d", q$Revision: 1.1 $ =~ /(\d+)\.(\d+)/);
sub Version { $VERSION; }


use URI::URL ();
use strict;


=item $rules = new WWW::RobotRules 'MOMspider/1.0'

This is the constructor for WWW::RobotRules objects.  The first 
argument given to new() is the name of the robot. 

=cut

sub new {
    my($class, $ua) = @_;

    # This ugly hack is needed to ensure backwards compatability.
    # The "WWW::RobotRules" class is now really abstract.
    $class = "WWW::RobotRules::InCore" if $class eq "WWW::RobotRules";

    my $self = bless { }, $class;
    $self->agent($ua);
    $self;
}


=item $rules->parse($url, $content, $fresh_until)

The parse() method takes as arguments the URL that was used to
retrieve the F</robots.txt> file, and the contents of the file.

=cut

sub parse {
    my($self, $url, $txt, $fresh_until) = @_;

    $url = new URI::URL $url unless ref($url);	# make it URL
    my $netloc = $url->netloc;

    $self->clear_rules($netloc);
    $self->fresh_until($netloc, $fresh_until || (time + 365*24*3600));

    my $ua;
    my $is_me = 0;		# 1 iff this record is for me
    my $is_anon = 0;		# 1 iff this record is for *
    my @me_disallowed = ();	# rules disallowed for me
    my @anon_disallowed = ();	# rules disallowed for *

    # blank lines are significant, so turn CRLF into LF to avoid generating
    # false ones
    $txt =~ s/\015\012/\012/g;

    # split at \012 (LF) or \015 (CR) (Mac text files have just CR for EOL)
    for(split(/[\012\015]/, $txt)) {

	# Lines containing only a comment are discarded completely, and
        # therefore do not indicate a record boundary.
	next if /^\s*\#/;

	s/\s*\#.*//;        # remove comments at end-of-line

	if (/^\s*$/) {	    # blank line
	    last if $is_me; # That was our record. No need to read the rest.
	    $is_anon = 0;
	}
        elsif (/^User-Agent:\s*(.*)/i) {
	    $ua = $1;
	    $ua =~ s/\s+$//;
	    if ($is_me) {
		# This record already had a User-agent that
		# we matched, so just continue.
	    }
	    elsif ($ua eq '*') {
		$is_anon = 1;
	    }
	    elsif($self->is_me($ua)) {
		$is_me = 1;
	    }
	}
	elsif (/^Disallow:\s*(.*)/i) {
	    unless (defined $ua) {
		warn "RobotRules: Disallow without preceding User-agent\n";
		$is_anon = 1;  # assume that User-agent: * was intended
	    }
	    my $disallow = $1;
	    $disallow =~ s/\s+$//;
	    if (length $disallow) {
		$disallow = URI::URL->new($disallow, $url)->full_path;
	    }

	    if ($is_me) {
		push(@me_disallowed, $disallow);
	    }
	    elsif ($is_anon) {
		push(@anon_disallowed, $disallow);
	    }
	}
	else {
	    warn "RobotRules: Unexpected line: $_\n";
	}
    }

    if ($is_me) {
	$self->push_rules($netloc, @me_disallowed);
    } else {
	$self->push_rules($netloc, @anon_disallowed);
    }
}

# is_me()
#
# Returns TRUE if the given name matches the
# name of this robot
#
sub is_me {
    my($self, $ua) = @_;
    my $me = $self->agent;
    return index(lc($ua), lc($me)) >= 0;
}

=item $rules->allowed($url)

Returns TRUE if this robot is allowed to retrieve this URL.

=cut

sub allowed {
    my($self, $url) = @_;
    $url = URI::URL->new($url) unless ref $url;	# make it URL

    my $netloc = $url->netloc;

    my $fresh_until = $self->fresh_until($netloc);
    return -1 if !defined($fresh_until) || $fresh_until < time;

    my $str = $url->full_path;
    my $rule;
    for $rule ($self->rules($netloc)) {
	return 1 unless length $rule;
	return 0 if index($str, $rule) == 0;
    }
    return 1;
}

# The following methods must be provided by the subclass.
sub agent;
sub visit;
sub no_visits;
sub last_visits;
sub fresh_until;
sub push_rules;
sub clear_rules;
sub rules;
sub dump;

package WWW::RobotRules::InCore;

use vars qw(@ISA);
@ISA = qw(WWW::RobotRules);

=item $rules->agent([$name])

Get/set the agent name. NOTE: Changing the agent name will clear the robots.txt
rules and expire times out of the cache.

=cut

sub agent {
    my ($self, $name) = @_;
    my $old = $self->{'ua'};
    if ($name) {
	delete $self->{'loc'};   # all old info is now stale
	$name =~ s!/?\s*\d+.\d+\s*$!!;  # loose version
	$self->{'ua'}=$name;
    }
    $old;
}

sub visit {
    my($self, $netloc, $time) = @_;
    $time ||= time;
    $self->{'loc'}{$netloc}{'last'} = $time;
    
    my $count = \$self->{'loc'}{$netloc}{'count'};
    if (!defined $$count) {
	$$count = 1;
    } else {
	$$count++;
    }
}

sub no_visits {
    my ($self, $netloc) = @_;
    $self->{'loc'}{$netloc}{'count'};
}

sub last_visit {
    my ($self, $netloc) = @_;
    $self->{'loc'}{$netloc}{'last'};
}

sub fresh_until {
    my ($self, $netloc, $fresh_until) = @_;
    my $old = $self->{'loc'}{$netloc}{'fresh'};
    if (defined $fresh_until) {
	$self->{'loc'}{$netloc}{'fresh'} = $fresh_until;
    }
    $old;
}

sub push_rules {
    my($self, $netloc, @rules) = @_;
    push (@{$self->{'loc'}{$netloc}{'rules'}}, @rules);
}

sub clear_rules {
    my($self, $netloc) = @_;
    delete $self->{'loc'}{$netloc}{'rules'};
}

sub rules {
    my($self, $netloc) = @_;
    if (defined $self->{'loc'}{$netloc}{'rules'}) {
	return @{$self->{'loc'}{$netloc}{'rules'}};
    } else {
	return ();
    }
}

sub dump
{
    my $self = shift;
    for (keys %$self) {
	next if $_ eq 'loc';
	print "$_ = $self->{$_}\n";
    }
    for (keys %{$self->{'loc'}}) {
	my @rules = $self->rules($_);
	print "$_: ", join("; ", @rules), "\n";
	
    }
}

1;

__END__

=back

=head1 ROBOTS.TXT

The format and semantics of the "/robots.txt" file are as follows
(this is an edited abstract of
<URL:http://info.webcrawler.com/mak/projects/robots/norobots.html>):

The file consists of one or more records separated by one or more
blank lines. Each record contains lines of the form

  <field-name>: <value>

The field name is case insensitive.  Text after the '#' character on a
line is ignored during parsing.  This is used for comments.  The
following <field-names> can be used:

=over 3

=item User-Agent

The value of this field is the name of the robot the record is
describing access policy for.  If more than one I<User-Agent> field is
present the record describes an identical access policy for more than
one robot. At least one field needs to be present per record.  If the
value is '*', the record describes the default access policy for any
robot that has not not matched any of the other records.

=item Disallow

The value of this field specifies a partial URL that is not to be
visited. This can be a full path, or a partial path; any URL that
starts with this value will not be retrieved

=back

=head1 ROBOTS.TXT EXAMPLES

The following example "/robots.txt" file specifies that no robots
should visit any URL starting with "/cyberworld/map/" or "/tmp/":

  User-agent: *
  Disallow: /cyberworld/map/ # This is an infinite virtual URL space
  Disallow: /tmp/ # these will soon disappear

This example "/robots.txt" file specifies that no robots should visit
any URL starting with "/cyberworld/map/", except the robot called
"cybermapper":

  User-agent: *
  Disallow: /cyberworld/map/ # This is an infinite virtual URL space

  # Cybermapper knows where to go.
  User-agent: cybermapper
  Disallow:

This example indicates that no robots should visit this site further:

  # go away
  User-agent: *
  Disallow: /

=head1 SEE ALSO

L<LWP::RobotUA>, L<WWW::RobotRules::AnyDBM_File>

=cut
