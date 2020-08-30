#include <iostream>

#include "../clutchlog/clutchlog.h"

void h()
{
    CLUTCHLOG(info, "!");
    std::clog << "--" << std::endl;
}

void g()
{
    CLUTCHLOG(warning, "world");
    h();
}

void f()
{
    CLUTCHLOG(error, "hello ");
    g();
}

int main(/*const int argc, char* argv[]*/)
{
#ifdef WITH_CLUTCHLOG

    auto& log = clutchlog::logger();

    log.out(std::clog);

    std::clog << "depth: 99; threshold: xdebug; location: .*" << std::endl;
    log.depth(99);
    log.threshold(clutchlog::level::xdebug);
    log.location(".*",".*");
    f();

    std::clog << "depth: 4; threshold: xdebug; location: ,*" << std::endl;
    log.depth(4);
    log.threshold(clutchlog::level::xdebug);
    log.location(".*");
    f();

    std::clog << "depth: 99; threshold: warning; location: .*" << std::endl;
    log.depth(99);
    log.threshold(clutchlog::level::warning);
    log.location(".*");
    f();

    std::clog << "depth: 99; threshold: xdebug; location: 'core','g'" << std::endl;
    log.depth(99);
    log.threshold(clutchlog::level::xdebug);
    log.location("core","g");
    f();

    std::clog << "depth: 99; threshold: debug; location: '.*','(g|h)'" << std::endl;
    log.depth(99);
    log.threshold(clutchlog::level::debug);
    log.location(".*","(g|h)");
    f();

#endif
}
