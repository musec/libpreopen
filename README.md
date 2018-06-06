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


## Build

`libpreopen` can be built with the usual CMake workflow:

```sh
$ mkdir -p build/Debug
$ cd build/Debug
$ cmake -D CMAKE_BUILD_TYPE=Debug -G Ninja ../..
$ ninja
```

(or omit `-G Ninja` and use `make` instead of
[Ninja](https://ninja-build.org)... but why would you want to do that?)

It can also be built by running the `ci/build.sh` script with an argument of
either `Debug` or `Release`.


## Testing

You can run the `libpreopen` tests with the `check` target, i.e., run
`make check` or `ninja check` in the build directory.
This will require [lit](https://pypi.python.org/pypi/lit) to be installed in
your `PATH`, as well as LLVM `FileCheck`.
YMMV, but I find it best to use `lit` from PyPi (link above) early in my `PATH`
because it gets the lit `site-packages` directory more correct than some
distribution packages.
Depending on your environment, `FileCheck` may have to come from a full-blown
LLVM package (e.g., something like `devel/llvm60` on FreeBSD); I usually put
this at the end of my `PATH` to avoid picking up the wrong `lit`.
You can also run individual tests with `lit path/to/test`, but that will require
you to tell `lit` where the `libpreopen` source and build directories are
(either via `--param` options or environment variables: you will receive helpful
error messages if you don't).
