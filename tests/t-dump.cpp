#include <iostream>
#include <algorithm>
#include <random>

#include "../clutchlog/clutchlog.h"

int main(/*const int argc, char* argv[]*/)
{
    auto& log = clutchlog::logger();

    log.out(std::clog);
    log.threshold(clutchlog::level::xdebug);

    std::vector<std::string> msg = {"hello", "world", "!"};

    CLUTCHDUMP(xdebug, msg, "test_{n}.dat");

    std::vector<int> v(3);
    std::generate(v.begin(), v.end(), std::rand);
    CLUTCHDUMP(info, v, "rand_{n}.dat");
}
