#include <iostream>

#include "../clutchlog/clutchlog.h"

int main(/*const int argc, char* argv[]*/)
{
#ifdef WITH_CLUTCHLOG

    auto& log = clutchlog::logger();

    log.out(std::clog);
    log.threshold(clutchlog::level::xdebug);

    std::vector<std::string> msg = {"hello", "world", "!"};

    CLUTCHDUMP(xdebug, msg);
#endif
}
