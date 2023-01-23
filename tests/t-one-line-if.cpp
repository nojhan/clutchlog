#include "../clutchlog/clutchlog.h"

int main()
{
    if(true)
        CLUTCHLOG(error, "WHAT?");
    else
        CLUTCHLOG(info, "OH!");

    if(false)
        CLUTCHLOG(info, "AH!");
    else
        CLUTCHLOG(error, "NO!");
}
