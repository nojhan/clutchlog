#include <iostream>
#include <limits>

#include "../clutchlog/clutchlog.h"

int main(/*const int argc, char* argv[]*/)
{
    auto& log = clutchlog::logger();
    log.format("{msg}\t= {filehash_fmt}{file}\n");
    log.threshold(clutchlog::level::xdebug);

    log.filename(clutchlog::filename::path);
    CLUTCHLOG(note,"clutchlog::filename::path");

    log.filename(clutchlog::filename::base);
    CLUTCHLOG(note,"clutchlog::filename::base");

    log.filename(clutchlog::filename::dir);
    CLUTCHLOG(note,"clutchlog::filename::dir");

    log.filename(clutchlog::filename::dirbase);
    CLUTCHLOG(note,"clutchlog::filename::dirbase");

    log.filename(clutchlog::filename::stem);
    CLUTCHLOG(note,"clutchlog::filename::stem");

    log.filename(clutchlog::filename::dirstem);
    CLUTCHLOG(note,"clutchlog::filename::dirstem");
}


