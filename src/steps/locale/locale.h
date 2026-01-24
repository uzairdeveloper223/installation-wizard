#pragma once
#include "../all.h"

/**
 * Populates the options array with available system locales.
 *
 * @param out_options Array to store locale options.
 * @param max_count Maximum number of options to populate.
 *
 * @return Number of options populated.
 */
int populate_locale_options(StepOption *out_options, int max_count);

/**
 * Runs the locale selection step interactively.
 *
 * @param modal The modal window to draw in.
 * @param step_index The index of this step in the wizard (0-based).
 *
 * @return - `1` - Indicates user confirmed selection.
 * @return - `0` - Indicates user went back.
 */
int run_locale_step(WINDOW *modal, int step_index);
