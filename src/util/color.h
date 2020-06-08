#ifdef COLOR_DISABLE
#define RESET

#define BLACK
#define RED
#define GREEN
#define YELLOW
#define BLUE
#define MAGENTA
#define CYAN
#define WHITE

#define BOLDBLACK
#define BOLDRED
#define BOLDGREEN
#define BOLDYELLOW
#define BOLDBLUE
#define BOLDMAGENTA
#define BOLDCYAN
#define BOLDWHITE

#define CLEAR

#else
#define RESET       "\033[0m"

#define BLACK       "\033[30m"
#define RED         "\033[31m"
#define GREEN       "\033[32m"
#define YELLOW      "\033[33m"
#define BLUE        "\033[34m"
#define MAGENTA     "\033[35m"
#define CYAN        "\033[36m"
#define WHITE       "\033[37m"

#define BOLDBLACK   "\033[1m\033[30m"
#define BOLDRED     "\033[1m\033[31m"
#define BOLDGREEN   "\033[1m\033[32m"
#define BOLDYELLOW  "\033[1m\033[33m"
#define BOLDBLUE    "\033[1m\033[34m"
#define BOLDMAGENTA "\033[1m\033[35m"
#define BOLDCYAN    "\033[1m\033[36m"
#define BOLDWHITE   "\033[1m\033[37m"

// Clears current line. Return to the start of this line with '\r'.
#define CLEAR "\033[2K"
#endif

#if 0

///# Color

//// Macros
// All output can be colored with different color-macros.
LOG(RED "R" GREEN "G" BLUE "B" RESET);
// Make sure to always reset to normal output.

//// CLEAR
// The <code>CLEAR</code>-macro clears to the end of the current line.
// This can be used with '\r' to create cli-style progress-output on a
// single line.
printf("Loading asset 1...");
printf("\r" CLEAR "\rLoading asset 2...");
printf("\r" CLEAR "\rLoading asset 3...");
printf("\r" CLEAR "Done loading assets!\n");


//// COLOR_DISABLE
// This flag disables all color output if you want to compile and run in
// something that doesn't support color text output.
#define COLOR_DISABLE

#endif
