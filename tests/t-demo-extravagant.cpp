#include <iostream>

#include "../clutchlog/clutchlog.h"

void dump_data()
{
    CLUTCHLOG(progress, "Dump parsed data...");
    CLUTCHLOG(debug, "Write in `data_dump.csv`");
    CLUTCHLOG(debug, "Data frame size: " << 0 << "x" << "150");
    CLUTCHLOG(xdebug, "Resolution: " << 0);
}

void reset()
{
    CLUTCHLOG(progress, "Reset data structures...");
    CLUTCHLOG(debug, "OK");
    CLUTCHLOG(info, "Reset functors...");
    CLUTCHLOG(critical, "Impossible to reset, I cannot recover.");
    dump_data();
}

void process()
{
    CLUTCHLOG(note, "Filling up data of size: " << 0);
    CLUTCHLOG(error, "Cannot parse input, I will reset stuff.");
    reset();
    CLUTCHLOG(xdebug, "Last seen state: " << 0);
}

void init_data()
{
    CLUTCHLOG(debug, "Data frame size: " << 2 << "x" << "150");
    CLUTCHLOG(xdebug, "Resolution: " << 0.001);
    CLUTCHLOG(warning, "Input height < " << 3);
}

void init_func()
{
    CLUTCHLOG(progress, "Allocate memory...");
    CLUTCHLOG(warning, "Dimension: " << 12);
    CLUTCHLOG(debug, "OK");
}

void init()
{
    CLUTCHLOG(progress, "Initialize data structures...");
    init_data();
    CLUTCHLOG(debug, "OK");
    CLUTCHLOG(progress, "Initialize functors...");
    init_func();
    CLUTCHLOG(debug, "OK");
    CLUTCHLOG(progress, "Process...");
    process();
    CLUTCHLOG(debug, "OK");
}

int main(const int argc, char* argv[])
{
    using level = clutchlog::level;
    using fmt   = clutchlog::fmt;
    using fg    = clutchlog::fmt::fg;
    using bg    = clutchlog::fmt::bg;
    using typo  = clutchlog::fmt::typo;

    auto& log = clutchlog::logger();

    log.style(level::critical, 197);
    log.style(level::error, 202);
    log.style(level::warning, 208);
    log.style(level::progress, 34);
    log.style(level::note, 35);
    log.style(level::info, 36);
    log.style(level::debug, 39);
    log.style(level::xdebug, 45);
    std::ostringstream format;
    fmt reset(typo::reset);
    fmt discreet(fg::black);
    fmt bold(fmt::typo::bold);

    log.depth_mark("| ");
    log.hfill_min(400);
    log.hfill_max(500);
    log.hfill_mark('-');

    const short dark = 238;
    const short lite = 250;

    format
           << fmt(dark,lite) << "{name}"
           << fmt(lite,dark) << ""
           << fmt(fg::none,dark) << "{level_fmt}" << " {level_short} " << reset
           << fmt(dark,bg::none) << "" << reset
           << fmt(dark,bg::none) << "{depth_marks}" << reset
           << "{level_fmt}"
           << bold("{msg}")
           << discreet(" {hfill} ")
           << fmt(dark,bg::none) << ""
           << fmt(fg::none,dark) << "{level_fmt} {func} "
           << fmt(lite,dark) << ""
           << fmt(dark,lite) << "{file}" << reset
           << fmt(dark,lite) << ""
           << fmt(lite,dark) << "{line}" << reset
           << "\n";
    log.format(format.str());

    log.out(std::clog);
    log.strip_calls(4);

    if(argc <= 2) {
        CLUTCHLOG(warning, "Log level not indicated, will default to xdebug");
        log.threshold(level::xdebug);
    } else {
        try {
            log.threshold(log.level_of(argv[1]));
        } catch(std::out_of_range& err) {
            CLUTCHLOG(critical,err.what());
            exit(100);
        }
    }

    CLUTCHLOG(progress,"Start analysis");
    init();
    CLUTCHLOG(progress,"I have stopped");
}

