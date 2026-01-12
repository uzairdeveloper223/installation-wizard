#pragma once
#include "../all.h"

/** Index for custom dark gray color. */
#define CUSTOM_COLOR_DARK_GRAY     8

/** Index for odd row background color. */
#define CUSTOM_COLOR_ROW_ODD_BG    9

/** Index for header background color. */
#define CUSTOM_COLOR_HEADER_BG     10

/** Index for even row background color. */
#define CUSTOM_COLOR_ROW_EVEN_BG   11

/** Index for custom blue color. */
#define CUSTOM_COLOR_BLUE          12

/** Index for custom orange color. */
#define CUSTOM_COLOR_ORANGE        13

/** Index for custom red color. */
#define CUSTOM_COLOR_RED           15

/** Index for custom purple color. */
#define CUSTOM_COLOR_PURPLE        14

/** Color pair for main UI elements. */
#define CUSTOM_COLOR_PAIR_MAIN         1

/** Color pair for odd table rows. */
#define CUSTOM_COLOR_PAIR_ROW_ODD      2

/** Color pair for dimmed text. */
#define CUSTOM_COLOR_PAIR_DIM          3

/** Color pair for table headers. */
#define CUSTOM_COLOR_PAIR_HEADER       4

/** Color pair for even table rows. */
#define CUSTOM_COLOR_PAIR_ROW_EVEN     5

/** Color pair for selected items. */
#define CUSTOM_COLOR_PAIR_SELECTED     6

/** Color pair for note box background. */
#define CUSTOM_COLOR_PAIR_NOTE_BG      7

/** Color pair for note box text. */
#define CUSTOM_COLOR_PAIR_NOTE_TEXT    8

/** Color pair for info note accent. */
#define CUSTOM_COLOR_PAIR_INFO_NOTE    9

/** Color pair for warning note accent. */
#define CUSTOM_COLOR_PAIR_WARNING_NOTE 10

/** Color pair for error note accent. */
#define CUSTOM_COLOR_PAIR_ERROR_NOTE   11

/** Color pair for primary user marker on odd row. */
#define CUSTOM_COLOR_PAIR_PRIMARY_ODD  12

/** Color pair for primary user marker on even row. */
#define CUSTOM_COLOR_PAIR_PRIMARY_EVEN 13

/**
 * Returns non-zero if terminal supports extended colors (16+).
 */
int ui_has_extended_colors(void);

/**
 * Initializes the ncurses library and configures color pairs.
 */
void initialize_ui(void);

/**
 * Cleans up ncurses and restores terminal state.
 */
void cleanup_ui(void);
