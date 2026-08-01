user should only implement logic.c (not main.c!)

~~engine uses screen~~: NO! engine does not use screen (engine is just math)
screen uses terminal

TODO: make it so get_terminal_size returns a reference to a pre-calculated size, and recalculate it at the start of each frame in main.c
TODO: make it so terminal drawing (print_at) remembers which characters changed, and only reprint those characters
