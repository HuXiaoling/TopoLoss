Frequently asked questions
##########################

(under construction)

ImportError: dynamic module does not define init function
=========================================================

1. Make sure that the name specified in ``pybind::module`` and
   ``PYBIND11_PLUGIN`` is consistent and identical to the filename of the
   extension library. The latter should not contain any extra prefixes (e.g.
   ``test.so`` instead of ``libtest.so``).

2. If the above did not fix your issue, then you are likely using an
   incompatible version of Python (for instance, the extension library was
   compiled against Python 2, while the interpreter is running on top of some
   version of Python 3)

Limitations involving reference arguments
=========================================

In C++, it's fairly common to pass arguments using mutable references or
mutable pointers, which allows both read and write access to the value
supplied by the caller. This is sometimes done for efficiency reasons, or to
realize functions that have multiple return values. Here are two very basic
examples:

.. code-block:: cpp

    void increment(int &i) { i++; }
    void increment_ptr(int *i) { (*i)++; }

In Python, all arguments are passed by reference, so there is no general
issue in binding such code from Python.

However, certain basic Python types (like ``str``, ``int``, ``bool``,
``float``, etc.) are **immutable**. This means that the following attempt
to port the function to Python doesn't have the same effect on the value
provided by the caller -- in fact, it does nothing at all.

.. code-block:: python

    def increment(i):
        i += 1 # nope..

pybind11 is also affected by such language-level conventions, which means that
binding ``increment`` or ``increment_ptr`` will also create Python functions
that don't modify their arguments.

Although inconvenient, one workaround is to encapsulate the immutable types in
a custom type that does allow modifications.

An other alternative involves binding a small wrapper lambda function that
returns a tuple with all output arguments (see the remainder of the
documentation for examples on binding lambda functions). An example:

.. code-block:: cpp

    int foo(int &i) { i++; return 123; }

and the binding code

.. code-block:: cpp

   m.def("foo", [](int i) { int rv = foo(i); return std::make_tuple(rv, i); });

CMake doesn't detect the right Python version, or it finds mismatched interpreter and library versions
======================================================================================================

The Python detection logic of CMake is flawed and can sometimes fail to find
the desired Python version, or it chooses mismatched interpreter and library
versions. A longer discussion is available on the pybind11 issue tracker
[#f1]_, though this is ultimately not a pybind11 issue.

To force the build system to choose a particular version, delete CMakeCache.txt
and then invoke CMake as follows:

.. code-block:: bash

    cmake -DPYTHON_EXECUTABLE:FILEPATH=<...> \
          -DPYTHON_LIBRARY:FILEPATH=<...>  \
          -DPYTHON_INCLUDE_DIR:PATH=<...> .

.. [#f1] http://github.com/pybind/pybind11/issues/99

Working with ancient Visual Studio 2009 builds on Windows
=========================================================

The official Windows distributions of Python are compiled using truly
ancient versions of Visual Studio that lack good C++11 support. Some users
implicitly assume that it would be impossible to load a plugin built with
Visual Studio 2015 into a Python distribution that was compiled using Visual
Studio 2009. However, no such issue exists: it's perfectly legitimate to
interface DLLs that are built with different compilers and/or C libraries.
Common gotchas to watch out for involve not ``free()``-ing memory region
that that were ``malloc()``-ed in another shared library, using data
structures with incompatible ABIs, and so on. pybind11 is very careful not
to make these types of mistakes.

How can I reduce the build time?
================================

It's good practice to split binding code over multiple files, as is done in
the included file :file:`example/example.cpp`.

.. code-block:: cpp

    void init_ex1(py::module &);
    void init_ex2(py::module &);
    /* ... */

    PYBIND11_PLUGIN(example) {
        py::module m("example", "pybind example plugin");

        init_ex1(m);
        init_ex2(m);

        /* ... */

        return m.ptr();
    }

The various ``init_ex`` functions are contained in separate files that can be
compiled independently from another. Following this approach will

1. enable parallel builds (if desired).

2. allow for faster incremental builds (e.g. if a single class definiton is
   changed, only a subset of the binding code may need to be recompiled).

3. reduce memory requirements.

How can I create smaller binaries?
==================================

To do its job, pybind11 extensively relies on a programming technique known as
*template metaprogramming*, which is a way of performing computation at compile
time using type information. Template metaprogamming usually instantiates code
involving significant numbers of deeply nested types that are either completely
removed or reduced to just a few instrutions during the compiler's optimization
phase. However, due to the nested nature of these types, the resulting symbol
names in the compiled extension library can be extremely long. For instance,
the included test suite contains the following symbol:

.. code-block:: cpp

    __ZN8pybind1112cpp_functionC1Iv8Example2JRNSt3__16vectorINS3_12basic_stringIwNS3_11char_traitsIwEENS3_9allocatorIwEEEENS8_ISA_EEEEEJNS_4nameENS_7siblingENS_9is_methodEA28_cEEEMT0_FT_DpT1_EDpRKT2_

which is the mangled form of the following function type:

.. code-block:: cpp

    pybind11::cpp_function::cpp_function<void, Example2, std::__1::vector<std::__1::basic_string<wchar_t, std::__1::char_traits<wchar_t>, std::__1::allocator<wchar_t> >, std::__1::allocator<std::__1::basic_string<wchar_t, std::__1::char_traits<wchar_t>, std::__1::allocator<wchar_t> > > >&, pybind11::name, pybind11::sibling, pybind11::is_method, char [28]>(void (Example2::*)(std::__1::vector<std::__1::basic_string<wchar_t, std::__1::char_traits<wchar_t>, std::__1::allocator<wchar_t> >, std::__1::allocator<std::__1::basic_string<wchar_t, std::__1::char_traits<wchar_t>, std::__1::allocator<wchar_t> > > >&), pybind11::name const&, pybind11::sibling const&, pybind11::is_method const&, char const (&) [28])

The memory needed to store just the name of this function (196 bytes) is larger
than the actual piece of code (111 bytes) it represents! On the other hand,
it's silly to even give this function a name -- after all, it's just a tiny
cog in a bigger piece of machinery that is not exposed to the outside world.
So we'll generally only want to export symbols for those functions which are
actually called from the outside.

This can be achieved by specifying the parameter ``-fvisibility=hidden`` to GCC
and Clang, which sets the default symbol visibility to *hidden*. It's best to
do this only for release builds, since the symbol names can be helpful in
debugging sessions. On Visual Studio, symbols are already hidden by default, so
nothing needs to be done there. Needless to say, this has a tremendous impact
on the final binary size of the resulting extension library.

Another aspect that can require a fair bit of code are function signature
descriptions. pybind11 automatically generates human-readable function
signatures for docstrings, e.g.:

.. code-block:: none

     |  __init__(...)
     |      __init__(*args, **kwargs)
     |      Overloaded function.
     |
     |      1. __init__(example.Example1) -> NoneType
     |
     |      Docstring for overload #1 goes here
     |
     |      2. __init__(example.Example1, int) -> NoneType
     |
     |      Docstring for overload #2 goes here
     |
     |      3. __init__(example.Example1, example.Example1) -> NoneType
     |
     |      Docstring for overload #3 goes here


In C++11 mode, these are generated at run time using string concatenation,
which can amount to 10-20% of the size of the resulting binary. If you can,
enable C++14 language features (using ``-std=c++14`` for GCC/Clang), in which
case signatures are efficiently pre-generated at compile time. Unfortunately,
Visual Studio's C++14 support (``constexpr``) is not good enough as of April
2016, so it always uses the more expensive run-time approach.
