#ifndef __CLUTCHLOG_H__
#define __CLUTCHLOG_H__
#pragma once

#include <filesystem>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cassert>
#include <cstdlib>
// #include <iomanip>
#include <string>
#include <limits>
#include <regex>
#include <map>

// #ifdef __unix__
#include <execinfo.h>
#include <stdlib.h>
#include <libgen.h>
// #endif

/**********************************************************************
 * Enable by default in Debug builds.
 **********************************************************************/
#ifndef WITH_CLUTCHLOG
#ifndef NDEBUG
#define WITH_CLUTCHLOG
#endif
#endif

/**********************************************************************
 * Macros definitions
 **********************************************************************/
#ifdef WITH_CLUTCHLOG

#ifndef CLUTCHLOG_DEFAULT_FORMAT
//! Default format of the messages.
#define CLUTCHLOG_DEFAULT_FORMAT "[{name}] {level_letter}:{depth_marks} {msg}\t\t\t\t\t{func} @ {file}:{line}\n"
#endif // CLUTCHLOG_DEFAULT_FORMAT

#ifndef CLUTCHDUMP_DEFAULT_FORMAT
//! Default format of the comment line in file dump.
#define CLUTCHDUMP_DEFAULT_FORMAT "# [{name}] {level} in {func} (at depth {depth}) @ {file}:{line}"
#endif // CLUTCHDUMP_DEFAULT_FORMAT

#ifndef CLUTCHDUMP_DEFAULT_SEP
//! Default item separator for dump.
#define CLUTCHDUMP_DEFAULT_SEP "\n"
#endif // CLUTCHDUMP_DEFAULT_SEP

#ifndef CLUTCHLOG_DEFAULT_DEPTH_MARK
#define CLUTCHLOG_DEFAULT_DEPTH_MARK ">"
#endif // CLUTCHLOG_DEFAULT_DEPTH_MARK

#ifndef CLUTCHLOG_DEFAULT_DEPTH_BUILT_NODEBUG
#define CLUTCHLOG_DEFAULT_DEPTH_BUILT_NODEBUG clutchlog::level::progress
#endif // CLUTCHLOG_DEFAULT_DEPTH_BUILT

//! Handy shortcuts to location.
#define CLUTCHLOC __FILE__, __FUNCTION__, __LINE__

//! Log a message at the given level.
#ifndef NDEBUG
#define CLUTCHLOG( LEVEL, WHAT ) { \
    auto& logger = clutchlog::logger(); \
    std::ostringstream msg ; msg << WHAT; \
    logger.log(clutchlog::level::LEVEL, msg.str(), CLUTCHLOC); \
}
#else // not Debug build.
#define CLUTCHLOG( LEVEL, WHAT ) { \
    if(clutchlog::level::LEVEL <= CLUTCHLOG_DEFAULT_DEPTH_BUILT_NODEBUG) { \
        auto& logger = clutchlog::logger(); \
        std::ostringstream msg ; msg << WHAT; \
        logger.log(clutchlog::level::LEVEL, msg.str(), CLUTCHLOC); \
    } \
}
#endif // NDEBUG

//! Dump the given container.
#ifndef NDEBUG
#define CLUTCHDUMP( LEVEL, CONTAINER, FILENAME ) { \
    auto& logger = clutchlog::logger(); \
    logger.dump(clutchlog::level::LEVEL, std::begin(CONTAINER), std::end(CONTAINER), \
                CLUTCHLOC, FILENAME, CLUTCHDUMP_DEFAULT_SEP); \
}
#else // not Debug build.
#define CLUTCHDUMP( LEVEL, CONTAINER, FILENAME ) { \
    if(clutchlog::level::LEVEL <= CLUTCHLOG_DEFAULT_DEPTH_BUILT_NODEBUG) { \
        auto& logger = clutchlog::logger(); \
        logger.dump(clutchlog::level::LEVEL, std::begin(CONTAINER), std::end(CONTAINER), \
                    CLUTCHLOC, FILENAME, CLUTCHDUMP_DEFAULT_SEP); \
    } \
}
#endif // NDEBUG

#else // not WITH_CLUTCHLOG
// Disabled macros can still be used in Release builds.
#define CLUTCHLOG( LEVEL, WHAT ) { do {/*nothing*/} while(false); }
#define CLUTCHDUMP( LEVEL, CONTAINER, FILENAME ) { do {/*nothing*/} while(false); }
#endif // WITH_CLUTCHLOG

/**********************************************************************
 * Implementation
 **********************************************************************/

#ifdef WITH_CLUTCHLOG
//! Singleton class.
class clutchlog
{
    public:
        /** High-level API @{ */

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
        enum level {quiet=0, error=1, warning=2, progress=3, info=4, debug=5, xdebug=6};

        /** }@ High-level API */

    /** Internal details @{ */

    public:
        clutchlog(clutchlog const&)      = delete;
        void operator=(clutchlog const&) = delete;

    private:
        clutchlog() :
            // system, main, log
            _strip_calls(3),
            _level_words({
                {level::quiet,"Quiet"},
                {level::error,"Error"},
                {level::warning,"Warning"},
                {level::progress,"Progress"},
                {level::info,"Info"},
                {level::debug,"Debug"},
                {level::xdebug,"XDebug"}
            }),
            _format_log(CLUTCHLOG_DEFAULT_FORMAT),
            _format_dump(CLUTCHDUMP_DEFAULT_FORMAT),
            _out(&std::clog),
            _depth(std::numeric_limits<size_t>::max() - _strip_calls),
            _stage(level::error),
            _in_file(".*"),
            _in_func(".*"),
            _in_line(".*"),
            _depth_mark(CLUTCHLOG_DEFAULT_DEPTH_MARK)
        {}

    protected:
        const size_t _strip_calls;
        const std::map<level,std::string> _level_words;
        std::string _format_log;
        std::string _format_dump;
        std::ostream* _out;
        size_t _depth;
        level _stage;
        std::regex _in_file;
        std::regex _in_func;
        std::regex _in_line;
        std::string _depth_mark;

        struct scope_t {
            bool matches; // everything is compatible
            level stage; // current log level
            size_t depth; // current depth
            bool there; // location is compatible
            scope_t() :
                matches(false),
                stage(level::xdebug),
                depth(0),
                there(false)
            {}
        };

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

            /***** Stack depth *****/
            // Backtrace in second, quite fast.
            const size_t max_buffer = 4096;
            size_t stack_depth;
            void *buffer[max_buffer];
            stack_depth = backtrace(buffer, max_buffer);
            scope.depth = stack_depth;
            if(not (scope.depth <= _depth + _strip_calls)) {
                // Bypass if no match.
                return scope;
            }

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
        }

    /** }@ Internal details */

    public:

        /** Configuration accessors @{ */

        void format(const std::string& format) {_format_log = format;}
        std::string format() const {return _format_log;}

        void format_comment(const std::string& format) {_format_dump = format;}
        std::string format_comment() const {return _format_dump;}

        void out(std::ostream& out) {_out = &out;}
        std::ostream& out() {return *_out;}

        void depth(size_t d) {_depth = d;}
        size_t depth() const {return _depth;}

        void depth_mark(std::string mark) {_depth_mark = mark;}
        std::string depth_mark() const {return _depth_mark;}

        void  threshold(level l) {_stage = l;}
        level threshold() const {return _stage;}

        void file(std::string file) {_in_file = file;}
        void func(std::string func) {_in_func = func;}
        void line(std::string line) {_in_line = line;}

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

        /** }@ Configuration */

    public:

        /** Low-level API @{ */

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

        std::string replace(
                const std::string& form,
                const std::string& mark,
                const size_t tag
            ) const
        {
            std::ostringstream stag; stag << tag;
            return replace(form, mark, stag.str());
        }

        std::string format(
                std::string format,
                const std::string& what,
                const std::string& name,
                const level& stage,
                const std::string& file,
                const std::string& func,
                const size_t line,
                const size_t depth
            ) const
        {
            format = replace(format, "\\{msg\\}", what);
            format = replace(format, "\\{name\\}", name);
            format = replace(format, "\\{file\\}", file);
            format = replace(format, "\\{func\\}", func);
            format = replace(format, "\\{level\\}", _level_words.at(stage));
            format = replace(format, "\\{line\\}", line);
            format = replace(format, "\\{depth\\}", depth);

            std::string letter(1, _level_words.at(stage).at(0)); // char -> string
            format = replace(format, "\\{level_letter\\}", letter);

            std::ostringstream chevrons;
            for(size_t i = _strip_calls; i < depth; ++i) {
                chevrons << ">";
            }
            format = replace(format, "\\{depth_marks\\}", chevrons.str());

            return format;
        }

        void log(
                const level& stage,
                const std::string& what,
                const std::string& file, const std::string& func, size_t line
            ) const
        {
            scope_t scope = locate(stage, file, func, line);

            if(scope.matches) {
                *_out << format(_format_log, what, basename(getenv("_")),
                                stage, file, func,
                                line, scope.depth );
                _out->flush();
            } // if scopes.matches
        }

        template<class In>
        void dump(
                const level& stage,
                const In container_begin, const In container_end,
                const std::string& file, const std::string& func, size_t line,
                const std::string& filename_template="dump_{n}.dat",
                const std::string sep=CLUTCHDUMP_DEFAULT_SEP
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
                    } while( std::filesystem::exists( outfile ) );

                } else {
                    // Use the parameter as is.
                    outfile = filename_template;
                }

                std::ofstream fd(outfile);

                if(_format_dump.size() > 0) {
                    fd << format(_format_dump, "", basename(getenv("_")),
                            stage, file, func,
                            line, scope.depth );
                    fd << sep; // sep after comment line.
                }

                std::copy(container_begin, container_end,
                    std::ostream_iterator<typename In::value_type>(fd, sep.c_str()));

                fd.close();
            } // if scopes.matches
        }

        /** }@ Low-level API */
};

#else // not WITH_CLUTCHLOG

// Equivalent class with empty methods, will be optimized out
// while allowing to actually have calls implemented without WITH_CLUTCHLOG guards.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
class clutchlog
{
    public:
        static clutchlog& logger() { }
        enum level {quiet=0, error=1, warning=2, progress=3, info=4, debug=5, xdebug=6};
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
        { }
    public:
        void format(const std::string&) {}
        std::string format() const {}

        void format_comment(const std::string&) {}
        std::string format_comment() const {}

        void out(std::ostream&) {}
        std::ostream& out() {}

        void depth(size_t) {}
        size_t depth() const {}

        void depth_mark(std::string) {}
        std::string depth_mark() const {}

        void  threshold(level) {}
        level threshold() const {}

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
        { }
#pragma GCC diagnostic pop
    public:
        std::string replace(
                const std::string&,
                const std::string&,
                const std::string&
            ) const
        { }

        std::string replace(
                const std::string&,
                const std::string&,
                const size_t
            ) const
        { }

        std::string format(
                std::string,
                const std::string&,
                const std::string&,
                const level&,
                const std::string&,
                const std::string&,
                const size_t,
                const size_t
            ) const
        { }

        void log(
                const level&,
                const std::string&,
                const std::string&, const std::string&, size_t
            ) const
        { }

        template<class In>
        void dump(
                const level&,
                const In, const In,
                const std::string&, const std::string&, size_t,
                const std::string&,
                const std::string
            ) const
        { }
};
#pragma GCC diagnostic pop
#endif // WITH_CLUTCHLOG

#endif // __CLUTCHLOG_H__
