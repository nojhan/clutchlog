***Clutchlog is a logging system which targets versatile debugging.***
***It allows to (de)clutch messages for a given: log level, source code location or call stack depth.***

[TOC]

Features
========

Clutchlog allows to select which log messages will be displayed, based on their locations:

- **Classical log levels**: each message has a given detail level and it is displayed if you ask for a at least the same
  one.
- **Call stack depth**: you can ask to display messages within functions which are called up to a given stack depth.
- **Source code location**: you can ask to display messages called from given files, functions and line number, all based on
  regular expressions.

Additionally, Clutchlog will do its best to allow the compiler to optimize out calls,
for instance debug messages in "Release" builds.

Additional features:

- **Templated log format**, to easily design your own format.
- **Colored log**. By default only important ones are colored (critical and error in red, warning in magenta).
- **Macro to dump the content of a container in a file** with automatic naming (yes, it is useful for fast debugging).
- **Generic clutching wrapper**, to wrap any function call. Useful to (de)clutch *asserts* for example.


Example
=======

Adding a message is a simple as calling a macro (which is declutched in Debug build type, when `NDEBUG` is not defined):
```cpp
CLUTCHLOG(info, "matrix size: " << m << "x" << n);
```

To configure the display, you indicate the three types of locations, for example in your `main` function:
```cpp
    auto& log = clutchlog::logger();
    log.depth(2);                          // Log functions called from "main" but not below.
    log.threshold(clutchlog::level::info); // Log only "info", "warning", "error" or "critical" messages.
    log.file("algebra/.*");                // Will match any file in the "algebra" directory.
    log.func("(mul|add|sub|div)");         // Will match "multiply", for instance.
```

For more detailled examples, see the "API documentation" section below and the `tests` directory.


Rationale
=========

Most of existing logging systems targets service events storage, like fast queuing of transactions in a round-robin
database.
Their aim is to provide a simple interface to efficiently store messages somewhere, which is appropriated when you have
a well known service running and you want to be able to trace complex users interactions across its states.

Clutchlog, however, targets the debugging of a (typically single-run) program.
While you develop your software, it's common practice to output several detailled informations on the internal states
around the feature you are currently programming.
However, once the feature is up and running, those detailled informations are only useful if you encounter a bug
traversing this specific part.

While tracing a bug, it is tedious to uncomment old debugging code (and go on the build-test cycle)
or to set up a full debugger session which displays all appropriate data (with ad-hoc fancy hooks).

To solve this problem, Clutchlog allows to disengage your debug log messages in various parts of the program,
allowing for the fast tracking of a bug across the execution.


API documentation
=================

Calls
-----

The main entrypoint is the `CLUTCHLOG` macro, which takes the desired log level and message.
The message can be anything which can be output in an `ostringstream`.
```cpp
// Simple string:
CLUTCHLOG(info, "hello world");

// Serialisable variable:
double value = 0;
CLUTCHLOG(error, value);

// passed using inline output stream operators:
CLUTCHLOG(debug, "hello " << value << " world");
```

There is also a macro to dump the content of an iterable within a separate file: `CLUTCHDUMP`.
This function takes care of incrementing a numeric suffix in the file name,
if an existing file with this name exists.
```cpp
std::vector<int> v(10);
std::generate(v.begin(), v.end(), std::rand);
CLUTCHDUMP(debug, vec, "test_{n}.dat");
/* Will output in cat "rand_0.dat"
* # [t-dump] Info in main (at depth 5) @ /home/nojhan/code/clutchlog/tests/t-dump.cpp:22
* 1804289383
* 846930886
* 1681692777
*/
```
Note that if you pass a file name without the `{n}` tag, the file will be overwritten as is.


Location filtering
------------------

To configure the global behaviour of the logger, you must first get a reference on its (singleton) instance:
```cpp
auto& log = clutchlog::logger();
```

One can configure the location(s) at which messages should actually be logged:
```cpp
log.depth(3); // Depth of the call stack, defaults to the maximum possible value.
log.threshold(clutchlog::level::error); // Log level, defaults to error.
```
Current levels are defined in an enumeration as `clutchlog::level`:
```cpp
enum level {critical=0, error=1, warning=2, progress=3, note=4, info=5, debug=6, xdebug=7};
```

File, function and line filters are indicated using (ECMAScript) regular expressions:
```cpp
log.file(".*"); // File location, defaults to any.
log.func(".*"); // Function location, defaults to any.
log.line(".*"); // Line location, defaults to any.
```
A shortcut function can be used to filter all at once:
```cpp
log.location(file, func, line); // Defaults to any, second and last parameters being optional.
```

Strings may be used to set up the threshold, using `level_of`:
```cpp
log.threshold( log.level_of("XDebug") ); // You have to know the exact string.
```
Note that the case of the log levels strings matters (see below).


Output Configuration
--------------------

The output stream can be configured using the `out` method:
```cpp
log.out(std::clog); // Defaults to clog.
```

The format of the messages can be defined with the `format` method, passing a string with standardized tags surrounded by `{}`:
```cpp
log.format("{msg}");
```
Available tags are:

- `{msg}`: the logged message,
- `{name}`: the name of the current binary,
- `{level}`: the current log level (i.e. `Critical`, `Error`, `Warning`, `Progress`, `Note`, `Info`, `Debug` or `XDebug`),
- `{level_letter}`: the first letter of the current log level,
- `{file}`: the current file (absolute path),
- `{func}`: the current function,
- `{line}`: the current line number,
- `{depth}`: the current depth of the call stack,
- `{depth_marks}`: as many chevrons `>` as there is calls in the stack.

The default log format is `"[{name}] {level_letter}:{depth_marks} {msg}\t\t\t\t\t{func} @ {file}:{line}\n"`,
it can be overriden at compile time by defining the `CLUTCHLOG_DEFAULT_FORMAT` macro.

The default format of the first line of comment added with the dump macro is 
`"# [{name}] {level} in {func} (at depth {depth}) @ {file}:{line}"`.
It can be edited with the `format_comment` method.
If it is set to an empty string, then no comment line is added.
The default can be modified at compile time with `CLUTCHDUMP_DEFAULT_FORMAT`.
By default, the separator between items in the container is a new line.
To change this behaviour, you can change `CLUTCHDUMP_DEFAULT_SEP` or
call the low-level `dump` method.

The mark used with the `{depth_marks}` tag can be configured with the `depth_mark` method,
and its default with the `CLUTCHLOG_DEFAULT_DEPTH_MARK` macro:
```cpp
log.depth_mark(CLUTCHLOG_DEFAULT_DEPTH_MARK); // Defaults to ">".
```


Output style
------------

The output can be colored differently depending on the log level.
```cpp
// Print error messages in bold red:
log.style(clutchlog::level::error, // First, the log level.
    clutchlog::fmt::fg::red,       // Then the styles, in any order...
    clutchlog::fmt::typo::bold);
```

Or, if you want to declare some semantics beforehand:
```cpp
// Print warning messages in bold magenta:
using fmt = clutchlog::fmt;
fmt warn(fmt::fg::magenta, fmt::typo::bold);
log.style(clutchlog::level::warning, warn);
```

Using the `clutchlog::fmt` class, you can style:

- the foreground color, passing a `clutchlog::fmt::fg`,
- the background color, passing a `clutchlog::fmt::bg`,
- some typographic style, passing a `clutchlog::fmt::typo`.

Any of the three arguments may be passed, in any order,
if an argument is omitted, it defaults to no color/style.

Available colors are:

- black,
- red,
- green,
- yellow,
- blue,
- magenta,
- cyan,
- white,
- none.

Available typographies:

- reset (remove any style),
- bold,
- underline,
- inverse,
- none.

You may use styling within the format message template itself, to add even more colors:
```cpp
using fmt = clutchlog::fmt;
std::ostringstream format;
fmt discreet(fmt::fg::blue);
format << "{level}: "
    << discreet("{file}:") // Used as a function (inserts a reset at the end).
    << fmt(fmt::fg::yellow) << "{line}" // Used as a tag (no reset inserted).
    << fmt(fmt::typo::reset) << " {msg}" << std::endl; // This is a reset.
log.format(format.str());
```
Note: messages at the "critical", "error" and "warning" log levels are colored by default.
You may want to set their style to `none` if you want to stay in control of inserted colors in the format template.


Disabled calls
--------------

By default, clutchlog is always enabled if the `NDEBUG` preprocessor variable is not defined
(this variable is set by CMake in build types that differs from `Debug`).

You can however force clutchlog to be enabled in any build type
by setting the `WITH_CLUTCHLOG` preprocessor variable.

When the `NDEBUG` preprocessor variable is set (e.g. in `Release` build),
clutchlog will do its best to allow the compiler to optimize out any calls
for log levels which are under or equal to `progress`.

You can change this behavior at compile time by setting the
`CLUTCHLOG_DEFAULT_DEPTH_BUILT_NODEBUG` preprocessor variable
to the desired maximum log level, for example:
```cpp
// Will always allow to log everything even in Release mode.
#define CLUTCHLOG_DEFAULT_DEPTH_BUILT_NODEBUG clutchlog::level::xdebug
```

Note that allowing a log level does not mean that it will actually output something.
If the configured log level at runtime is lower than the log level of the message,
it will still not be printed.

This behavior intend to remove as many conditional statements as possible
when not debugging, without having to use preprocessor guards around
calls to clutchlog, thus saving run time at no readability cost.


Low-level API
-------------

All configuration setters have a getters counterpart, with the same name but taking no parameter,
for example:
```cpp
std::string mark = log.depth_mark();
```

To control more precisely the logging, one can use the low-level `log` method:
```cpp
log.log(clutchlog::level::xdebug, "hello world", "main.cpp", "main", 122);
```
A helper macro can helps to fill in the location with the actual one, as seen by the compiler:
```cpp
log.log(clutchlog::level::xdebug, "hello world", CLUTCHLOC);
```
A similar `dump` method exists:
```cpp
log.dump(clutchlog::level::xdebug, cont.begin(), cont.end(), CLUTCHLOC, "dumped_{n}.dat", "\n");
log.dump(clutchlog::level::xdebug, cont.begin(), cont.end(), "main.cpp", "main", 122, "dumped.dat", "\n\n");
```


(De)clutch any function call
----------------------------

The `CLUTHFUNC` macro allows to wrap any function within the current logger.

For instance, this can be useful if you want to (de)clutch calls to `assert`s.
To do that, just declare your own macro:
```cpp
#define ASSERT(LEVEL, ...) { CLUTCHFUNC(LEVEL, assert, __VA_ARGS__) }
```
Thus, any call like `ASSERT(error, x > 3);` will be declutchable
with the same configuration than a call to `CLUTCHLOG`.


(De)clutch any code section
---------------------------

The `CLUTCHCODE` macro allows to wrap any code within the current logger.

For instance:
```cpp
CLUTCHCODE(info,
    std::clog << "We are clutched!\n";
);
```

Log level semantics
===================

Log levels use a classical semantics for a human skilled in the art, in decreasing order of importance:

- *Critical*: an error which cannot be recovered. For instance, something which will make a server stop right here.
- *Error*: an error which invalidates a function, but may still be recovered. For example, a bad user input which will make a server reset its state, but not crash.
- *Warning*: something that is strange, but is probably legit. For example a default parameter is set because the user forgot to indicate its preference.
- *Progress*: the state at which computation currently is.
- *Note*: some state worth noting to understand what's going on.
- *Info*: any information which would help ensuring that everything is going well.
- *Debug*: data which would help debugging the program if there was a bug later on.
- *XDebug*: debugging information which would be heavy to read.

Note: the log levels constants are lower case (for example: `clutchlog::level::xdebug`), but their string representation is not (e.g. "XDebug", this should be taken into account when using `level_of`).


Limitations
===========

Because the call stack depth and program name access are system-dependent,
the features relying on the depth of the call stack and the display of the program name
are only available for operating systems having the following headers:
`execinfo.h`, `stdlib.h` and `libgen.h` (so far, tested with Linux).

Some colors/styles may not be supported by some exotic terminal emulators.

Clutchlog needs `C++-17` with the `filesystem` feature.
You may need to indicate `-std=c++17 -lstdc++fs` to some compilers.

What Clutchlog do not provide at the moment (but may in a near future):

- Super fast log writing.
- Thread safety.

What Clutchlog will most certainly never provide:

- Round-robin log managers.
- Duplicated messages management.
- External output systems (only allow output stream, you can still do the proxy yourself).
- External error handlers (not my job, come on).
- Automatic argument parser (please, use a dedicated lib).
- Signal handling (WTF would you do that, anyway?).


Build and tests
===============

To use clutchlog, just include its header in your code
and either ensure that the `NDEBUG` preprocessor variable is not set,
either define the `WITH_CLUTCHLOG` preprocessor variable.

If you're using CMake (or another modern build system),
it will unset `NDEBUG` —and thus enable clutchlog—
only for the "Debug" build type,
which is usually what you want if you use clutchlog, anyway.

To build and run the tests, just use a classical CMake workflow:
```sh
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DWITH_CLUTCHLOG=ON ..
make
ctest
```

There's a script which tests all the build types combinations: `./build_all.sh`.

