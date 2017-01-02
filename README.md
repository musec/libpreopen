# libpreopen

`libpreopen` supports compartmentalized applications by storing pre-opened
directory descriptors and using them in capability-safe `libc` wrappers.

Many `libc` functions require *ambient authority*:
the ability to access global namespaces.
For example, you can't `open(2)` a file without access to the global filesystem
namespace.
Operating systems have been introducing compartmentalization primitives such as
[Capsicum](https://www.freebsd.org/cgi/man.cgi?query=capsicum) and
[seccomp-bpf](https://www.kernel.org/doc/Documentation/prctl/seccomp_filter.txt)
that restrict access to global namespaces, allowing *sandboxing* of processes
to mitigate security risks.
A sandboxed application can no longer use ambient authority, so it cannot call
common C functions such as `access(2)`, `open(2)`, `stat(2)`, etc.
Adapting applications to play nicely within a sandbox can require a significant
amount of manual adaptation from `open(2)` to `openat(2)` or even, in the worst
case, to complex IPC primitives.
