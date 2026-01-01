#pragma once
#include "../all.h"

/**
 * Semantic color pair constants for consistent UI styling.
 */
#define UI_COLOR_MAIN       1
#define UI_COLOR_ROW_ODD    2
#define UI_COLOR_DIM        3
#define UI_COLOR_HEADER     4
#define UI_COLOR_ROW_EVEN   5
#define UI_COLOR_SELECTED   6
#define UI_COLOR_WARNING    7

/**
 * Initializes the ncurses library and configures color pairs.
 */
void init_ui(void);

/**
 * Cleans up ncurses and restores terminal state.
 */
void cleanup_ui(void);

/**
 * Renders footer items with darker background, separated by spaces.
 *
 * @param modal The modal window to render the footer in.
 * @param items NULL-terminated array of strings.
 */
void render_footer(WINDOW *modal, const char **items);
