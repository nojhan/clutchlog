#include <iostream>
#include <limits>

#include "../clutchlog/clutchlog.h"

int main(/*const int argc, char* argv[]*/)
{
    using typo = clutchlog::fmt::typo;
    using fg = clutchlog::fmt::fg;
    using bg = clutchlog::fmt::bg;

    clutchlog::fmt     none;
    clutchlog::fmt      end(typo::reset);
    clutchlog::fmt     note(typo::bold);
    clutchlog::fmt     info(fg::green);
    clutchlog::fmt  warning(fg::magenta, typo::bold);
    clutchlog::fmt    error(fg::red,     typo::bold);
    clutchlog::fmt critical(bg::red,     typo::underline, fg::black);

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
    clutchlog::fmt discreet(clutchlog::fmt::fg::white);
    format << "{level}: "
        << discreet("{file}:")
        << clutchlog::fmt(clutchlog::fmt::fg::yellow) << "{line}"
        << clutchlog::fmt(clutchlog::fmt::typo::reset) << " {msg} ! " << std::endl;
    log.format(format.str());
    CLUTCHLOG(critical,"After having inserted styles within a new format template");
}
