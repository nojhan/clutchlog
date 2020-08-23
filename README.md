Clutchlog is a logging system whith targets versatile debugging.
It allows to (de)clutch messages either for a given: log level, source code location or call stack depth.

Features
========

Clutchlog allows to select which log messages will be displayed, based on their locations:

- classical log levels: each message has a given detail level and it is displayed if you ask for a at least the same one
  (current ones: quiet, error, warning, info, debug, xdebug).
- call stack depth: you can ask to display messages within functions which are called up to a given stack depth.
- source code location: you can ask to display messages called from given files, functions and line number, all based on
  regular expressions.

Appart from those, Clutchlog have classical logging system features: output selection, default to unbuffered mode, etc.

Of course, Clutchlog is disabled by default if `NDEBUG` is not defined.


Example
=======

Adding a message is a simple as calling a macro (which is declutched in Debug build type):
```cpp
CLUTCHLOG(info, "matrix size: " << m << "x" << n);
```

To configure the display, you indicate the three types of locations, for example in your `main` function:
```cpp
    auto& log = clutchlog::logger();
    log.depth(2);                          // Log functions called from "main" but not below.
    log.threshold(clutchlog::level::info); // Log "info", "warning", "error" or "quiet" messages.
    log.file("algebra/.*");                // Will match any file in the "algebra" directory.
    log.func("(mul|add|sub|div)");         // Will match "multiply", for instance.
```

For more detailled examples, see the `tests` directory.


Rationale
=========

Most of existing logging systems targets service events storage, like fast queuing of transactions in a round-robin
database.
Their aim is to provide a simple interface to efficiently store messages somewhere, which is appropriated when you have
a well known service running and you want to be able to trace complex users interactions across its states.

Clutchlog, however, targets the debugging of a (single-run) program.
While you develop your software, it's common practice to output several detailled informations on the internal states
around the feature you are currently programming.
However, once the feature is up and running, those detailled informations are only useful if you encounter a bug
traversing this specific part.

While tracing a bug, it is tedious to uncomment old debugging code (and go on the build-test cycle)
or to set up a full debugger session which displays all appropriate data (with ad-hoc fancy hooks).

To solve this problem, Clutchlog allows to disengage your debug log messages in various parts of the program,
allowing for the fast tracking of a bug across the execution.


Limitations
===========

Call stack depth is only implemented for Linux.


Build and tests
===============

To build and run the tests, just use a classical CMake workflow:
```sh
mkdir build
cd build
cmake ..
make
ctest
```

