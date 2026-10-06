package My::Suite::Tcmalloc_profiler;

@ISA = qw(My::Suite);

# RPM-based distros use /usr/lib64, Debian/Ubuntu use the multiarch dir
my ($tcmalloc_lib) = grep { -r $_ } (
  "/usr/lib64/libtcmalloc_and_profiler.so",
  glob("/usr/lib/*-linux-gnu/libtcmalloc_and_profiler.so"),
  "/usr/lib/libtcmalloc_and_profiler.so",
);

# Upstream gperftools installs pprof, Debian/Ubuntu rename it google-pprof
my ($pprof_bin) = grep { -x $_ } ("/usr/bin/pprof", "/usr/bin/google-pprof");

return "No TCMALLOC_PROFILER plugin" unless $ENV{TCMALLOC_PROFILER_SO};
return "Not run for embedded server" if $::opt_embedded_server;
return "libtcmalloc_and_profiler.so not found" unless $tcmalloc_lib;

return "pprof not found" unless $pprof_bin;

$ENV{LD_PRELOAD} = $tcmalloc_lib;
$ENV{TCMALLOC_PPROF_BINARY} = $pprof_bin;

sub is_default { 1 }

bless { };
