# Cannot use real hostname program because test would fail
define(`hostname', esyscmd(`echo www.gnu.org'))dnl
`hostname = >>'hostname`<<'
define(`hostname', 
pushdef(`_tmp', `$1')_tmp(translit(esyscmd(`echo www.gnu.org'), `.', `,'))`'popdef(`_tmp'))dnl
`hostname = >>'hostname`<<'
