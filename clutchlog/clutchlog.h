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
        static clutchlog& logger()
        {
            static clutchlog instance;
            return instance;
        }

        enum level {quiet, error, warning, info, debug, xdebug};

    public:
        clutchlog(clutchlog const&)      = delete;
        void operator=(clutchlog const&) = delete;

    private:
        clutchlog() :
            _out(&std::clog),
            _depth(std::numeric_limits<size_t>::max()),
            _level(level::error),
            _in_file(".*"),
            _in_func(".*"),
            _in_line(".*")
        {}

    protected:
        // system, main, log
        const size_t _strip_calls = 3;
        std::ostream* _out;
        size_t _depth;
        level _level;
        std::regex _in_file;
        std::regex _in_func;
        std::regex _in_line;

    public:
        void out(std::ostream& out) {_out = &out;}
        std::ostream& out() {return *_out;}

        void depth(size_t d) {_depth = d;}
        size_t depth() const {return _depth;}

        void  threshold(level l) {_level = l;}
        level threshold() const {return _level;}

        void file(std::string file) {_in_file = file;}
        void func(std::string func) {_in_func = func;}
        void line(std::string line) {_in_line = line;}

        void location(std::string in_file, std::string in_function=".*", std::string in_line=".*")
        {
            file(in_file);
            func(in_function);
            line(in_line);
        }

        void log(std::string what, level log_level, std::string file, std::string func, size_t line, bool newline)
        {
            const size_t max_buffer = 1024;
            size_t stack_depth;
            void *buffer[max_buffer];
            stack_depth = backtrace(buffer, max_buffer);
            if(log_level <= _level and stack_depth <= _depth + _strip_calls) {

                std::ostringstream sline; sline << line;
                if(    std::regex_search(file, _in_file)
                   and std::regex_search(func, _in_func)
                   and std::regex_search(sline.str(), _in_line)) {

                    *_out << "[" << basename(getenv("_")) << "] ";
                    for(size_t i = _strip_calls; i < stack_depth; ++i) {
                        *_out << ">";
                    }
                    if(stack_depth > _strip_calls) {
                        *_out << " ";
                    }
                    *_out << what;
                    *_out << "\t\t\t\t\t" << file << ":" << line << " (" << func << ")";
                    if(newline) {
                        *_out << std::endl;
                    }
                } // regex location
            } // log level and stack depth
        }
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
