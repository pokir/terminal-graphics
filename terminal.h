typedef struct {
    int width;
    int height;
    int width_in_pixels;
    int height_in_pixels;
} TerminalSize;

void clear_screen(void);
void hide_cursor(void);
void show_cursor(void);
void print_at(int x, int y, const char* text);

TerminalSize get_terminal_size(void);
