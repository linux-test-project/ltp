# NOTE: Derived from ./blib/lib/URI/URL/_generic.pm.  Changes made here will be lost.
package URI::URL::_generic;

sub eparams  { shift->_elem('params',  @_); }

1;
