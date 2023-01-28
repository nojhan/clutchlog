#include <iostream>
#include <limits>

#include "../clutchlog/clutchlog.h"

void deepcall()
{
    CLUTCHLOG(warning,"at depth 4");
}

void subsubsubcall()
{
    CLUTCHLOG(warning,"at depth 3");
    deepcall();
}

void subsubcall()
{
    CLUTCHLOG(warning,"at depth 2");
    subsubsubcall();
}

void subcall()
{
    CLUTCHLOG(warning,"at depth 1");
    subsubcall();
}

int main(/*const int argc, char* argv[]*/)
{
    auto& log = clutchlog::logger();
    using fmt = clutchlog::fmt;
    using typo = clutchlog::fmt::typo;

    fmt reset(typo::reset);
    std::ostringstream tpl;
    tpl << "{level_fmt}Having a {level} {filehash_fmt}within {file} {funchash_fmt}calling {func} {depth_fmt}at level {depth}"
        << reset << " : {msg}\n";
    log.format(tpl.str());
    log.threshold(clutchlog::level::xdebug);

    log.filehash_styles( {fmt(fmt::fg::red), fmt(fmt::fg::yellow)} );
    log.funchash_styles( {fmt(fmt::fg::green), fmt(fmt::fg::blue),
                          fmt(fmt::fg::bright_green), fmt(fmt::fg::bright_blue), fmt(fmt::fg::magenta)} );
    log.depth_styles( {fmt(255),fmt(250),fmt(245),fmt(240)} );

    CLUTCHLOG(warning,"in main");
    subcall();
}
