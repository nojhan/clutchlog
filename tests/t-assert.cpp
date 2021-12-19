#include <iostream>
#include <cassert>

#include "../clutchlog/clutchlog.h"

// Make asserts (de)clutchable.
#define ASSERT(LEVEL, ...) { CLUTCHFUNC(LEVEL, assert, __VA_ARGS__) }

void h()
{
    CLUTCHLOG(info, "!");
    ASSERT(info, true == true);
    std::clog << "--" << std::endl;
}

void g()
{
    CLUTCHLOG(warning, "world");
    ASSERT(warning, strcmp("life","life") == 0);
    h();
}

void f()
{
    CLUTCHLOG(error, "hello ");
    ASSERT(error, strcmp("no more","please")!=0);
    g();
}

int main(/*const int argc, char* argv[]*/)
{
    auto& log = clutchlog::logger();

    log.func("f");
    f();

    log.func("g");
    f();

    log.func("h");
    f();
}
