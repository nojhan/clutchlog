**Clutchlog is a logging system whith targets versatile debugging.**
**It allows to (de)clutch messages either for a given: log level, source code location or call stack depth.**

Features
========

Clutchlog allows to select which log messages will be displayed, based on their locations:

- *classical log levels*: each message has a given detail level and it is displayed if you ask for a at least the same
  one.
- *call stack depth*: you can ask to display messages within functions which are called up to a given stack depth.
- *source code location*: you can ask to display messages called from given files, functions and line number, all based on
  regular expressions.

Of course, Clutchlog is disabled by default if `NDEBUG` is not defined.


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
    log.threshold(clutchlog::level::info); // Log only "info", "warning", "error" or "quiet" messages.
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
CLUTCHLOG(debug, vec, "test_{n}.dat");
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
enum level {quiet=0, error=1, warning=2, progress=3, info=4, debug=5, xdebug=6};
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
- `{level}`: the current log level (i.e. `Quiet`, `Error`, `Warning`, `Progress`, `Info`, `Debug` or `XDebug`),
- `{level_letter}`: the first letter of the current log level,
- `{file}`: the current file (absolute path),
- `{func}`: the current function,
- `{line}`: the current line number,
- `{depth}`: the current depth of the call stack,
- `{depth_marks}`: as many chevrons `>` as there is calls in the stack.

The default log format is `"[{name}] {level_letter}:{depth_marks} {msg}\t\t\t\t\t{func} @ {file}:{line}\n"`,
it can be overriden at compile time by defining the `CLUTCHLOG_DEFAULT_FORMAT` macro.

The default format of the comment added with the dump macro is 
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


Limitations
===========

Because the call stack depth and binary name access are system-dependent,
Clutchlog is only implemented for Linux at the moment.


Build and tests
===============

To use clutchlog, just include its header in your code.

To build and run the tests, just use a classical CMake workflow:
```sh
mkdir build
cd build
# There's no point building in Release mode, at clutchlog is declutched.
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
ctest
```

