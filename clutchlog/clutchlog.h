#ifndef __CLUTCHLOG_H__
#define __CLUTCHLOG_H__

#include <iostream>
#include <sstream>
#include <cassert>
#include <cstdlib>
#include <iomanip>
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
#endif

#ifndef CLUTCHLOG_DEFAULT_DEPTH_MARK
#define CLUTCHLOG_DEFAULT_DEPTH_MARK ">"
#endif

//! Handy shortcuts to location.
#define CLUTCHLOC __FILE__, __FUNCTION__, __LINE__

//! Log a message at the given level.
#define CLUTCHLOG( LEVEL, WHAT ) { \
    auto& logger = clutchlog::logger(); \
    std::ostringstream msg ; msg << WHAT; \
    logger.log(clutchlog::level::LEVEL, msg.str(), CLUTCHLOC); \
}

//! Dump the given container.
#define CLUTCHDUMP( LEVEL, CONTAINER ) { \
    auto& logger = clutchlog::logger(); \
    logger.dump(clutchlog::level::LEVEL, std::begin(CONTAINER), std::end(CONTAINER), CLUTCHLOC, "\n"); \
}

#else
// Disabled macros can still be used in Release builds.
#define CLUTCHLOG  ( LEVEL, WHAT ) { do {/*nothing*/} while(false); }
#define CLUTCHDUMP ( LEVEL, WHAT ) { do {/*nothing*/} while(false); }
#endif

/**********************************************************************
 * Implementation
 **********************************************************************/

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
        enum level {quiet=0, error=1, warning=2, info=3, debug=4, xdebug=5};

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
                {level::info,"Info"},
                {level::debug,"Debug"},
                {level::xdebug,"XDebug"}
            }),
            _format(CLUTCHLOG_DEFAULT_FORMAT),
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
        std::string _format;
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
        };

        //! Gather information on the current location of the call.
        scope_t locate(
                const level& stage,
                const std::string& file,
                const std::string& func,
                const size_t line
            ) const
        {
            const size_t max_buffer = 4096;
            size_t stack_depth;
            void *buffer[max_buffer];
            stack_depth = backtrace(buffer, max_buffer);

            scope_t scope;
            scope.stage = stage;
            scope.depth = stack_depth;

            std::ostringstream sline; sline << line;
            scope.there =
                       std::regex_search(file, _in_file)
                   and std::regex_search(func, _in_func)
                   and std::regex_search(sline.str(), _in_line);

            scope.matches =    scope.stage <= _stage
                           and scope.depth <= _depth + _strip_calls
                           and scope.there;

            return scope;
        }

    /** }@ Internal details */

    public:

        /** Configuration accessors @{ */

        void format(const std::string& format) {_format = format;}
        std::string format() const {return _format;}

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

            std::regex re(mark);
            return std::regex_replace(form, re, tag);
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

            std::ostringstream sline; sline << line;
            format = replace(format, "\\{line\\}", sline.str());

            std::string letter(1, _level_words.at(stage).at(0));
            format = replace(format, "\\{level_letter\\}", letter);

            std::ostringstream sdepth; sdepth << depth;
            format = replace(format, "\\{depth\\}", sdepth.str());

            std::ostringstream chevrons;
            for(size_t i = _strip_calls; i < depth; ++i) {
                chevrons << ">"; }
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
                *_out << format(_format, what, basename(getenv("_")),
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
                const std::string sep="\n"
            ) const
                   // FIXME use a file name template as input
        {
            scope_t scope = locate(stage, file, func, line);

            if(scope.matches) {
                *_out << "#" // FIXME add a _format_dump parameter?
                      << format(_format, "", basename(getenv("_")),
                                stage, file, func,
                                line, scope.depth );

                std::copy(container_begin, container_end,
                    std::ostream_iterator<typename In::value_type>(*_out, sep.c_str()));
                // No flush
            } // if scopes.matches
        }

        /** }@ Low-level API */
};

#endif // __CLUTCHLOG_H__
