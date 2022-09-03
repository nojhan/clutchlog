#ifndef __CLUTCHLOG_H__
#define __CLUTCHLOG_H__
#pragma once

/** @file */
#include <ciso646>
    #ifdef FSEXPERIMENTAL
    #include <experimental/filesystem>
    namespace fs = std::experimental::filesystem;
#else
    #include <filesystem>
    namespace fs = std::filesystem;
#endif

#include <iostream>
#include <sstream>
#include <fstream>
#include <cassert>
#include <cstdlib>
#include <string>
#include <limits>
#include <regex>
#include <map>

//! POSIX headers necessary for stack depth management are available.
#if __has_include(<execinfo.h>) && __has_include(<stdlib.h>) && __has_include(<libgen.h>)
    #include <execinfo.h> // execinfo
    #include <stdlib.h>   // getenv
    #include <libgen.h>   // basename
    #define CLUTCHLOG_HAVE_UNIX_SYSINFO 1
#else
    #define CLUTCHLOG_HAVE_UNIX_SYSINFO 0
#endif

#if __has_include(<sys/ioctl.h>) && __has_include(<stdio.h>) && __has_include(<unistd.h>)
    #include <sys/ioctl.h>
    #include <stdio.h>
    #include <unistd.h>
    #define CLUTCHLOG_HAVE_UNIX_SYSIOCTL 1
#else
    #define CLUTCHLOG_HAVE_UNIX_SYSIOCTL 0
#endif


/**********************************************************************
 * Enable by default in Debug builds.
 **********************************************************************/
#ifndef WITH_CLUTCHLOG
    #ifndef NDEBUG
        //! Actually enable clutchlog features.
        #define WITH_CLUTCHLOG
    #endif
#endif

/**********************************************************************
 * Macros definitions
 **********************************************************************/
#ifdef WITH_CLUTCHLOG

/** @addtogroup DefaultConfigMacros Default configuration macros
 * @{ **/

#ifndef CLUTCHLOG_DEFAULT_DEPTH_BUILT_NODEBUG
    //! Default level over which calls to the logger are optimized out when NDEBUG is defined.
    #define CLUTCHLOG_DEFAULT_DEPTH_BUILT_NODEBUG clutchlog::level::progress
#endif // CLUTCHLOG_DEFAULT_DEPTH_BUILT

/** @} */


/** @addtogroup UseMacros High-level API macros
 * @{ */

//! Handy shortcuts to location.
#define CLUTCHLOC __FILE__, __FUNCTION__, __LINE__

//! Log a message at the given level.
#ifndef NDEBUG
    #define CLUTCHLOG( LEVEL, WHAT ) do {                                                \
        auto& clutchlog__logger = clutchlog::logger();                                   \
        std::ostringstream clutchlog__msg ; clutchlog__msg << WHAT;                      \
        clutchlog__logger.log(clutchlog::level::LEVEL, clutchlog__msg.str(), CLUTCHLOC); \
    } while(0)
#else // not Debug build.
    #define CLUTCHLOG( LEVEL, WHAT ) do {                                                    \
        if(clutchlog::level::LEVEL <= CLUTCHLOG_DEFAULT_DEPTH_BUILT_NODEBUG) {               \
            auto& clutchlog__logger = clutchlog::logger();                                   \
            std::ostringstream clutchlog__msg ; clutchlog__msg << WHAT;                      \
            clutchlog__logger.log(clutchlog::level::LEVEL, clutchlog__msg.str(), CLUTCHLOC); \
        }                                                                                    \
    } while(0)
#endif // NDEBUG

//! Dump the given container.
#ifndef NDEBUG
    #define CLUTCHDUMP( LEVEL, CONTAINER, FILENAME ) do {                                           \
        auto& clutchlog__logger = clutchlog::logger();                                              \
        clutchlog__logger.dump(clutchlog::level::LEVEL, std::begin(CONTAINER), std::end(CONTAINER), \
                    CLUTCHLOC, FILENAME, CLUTCHDUMP_DEFAULT_SEP);                                   \
    } while(0)
#else // not Debug build.
    #define CLUTCHDUMP( LEVEL, CONTAINER, FILENAME ) do {                                               \
        if(clutchlog::level::LEVEL <= CLUTCHLOG_DEFAULT_DEPTH_BUILT_NODEBUG) {                          \
            auto& clutchlog__logger = clutchlog::logger();                                              \
            clutchlog__logger.dump(clutchlog::level::LEVEL, std::begin(CONTAINER), std::end(CONTAINER), \
                        CLUTCHLOC, FILENAME, CLUTCHDUMP_DEFAULT_SEP);                                   \
        }                                                                                               \
    } while(0)
#endif // NDEBUG

//! Call any function if the scope matches.
#ifndef NDEBUG
    #define CLUTCHFUNC( LEVEL, FUNC, ... ) do {                                                             \
        auto& clutchlog__logger = clutchlog::logger();                                                      \
        clutchlog::scope_t clutchlog__scope = clutchlog__logger.locate(clutchlog::level::LEVEL, CLUTCHLOC); \
        if(clutchlog__scope.matches) {                                                                      \
            FUNC(__VA_ARGS__);                                                                              \
        }                                                                                                   \
    } while(0)
#else // not Debug build.
    #define CLUTCHFUNC( LEVEL, FUNC, ... ) do {                                                                 \
        if(clutchlog::level::LEVEL <= CLUTCHLOG_DEFAULT_DEPTH_BUILT_NODEBUG) {                                  \
            auto& clutchlog__logger = clutchlog::logger();                                                      \
            clutchlog::scope_t clutchlog__scope = clutchlog__logger.locate(clutchlog::level::LEVEL, CLUTCHLOC); \
            if(clutchlog__scope.matches) {                                                                      \
                FUNC(__VA_ARGS__);                                                                              \
            }                                                                                                   \
        }                                                                                                       \
    } while(0)
#endif // NDEBUG

//! Run any code if the scope matches.
#ifndef NDEBUG
    #define CLUTCHCODE( LEVEL, ... ) do {                                                                   \
        auto& clutchlog__logger = clutchlog::logger();                                                      \
        clutchlog::scope_t clutchlog__scope = clutchlog__logger.locate(clutchlog::level::LEVEL, CLUTCHLOC); \
        if(clutchlog__scope.matches) {                                                                      \
            __VA_ARGS__                                                                                     \
        }                                                                                                   \
    } while(0)
#else // not Debug build.
    #define CLUTCHCODE( LEVEL, CODE ) do {                                                                      \
        if(clutchlog::level::LEVEL <= CLUTCHLOG_DEFAULT_DEPTH_BUILT_NODEBUG) {                                  \
            auto& clutchlog__logger = clutchlog::logger();                                                      \
            clutchlog::scope_t clutchlog__scope = clutchlog__logger.locate(clutchlog::level::LEVEL, CLUTCHLOC); \
            if(clutchlog__scope.matches) {                                                                      \
                CODE                                                                                            \
            }                                                                                                   \
        }                                                                                                       \
    } while(0)
#endif // NDEBUG

/** @} */

#else // not WITH_CLUTCHLOG
    // Disabled macros can still be called in Release builds.
    #define CLUTCHLOG(  LEVEL, WHAT )                do {/*nothing*/} while(0)
    #define CLUTCHDUMP( LEVEL, CONTAINER, FILENAME ) do {/*nothing*/} while(0)
    #define CLUTCHFUNC( LEVEL, FUNC, ... )           do {/*nothing*/} while(0)
    #define CLUTCHCODE( LEVEL, CODE )                do {/*nothing*/} while(0)
#endif // WITH_CLUTCHLOG

/**********************************************************************
 * Implementation
 **********************************************************************/

#ifdef WITH_CLUTCHLOG
/** The single class which holds everything.
 *
 * This is a Singleton class.
 *
 * @addtogroup Main Main class
 * @{
 */
class clutchlog
{
    protected:

    /** @addtogroup UseMacros High-level API macros
     * @{ */
        #ifndef NDEBUG
            #ifndef CLUTCHLOG_DEFAULT_FORMAT
                //! Compile-time default format of the messages (debug mode: with absolute location).
                #if CLUTCHLOG_HAVE_UNIX_SYSINFO == 1 // Enables: name, depth and depth_marks
                    #if CLUTCHLOG_HAVE_UNIX_SYSIOCTL == 1 // Enables: hfill
                        #define CLUTCHLOG_DEFAULT_FORMAT "[{name}] {level_letter}:{depth_marks} {msg} {hfill} {func} @ {file}:{line}\n"
                    #else
                        #define CLUTCHLOG_DEFAULT_FORMAT "[{name}] {level_letter}:{depth_marks} {msg}\t\t\t\t\t{func} @ {file}:{line}\n"
                    #endif
                #else
                    #if CLUTCHLOG_HAVE_UNIX_SYSIOCTL == 1
                        #define CLUTCHLOG_DEFAULT_FORMAT "{level_letter} {msg} {hfill} {func} @ {file}:{line}\n"
                    #else
                        #define CLUTCHLOG_DEFAULT_FORMAT "{level_letter} {msg}\t\t\t\t\t{func} @ {file}:{line}\n"
                    #endif
                #endif
            #endif
        #else
            #ifndef CLUTCHLOG_DEFAULT_FORMAT
                //! Compile-time default format of the messages (non-debug mode: without absolute location).
                #if CLUTCHLOG_HAVE_UNIX_SYSINFO == 1
                    #define CLUTCHLOG_DEFAULT_FORMAT "{level_letter}:{depth_marks} {msg} {hfill} {func}\n"
                #else
                    #define CLUTCHLOG_DEFAULT_FORMAT "{level_letter} {msg}\t\t\t\t\t{func}\n"
                #endif
            #endif
        #endif
        //! Default format of the messages.
        static inline std::string default_format = CLUTCHLOG_DEFAULT_FORMAT;

        #ifndef NDEBUG
            #ifndef CLUTCHDUMP_DEFAULT_FORMAT
                //! Compile-time default format of the comment line in file dump.
                #if CLUTCHLOG_HAVE_UNIX_SYSINFO == 1
                    #define CLUTCHDUMP_DEFAULT_FORMAT "# [{name}] {level} in {func} (at depth {depth}) @ {file}:{line}"
                #else
                    #define CLUTCHDUMP_DEFAULT_FORMAT "# {level} in {func} @ {file}:{line}"
                #endif
            #endif // CLUTCHDUMP_DEFAULT_FORMAT
        #else
            #ifndef CLUTCHDUMP_DEFAULT_FORMAT
                //! Compile-time default format of the comment line in file dump.
                #if CLUTCHLOG_HAVE_UNIX_SYSINFO == 1
                    #define CLUTCHDUMP_DEFAULT_FORMAT "# [{name}] {level} in {func} (at depth {depth})"
                #else
                    #define CLUTCHDUMP_DEFAULT_FORMAT "# {level} in {func}"
                #endif
            #endif // CLUTCHDUMP_DEFAULT_FORMAT
        #endif
        //! Default format of the comment line in file dump.
        static inline std::string dump_default_format = CLUTCHDUMP_DEFAULT_FORMAT;

        #ifndef CLUTCHDUMP_DEFAULT_SEP
            //! Compile-time default item separator for dump.
            #define CLUTCHDUMP_DEFAULT_SEP "\n"
        #endif // CLUTCHDUMP_DEFAULT_SEP
        //! Default item separator for dump.
        static inline std::string dump_default_sep = CLUTCHDUMP_DEFAULT_SEP;

        #ifndef CLUTCHLOG_DEFAULT_DEPTH_MARK
            //! Compile-time default mark for stack depth.
            #define CLUTCHLOG_DEFAULT_DEPTH_MARK ">"
        #endif // CLUTCHLOG_DEFAULT_DEPTH_MARK
        //! Default mark for stack depth.
        static inline std::string default_depth_mark = CLUTCHLOG_DEFAULT_DEPTH_MARK;

        #ifndef CLUTCHLOG_STRIP_CALLS
            //! Compile-time number of call stack levels to remove from depth display by default.
            #define CLUTCHLOG_STRIP_CALLS 5
        #endif // CLUTCHLOG_STRIP_CALLS
        //! Number of call stack levels to remove from depth display by default.
        static inline unsigned int default_strip_calls = CLUTCHLOG_STRIP_CALLS;

        #ifndef CLUTCHLOG_HFILL_MARK
            //! Character used as a filling for right-align the right part of messages with "{hfill}".
            #define CLUTCHLOG_HFILL_MARK '.'
        #endif // CLUTCHLOG_HFILL_MARK
        //! Default character used as a filling for right-align the right part of messages with "{hfill}".
        static inline char default_hfill_char = CLUTCHLOG_HFILL_MARK;
    /* @} */


    public:
        /** @name High-level API
         * @{ */

        /** Get the logger instance.
         *
         * @code
         * auto& log = clutchlog::logger();
         * @endcode
         */
        static clutchlog& logger()
        {
            static clutchlog instance;
            return instance;
        }

        //! Available log levels.
        enum level {critical=0, error=1, warning=2, progress=3, note=4, info=5, debug=6, xdebug=7};

        /** @} */

        /** @addtogroup Formating Formating tools
         * @{ */

        /** @name Formating API
         * @{ */

        /** Color and style formatter for ANSI terminal escape sequences.
         *
         * @note All styles may not be supported by a given terminal/operating system.
         */
        class fmt {
            public:
                //! Foreground color codes.
                enum class fg {
                    black   = 30,
                    red     = 31,
                    green   = 32,
                    yellow  = 33,
                    blue    = 34,
                    magenta = 35,
                    cyan    = 36,
                    white   = 37,
                    none
                } /** Foreground color */ fore;

                //! Background color codes.
                enum class bg {
                    black   = 40,
                    red     = 41,
                    green   = 42,
                    yellow  = 43,
                    blue    = 44,
                    magenta = 45,
                    cyan    = 46,
                    white   = 47,
                    none
                } /** Background color */ back;

                //! Typographic style codes.
                enum class typo {
                    reset     =  0,
                    bold      =  1,
                    underline =  4,
                    inverse   =  7,
                    none
                } /** Typographic style*/  style;

                //! Empty constructor, only useful for a no-op formatter.
                fmt() : fore(fg::none), back(bg::none), style(typo::none) {}

                /** @name All combination of constructors with different parameters orders.
                 * @{ */
                fmt(  fg f,   bg b = bg::none, typo s = typo::none) : fore(f), back(b), style(s) {}
                fmt(  fg f, typo s           ,   bg b =   bg::none) : fore(f), back(b), style(s) {}
                fmt(  bg b,   fg f = fg::none, typo s = typo::none) : fore(f), back(b), style(s) {}
                fmt(  bg b, typo s           ,   fg f =   fg::none) : fore(f), back(b), style(s) {}
                fmt(typo s,   fg f = fg::none,   bg b =   bg::none) : fore(f), back(b), style(s) {}
                fmt(typo s,   bg b           ,   fg f =   fg::none) : fore(f), back(b), style(s) {}
                /** @} */

            protected:
                //! Print the currently encoded format escape code on the given output stream.
                std::ostream& print_on( std::ostream& os) const
                {
                    std::vector<int> codes; codes.reserve(3);
                    if(this->fore  !=   fg::none) { codes.push_back(static_cast<int>(this->fore ));}
                    if(this->back  !=   bg::none) { codes.push_back(static_cast<int>(this->back ));}
                    if(this->style != typo::none) { codes.push_back(static_cast<int>(this->style));}
                    if(codes.size() == 0) {return os;}

                    os << "\033[";
                    assert(codes.size() > 0);
                    os << codes[0];
                    for(size_t i=1; i < codes.size(); ++i) {
                        os << ";" << codes[i];
                    }
                    os << "m";
                    return os;
                }

            public:
                /** Output stream overload.
                 *
                 * Allow to use a formatter as a tag within a stream:
                 * @code
                 * clutchlog::fmt end(clutchlog::fmt::typo::reset);
                 * clutchlog::fmt error(clutchlog::fmt::fg::red, clutchlog::fmt::typo::bold);
                 * std::cout << error << "ERROR" << end << std::endl;
                 * @endcode
                 *
                 * @note An formatter called this way will NOT output a reset escape code.
                 */
                friend std::ostream& operator<<(std::ostream& os, const fmt& fmt)
                {
                    return fmt.print_on(os);
                }

                /** Format the given string with the currently encoded format.
                 *
                 * Allow to use a formatter as a function:
                 * @code
                 * clutchlog::fmt error(clutchlog::fmt::fg::red, clutchlog::fmt::typo::bold);
                 * std::cout << error("ERROR") << std::endl;
                 * @endcode
                 *
                 * @note A formatter called this way WILL output a reset escape code at the end.
                 */
                std::string operator()( const std::string& msg ) const
                {
                    std::ostringstream os;
                    this->print_on(os);
                    fmt reset(fmt::typo::reset);
                    os << msg;
                    reset.print_on(os);
                    return os.str();
                }
        }; // fmt class

        /** @} */
        /** @} */

    /** @name Internal details
     * @{ */

    public:
        clutchlog(clutchlog const&)      = delete;
        void operator=(clutchlog const&) = delete;

    private:
        clutchlog() :
            // system, main, log
            _strip_calls(clutchlog::default_strip_calls),
            _level_word({
                {level::critical,"Critical"},
                {level::error   ,"Error"},
                {level::warning ,"Warning"},
                {level::progress,"Progress"},
                {level::note    ,"Note"},
                {level::info    ,"Info"},
                {level::debug   ,"Debug"},
                {level::xdebug  ,"XDebug"}
            }),
            _level_fmt({
                {level::critical,fmt(fmt::fg::red,     fmt::typo::underline)},
                {level::error   ,fmt(fmt::fg::red,     fmt::typo::bold)},
                {level::warning ,fmt(fmt::fg::magenta, fmt::typo::bold)},
                {level::progress,fmt()},
                {level::note    ,fmt()},
                {level::info    ,fmt()},
                {level::debug   ,fmt()},
                {level::xdebug  ,fmt()}
            }),
            _format_log(clutchlog::default_format),
            _format_dump(clutchlog::dump_default_format),
            _hfill_char(clutchlog::default_hfill_char),
            _out(&std::clog),
#if CLUTCHLOG_HAVE_UNIX_SYSINFO == 1
            _depth(std::numeric_limits<size_t>::max() - _strip_calls),
            _depth_mark(clutchlog::default_depth_mark),
#endif
            _stage(level::error),
            _in_file(".*"),
            _in_func(".*"),
            _in_line(".*")
        {
            // Reverse the level->word map into a word->level map.
            for(auto& lw : _level_word) {
                _word_level[lw.second] = lw.first;
            }
#if CLUTCHLOG_HAVE_UNIX_SYSIOCTL
            struct winsize w;
            ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
            _nb_columns = w.ws_col;
#endif
        }

    protected:
        /** Current number of call stack levels to remove from depth display. */
        const size_t _strip_calls;
        /** Dictionary of level identifier to their string representation. */
        const std::map<level,std::string> _level_word;
        /** Dictionary of level string to their identifier. */
        std::map<std::string,level> _word_level;
        /** Dictionary of level identifier to their format. */
        std::map<level,fmt> _level_fmt;
        /** Current format of the standard output. */
        std::string _format_log;
        /** Current format of the file output. */
        std::string _format_dump;
        /** Character for filling. */
        char _hfill_char;
        /** Standard output. */
        std::ostream* _out;
#if CLUTCHLOG_HAVE_UNIX_SYSINFO == 1
        /** Current stack depth (above which logs are not printed). */
        size_t _depth;
        /** Current depth mark. */
        std::string _depth_mark;
#endif
        /** Current log level. */
        level _stage;
        /** Current file location filter. */
        std::regex _in_file;
        /** Current function location filter. */
        std::regex _in_func;
        /** Current line location filter. */
        std::regex _in_line;

#if CLUTCHLOG_HAVE_UNIX_SYSINFO == 1
        /** Maximum buffer size for backtrace message. */
        static const size_t _max_buffer = 4096;
#endif

#if CLUTCHLOG_HAVE_UNIX_SYSIOCTL
        /** Current terminal size (for right-alignment). */
        size_t _nb_columns;
#endif
    /** @}*/

    public:

        /** @name Configuration accessors
         * @{ */

        //! Set the template string.
        void format(const std::string& format) {_format_log = format;}
        //! Get the template string.
        std::string format() const {return _format_log;}

        //! Set the template string for dumps.
        void format_comment(const std::string& format) {_format_dump = format;}
        //! Get the template string for dumps.
        std::string format_comment() const {return _format_dump;}

        //! Set the output stream on which to print.
        void out(std::ostream& out) {_out = &out;}
        //! Get the output stream on which to print.
        std::ostream& out() {return *_out;}

#if CLUTCHLOG_HAVE_UNIX_SYSINFO == 1
        //! Set the stack depth above which logs are not printed.
        void depth(size_t d) {_depth = d;}
        //! Get the stack depth above which logs are not printed.
        size_t depth() const {return _depth;}

        //! Set the string mark with which stack depth is indicated.
        void depth_mark(const std::string mark) {_depth_mark = mark;}
        //! Get the string mark with which stack depth is indicated.
        std::string depth_mark() const {return _depth_mark;}
#endif
#if CLUTCHLOG_HAVE_UNIX_SYSIOCTL == 1
        //! Set the character for the stretching hfill marker.
        void hfill_mark(const char mark) {_hfill_char = mark;}
        //! Get the character for the stretching hfill marker.
        char hfill_mark() const {return _hfill_char;}
#endif

        //! Set the log level (below which logs are not printed) with an identifier.
        void  threshold(level l) {_stage = l;}
        //! Set the log level (below which logs are not printed) with a string.
        void  threshold(const std::string& l) {_stage = this->level_of(l);}
        //! Get the log level below which logs are not printed.
        level threshold() const {return _stage;}
        //! Get the map of available log levels string representations toward their identifier. */
        const std::map<std::string,level>& levels() const { return _word_level;}

        /** Return the log level tag corresponding to the given pre-configured name.
         *
         * @note This is case sensitive, see the pre-configured `_level_word`.
         */
        level level_of(const std::string name)
        {
            const auto ilevel = _word_level.find(name);
            if( ilevel != std::end(_word_level)) {
                return ilevel->second;
            } else {
                throw std::out_of_range("'" + name + "' is not a valid log level name");
            }
        }

        //! Set the regular expression filtering the file location.
        void file(std::string file) {_in_file = file;}
        //! Set the regular expression filtering the function location.
        void func(std::string func) {_in_func = func;}
        //! Set the regular expression filtering the line location.
        void line(std::string line) {_in_line = line;}

        //! Set the regular expressions filtering the location.
        void location(
                const std::string& in_file,
                const std::string& in_function=".*",
                const std::string& in_line=".*"
            )
        {
            file(in_file);
            func(in_function);
            line(in_line);
        }

        /** Set the style (color and typo) of the given log level.
         *
         * This version accept style arguments as if they were passed to `clutchlog::fmt`.
         */
        template<class ... FMT>
        void style(level stage, FMT... styles) { this->style(stage,fmt(styles...)); }
        //! Set the style (color and typo) of the given log level, passing a `fmt` instance.
        void style(level stage, fmt style) { _level_fmt.at(stage) = style; }
        //! Get the configured fmt instance of the given log level.
        fmt  style(level stage) const { return _level_fmt.at(stage); }

        /** @} */

    public:

        /** @name Low-level API
         * @{ */

        //! Structure holding a location matching.
        struct scope_t {
            /** Everything is compatible. */
            bool matches;
            /** Current log level. */
            level stage;
#if CLUTCHLOG_HAVE_UNIX_SYSINFO == 1
            /** Current depth. */
            size_t depth;
#endif
            /** Location is compatible. */
            bool there;
            /** Constructor. */
            scope_t() :
                matches(false),
                stage(level::xdebug),
#if CLUTCHLOG_HAVE_UNIX_SYSINFO == 1
                depth(0),
#endif
                there(false)
            {}
        }; // scope_t


        //! Gather information on the current location of the call.
        scope_t locate(
                const level& stage,
                const std::string& file,
                const std::string& func,
                const size_t line
            ) const
        {
            scope_t scope; // False scope by default.

            /***** Log level stage *****/
            // Test stage first, because it's fastest.
            scope.stage = stage;
            if(not (scope.stage <= _stage)) {
                // Bypass useless computations if no match
                // because of the stage.
                return scope;
            }
#if CLUTCHLOG_HAVE_UNIX_SYSINFO == 1
            /***** Stack depth *****/
            // Backtrace in second, quite fast.
            size_t stack_depth;
            void *buffer[_max_buffer];
            stack_depth = backtrace(buffer, _max_buffer);
            scope.depth = stack_depth;
            if(not (scope.depth <= _depth + _strip_calls)) {
                // Bypass if no match.
                return scope;
            }
#endif

            /***** Location *****/
            // Location last, slowest.
            std::ostringstream sline; sline << line;
            scope.there =
                       std::regex_search(file, _in_file)
                   and std::regex_search(func, _in_func)
                   and std::regex_search(sline.str(), _in_line);

            // No need to retest stage and depth, which are true here.
            scope.matches = scope.there;

            return scope;
        } // locate

        /** Replace `mark` by `tag` in `form`.
         *
         * @code
         * log.replace("{greet} {world}", "\\{greet\\}", "hello");
         * // returns "hello {world}"
         * @endcode
         */
        std::string replace(
                const std::string& form,
                const std::string& mark,
                const std::string& tag
            ) const
        {
            // Useless debug code, unless something fancy would be done with name tags.
            // std::regex re;
            // try {
            //     re = std::regex(mark);
            //
            // } catch(const std::regex_error& e) {
            //     std::cerr << "ERROR with a regular expression \"" << mark << "\": ";
            //     switch(e.code()) {
            //         case std::regex_constants::error_collate:
            //             std::cerr << "the expression contains an invalid collating element name";
            //             break;
            //         case std::regex_constants::error_ctype:
            //             std::cerr << "the expression contains an invalid character class name";
            //             break;
            //         case std::regex_constants::error_escape:
            //             std::cerr << "the expression contains an invalid escaped character or a trailing escape";
            //             break;
            //         case std::regex_constants::error_backref:
            //             std::cerr << "the expression contains an invalid back reference";
            //             break;
            //         case std::regex_constants::error_brack:
            //             std::cerr << "the expression contains mismatched square brackets ('[' and ']')";
            //             break;
            //         case std::regex_constants::error_paren:
            //             std::cerr << "the expression contains mismatched parentheses ('(' and ')')";
            //             break;
            //         case std::regex_constants::error_brace:
            //             std::cerr << "the expression contains mismatched curly braces ('{' and '}')";
            //             break;
            //         case std::regex_constants::error_badbrace:
            //             std::cerr << "the expression contains an invalid range in a {} expression";
            //             break;
            //         case std::regex_constants::error_range:
            //             std::cerr << "the expression contains an invalid character range (e.g. [b-a])";
            //             break;
            //         case std::regex_constants::error_space:
            //             std::cerr << "there was not enough memory to convert the expression into a finite state machine";
            //             break;
            //         case std::regex_constants::error_badrepeat:
            //             std::cerr << "one of *?+{ was not preceded by a valid regular expression";
            //             break;
            //         case std::regex_constants::error_complexity:
            //             std::cerr << "the complexity of an attempted match exceeded a predefined level";
            //             break;
            //         case std::regex_constants::error_stack:
            //             std::cerr << "there was not enough memory to perform a match";
            //             break;
            //         default:
            //             std::cerr << "unknown error";
            //     }
            //     std::cerr << std::endl;
            //     throw;
            // } // catch

            const std::regex re(mark);
            return std::regex_replace(form, re, tag);
        }

        //! Replace `mark` by `tag` in `form`, converting tag to its string representation first.
        std::string replace(
                const std::string& form,
                const std::string& mark,
                const size_t tag
            ) const
        {
            std::ostringstream stag; stag << tag;
            return replace(form, mark, stag.str());
        }

        //! Substitute all tags in the format string with the corresponding information and apply the style corresponding to the log level.
        std::string format(
                std::string format,
                const std::string& what,
#if CLUTCHLOG_HAVE_UNIX_SYSINFO == 1
                const std::string& name,
#endif
                const level& stage,
                const std::string& file,
                const std::string& func,
                const size_t line
#if CLUTCHLOG_HAVE_UNIX_SYSINFO == 1
                ,
                const size_t depth
#endif
            ) const
        {
            format = replace(format, "\\{msg\\}", what);
            format = replace(format, "\\{file\\}", file);
            format = replace(format, "\\{func\\}", func);
            format = replace(format, "\\{line\\}", line);

            format = replace(format, "\\{level\\}", _level_word.at(stage));
            std::string letter(1, _level_word.at(stage).at(0)); // char -> string
            format = replace(format, "\\{level_letter\\}", letter);

#if CLUTCHLOG_HAVE_UNIX_SYSINFO == 1
            format = replace(format, "\\{name\\}", name);
            format = replace(format, "\\{depth\\}", depth - _strip_calls);

            std::ostringstream chevrons;
            for(size_t i = _strip_calls; i < depth; ++i) {
                chevrons << _depth_mark;
            }
            format = replace(format, "\\{depth_marks\\}", chevrons.str());
#endif
#if CLUTCHLOG_HAVE_UNIX_SYSIOCTL
            const std::string hfill_tag = "{hfill}";
            const size_t hfill_pos = format.find(hfill_tag);
            if(hfill_pos != std::string::npos) {
                if(_nb_columns > 0) {
                    const size_t left_len = hfill_pos;
                    const size_t right_len = format.size() - hfill_pos - hfill_tag.size();
                    if(right_len+left_len > _nb_columns) {
                        // The right part would go over the terminal width: add a new line.
                        const std::string hfill(std::max((size_t)0, _nb_columns-right_len), _hfill_char);
                        format = replace(format, "\\{hfill\\}", "\n"+hfill);
                    } else {
                        // There is some space in between left and right parts.
                        const std::string hfill(std::max((size_t)0, _nb_columns - (right_len+left_len)), _hfill_char);
                        format = replace(format, "\\{hfill\\}", hfill);
                    }
                } else {
                    // We don't know the terminal width.
                    format = replace(format, "\\{hfill\\}", _hfill_char);
                }
            }
#endif
            return _level_fmt.at(stage)(format);
        }

        //! Print a log message IF the location matches the given one.
        void log(
                const level& stage,
                const std::string& what,
                const std::string& file, const std::string& func, size_t line
            ) const
        {
            scope_t scope = locate(stage, file, func, line);

            if(scope.matches) {
#if CLUTCHLOG_HAVE_UNIX_SYSINFO == 1
                *_out << format(_format_log, what, basename(getenv("_")),
                                stage, file, func,
                                line, scope.depth );
#else
                *_out << format(_format_log, what,
                                stage, file, func,
                                line );

#endif
                _out->flush();
            } // if scopes.matches
        }

        //! Dump a serializable container after a comment line with log information.
        template<class In>
        void dump(
                const level& stage,
                const In container_begin, const In container_end,
                const std::string& file, const std::string& func, size_t line,
                const std::string& filename_template = "dump_{n}.dat",
                const std::string sep = dump_default_sep
            ) const
        {
            scope_t scope = locate(stage, file, func, line);

            if(scope.matches) {
                const std::string tag = "\\{n\\}";
                const std::regex re(tag);
                std::string outfile = "";

                // If the file name template has the {n} tag.
                if(std::regex_search(filename_template, re)) {
                    // Increment n until a free one is found.
                    size_t n = 0;
                    do {
                        outfile = replace(filename_template, tag, n);
                        n++;
                    } while( fs::exists( outfile ) );

                } else {
                    // Use the parameter as is.
                    outfile = filename_template;
                }

                std::ofstream fd(outfile);

                if(_format_dump.size() > 0) {
#if CLUTCHLOG_HAVE_UNIX_SYSINFO == 1
                    fd << format(_format_dump, "", basename(getenv("_")),
                            stage, file, func,
                            line, scope.depth );
#else
                    fd << format(_format_dump, "",
                            stage, file, func,
                            line );
#endif
                    fd << sep; // sep after comment line.
                }

                std::copy(container_begin, container_end,
                    std::ostream_iterator<typename In::value_type>(fd, sep.c_str()));

                fd.close();
            } // if scopes.matches
        }

        /** @} */
};

/** @} */

#else // not WITH_CLUTCHLOG


/**********************************************************************
 * Fake implementation
 **********************************************************************/

// Equivalent class with empty methods, will be optimized out
// while allowing to actually have calls implemented without WITH_CLUTCHLOG guards.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
class clutchlog
{
    public:
        static clutchlog& logger() {}
        enum level {critical=0, error=1, warning=2, progress=3, note=4, info=5, debug=6, xdebug=7};
        class fmt {
            public:
                enum class fg { black, red, green, yellow, blue, magenta, cyan, white, none } fore;
                enum class bg { black, red, green, yellow, blue, magenta, cyan, white, none } back;
                enum class typo { reset, bold, underline, inverse, none } style;
                fmt() : fore(fg::none), back(bg::none), style(typo::none) {}
                fmt(  fg f,   bg b = bg::none, typo s = typo::none) : fore(f), back(b), style(s) {}
                fmt(  fg f, typo s           ,   bg b =   bg::none) : fore(f), back(b), style(s) {}
                fmt(  bg b,   fg f = fg::none, typo s = typo::none) : fore(f), back(b), style(s) {}
                fmt(  bg b, typo s           ,   fg f =   fg::none) : fore(f), back(b), style(s) {}
                fmt(typo s,   fg f = fg::none,   bg b =   bg::none) : fore(f), back(b), style(s) {}
                fmt(typo s,   bg b           ,   fg f =   fg::none) : fore(f), back(b), style(s) {}
            protected:
                std::ostream& print_on(std::ostream&) const {}
            public:
                friend std::ostream& operator<<(std::ostream&, const fmt&) {}
                std::string operator()(const std::string&) const {}
        };
    public:
        clutchlog(clutchlog const&)      = delete;
        void operator=(clutchlog const&) = delete;
    private:
        clutchlog() {}
    protected:
        struct scope_t {};
        scope_t locate(
                const level&,
                const std::string&,
                const std::string&,
                const size_t
            ) const
        {}
    public:
        void format(const std::string&) {}
        std::string format() const {}

        void format_comment(const std::string&) {}
        std::string format_comment() const {}

        void out(std::ostream&) {}
        std::ostream& out() {}

#if CLUTCHLOG_HAVE_UNIX_SYSINFO == 1
        void depth(size_t) {}
        size_t depth() const {}

        void depth_mark(const std::string) {}
        std::string depth_mark() const {}
#endif
#if CLUTCHLOG_HAVE_UNIX_SYSIOCTL == 1
        void hfill_mark(const char) {}
        char hfill_mark() const {}
#endif

        void  threshold(level) {}
        void  threshold(const std::string&) {}
        level threshold() const {}
        const std::map<std::string,level> levels() const {};
        level level_of(const std::string) {}

        void file(std::string) {}
        void func(std::string) {}
        void line(std::string) {}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
        void location(
                const std::string&,
                const std::string& in_function=".*",
                const std::string& in_line=".*"
            )
        {}
#pragma GCC diagnostic pop
        template<class ... FMT>
        void style(level stage, FMT... styles) {}
        void style(level, fmt) {}
        fmt style(level) const {}
    public:
        std::string replace(
                const std::string&,
                const std::string&,
                const std::string&
            ) const
        {}

        std::string replace(
                const std::string&,
                const std::string&,
                const size_t
            ) const
        {}

        std::string format(
                std::string,
                const std::string&,
#if CLUTCHLOG_HAVE_UNIX_SYSINFO == 1
                const std::string&,
#endif
                const level&,
                const std::string&,
                const std::string&,
                const size_t
#if CLUTCHLOG_HAVE_UNIX_SYSINFO == 1
                ,
                const size_t
#endif
            ) const
        {}

        void log(
                const level&,
                const std::string&,
                const std::string&, const std::string&, size_t
            ) const
        {}

        template<class In>
        void dump(
                const level&,
                const In, const In,
                const std::string&, const std::string&, size_t,
                const std::string&,
                const std::string
            ) const
        {}
};
#pragma GCC diagnostic pop
#endif // WITH_CLUTCHLOG

#endif // __CLUTCHLOG_H__
