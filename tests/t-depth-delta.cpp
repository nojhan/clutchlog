#include <iostream>
#include <limits>

#include "../clutchlog/clutchlog.h"

void deepcall()
{
    CLUTCHLOG(warning,"at depth 4");
    CLUTCHLOGD(warning,"at depth 4+1", 1);
    CLUTCHLOGD(warning,"at depth 4+2", 2);
}

void subsubsubcall()
{
    CLUTCHLOG(warning,"at depth 3");
    CLUTCHLOGD(warning,"at depth 3+1", 1);
    CLUTCHLOGD(warning,"at depth 3+2", 2);
    deepcall();
}

void subsubcall()
{
    CLUTCHLOG(warning,"at depth 2");
    CLUTCHLOGD(warning,"at depth 2+1", 1);
    CLUTCHLOGD(warning,"at depth 2+2", 2);
    subsubsubcall();
}

void subcall()
{
    CLUTCHLOG(warning,"at depth 1");
    CLUTCHLOGD(warning,"at depth 1+1", 1);
    CLUTCHLOGD(warning,"at depth 1+2", 2);
    subsubcall();
}

int main(/*const int argc, char* argv[]*/)
{
    auto& log = clutchlog::logger();
    using fmt = clutchlog::fmt;
    using typo = clutchlog::fmt::typo;

    fmt reset(typo::reset);
    std::ostringstream tpl;
    tpl << "{depth_fmt}{depth} {depth_marks}"
        << reset << "{funchash_fmt}in {func} {msg}\t\n";
    log.format(tpl.str());
    log.threshold(clutchlog::level::xdebug);
    std::vector<fmt> greys = {fmt(15)};
    for(unsigned short i=255; i > 231; i-=3) {
        greys.push_back( fmt(i) ); }
    log.depth_styles( greys );
    log.depth_mark("| ");

    CLUTCHLOG(warning,"in main");
    subcall();
}

