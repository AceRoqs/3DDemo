#ifndef PRECPP11_H
#define PRECPP11_H

// Define C++11 constructs not available in pre-C++11 compilers.

#ifdef _MSC_VER
#if _MSC_VER != 1600
#error Revisit the OVERRIDE, EQUALS_DELETE, and NOEXCEPT macros and use the C++11 version if the compiler supports them.
#endif
#endif

// "override" keyword.
// http://en.wikipedia.org/wiki/C++11#Explicit_overrides_and_final
#define OVERRIDE

// = delete.
// http://en.wikipedia.org/wiki/C++11#Explicitly_defaulted_and_deleted_special_member_functions
#define EQUALS_DELETE

#if defined(_MSC_VER) && !defined(NDEBUG)
// MSVC + Debug build: use non-standard empty throw exception specification.
#define NOEXCEPT throw()
#else
#define NOEXCEPT
#endif

#endif

