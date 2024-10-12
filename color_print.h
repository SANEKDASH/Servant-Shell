#ifndef COLOR_PRINT_HEADER
#define COLOR_PRINT_HEADER

typedef enum
{
    kBlack   = 30,
    kRed     = 31,
    kGreen   = 32,
    kYellow  = 33,
    kBlue    = 34,
    kMagenta = 35,
    kCyan    = 36,
    kWhite   = 37
} color_code_t;

int color_printf(color_code_t color_code, const char *fmt, ...);

#endif // COLOR_PRINT_HEADER
