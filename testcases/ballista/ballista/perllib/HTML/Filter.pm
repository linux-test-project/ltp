package HTML::Filter;

require HTML::Parser;
@ISA=qw(HTML::Parser);

$VERSION = sprintf("%d.%02d", q$Revision: 1.1 $ =~ /(\d+)\.(\d+)/);

sub declaration { $_[0]->output("<!$_[1]>")     }
sub comment     { $_[0]->output("<!--$_[1]-->") }
sub start       { $_[0]->output($_[4])          }
sub end         { $_[0]->output($_[2])          }
sub text        { $_[0]->output($_[1])          }

sub output      { print $_[1] }

1;

__END__

=head1 NAME

HTML::Filter - Filter HTML text through the parser

=head1 SYNOPSIS

 require HTML::Filter;
 $p = HTML::Filter->new->parse_file("index.html");

=head1 DESCRIPTION

The I<HTML::Filter> is an HTML parser that by default prints the
original text parsed (a slow version of cat(1) basically).  You can
override the callback methods to modify the filtering for some of the
HTML elements and you can override output() method which is called to
print the HTML text.

The I<HTML::Filter> is a subclass of I<HTML::Parser>. This means that
the document should be given to the parser by calling the $p->parse()
or $p->parse_file() methods.

=head1 EXAMPLES

The first example is a filter that will remove all comments from an
HTML file.  This is achieved by simply overriding the comment method
to do nothing.

  package CommentStripper;
  require HTML::Filter;
  @ISA=qw(HTML::Filter);
  sub comment { }  # ignore comments

The second example shows a filter that will remove any E<lt>TABLE>s
found in the HTML file.  We specialize the start() and end() methods
to count table tags and then make output not happen when inside a
table.

  package TableStripper;
  require HTML::Filter;
  @ISA=qw(HTML::Filter);
  sub start
  {
     my $self = shift;
     $self->{table_seen}++ if $_[0] eq "table";
     $self->SUPER::start(@_);
  }  

  sub end
  {
     my $self = shift;
     $self->SUPER::end(@_);
     $self->{table_seen}-- if $_[0] eq "table";
  }

  sub output
  {
      my $self = shift;
      unless ($self->{table_seen}) {
	  $self->SUPER::output(@_);
      }
  }

If you want to collect the parsed text internally you might want to do
something like this:

  package FilterIntoString;
  require HTML::Filter;
  @ISA=qw(HTML::Filter);
  sub output { push(@{$_[0]->{fhtml}}, $_[1]) }
  sub filtered_html { join("", @{$_[0]->{fhtml}}) }

=head1 BUGS

Comments in declarations are removed from the declarations and then
inserted as separate comments after the declaration.  If you turn on
strict_comment(), then comments with embedded "--" are split into
multiple comments.

=head1 SEE ALSO

L<HTML::Parser>

=head1 COPYRIGHT

Copyright 1997-1998 Gisle Aas.

This library is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.

=cut
