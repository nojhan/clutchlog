#pragma once
#ifndef CLUTCHLOG_H
//! Header guard.
#define CLUTCHLOG_H

/** @file */
#include <ciso646>
    #ifdef FSEXPERIMENTAL
    #include <experimental/filesystem>
    namespace fs = std::experimental::filesystem;
#else
    #include <filesystem>
    namespace fs = std::filesystem;
#endif

#include <iostream>
#include <sstream>
#include <fstream>
#include <cassert>
#include <cstdlib>
#include <string>
#include <limits>
#include <regex>
#include <map>

//! True if POSIX headers necessary for stack depth management are available.
#if __has_include(<execinfo.h>) && __has_include(<stdlib.h>) && __has_include(<libgen.h>)
    #include <execinfo.h> // execinfo
    #include <stdlib.h>   // getenv
    #include <libgen.h>   // basename
    #define CLUTCHLOG_HAVE_UNIX_SYSINFO 1
#else
    #define CLUTCHLOG_HAVE_UNIX_SYSINFO 0
#endif

//! True if the system can handle the `hfill` feature.
#if __has_include(<sys/ioctl.h>) && __has_include(<stdio.h>) && __has_include(<unistd.h>)
    #include <sys/ioctl.h>
    #include <stdio.h>
    #include <unistd.h>
    #define CLUTCHLOG_HAVE_UNIX_SYSIOCTL 1
#else
    #define CLUTCHLOG_HAVE_UNIX_SYSIOCTL 0
#endif


/**********************************************************************
 * Enable by default in Debug builds.
 **********************************************************************/
#ifndef WITH_CLUTCHLOG
    #ifndef NDEBUG
        //! Actually enable clutchlog features.
        #define WITH_CLUTCHLOG
    #endif
#endif

/**********************************************************************
 * Macros definitions
 **********************************************************************/
#ifdef WITH_CLUTCHLOG

/** @defgroup DefaultConfig Default configuration management
 * @{ **/

#ifndef CLUTCHLOG_DEFAULT_DEPTH_BUILT_NODEBUG
    //! Default level over which calls to the logger are optimized out when NDEBUG is defined.
    #define CLUTCHLOG_DEFAULT_DEPTH_BUILT_NODEBUG clutchlog::level::progress
#endif // CLUTCHLOG_DEFAULT_DEPTH_BUILT

/** @} DefaultConfig */


/** @defgroup UseMacros High-level API macros
 * @{ */

//! Handy shortcuts to location.
#define CLUTCHLOC __FILE__, __FUNCTION__, __LINE__

//! Log a message at the given level and with a given depth delta.
#ifndef NDEBUG
    #define CLUTCHLOGD( LEVEL, WHAT, DEPTH_DELTA ) do {                                                   \
        auto& clutchlog__logger = clutchlog::logger();                                                    \
        std::ostringstream clutchlog__msg ; clutchlog__msg << WHAT;                                       \
        clutchlog__logger.log(clutchlog::level::LEVEL, clutchlog__msg.str(), CLUTCHLOC, DEPTH_DELTA);     \
    } while(0)
#else // not Debug build.
    #define CLUTCHLOGD( LEVEL, WHAT, DEPTH_DELTA ) do {                                                   \
        if(clutchlog::level::LEVEL <= CLUTCHLOG_DEFAULT_DEPTH_BUILT_NODEBUG) {                            \
            auto& clutchlog__logger = clutchlog::logger();                                                \
            std::ostringstream clutchlog__msg ; clutchlog__msg << WHAT;                                   \
            clutchlog__logger.log(clutchlog::level::LEVEL, clutchlog__msg.str(), CLUTCHLOC, DEPTH_DELTA); \
        }                                                                                                 \
    } while(0)
#endif // NDEBUG

//! Log a message at the given level.
#ifndef NDEBUG
    #define CLUTCHLOG( LEVEL, WHAT ) \
        CLUTCHLOGD(LEVEL, WHAT, 0)
#else // not Debug build.
    #define CLUTCHLOG( LEVEL, WHAT ) \
        CLUTCHLOGD(LEVEL, WHAT, 0)
#endif // NDEBUG

//! Dump the given container.
#ifndef NDEBUG
    #define CLUTCHDUMP( LEVEL, CONTAINER, FILENAME ) do {                                           \
        auto& clutchlog__logger = clutchlog::logger();                                              \
        clutchlog__logger.dump(clutchlog::level::LEVEL, std::begin(CONTAINER), std::end(CONTAINER), \
                    CLUTCHLOC, FILENAME, CLUTCHDUMP_DEFAULT_SEP);                                   \
    } while(0)
#else // not Debug build.
    #define CLUTCHDUMP( LEVEL, CONTAINER, FILENAME ) do {                                               \
        if(clutchlog::level::LEVEL <= CLUTCHLOG_DEFAULT_DEPTH_BUILT_NODEBUG) {                          \
            auto& clutchlog__logger = clutchlog::logger();                                              \
            clutchlog__logger.dump(clutchlog::level::LEVEL, std::begin(CONTAINER), std::end(CONTAINER), \
                        CLUTCHLOC, FILENAME, CLUTCHDUMP_DEFAULT_SEP);                                   \
        }                                                                                               \
    } while(0)
#endif // NDEBUG

//! Call any function if the scope matches.
#ifndef NDEBUG
    #define CLUTCHFUNC( LEVEL, FUNC, ... ) do {                                                             \
        auto& clutchlog__logger = clutchlog::logger();                                                      \
        clutchlog::scope_t clutchlog__scope = clutchlog__logger.locate(clutchlog::level::LEVEL, CLUTCHLOC); \
        if(clutchlog__scope.matches) {                                                                      \
            FUNC(__VA_ARGS__);                                                                              \
        }                                                                                                   \
    } while(0)
#else // not Debug build.
    #define CLUTCHFUNC( LEVEL, FUNC, ... ) do {                                                                 \
        if(clutchlog::level::LEVEL <= CLUTCHLOG_DEFAULT_DEPTH_BUILT_NODEBUG) {                                  \
            auto& clutchlog__logger = clutchlog::logger();                                                      \
            clutchlog::scope_t clutchlog__scope = clutchlog__logger.locate(clutchlog::level::LEVEL, CLUTCHLOC); \
            if(clutchlog__scope.matches) {                                                                      \
                FUNC(__VA_ARGS__);                                                                              \
            }                                                                                                   \
        }                                                                                                       \
    } while(0)
#endif // NDEBUG

//! Run any code if the scope matches.
#ifndef NDEBUG
    #define CLUTCHCODE( LEVEL, ... ) do {                                                                   \
        auto& clutchlog__logger = clutchlog::logger();                                                      \
        clutchlog::scope_t clutchlog__scope = clutchlog__logger.locate(clutchlog::level::LEVEL, CLUTCHLOC); \
        if(clutchlog__scope.matches) {                                                                      \
            __VA_ARGS__                                                                                     \
        }                                                                                                   \
    } while(0)
#else // not Debug build.
    #define CLUTCHCODE( LEVEL, CODE ) do {                                                                      \
        if(clutchlog::level::LEVEL <= CLUTCHLOG_DEFAULT_DEPTH_BUILT_NODEBUG) {                                  \
            auto& clutchlog__logger = clutchlog::logger();                                                      \
            clutchlog::scope_t clutchlog__scope = clutchlog__logger.locate(clutchlog::level::LEVEL, CLUTCHLOC); \
            if(clutchlog__scope.matches) {                                                                      \
                CODE                                                                                            \
            }                                                                                                   \
        }                                                                                                       \
    } while(0)
#endif // NDEBUG

/** @} UseMacros */

#else // not WITH_CLUTCHLOG
    // Disabled macros can still be called in Release builds.
    #define CLUTCHLOG(  LEVEL, WHAT )                do {/*nothing*/} while(0)
    #define CLUTCHLOGD( LEVEL, WHAT, DEPTH_DELTA )   do {/*nothing*/} while(0)
    #define CLUTCHDUMP( LEVEL, CONTAINER, FILENAME ) do {/*nothing*/} while(0)
    #define CLUTCHFUNC( LEVEL, FUNC, ... )           do {/*nothing*/} while(0)
    #define CLUTCHCODE( LEVEL, CODE )                do {/*nothing*/} while(0)
#endif // WITH_CLUTCHLOG

/**********************************************************************
 * Implementation
 **********************************************************************/

#ifdef WITH_CLUTCHLOG
/** @defgroup Main Main class
 * @{
 */
/** The single class which holds everything.
 *
 * This is a Singleton class.
 */
class clutchlog
{
    protected:

    /** @name Default configuration members
     * @{ */
    /** @ingroup DefaultConfig
     * @{ */
        #ifndef NDEBUG
            #ifndef CLUTCHLOG_DEFAULT_FORMAT
                //! Compile-time default format of the messages (debug mode: with absolute location).
                #if CLUTCHLOG_HAVE_UNIX_SYSINFO == 1 // Enables: name, depth and depth_marks
                    #if CLUTCHLOG_HAVE_UNIX_SYSIOCTL == 1 // Enables: hfill
                        #define CLUTCHLOG_DEFAULT_FORMAT "[{name}] {level_letter}:{depth_marks} {msg} {hfill} {func} @ {file}:{line}\n"
                    #else
                        #define CLUTCHLOG_DEFAULT_FORMAT "[{name}] {level_letter}:{depth_marks} {msg}\t\t\t\t\t{func} @ {file}:{line}\n"
                    #endif
                #else
                    #if CLUTCHLOG_HAVE_UNIX_SYSIOCTL == 1
                        #define CLUTCHLOG_DEFAULT_FORMAT "{level_letter} {msg} {hfill} {func} @ {file}:{line}\n"
                    #else
                        #define CLUTCHLOG_DEFAULT_FORMAT "{level_letter} {msg}\t\t\t\t\t{func} @ {file}:{line}\n"
                    #endif
                #endif
            #endif
        #else
            #ifndef CLUTCHLOG_DEFAULT_FORMAT
                //! Compile-time default format of the messages (non-debug mode: without absolute location).
                #if CLUTCHLOG_HAVE_UNIX_SYSINFO == 1
                    #define CLUTCHLOG_DEFAULT_FORMAT "[{name}] {level_letter}:{depth_marks} {msg} {hfill} {func}\n"
                #else
                    #define CLUTCHLOG_DEFAULT_FORMAT "{level_letter} {msg}\t\t\t\t\t{func}\n"
                #endif
            #endif
        #endif
        //! Default format of the messages.
        static inline std::string default_format = CLUTCHLOG_DEFAULT_FORMAT;

        #ifndef NDEBUG
            #ifndef CLUTCHDUMP_DEFAULT_FORMAT
                //! Compile-time default format of the comment line in file dump.
                #if CLUTCHLOG_HAVE_UNIX_SYSINFO == 1
                    #define CLUTCHDUMP_DEFAULT_FORMAT "# [{name}] {level} in {func} (at depth {depth}) @ {file}:{line}"
                #else
                    #define CLUTCHDUMP_DEFAULT_FORMAT "# {level} in {func} @ {file}:{line}"
                #endif
            #endif // CLUTCHDUMP_DEFAULT_FORMAT
        #else
            #ifndef CLUTCHDUMP_DEFAULT_FORMAT
                //! Compile-time default format of the comment line in file dump.
                #if CLUTCHLOG_HAVE_UNIX_SYSINFO == 1
                    #define CLUTCHDUMP_DEFAULT_FORMAT "# [{name}] {level} in {func} (at depth {depth})"
                #else
                    #define CLUTCHDUMP_DEFAULT_FORMAT "# {level} in {func}"
                #endif
            #endif // CLUTCHDUMP_DEFAULT_FORMAT
        #endif
        //! Default format of the comment line in file dump.
        static inline std::string dump_default_format = CLUTCHDUMP_DEFAULT_FORMAT;

        #ifndef CLUTCHDUMP_DEFAULT_SEP
            //! Compile-time default item separator for dump.
            #define CLUTCHDUMP_DEFAULT_SEP "\n"
        #endif // CLUTCHDUMP_DEFAULT_SEP
        //! Default item separator for dump.
        static inline std::string dump_default_sep = CLUTCHDUMP_DEFAULT_SEP;

        #ifndef CLUTCHLOG_DEFAULT_DEPTH_MARK
            //! Compile-time default mark for stack depth.
            #define CLUTCHLOG_DEFAULT_DEPTH_MARK ">"
        #endif // CLUTCHLOG_DEFAULT_DEPTH_MARK
        //! Default mark for stack depth.
        static inline std::string default_depth_mark = CLUTCHLOG_DEFAULT_DEPTH_MARK;

        #ifndef CLUTCHLOG_STRIP_CALLS
            //! Compile-time number of call stack levels to remove from depth display by default.
            #define CLUTCHLOG_STRIP_CALLS 5
        #endif // CLUTCHLOG_STRIP_CALLS
        //! Number of call stack levels to remove from depth display by default.
        static inline unsigned int default_strip_calls = CLUTCHLOG_STRIP_CALLS;

        #ifndef CLUTCHLOG_DEFAULT_HFILL_MARK
            //! Character used as a filling for right-align the right part of messages with "{hfill}".
            #define CLUTCHLOG_DEFAULT_HFILL_MARK '.'
        #endif // CLUTCHLOG_DEFAULT_HFILL_MARK
        //! Default character used as a filling for right-align the right part of messages with "{hfill}".
        static inline char default_hfill_char = CLUTCHLOG_DEFAULT_HFILL_MARK;


        #if CLUTCHLOG_HAVE_UNIX_SYSIOCTL == 1
            #ifndef CLUTCHLOG_DEFAULT_HFILL_MAX
                #define CLUTCHLOG_DEFAULT_HFILL_MAX 300
            #endif
            #ifndef CLUTCHLOG_DEFAULT_HFILL_MIN
                #define CLUTCHLOG_DEFAULT_HFILL_MIN 150
            #endif
        #endif
        //! Default maximum width (number of characters) for which to fill for right-aligning the right part of messages (using "{hfill}").
        static inline size_t default_hfill_max = CLUTCHLOG_DEFAULT_HFILL_MAX;
        //! Default minimum width (number of characters) at which to fill for right-aligning the right part of messages (using "{hfill}").
        static inline size_t default_hfill_min = CLUTCHLOG_DEFAULT_HFILL_MIN;

        // NOTE: there is no CLUTCHLOG_HFILL_STYLE for defaulting,
        // but you can still set `hfill_style(...)` on the logger singleton.
    /* @} DefaultConfig */
    /* @} */


    public:

        /** @name High-level API
         * @{ */

        /** Get the logger instance.
         *
         * @code
         * auto& log = clutchlog::logger();
         * @endcode
         */
        static clutchlog& logger()
        {
            static clutchlog instance;
            return instance;
        }

        //! Available log levels.
        enum level {critical=0, error=1, warning=2, progress=3, note=4, info=5, debug=6, xdebug=7};

        //! Available filename rendering methods.
        enum filename {path, base, dir, dirbase, stem, dirstem};

        /** @} */

        /** @addtogroup Formating Formating tools
         * @{ */

        /** Color and style formatter for ANSI terminal escape sequences.
         *
         * The formatter supports typographic "styles" and colors.
         * Typographic styles are available as named tag in `fmt::typo`.
         *
         * The formatter supports the three ANSI modes, which are automatically selected depending the argument types:
         * - 16 colors (using named tags),
         * - 256 colors (using integers),
         * - 16 millions ("true") colors (using RGB integer triplets or web hex strings).
         *
         * For 16-colors mode, colors are named tag in:
         * - `fmt::fg` for foreground colors.
         * - `fmt::bg` for background colors.
         *
         * @note The order in which you pass foreground, background and style does not matter in 16-colors mode.
         *
         * The following colors are available for both foregrounds (see `fmt::fg`) and backgrounds (see `fmt::bg`):
         * - none,
         * - black,
         * - red,
         * - green,
         * - yellow,
         * - blue,
         * - magenta,
         * - cyan,
         * - white,
         * - bright_black (i.e. grey),
         * - bright_red,
         * - bright_green,
         * - bright_yellow,
         * - bright_blue,
         * - bright_magenta,
         * - bright_cyan,
         * - bright_white.
         *
         * @note Some terminal are configured to display colored text set in bold
         *       using the bright color counterpart.
         *
         * For 256-colors mode, colors are expected to be passed as integers in [-1,255]
         * or the `fg::none` and `bg::none` tags.
         *
         * @note In 256-colors mode, if you want to only encode the background color,
         *       you cannot just omit the foreground color,
         *       you have to bass a `fg::none` tag as first argument.
         *
         * For 16M-colors mode, colors can be encoded as:
         * - three integer arguments,
         * - a "web color" hexadecimal triplet string, starting with a leading number sign (e.g. "#0055ff").
         * - the `fg::none` and `bg::none` tags.
         *
         * @note In 16M-colors mode, if you want to only encode the background color,
         *       you cannot just omit the foreground color,
         *       you have to bass a `fg::none` tag as first argument.
         *
         * @note All styles may not be supported by a given terminal/operating system.
         */
        class fmt {
            public:
                //! ANSI code configuring the available number of colors.
                enum class ansi {
                    //! 16 colors mode.
                    colors_16  = -1, // Not supposed to be casted.
                    //! 256 colors mode.
                    colors_256 = 5,  // Casted as short in color::operator<<.
                    //! 16 millions ("true") colors mode.
                    colors_16M = 2   // Casted as short in color::operator<<
                } /** Current ANSI color mode. */ mode;

                //! Typographic style codes.
                enum class typo {
                    reset     =  0,
                    bold      =  1,
                    underline =  4,
                    inverse   =  7,
                    none = -1
                } /** Typographic style. */  style;

                /** @addtogroup colors16 Colors management in 16 colors mode (4-bits ANSI).
                 * @{ */
                //! Foreground color codes.
                enum class fg {
                    black   = 30,
                    red     = 31,
                    green   = 32,
                    yellow  = 33,
                    blue    = 34,
                    magenta = 35,
                    cyan    = 36,
                    white   = 37,
                    bright_black   = 90,
                    bright_red     = 91,
                    bright_green   = 92,
                    bright_yellow  = 93,
                    bright_blue    = 94,
                    bright_magenta = 95,
                    bright_cyan    = 96,
                    bright_white   = 97,
                    none = -1
                } /** Foreground color. */ fore;

                //! Background color codes.
                enum class bg {
                    black   = 40,
                    red     = 41,
                    green   = 42,
                    yellow  = 43,
                    blue    = 44,
                    magenta = 45,
                    cyan    = 46,
                    white   = 47,
                    bright_black   = 100,
                    bright_red     = 101,
                    bright_green   = 102,
                    bright_yellow  = 103,
                    bright_blue    = 104,
                    bright_magenta = 105,
                    bright_cyan    = 106,
                    bright_white   = 107,
                    none = -1
                } /** Background color. */ back;

            protected:
                //! Output stream operator for a 3-tuple of 16-colors mode tags.
                friend std::ostream& operator<<(std::ostream& os, const std::tuple<fg,bg,typo>& fbs)
                {
                    auto [f,b,s] = fbs;
                        std::vector<short> codes; codes.reserve(3);
                        if(f !=   fg::none) { codes.push_back(static_cast<short>(f));}
                        if(b !=   bg::none) { codes.push_back(static_cast<short>(b));}
                        if(s != typo::none) { codes.push_back(static_cast<short>(s));}
                        if(codes.size() == 0) {
                            return os;

                        } else {
                            os << "\033[";
                            os << codes[0];
                            for(size_t i=1; i < codes.size(); ++i) {
                                os << ";" << codes[i];
                            }
                            os << "m";
                        }
                        return os;
                }

                //! Output stream operator for a typo tag alone, in 16-colors mode.
                friend std::ostream& operator<<(std::ostream& os, const typo& s)
                {
                    if(s != typo::none) {
                        os << "\033[" << static_cast<short>(s) << "m";
                    }
                    return os;
                }

                /** @} colors16 */

            protected:
                /** @addtogroup colors256_16M Internal colors management in 256 and 16M colors modes.
                 * @{ */

                /** Interface class for colors representation. */
                struct color {
                    ansi mode; // Not const to allow for the implicit copy assignemnt operator.

                    //! Codes for representing foreground or background.
                    enum class ground { // idem.
                        fore = 38,
                        back = 48
                    } /** Type of color (foreground or background). */ type;

                    /** Constructor.
                     *
                     * @param a ANSI mode (i.e. number of colors).
                     * @param g Color type (i.e. foreground or background).
                     */
                    color(ansi a, ground g) : mode(a), type(g) {}

                    //! Should return true if the underying representation encodes an existing color.
                    virtual bool is_set() const = 0;

                    //! Should print the underlying representation on the given stream.
                    virtual std::ostream& print_on( std::ostream& os) const = 0;

                    //! Print the actually encoded escaped color sequence on the given stream.
                    friend std::ostream& operator<<(std::ostream& os, const color& c)
                    {
                        if(c.is_set()) {
                            os << "\033[" << static_cast<short>(c.type) << ";" << static_cast<short>(c.mode) << ";";
                            c.print_on(os);
                            os << "m";
                        }
                        return os;
                    }
                };

                // There is no color_16 because it would be the same as color_256, only with different indices,
                // hence making it more complicated for the user to select the right constructor.
                // Here, we just use enum for 16 colors, and indices for 256 colors.

                //! Abstract base class for 256 colors objects (8-bits ANSI).
                struct color_256 : public color {
                    /** The encoded color index in 4-bits ANSI.
                     *
                     * "No color" is encoded as -1. */
                    short index;

                    /** Constructor
                     *
                     * @param t Foreground or background tag. */
                    color_256(ground t) : color(ansi::colors_256, t), index(-1) {}

                    /** Constructor
                     *
                     * @param t Foreground or background tag.
                     * @param i Color index (within [-1,255], -1 being "no color").
                     */
                    color_256(ground t, const short i) : color(ansi::colors_256, t), index(i) {assert(-1 <= i and i <= 255);}

                    //! Returns true if the underying representation encodes an existing color.
                    bool is_set() const {return index > -1;}

                    //! Print the color index on the given stream.
                    std::ostream& print_on( std::ostream& os) const
                    {
                        os << index;
                        return os;
                    }
                };

                //! Foreground in 256-colors mode.
                struct fg_256 : public color_256 {
                    //! Empty constructor: no color.
                    fg_256() : color_256(ground::fore) {}

                    /** Constructor.
                     *
                     * @param f Foreground color index (within [-1,255], -1 being "no color"). */
                    fg_256(const short f) : color_256(ground::fore, f) {}

                    /** Conversion constructor from 16-colors mode.
                     *
                     * @warning Only encodes "no color", whatever is passed. */
                    fg_256(const fg&) : color_256(ground::fore, -1) {}

                } /** Current foreground in 256-colors mode. */ fore_256;

                //! Background in 256-colors mode.
                struct bg_256 : public color_256 {
                    //! Empty constructor: no color.
                    bg_256() : color_256(ground::back) {}

                    /** Constructor.
                     *
                     * @param b Background color index (within [-1,255], -1 being "no color"). */
                    bg_256(const short b) : color_256(ground::back, b) {}

                    /** Conversion constructor from 16-colors mode.
                     *
                     * @warning Only encodes "no color", whatever is passed. */
                    bg_256(const bg&) : color_256(ground::back, -1) {}

                } /** Current background in 256-colors mode. */ back_256;

                //! Abstract base class for 16M colors objects (24-bits ANSI).
                struct color_16M : public color {
                    /** The encoded RGB indices.
                     *
                     * "No color" is encoded as -1. */
                    short red, green, blue;

                    /** Constructor.
                     *
                     * @param t Foreground or background tag. */
                    color_16M(ground t) : color(ansi::colors_16M, t), red(-1), green(-1), blue(-1) {}

                    /** Numeric triplet constructor.
                     *
                     * @param t Foreground or background tag.
                     * @param r Red color component.
                     * @param g Green color component.
                     * @param b Blue color component.
                     */
                    color_16M(ground t, short r, short g, short b)
                        : color(ansi::colors_16M, t), red(r), green(g), blue(b) {}

                    /** Hex triplet string constructor.
                     *
                     * @note If the given string is ill-formed, it will silently encode a "no color".
                     *
                     * @param t Foreground or background tag.
                     * @param srgb A "web color" hexadecimal triplet of two characters, starting with a leading number sign (e.g. "#0055ff").
                     */
                    color_16M(ground t, const std::string& srgb) : color(ansi::colors_16M, t)
                    {
                        assert(srgb.size() == 7);
                        if(srgb.size() != 7) {
                            red = -1;
                            green = -1;
                            blue = -1;
                        } else {
                            char i = 0;
                            if(srgb.at(0) == '#') {
                                i = 1;
                            }
                            std::istringstream(srgb.substr(0+i,2)) >> std::hex >> red;
                            std::istringstream(srgb.substr(2+i,2)) >> std::hex >> green;
                            std::istringstream(srgb.substr(4+i,2)) >> std::hex >> blue;
                        }
                        assert(-1 <= red   and red   <= 255);
                        assert(-1 <= green and green <= 255);
                        assert(-1 <= blue  and blue  <= 255);
                    }

                    //! Returns true if the underying representation encodes an existing color.
                    bool is_set() const {return red > -1 and green > -1 and blue > -1;}

                    //! Print the color RGB triplet on the given stream.
                    std::ostream& print_on( std::ostream& os) const
                    {
                        os << red << ";" << green << ";" << blue;
                        return os;
                    }
                };

                //! Foreground in 256-colors mode.
                struct fg_16M : public color_16M {
                    //! Empty constructor: no color.
                    fg_16M() : color_16M(ground::fore) {}

                    /** Numeric triplet constructor.
                     *
                     * Parameters are expected to be in [0,255].
                     *
                     * @param r Red color component.
                     * @param g Green color component.
                     * @param b Blue color component.
                     */
                    fg_16M(short r, short g, short b) : color_16M(ground::fore, r,g,b) {}

                    /** Hex triplet string constructor.
                     *
                     * @note If the given string is ill-formed, it will silently encode a "no color".
                     *
                     * @param srgb A "web color" hexadecimal triplet of two characters, starting with a leading number sign (e.g. "#0055ff").
                     */
                    fg_16M(const std::string& srgb) : color_16M(ground::fore, srgb) {}

                    /** Conversion constructor from 16-colors mode.
                     *
                     * @warning Only encodes "no color", whatever is passed. */
                    fg_16M(const fg&) : color_16M(ground::fore, -1,-1,-1) {}

                } /** Current foreground in 16M-colors mode. */ fore_16M;

                //! background in 256-colors mode.
                struct bg_16M : public color_16M {
                    //! Empty constructor: no color.
                    bg_16M() : color_16M(ground::back) {}

                    /** Numeric triplet constructor.
                     *
                     * Parameters are expected to be in [0,255].
                     *
                     * @param r Red color component.
                     * @param g Green color component.
                     * @param b Blue color component.
                     */
                    bg_16M(short r, short g, short b) : color_16M(ground::back, r,g,b) {}

                    /** Hex triplet string constructor.
                     *
                     * @note If the given string is ill-formed, it will silently encode a "no color".
                     *
                     * @param srgb A "web color" hexadecimal triplet of two characters, starting with a leading number sign (e.g. "#0055ff").
                     */
                    bg_16M(const std::string& srgb) : color_16M(ground::back, srgb) {}

                    /** Conversion constructor from 16-colors mode.
                     *
                     * @warning Only encodes "no color", whatever is passed. */
                    bg_16M(const bg&) : color_16M(ground::back, -1,-1,-1) {}

                } /** Current background in 16M-colors mode. */ back_16M;

                /** @} colors256_16M */

            public:
                //! Empty constructor, only useful for a no-op formatter.
                fmt() : mode(ansi::colors_16), style(typo::none), fore(fg::none), back(bg::none) {}

                /** @name All combination of 16-colors mode constructors with different parameters orders.
                 * @{ */
                fmt(  fg f,   bg b = bg::none, typo s = typo::none) : mode(ansi::colors_16), style(s), fore(f), back(b) {}
                fmt(  fg f, typo s           ,   bg b =   bg::none) : mode(ansi::colors_16), style(s), fore(f), back(b) {}
                fmt(  bg b,   fg f = fg::none, typo s = typo::none) : mode(ansi::colors_16), style(s), fore(f), back(b) {}
                fmt(  bg b, typo s           ,   fg f =   fg::none) : mode(ansi::colors_16), style(s), fore(f), back(b) {}
                fmt(typo s,   fg f = fg::none,   bg b =   bg::none) : mode(ansi::colors_16), style(s), fore(f), back(b) {}
                fmt(typo s,   bg b           ,   fg f =   fg::none) : mode(ansi::colors_16), style(s), fore(f), back(b) {}
                /** @} */

                /** @name All combination of 256-colors mode constructors with different parameters orders.
                 * @{ */
                fmt(fg_256 f, bg_256 b, typo s = typo::none) : mode(ansi::colors_256), style(s), fore_256(f),        back_256(b) {}
                fmt(fg_256 f, typo s = typo::none)           : mode(ansi::colors_256), style(s), fore_256(f),        back_256(bg::none) {}
                fmt(fg, bg_256 b, typo s = typo::none)       : mode(ansi::colors_256), style(s), fore_256(fg::none), back_256(b) {}
                /** @} */

                /** @name All combination of 16M-colors mode constructors with different parameters orders.
                 * @{ */
                fmt(const short fr, const short fg, const short fb,
                    const short gr, const short gg, const short gb,
                    typo s = typo::none)
                    : mode(ansi::colors_16M), style(s), fore_16M(fr,fg,fb), back_16M(gr,gg,gb) {}
                fmt(fg,
                    const short gr, const short gg, const short gb,
                    typo s = typo::none)
                    : mode(ansi::colors_16M), style(s), fore_16M(fg::none), back_16M(gr,gg,gb) {}
                fmt(const short fr, const short fg, const short fb,
                    bg, typo s = typo::none)
                    : mode(ansi::colors_16M), style(s), fore_16M(fr,fg,fb), back_16M(bg::none) {}
                fmt(const short fr, const short fg, const short fb,
                    typo s = typo::none)
                    : mode(ansi::colors_16M), style(s), fore_16M(fr,fg,fb), back_16M(bg::none) {}

                fmt(const std::string& f, const std::string& b, typo s = typo::none)
                    : mode(ansi::colors_16M), style(s), fore_16M(f), back_16M(b) {}
                fmt(fg, const std::string& b, typo s = typo::none)
                    : mode(ansi::colors_16M), style(s), fore_16M(fg::none), back_16M(b) {}
                fmt(const std::string& f, bg, typo s = typo::none)
                    : mode(ansi::colors_16M), style(s), fore_16M(f), back_16M(bg::none) {}
                fmt(const std::string& f, typo s = typo::none)
                    : mode(ansi::colors_16M), style(s), fore_16M(f), back_16M(bg::none) {}
                /** @} */

            protected:

                //! Print the currently encoded format escape code on the given output stream.
                std::ostream& print_on( std::ostream& os) const
                {
                    if(mode == ansi::colors_16) {
                        // Print all in a single escape.
                        os << std::make_tuple(fore,back,style);

                    } else {
                        // 256 or 16M: always print separated escapes for foreground/background.
                        if(mode == ansi::colors_256) {
                            os << fore_256;
                            os << back_256;

                        } else if(mode == ansi::colors_16M) {
                            os << fore_16M;
                            os << back_16M;
                        }
                        // In any case, print the style separately.
                        os << style;
                    }
                    return os;
                }

            public:
                /** Output stream overload.
                 *
                 * Allow to use a formatter as a tag within a stream:
                 * @code
                 * clutchlog::fmt end(clutchlog::fmt::typo::reset);
                 * clutchlog::fmt error(clutchlog::fmt::fg::red, clutchlog::fmt::typo::bold);
                 * std::cout << error << "ERROR" << end << std::endl;
                 * @endcode
                 *
                 * @note An formatter called this way will NOT output a reset escape code.
                 */
                friend std::ostream& operator<<(std::ostream& os, const fmt& fmt)
                {
                    return fmt.print_on(os);
                }

                /** Format the given string with the currently encoded format.
                 *
                 * Allow to use a formatter as a function:
                 * @code
                 * clutchlog::fmt error(clutchlog::fmt::fg::red, clutchlog::fmt::typo::bold);
                 * std::cout << error("ERROR") << std::endl;
                 * @endcode
                 *
                 * @note A formatter called this way WILL output a reset escape code at the end.
                 */
                std::string operator()( const std::string& msg ) const
                {
                    std::ostringstream os;
                    this->print_on(os);
                    fmt reset(fmt::typo::reset);
                    os << msg;
                    reset.print_on(os);
                    return os.str();
                }

                /** Return the formatting code as a string.
                 */
                std::string str() const
                {
                    std::ostringstream os;
                    this->print_on(os);
                    return os.str();
                }

                static fmt hash( const std::string& str, const std::vector<fmt> domain = {})
                {
                    size_t h = std::hash<std::string>{}(str);
                    if(domain.size() == 0) {
                        return fmt(static_cast<short>(h % 256));
                    } else {
                        return fmt(domain[h % domain.size()]);
                    }
                }
        }; // fmt class

        /** @} */

    /** @name Internal details
     * @{ */

    public:
        clutchlog(clutchlog const&)      = delete;
        void operator=(clutchlog const&) = delete;

    private:
        clutchlog() :
            // system, main, log
            _strip_calls(clutchlog::default_strip_calls),
            _level_word({
                {level::critical,"Critical"},
                {level::error   ,"Error"},
                {level::warning ,"Warning"},
                {level::progress,"Progress"},
                {level::note    ,"Note"},
                {level::info    ,"Info"},
                {level::debug   ,"Debug"},
                {level::xdebug  ,"XDebug"}
            }),
            _level_short({
                {level::critical, "Crit"},
                {level::error   , "Erro"},
                {level::warning , "Warn"},
                {level::progress, "Prog"},
                {level::note    , "Note"},
                {level::info    , "Info"},
                {level::debug   , "Dbug"},
                {level::xdebug  , "XDbg"}
            }),
            _level_fmt({
                {level::critical,fmt(fmt::fg::red,     fmt::typo::underline)},
                {level::error   ,fmt(fmt::fg::red,     fmt::typo::bold)},
                {level::warning ,fmt(fmt::fg::magenta, fmt::typo::bold)},
                {level::progress,fmt()},
                {level::note    ,fmt()},
                {level::info    ,fmt()},
                {level::debug   ,fmt()},
                {level::xdebug  ,fmt()}
            }),
            _format_log(clutchlog::default_format),
            _format_dump(clutchlog::dump_default_format),
            #if CLUTCHLOG_HAVE_UNIX_SYSIOCTL
                _hfill_char(clutchlog::default_hfill_char),
                _hfill_fmt(fmt::fg::none),
                _hfill_max(clutchlog::default_hfill_max),
                _hfill_min(clutchlog::default_hfill_min),
            #endif
            _out(&std::clog),
            #if CLUTCHLOG_HAVE_UNIX_SYSINFO == 1
                _depth(std::numeric_limits<size_t>::max() - _strip_calls),
                _depth_mark(clutchlog::default_depth_mark),
            #endif
            _stage(level::error),
            _in_file(".*"),
            _in_func(".*"),
            _in_line(".*"),
            // Empty vectors by default:
            // _filehash_fmts
            // _funchash_fmts
            // _depth_fmts
            _filename(filename::path)
        {
            // Reverse the level->word map into a word->level map.
            for(auto& lw : _level_word) {
                _word_level[lw.second] = lw.first;
            }
#if CLUTCHLOG_HAVE_UNIX_SYSIOCTL
            struct winsize w;
            ioctl(STDERR_FILENO, TIOCGWINSZ, &w);
            _nb_columns = std::max(std::min((size_t)w.ws_col, default_hfill_max), default_hfill_min);
#endif
        }

    protected:
        /** Current number of call stack levels to remove from depth display. */
        size_t _strip_calls;
        /** Dictionary of level identifier to their string representation. */
        const std::map<level,std::string> _level_word;
        /** Dictionary of level string to their identifier. */
        std::map<std::string,level> _word_level;
        /** dictionary of level identifier to their 4-letters representation. */
        std::map<level,std::string> _level_short;
        /** Dictionary of level identifier to their format. */
        std::map<level,fmt> _level_fmt;
        /** Current format of the standard output. */
        std::string _format_log;
        /** Current format of the file output. */
        std::string _format_dump;
        #if CLUTCHLOG_HAVE_UNIX_SYSIOCTL
            /** Character for filling. */
            char _hfill_char;
            /** Style of the filling. */
            fmt _hfill_fmt;
            /** Maximum width for which to hfill. */
            size_t _hfill_max;
            /** Minimum width at which to hfill. */
            size_t _hfill_min;
        #endif
        /** Standard output. */
        std::ostream* _out;
        #if CLUTCHLOG_HAVE_UNIX_SYSINFO == 1
            /** Current stack depth (above which logs are not printed). */
            size_t _depth;
            /** Current depth mark. */
            std::string _depth_mark;
        #endif
        /** Current log level. */
        level _stage;
        /** Current file location filter. */
        std::regex _in_file;
        /** Current function location filter. */
        std::regex _in_func;
        /** Current line location filter. */
        std::regex _in_line;

        /** List of candidate format objects for value-dependant file name styling. */
        std::vector<fmt> _filehash_fmts;
        /** List of candidate format objects for value-dependant function name styling. */
        std::vector<fmt> _funchash_fmts;

#if CLUTCHLOG_HAVE_UNIX_SYSINFO == 1
        /** Maximum buffer size for backtrace message. */
        static const size_t _max_buffer = 4096;
        /** Ordered list of format objects for value-dependant depth styling. */
        std::vector<fmt> _depth_fmts;
#endif

#if CLUTCHLOG_HAVE_UNIX_SYSIOCTL
        /** Current terminal size (for right-alignment). */
        size_t _nb_columns;
#endif

        /** Filename rendering method. */
        filename _filename;
    /** @} Internal details */

    public:

        /** @name Configuration accessors
         * @{ */

        //! Set the template string.
        void format(const std::string& format) {_format_log = format;}
        //! Get the template string.
        std::string format() const {return _format_log;}

        //! Set the template string for dumps.
        void format_comment(const std::string& format) {_format_dump = format;}
        //! Get the template string for dumps.
        std::string format_comment() const {return _format_dump;}

        //! Set the output stream on which to print.
        void out(std::ostream& out) {_out = &out;}
        //! Get the output stream on which to print.
        std::ostream& out() {return *_out;}

#if CLUTCHLOG_HAVE_UNIX_SYSINFO == 1
        //! Set the stack depth above which logs are not printed.
        void depth(size_t d) {_depth = d;}
        //! Get the stack depth above which logs are not printed.
        size_t depth() const {return _depth;}

        //! Set the string mark with which stack depth is indicated.
        void depth_mark(const std::string mark) {_depth_mark = mark;}
        //! Get the string mark with which stack depth is indicated.
        std::string depth_mark() const {return _depth_mark;}

        //! Set the number of stack calls which are removed for computing depth. */
        void strip_calls(const size_t n) {_strip_calls = n;}
        //! Get the number of stack calls which are removed for computing depth. */
        size_t strip_calls() const {return _strip_calls;}
#endif
#if CLUTCHLOG_HAVE_UNIX_SYSIOCTL == 1
        //! Set the character for the stretching `{hfill}` template tag marker.
        void hfill_mark(const char mark) {_hfill_char = mark;}
        //! Get the character for the stretching `{hfill}` template tag marker.
        char hfill_mark() const {return _hfill_char;}
        //! Set the style for the stretching `{hfill}` template tag marker, with a `fmt` object.
        void hfill_style(fmt style) {_hfill_fmt = style;}
        /** Set the style for the stretching hfill marker.
         *
         * This version accept style arguments as if they were passed to `clutchlog::fmt`.
         */
        template<class ... FMT>
        void hfill_style(FMT... styles) { this->hfill_style(fmt(styles...)); }
        //! Get the character for the stretching `{hfill}` template tag marker.
        fmt hfill_style() const {return _hfill_fmt;}
        //! Set the maximum width for which to `{hfill}`.
        void hfill_max(const size_t nmax) {_hfill_max = nmax;}
        //! Get the maximum width for which to `{hfill}`.
        size_t hfill_max() {return _hfill_max;}
        //! Set the minimum width at which to `{hfill}`.
        void hfill_min(const size_t nmin) {_hfill_min = nmin;}
        //! Get the minimum width at which to `{hfill}`.
        size_t hfill_min() {return _hfill_min;}
#endif
        /** Set the candidate styles for value-dependant file name formatting.
         *
         *   Style will be chosen based on the hash value of the filename
         *   among the candidate ones.
         *
         *   See the `{filehash_fmt}` template tag.
         */
        void filehash_styles(std::vector<fmt> styles) {_filehash_fmts = styles;}
        /** Set the candidate styles for value-dependant function name formatting.
         *
         *   Style will be chosen based on the hash value of the filename
         *   among the candidate ones.
         *
         *   See the `{funchash_fmt}` template tag.
         */
        void funchash_styles(std::vector<fmt> styles) {_funchash_fmts = styles;}
        /** Set the styles for value-dependant depth formatting.
         *
         *   The given list should be ordered, styles will be applied
         *   for the corresponding depth level. If the actual depth is
         *   larger than the number of styles, the last one is used.
         *
         *   See the `{depth_fmt}` template tag.
         */
        void depth_styles(std::vector<fmt> styles) {_depth_fmts = styles;}

        //! Set the log level (below which logs are not printed) with an identifier.
        void  threshold(level l) {_stage = l;}
        //! Set the log level (below which logs are not printed) with a string.
        void  threshold(const std::string& l) {_stage = this->level_of(l);}
        //! Get the log level below which logs are not printed.
        level threshold() const {return _stage;}
        //! Get the map of available log levels string representations toward their identifier. */
        const std::map<std::string,level>& levels() const { return _word_level;}

        /** Return the log level tag corresponding to the given pre-configured name.
         *
         * @note This is case sensitive, see the pre-configured `_level_word`.
         */
        level level_of(const std::string name)
        {
            const auto ilevel = _word_level.find(name);
            if( ilevel != std::end(_word_level)) {
                return ilevel->second;
            } else {
                throw std::out_of_range("'" + name + "' is not a valid log level name");
            }
        }

        //! Set the regular expression filtering the file location.
        void file(std::string file) {_in_file = file;}
        //! Set the regular expression filtering the function location.
        void func(std::string func) {_in_func = func;}
        //! Set the regular expression filtering the line location.
        void line(std::string line) {_in_line = line;}

        //! Set the regular expressions filtering the location.
        void location(
                const std::string& in_file,
                const std::string& in_function=".*",
                const std::string& in_line=".*"
            )
        {
            file(in_file);
            func(in_function);
            line(in_line);
        }

        /** Set the style (color and typo) of the given log level.
         *
         * This version accept style arguments as if they were passed to `clutchlog::fmt`.
         */
        template<class ... FMT>
        void style(level stage, FMT... styles) { this->style(stage,fmt(styles...)); }
        //! Set the style (color and typo) of the given log level, passing a `fmt` instance.
        void style(level stage, fmt style) { _level_fmt.at(stage) = style; }
        //! Get the configured fmt instance of the given log level.
        fmt  style(level stage) const { return _level_fmt.at(stage); }

        //! Sets the file naming scheme. */
        void filename(filename f) {_filename = f;}

        /** @} Configuration accessors */

    public:

        /** @name Low-level API
         * @{ */

        //! Structure holding a location matching.
        struct scope_t {
            /** Everything is compatible. */
            bool matches;
            /** Current log level. */
            level stage;
#if CLUTCHLOG_HAVE_UNIX_SYSINFO == 1
            /** Current depth. */
            size_t depth;
#endif
            /** Location is compatible. */
            bool there;
            /** Constructor. */
            scope_t() :
                matches(false),
                stage(level::xdebug),
#if CLUTCHLOG_HAVE_UNIX_SYSINFO == 1
                depth(0),
#endif
                there(false)
            {}
        }; // scope_t


        //! Gather information on the current location of the call.
        scope_t locate(
                const level& stage,
                const std::string& file,
                const std::string& func,
                const size_t line
            ) const
        {
            scope_t scope; // False scope by default.

            /***** Log level stage *****/
            // Test stage first, because it's fastest.
            scope.stage = stage;
            if(not (scope.stage <= _stage)) {
                // Bypass useless computations if no match
                // because of the stage.
                return scope;
            }
#if CLUTCHLOG_HAVE_UNIX_SYSINFO == 1
            /***** Stack depth *****/
            // Backtrace in second, quite fast.
            size_t stack_depth;
            void *buffer[_max_buffer];
            stack_depth = backtrace(buffer, _max_buffer);
            scope.depth = stack_depth;
            if(not (scope.depth <= _depth + _strip_calls)) {
                // Bypass if no match.
                return scope;
            }
#endif

            /***** Location *****/
            // Location last, slowest.
            std::ostringstream sline; sline << line;
            scope.there =
                       std::regex_search(file, _in_file)
                   and std::regex_search(func, _in_func)
                   and std::regex_search(sline.str(), _in_line);

            // No need to retest stage and depth, which are true here.
            scope.matches = scope.there;

            return scope;
        } // locate

        /** Replace `mark` by `tag` in `form`.
         *
         * @code
         * log.replace("{greet} {world}", "\\{greet\\}", "hello");
         * // returns "hello {world}"
         * @endcode
         */
        std::string replace(
                const std::string& form,
                const std::string& mark,
                const std::string& tag
            ) const
        {
            // Useless debug code, unless something fancy would be done with name tags.
            // std::regex re;
            // try {
            //     re = std::regex(mark);
            //
            // } catch(const std::regex_error& e) {
            //     std::cerr << "ERROR with a regular expression \"" << mark << "\": ";
            //     switch(e.code()) {
            //         case std::regex_constants::error_collate:
            //             std::cerr << "the expression contains an invalid collating element name";
            //             break;
            //         case std::regex_constants::error_ctype:
            //             std::cerr << "the expression contains an invalid character class name";
            //             break;
            //         case std::regex_constants::error_escape:
            //             std::cerr << "the expression contains an invalid escaped character or a trailing escape";
            //             break;
            //         case std::regex_constants::error_backref:
            //             std::cerr << "the expression contains an invalid back reference";
            //             break;
            //         case std::regex_constants::error_brack:
            //             std::cerr << "the expression contains mismatched square brackets ('[' and ']')";
            //             break;
            //         case std::regex_constants::error_paren:
            //             std::cerr << "the expression contains mismatched parentheses ('(' and ')')";
            //             break;
            //         case std::regex_constants::error_brace:
            //             std::cerr << "the expression contains mismatched curly braces ('{' and '}')";
            //             break;
            //         case std::regex_constants::error_badbrace:
            //             std::cerr << "the expression contains an invalid range in a {} expression";
            //             break;
            //         case std::regex_constants::error_range:
            //             std::cerr << "the expression contains an invalid character range (e.g. [b-a])";
            //             break;
            //         case std::regex_constants::error_space:
            //             std::cerr << "there was not enough memory to convert the expression into a finite state machine";
            //             break;
            //         case std::regex_constants::error_badrepeat:
            //             std::cerr << "one of *?+{ was not preceded by a valid regular expression";
            //             break;
            //         case std::regex_constants::error_complexity:
            //             std::cerr << "the complexity of an attempted match exceeded a predefined level";
            //             break;
            //         case std::regex_constants::error_stack:
            //             std::cerr << "there was not enough memory to perform a match";
            //             break;
            //         default:
            //             std::cerr << "unknown error";
            //     }
            //     std::cerr << std::endl;
            //     throw;
            // } // catch

            const std::regex re(mark);
            return std::regex_replace(form, re, tag);
        }

        //! Replace `mark` by `tag` in `form`, converting tag to its string representation first.
        std::string replace(
                const std::string& form,
                const std::string& mark,
                const size_t tag
            ) const
        {
            std::ostringstream stag; stag << tag;
            return replace(form, mark, stag.str());
        }

        //! Substitute all tags in the format string with the corresponding information and apply the style corresponding to the log level.
        std::string format(
                std::string row,
                const std::string& what,
#if CLUTCHLOG_HAVE_UNIX_SYSINFO == 1
                const std::string& name,
#endif
                const level& stage,
                const std::string& file,
                const std::string& func,
                const size_t line
#if CLUTCHLOG_HAVE_UNIX_SYSINFO == 1
                ,
                const size_t depth
#endif
            ) const
        {
            row = replace(row, "\\{msg\\}", what);

            const std::filesystem::path filepath(file);
            assert(filepath.is_absolute());
            std::string filename;
            std::filesystem::path::iterator ip = filepath.end();
            std::advance(ip, -2);
            switch(_filename) {
                case filename::base:
                    filename = filepath.filename().string();
                    break;
                case filename::dir:
                    filename = ip->string();
                    break;
                case filename::dirbase:
                    filename = (*ip / filepath.filename()).string();
                    break;
                case filename::stem:
                    filename = filepath.stem().string();
                    break;
                case filename::dirstem:
                    filename = (*ip / filepath.stem()).string();
                    break;
                case filename::path:
                default:
                    filename = file;
                    break;
            }
            row = replace(row, "\\{file\\}", filename);


            row = replace(row, "\\{func\\}", func);
            row = replace(row, "\\{line\\}", line);

            row = replace(row, "\\{level\\}", _level_word.at(stage));
            std::string letter(1, _level_word.at(stage).at(0)); // char -> string
            row = replace(row, "\\{level_letter\\}", letter);
            row = replace(row, "\\{level_short\\}", _level_short.at(stage));

#if CLUTCHLOG_HAVE_UNIX_SYSINFO == 1
            size_t actual_depth = depth - _strip_calls;
            row = replace(row, "\\{name\\}", name);
            row = replace(row, "\\{depth\\}", actual_depth);

            if(_depth_fmts.size() == 0) {
                row = replace(row, "\\{depth_fmt\\}", fmt(actual_depth % 256).str() );

                std::ostringstream chevrons;
                for(size_t i = 0; i < actual_depth; ++i) {
                    chevrons << _depth_mark;
                }
                row = replace(row, "\\{depth_marks\\}", chevrons.str());

            } else {
                row = replace(row, "\\{depth_fmt\\}",
                    _depth_fmts[std::min(actual_depth,_depth_fmts.size()-1)].str() );

                std::ostringstream chevrons;
                for(size_t i = 0; i < actual_depth; ++i) {
                    chevrons << _depth_fmts[std::min(i+1,_depth_fmts.size()-1)].str()
                             << _depth_mark;
                }
                row = replace(row, "\\{depth_marks\\}", chevrons.str());
            }
#endif
            row = replace(row, "\\{level_fmt\\}", _level_fmt.at(stage).str());
            row = replace(row, "\\{filehash_fmt\\}", fmt::hash(file, _filehash_fmts).str() );
            row = replace(row, "\\{funchash_fmt\\}", fmt::hash(func, _funchash_fmts).str() );

#if CLUTCHLOG_HAVE_UNIX_SYSIOCTL
            // hfill is replaced last to allow for correct line width estimation.
            const std::string raw_row = replace(row, "(\\x9B|\\x1B\\[)[0-?]*[ -\\/]*[@-~]", "");
            const std::string hfill_tag = "{hfill}";
            const size_t hfill_pos = row.find(hfill_tag);
            const size_t raw_hfill_pos = raw_row.find(hfill_tag);
            const size_t nb_columns = std::max(std::min((size_t)_nb_columns, _hfill_max), _hfill_min);
            if(hfill_pos != std::string::npos) {
                assert(raw_hfill_pos != std::string::npos);
                if(nb_columns > 0) {
                    const size_t  left_len = raw_hfill_pos;
                    const size_t right_len = raw_row.size() - raw_hfill_pos - hfill_tag.size();
                    if(right_len+left_len > nb_columns) {
                        // The right part would go over the terminal width: add a new row.
                        if(right_len < nb_columns) {
                            // There is room for the right part on a new line.
                            const std::string hfill(std::max((size_t)0, nb_columns-right_len), _hfill_char);
                            const std::string hfill_styled = _hfill_fmt(hfill);
                            row = replace(row, "\\{hfill\\}", "\n"+hfill_styled);
                        } else {
                            // Right part still goes over columns: let it go.
                            const std::string hfill(1, _hfill_char);
                            const std::string hfill_styled = _hfill_fmt(hfill);
                            row = replace(row, "\\{hfill\\}", "\n"+hfill_styled);
                        }
                    } else {
                        // There is some space in between left and right parts.
                        const std::string hfill(std::max((size_t)0, nb_columns - (right_len+left_len)), _hfill_char);
                        const std::string hfill_styled = _hfill_fmt(hfill);
                        row = replace(row, "\\{hfill\\}", hfill_styled);
                    }
                } else {
                    // We don't know the terminal width.
                    const std::string hfill(1, _hfill_char);
                    const std::string hfill_styled = _hfill_fmt(hfill);
                    row = replace(row, "\\{hfill\\}", hfill_styled);
                }
            }
#else
            // We cannot know the terminal width.
            const std::string hfill(1, _hfill_char);
            const std::string hfill_styled = _hfill_fmt(hfill);
            row = replace(row, "\\{hfill\\}", hfill_styled);
#endif
            return _level_fmt.at(stage)(row);
        }

        //! Print a log message IF the location matches the given one.
        void log(
                const level& stage,
                const std::string& what,
                const std::string& file, const std::string& func, const size_t line,
                const size_t depth_delta = 0
            ) const
        {
            scope_t scope = locate(stage, file, func, line);

            if(scope.matches) {
#if CLUTCHLOG_HAVE_UNIX_SYSINFO == 1
                *_out << format(_format_log, what, basename(getenv("_")),
                                stage, file, func,
                                line, scope.depth + depth_delta );
#else
                *_out << format(_format_log, what,
                                stage, file, func,
                                line );
#endif
                _out->flush();
            } // if scopes.matches
        }

        //! Dump a serializable container after a comment line with log information.
        template<class In>
        void dump(
                const level& stage,
                const In container_begin, const In container_end,
                const std::string& file, const std::string& func, const size_t line,
                const std::string& filename_template = "dump_{n}.dat",
                const std::string sep = dump_default_sep
            ) const
        {
            scope_t scope = locate(stage, file, func, line);

            if(scope.matches) {
                const std::string tag = "\\{n\\}";
                const std::regex re(tag);
                std::string outfile = "";

                // If the file name template has the {n} tag.
                if(std::regex_search(filename_template, re)) {
                    // Increment n until a free one is found.
                    size_t n = 0;
                    do {
                        outfile = replace(filename_template, tag, n);
                        n++;
                    } while( fs::exists( outfile ) );

                } else {
                    // Use the parameter as is.
                    outfile = filename_template;
                }

                std::ofstream fd(outfile);

                if(_format_dump.size() > 0) {
#if CLUTCHLOG_HAVE_UNIX_SYSINFO == 1
                    fd << format(_format_dump, "", basename(getenv("_")),
                            stage, file, func,
                            line, scope.depth );
#else
                    fd << format(_format_dump, "",
                            stage, file, func,
                            line );
#endif
                    fd << sep; // sep after comment line.
                }

                std::copy(container_begin, container_end,
                    std::ostream_iterator<typename In::value_type>(fd, sep.c_str()));

                fd.close();
            } // if scopes.matches
        }

        /** @} */
};

/** @} */

#else // not WITH_CLUTCHLOG


/**********************************************************************
 * Fake implementation
 **********************************************************************/

// Equivalent class with empty methods, will be optimized out
// while allowing to actually have calls implemented without WITH_CLUTCHLOG guards.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
class clutchlog
{
    public:
        static clutchlog& logger() {}
        enum level {critical=0, error=1, warning=2, progress=3, note=4, info=5, debug=6, xdebug=7};
        enum filename {path, base, dir, dirbase, stem, dirstem};
        class fmt {
            public:
                enum class ansi { colors_16, colors_256,  colors_16M} mode;
                enum class typo { reset, bold, underline, inverse, none} style;
                enum class fg { black, red, green, yellow, blue, magenta, cyan, white, bright_black, bright_red, bright_green, bright_yellow, bright_blue, bright_magenta, bright_cyan, bright_white, none} fore;
                enum class bg { black, red, green, yellow, blue, magenta, cyan, white, bright_black, bright_red, bright_green, bright_yellow, bright_blue, bright_magenta, bright_cyan, bright_white, none } back;
            protected:
                friend std::ostream& operator<<(std::ostream&, const std::tuple<fg,bg,typo>&) {}
                friend std::ostream& operator<<(std::ostream&, const typo&) {}
            protected:
                struct color {
                    ansi mode;
                    enum class ground { fore, back } type;
                    color(ansi a, ground g) : mode(a), type(g) {}
                    virtual bool is_set() const = 0;
                    virtual std::ostream& print_on( std::ostream&) const = 0;
                    friend std::ostream& operator<<(std::ostream&, const color&) {}
                };
                struct color_256 : public color {
                    short index;
                    color_256(ground t) : color(ansi::colors_256, t), index(-1) {}
                    color_256(ground t, const short i) : color(ansi::colors_256, t), index(i) {}
                    bool is_set() const {}
                    std::ostream& print_on( std::ostream&) const {}
                };
                struct fg_256 : public color_256 {
                    fg_256() : color_256(ground::fore) {}
                    fg_256(const short f) : color_256(ground::fore, f) {}
                    fg_256(const fg&) : color_256(ground::fore, -1) {}
                } fore_256;
                struct bg_256 : public color_256 {
                    bg_256() : color_256(ground::back) {}
                    bg_256(const short b) : color_256(ground::back, b) {}
                    bg_256(const bg&) : color_256(ground::back, -1) {}
                } back_256;
                struct color_16M : public color {
                    short red, green, blue;
                    color_16M(ground t) : color(ansi::colors_16M, t), red(-1), green(-1), blue(-1) {}
                    color_16M(ground t, short r, short g, short b) : color(ansi::colors_16M, t), red(r), green(g), blue(b) {}
                    color_16M(ground t, const std::string&) : color(ansi::colors_16M, t) {}
                    bool is_set() const {return red > -1 and green > -1 and blue > -1;}
                    std::ostream& print_on( std::ostream&) const {}
                };
                struct fg_16M : public color_16M {
                    fg_16M() : color_16M(ground::fore) {}
                    fg_16M(short r, short g, short b) : color_16M(ground::fore, r,g,b) {}
                    fg_16M(const std::string& srgb) : color_16M(ground::fore, srgb) {}
                    fg_16M(const fg&) : color_16M(ground::fore, -1,-1,-1) {}
                } fore_16M;
                struct bg_16M : public color_16M {
                    bg_16M() : color_16M(ground::back) {}
                    bg_16M(short r, short g, short b) : color_16M(ground::back, r,g,b) {}
                    bg_16M(const std::string& srgb) : color_16M(ground::back, srgb) {}
                    bg_16M(const bg&) : color_16M(ground::back, -1,-1,-1) {}
                } back_16M;
            public:
                fmt() : mode(ansi::colors_16), style(typo::none), fore(fg::none), back(bg::none) {}
                fmt(  fg f,   bg b = bg::none, typo s = typo::none) : mode(ansi::colors_16), style(s), fore(f), back(b) {}
                fmt(  fg f, typo s           ,   bg b =   bg::none) : mode(ansi::colors_16), style(s), fore(f), back(b) {}
                fmt(  bg b,   fg f = fg::none, typo s = typo::none) : mode(ansi::colors_16), style(s), fore(f), back(b) {}
                fmt(  bg b, typo s           ,   fg f =   fg::none) : mode(ansi::colors_16), style(s), fore(f), back(b) {}
                fmt(typo s,   fg f = fg::none,   bg b =   bg::none) : mode(ansi::colors_16), style(s), fore(f), back(b) {}
                fmt(typo s,   bg b           ,   fg f =   fg::none) : mode(ansi::colors_16), style(s), fore(f), back(b) {}
                fmt(fg_256 f, bg_256 b, typo s = typo::none) : mode(ansi::colors_256), style(s), fore_256(f),        back_256(b) {}
                fmt(fg_256 f, typo s = typo::none)           : mode(ansi::colors_256), style(s), fore_256(f),        back_256(bg::none) {}
                fmt(fg, bg_256 b, typo s = typo::none)       : mode(ansi::colors_256), style(s), fore_256(fg::none), back_256(b) {}
                fmt(const short fr, const short fg, const short fb,
                    const short gr, const short gg, const short gb,
                    typo s = typo::none)
                    : mode(ansi::colors_16M), style(s), fore_16M(fr,fg,fb), back_16M(gr,gg,gb) {}
                fmt(fg,
                    const short gr, const short gg, const short gb,
                    typo s = typo::none)
                    : mode(ansi::colors_16M), style(s), fore_16M(fg::none), back_16M(gr,gg,gb) {}
                fmt(const short fr, const short fg, const short fb,
                    bg, typo s = typo::none)
                    : mode(ansi::colors_16M), style(s), fore_16M(fr,fg,fb), back_16M(bg::none) {}
                fmt(const short fr, const short fg, const short fb,
                    typo s = typo::none)
                    : mode(ansi::colors_16M), style(s), fore_16M(fr,fg,fb), back_16M(bg::none) {}

                fmt(const std::string& f, const std::string& b, typo s = typo::none)
                    : mode(ansi::colors_16M), style(s), fore_16M(f), back_16M(b) {}
                fmt(fg, const std::string& b, typo s = typo::none)
                    : mode(ansi::colors_16M), style(s), fore_16M(fg::none), back_16M(b) {}
                fmt(const std::string& f, bg, typo s = typo::none)
                    : mode(ansi::colors_16M), style(s), fore_16M(f), back_16M(bg::none) {}
                fmt(const std::string& f, typo s = typo::none)
                    : mode(ansi::colors_16M), style(s), fore_16M(f), back_16M(bg::none) {}
            protected:
                std::ostream& print_on( std::ostream&) const {}
            public:
                friend std::ostream& operator<<(std::ostream&, const fmt&) {}
                std::string operator()( const std::string&) const {}
                std::string str() const {}
                static fmt hash( const std::string&, const std::vector<fmt>) {}
        };
    public:
        clutchlog(clutchlog const&)      = delete;
        void operator=(clutchlog const&) = delete;
    private:
        clutchlog() {}
    protected:
        struct scope_t {};
        scope_t locate(
                const level&,
                const std::string&,
                const std::string&,
                const size_t
            ) const
        {}
    public:
        void format(const std::string&) {}
        std::string format() const {}

        void format_comment(const std::string&) {}
        std::string format_comment() const {}

        void out(std::ostream&) {}
        std::ostream& out() {}

#if CLUTCHLOG_HAVE_UNIX_SYSINFO == 1
        void depth(size_t) {}
        size_t depth() const {}

        void depth_mark(const std::string) {}
        std::string depth_mark() const {}
        void strip_calls(const size_t) {}
        size_t strip_calls() const {}
#endif
#if CLUTCHLOG_HAVE_UNIX_SYSIOCTL == 1
        void hfill_mark(const char) {}
        char hfill_mark() const {}
        void hfill_fmt(fmt) {}
        fmt hfill_fmt() const {}
        void hfill_min(const size_t) {}
        size_t hfill_min() {}
        void hfill_max(const size_t) {}
        size_t hfill_max() {}
#endif
        void filehash_styles(std::vector<fmt> ) {}
        void funchash_styles(std::vector<fmt> ) {}
        void depth_styles(std::vector<fmt>) {}

        void  threshold(level) {}
        void  threshold(const std::string&) {}
        level threshold() const {}
        const std::map<std::string,level> levels() const {}
        level level_of(const std::string) {}

        void file(std::string) {}
        void func(std::string) {}
        void line(std::string) {}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
        void location(
                const std::string&,
                const std::string& in_function=".*",
                const std::string& in_line=".*"
            )
        {}
#pragma GCC diagnostic pop
        template<class ... FMT>
        void style(level, FMT...) {}
        void style(level, fmt) {}
        fmt style(level) const {}
        void filename(filename) {}
    public:
        std::string replace(
                const std::string&,
                const std::string&,
                const std::string&
            ) const
        {}

        std::string replace(
                const std::string&,
                const std::string&,
                const size_t
            ) const
        {}

        std::string format(
                std::string,
                const std::string&,
#if CLUTCHLOG_HAVE_UNIX_SYSINFO == 1
                const std::string&,
#endif
                const level&,
                const std::string&,
                const std::string&,
                const size_t
#if CLUTCHLOG_HAVE_UNIX_SYSINFO == 1
                ,
                const size_t
#endif
            ) const
        {}

        void log(
                const level&,
                const std::string&,
                const std::string&, const std::string&, size_t
            ) const
        {}

        template<class In>
        void dump(
                const level&,
                const In, const In,
                const std::string&, const std::string&, size_t,
                const std::string&,
                const std::string
            ) const
        {}
};
#pragma GCC diagnostic pop
#endif // WITH_CLUTCHLOG

#endif // CLUTCHLOG_H
