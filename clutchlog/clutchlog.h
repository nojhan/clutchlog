#ifndef __CLUTCHLOG_H__
#define __CLUTCHLOG_H__

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <string>
#include <limits>
#include <sstream>
#include <regex>

// #ifdef __unix__
#include <execinfo.h>
#include <stdlib.h>
#include <libgen.h>
// #endif

// #ifdef WITH_THROWN_ASSERTS
// #undef assert
// #define assert( what ) { if(!(what)){CLUTCHLOG_RAISE(aion::Assert, "Failed assert: `" << #what << "`");} }
// #else
#include <cassert>
// #endif

#ifndef WITH_CLUTCHLOG
#ifndef NDEBUG
#define WITH_CLUTCHLOG
#endif
#endif

class clutchlog
{
    public:
        /** High-level API @{ */

        static clutchlog& logger()
        {
            static clutchlog instance;
            return instance;
        }

        enum level {quiet, error, warning, info, debug, xdebug};

        /** }@ High-level API */

    /** Internal details @{ */

    public:
        clutchlog(clutchlog const&)      = delete;
        void operator=(clutchlog const&) = delete;

    private:
        clutchlog() :
            _out(&std::clog),
            _depth(std::numeric_limits<size_t>::max()),
            _stage(level::error),
            _in_file(".*"),
            _in_func(".*"),
            _in_line(".*"),
            _show_name(true),
            _show_depth(true),
            _show_location(true)
        {}

    protected:
        // system, main, log
        const size_t _strip_calls = 3;
        std::ostream* _out;
        size_t _depth;
        level _stage;
        std::regex _in_file;
        std::regex _in_func;
        std::regex _in_line;
        bool _show_name;
        bool _show_depth;
        bool _show_location;

        struct scope_t {
            bool matches;
            level stage;
            size_t depth;
            bool there;
        };

        scope_t locate(level stage, std::string file, std::string func, size_t line)
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

        void out(std::ostream& out) {_out = &out;}
        std::ostream& out() {return *_out;}

        void depth(size_t d) {_depth = d;}
        size_t depth() const {return _depth;}

        void  threshold(level l) {_stage = l;}
        level threshold() const {return _stage;}

        void file(std::string file) {_in_file = file;}
        void func(std::string func) {_in_func = func;}
        void line(std::string line) {_in_line = line;}

        void location(std::string in_file, std::string in_function=".*", std::string in_line=".*")
        {
            file(in_file);
            func(in_function);
            line(in_line);
        }

        void show_name(bool n) {_show_name = n;}
        bool show_name() const {return _show_name;}

        void show_depth(bool d) {_show_depth = d;}
        bool show_depth() const {return _show_depth;}

        void show_location(bool l) {_show_location = l;}
        bool show_location() const {return _show_location;}

        /** }@ Configuration */

    public:

        /** Low-level API @{ */

        void log(std::string what, level stage, std::string file, std::string func, size_t line, bool newline)
        {
            scope_t scope = locate(stage, file, func, line);

            if(scope.matches) {
                if(_show_name) {
                    *_out << "[" << basename(getenv("_")) << "] ";
                }
                if(_show_depth) {
                    for(size_t i = _strip_calls; i < scope.depth; ++i) {
                        *_out << ">";
                    }
                    if(scope.depth > _strip_calls) {
                        *_out << " ";
                    }
                }

                *_out << what;

                if(_show_location) {
                    *_out << "\t\t\t\t\t" << file << ":" << line << " (" << func << ")";
                }
                if(newline) {
                    *_out << std::endl;
                }
            }
        }

        /** }@ Low-level API */
};

#ifdef WITH_CLUTCHLOG
#define CLUTCHLOG( LEVEL, WHAT ) { \
    auto& logger = clutchlog::logger(); \
    std::ostringstream msg ; msg  << WHAT; \
    logger.log(msg.str(), clutchlog::level::LEVEL, __FILE__, __FUNCTION__, __LINE__, true); \
}

#else
#define CLUTCHLOG ( LEVEL, WHAT ) { do {/*nothing*/} while(false); }
#endif

#endif // __CLUTCHLOG_H__
