#include <stdint.h>

typedef struct {
    int width;
    int height;
    int width_in_pixels;
    int height_in_pixels;
} TerminalSize;

// position of a terminal character
typedef struct {
    int x;
    int y;
} TerminalCharPos;

// must be called before rendering anything (before any stdout write!)
void init_renderer(void);

// must be called at the end of the program
void shutdown_renderer(void);

// must be called before every frame
void begin_frame(void);

// must be called after every frame
void end_frame(void);

// Writes a cell using an ANSI 24-bit foreground color.
void put_rgb_at(TerminalCharPos p, uint8_t red, uint8_t green, uint8_t blue);
void print_at(TerminalCharPos p, const char* text);
void put_char_at(TerminalCharPos p, char ch);
void clear_frame(void);

TerminalSize get_terminal_size(void);
