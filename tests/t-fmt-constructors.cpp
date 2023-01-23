#include <iostream>
#include <limits>

#include "../clutchlog/clutchlog.h"

int main(/*const int argc, char* argv[]*/)
{
    using fmt = clutchlog::fmt;
    using fg = clutchlog::fmt::fg;
    using bg = clutchlog::fmt::bg;
    using typo = clutchlog::fmt::typo;

    fmt none;
    fmt c16_full(fg::red   , bg::black  , typo::bold);
    fmt c16_nofg(bg::black , typo::bold);
    fmt c16_nobg(fg::red   , typo::bold);
    fmt c16_fg  (fg::red   );
    fmt c16_bg  (bg::red   );
    fmt c16_typo(typo::bold);
    fmt c16_bft (bg::black , fg::red   , typo::bold);
    fmt c16_bgfg(bg::black , fg::red   );
    fmt c16_tbf (typo::bold, bg::black , fg::red   );
    fmt c16_tfb (typo::bold, fg::red   , bg::black );
    fmt c16_tf  (typo::bold, fg::red   );
    fmt c16_tb  (typo::bold, bg::black );

    fmt c256_fbt(196     , 236       , typo::bold);
    fmt c256_ft (196     , typo::bold);
    fmt c256_fb (196     , 236       );
    fmt c256_nbt(fg::none, 236       , typo::bold);
    fmt c256_fnt(196     , bg::none  , typo::bold);
    fmt c256_nb (fg::none, 236       );
    fmt c256_fn (196     , bg::none  );

    fmt c16M_fbt(255,10,10 , 10,10,20  , typo::bold);
    fmt c16M_ft (255,10,10 , typo::bold);
    fmt c16M_fb (255,10,10 , 10,10,20  );
    fmt c16M_nbt(fg::none  , 10,10,20  , typo::bold);
    fmt c16M_fnt(255,10,10 , bg::none  , typo::bold);
    fmt c16M_nb (fg::none  , 10,10,20  );
    fmt c16M_fn (255,10,10 , bg::none  );
}



