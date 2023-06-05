#include <iostream>
#include <cassert>
#include <cstring>

#include "../clutchlog/clutchlog.h"

// Make asserts (de)clutchable.
#define ASSERT(...) CLUTCHFUNC(error, assert, __VA_ARGS__);

void h()
{
    CLUTCHLOG(info, "!");
    ASSERT(true == true);
    std::clog << "--" << std::endl;
}

void g()
{
    CLUTCHLOG(warning, "world");
    ASSERT(strcmp("life","life") == 0);
    h();
}

void f()
{
    CLUTCHLOG(error, "hello ");
    ASSERT(strcmp("no more","please")!=0);
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
