_PortableRuntime_ is a shared library that contains functionality that tends to be
core across all projects.  Since very little code tends to be fully general, this
library is small and attempts to be very portable.

The focus of the library is the following:
* Architecture abstractions for endianness and compiler macros.
* Tracing.
* Unicode string handling.
* Error handling.

All attempts are made at making shared libraries completely independent to keep
the layer map flat, and as such, _PortableRuntime_ may be the exception as a library
that may be a dependency for other libraries.

Toby Jones \([www.turbohex.com](http://www.turbohex.com), [ace.roqs.net](http://ace.roqs.net)\)
