#pragma once

/** Initializes the ncurses library and configures color pairs. */
void initialize_ui(void);

/** Cleans up ncurses and restores terminal state. */
void cleanup_ui(void);
