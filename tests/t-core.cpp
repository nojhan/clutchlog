#include "../clutchlog/clutchlog.h"

void g()
{
    CLUTCHLOG(info, "!");
}

void f()
{
    CLUTCHLOG(warning, "world");
    g();
}

int main(const int argc, char* argv[])
{
#ifdef WITH_CLUTCHLOG
    auto& log = clutchlog::logger();

    log.out(std::clog);

    size_t below = 2;
    if(argc >= 2) { below = atoi(argv[1]); }
    log.depth(below);

    log.threshold(clutchlog::level::warning);

    log.file("core");
    log.func("(main|f)");
#endif

    CLUTCHLOG(error, "hello " << argc);

    f();
}
