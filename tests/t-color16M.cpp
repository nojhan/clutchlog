#include <iostream>
#include <limits>

#include "../clutchlog/clutchlog.h"

int main(/*const int argc, char* argv[]*/)
{
    using typo = clutchlog::fmt::typo;
    // using fg = clutchlog::fmt::fg;
    // using bg = clutchlog::fmt::bg;

    clutchlog::fmt     none;
    clutchlog::fmt      end(typo::reset);
    clutchlog::fmt     note(typo::bold);
    clutchlog::fmt     info(120,255,120); // greenish
    clutchlog::fmt  warning("#ff0055", typo::bold); // magentaish
    clutchlog::fmt    error(255,100,150, typo::bold); // redish magenta
    clutchlog::fmt critical("#ffff00", "#ff0000"); // Yellow over red.

    auto& log = clutchlog::logger();
    log.threshold(clutchlog::level::info);

    // Change a style.
    log.style(clutchlog::level::critical, error);
    CLUTCHLOG(critical,"Styles demo");

    CLUTCHLOG(info,"Either using functions...");
    std::cout << none("No style: ") << std::endl;
    std::cout << note("NOTE: bold") << std::endl;
    std::cout << info("INFO: green") << std::endl;

    CLUTCHLOG(info,"... or tags.");
    std::cout << warning  << "WARNING"    << end << ": bold magenta" << std::endl;
    std::cout << error    << "ERROR"      << end << ": bold red" << std::endl;
    std::cout << critical << "CRITICAL"   << end << ": underlined black over red background" << std::endl;

    std::ostringstream format;
    clutchlog::fmt discreet("#888888", typo::inverse);
    format << "{level}: "
        << discreet("{file}") << ":"
        << clutchlog::fmt(/*front RGB*/200,150,0, /*back RGB*/0,0,0) << "{line}" // gold yellow over black
        << clutchlog::fmt(typo::reset) << " {msg} ! " << std::endl;
    log.format(format.str());
    CLUTCHLOG(critical,"After having inserted styles within a new format template");
}


