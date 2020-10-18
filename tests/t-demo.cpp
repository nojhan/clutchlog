#include <iostream>

#include "../clutchlog/clutchlog.h"

void i()
{
    CLUTCHLOG(progress, "Reset data structures...");
    CLUTCHLOG(debug, "OK");
    CLUTCHLOG(progress, "Reset functors...");
    CLUTCHLOG(critical, "Impossible to reset, I cannot recover.");
}

void h()
{
    CLUTCHLOG(note, "Filling up data of size: " << 0);
    CLUTCHLOG(error, "Cannot parse input, I will reset stuff.");
    i();
    CLUTCHLOG(xdebug, "Last seen state: " << 0);
}

void g()
{
    CLUTCHLOG(warning, "Input size < " << 1);
    h();
}

void f()
{
    CLUTCHLOG(progress, "Initialize data structures...");
    CLUTCHLOG(debug, "OK");
    CLUTCHLOG(progress, "Initialize functors...");
    CLUTCHLOG(debug, "OK");
    g();
}

int main(const int argc, char* argv[])
{
    auto& log = clutchlog::logger();

    log.out(std::clog);
    log.depth_mark("\t");
    log.format("[{name}] {level}: {depth_marks} {msg}\n");
    log.style(clutchlog::level::progress,clutchlog::fmt::fg::blue);
    log.threshold(clutchlog::level::warning);

    if(argc <= 2) {
        CLUTCHLOG(warning, "Log level not indicated, will default to xdebug");
        log.threshold(clutchlog::level::xdebug);
    } else {
        try {
            log.threshold(log.level_of(argv[1]));
        } catch(std::out_of_range err) {
            CLUTCHLOG(critical,err.what());
            exit(100);
        }
    }

    CLUTCHLOG(progress,"Start something");
    f();
    CLUTCHLOG(progress,"I have stopped");
}
