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
    clutchlog::fmt     info(106); // greenish
    clutchlog::fmt  warning(171, typo::bold); // magentaish
    clutchlog::fmt    error(198, typo::bold); // redish magenta
    clutchlog::fmt critical(226, 196, typo::underline); // Yellow over red.

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
    clutchlog::fmt discreet(254);
    format << "{level}: "
        << discreet("{file}:")
        << clutchlog::fmt(220, typo::inverse) << "{line}" // gold yellow
        << clutchlog::fmt(typo::reset) << " {msg} ! " << std::endl;
    log.format(format.str());
    CLUTCHLOG(critical,"After having inserted styles within a new format template");
}

