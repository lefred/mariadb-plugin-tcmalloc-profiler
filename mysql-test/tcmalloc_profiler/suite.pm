package My::Suite::Tcmalloc_profiler;

@ISA = qw(My::Suite);

my $tcmalloc_lib = "/usr/lib64/libtcmalloc_and_profiler.so";

return "No TCMALLOC_PROFILER plugin" unless $ENV{TCMALLOC_PROFILER_SO};
return "Not run for embedded server" if $::opt_embedded_server;
return "$tcmalloc_lib not found" unless -r $tcmalloc_lib;

$ENV{LD_PRELOAD} = $tcmalloc_lib;

sub is_default { 1 }

bless { };
