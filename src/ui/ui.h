#pragma once
#include "../all.h"

// Indexes for custom colors and color pairs.
#define CUSTOM_COLOR_DARK_GRAY     8
#define CUSTOM_COLOR_ROW_ODD_BG    9
#define CUSTOM_COLOR_HEADER_BG     10
#define CUSTOM_COLOR_ROW_EVEN_BG   11
#define CUSTOM_COLOR_BLUE          12
#define CUSTOM_COLOR_ORANGE        13
#define CUSTOM_COLOR_RED           15
#define CUSTOM_COLOR_PAIR_MAIN         1
#define CUSTOM_COLOR_PAIR_ROW_ODD      2
#define CUSTOM_COLOR_PAIR_DIM          3
#define CUSTOM_COLOR_PAIR_HEADER       4
#define CUSTOM_COLOR_PAIR_ROW_EVEN     5
#define CUSTOM_COLOR_PAIR_SELECTED     6
#define CUSTOM_COLOR_PAIR_NOTE_BG      7
#define CUSTOM_COLOR_PAIR_NOTE_TEXT    8
#define CUSTOM_COLOR_PAIR_INFO_NOTE    9
#define CUSTOM_COLOR_PAIR_WARNING_NOTE 10
#define CUSTOM_COLOR_PAIR_ERROR_NOTE   11

/**
 * Initializes the ncurses library and configures color pairs.
 */
void init_ui(void);

/**
 * Cleans up ncurses and restores terminal state.
 */
void cleanup_ui(void);
