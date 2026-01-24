#pragma once
#include "../all.h"

/** Maximum number of options in a selection list. */
#define STEPS_MAX_OPTIONS MAX_OPTIONS

/** A type representing a step execution function. */
typedef int (*StepFunction)(WINDOW *modal, int step_index);

/** A type representing a wizard step in the registry. */
typedef struct
{
    const char *display_name;
    StepFunction run;
} WizardStep;

/** The number of wizard steps. */
#define WIZARD_STEP_COUNT 5

/** The registry of all wizard steps. */
extern const WizardStep wizard_steps[WIZARD_STEP_COUNT];

/** A type representing a single installation step (legacy). */
typedef struct
{
    const char *name;
    const char **content;
    int content_lines;
    const char *footer;
} Step;

/** StepOption is an alias for StoreOption (same structure). */
typedef StoreOption StepOption;

/**
 * Displays a step in the modal window.
 *
 * @param modal The modal window to draw in.
 * @param step_number The current step number (1-indexed).
 * @param step The step data to display.
 */
void display_step(WINDOW *modal, int step_number, const Step *step);

/**
 * Waits for user input to proceed or quit.
 *
 * @param modal The modal window to capture input from.
 *
 * @return - `1` - Indicates user pressed Enter to proceed.
 * @return - `0` - Indicates user pressed 'q' to quit.
 */
int await_step_input(WINDOW *modal);

/**
 * Renders a selection list with scrolling support.
 *
 * @param modal The modal window to draw in.
 * @param options Array of options to display.
 * @param count Number of options in the array.
 * @param selected Index of the currently selected option.
 * @param start_y Y position to start rendering options.
 * @param scroll_offset Index of the first visible option.
 * @param max_visible Maximum number of options to display at once.
 */
void render_step_options(
    WINDOW *modal, const StepOption *options, int count, int selected, int start_y,
    int scroll_offset, int max_visible
);

/**
 * Runs an interactive selection step, returning the selected index.
 *
 * @param modal The modal window to draw in.
 * @param title The step title to display.
 * @param step_number The current step number (1-indexed).
 * @param description Text shown above the options.
 * @param options Array of options to choose from.
 * @param count Number of options in the array.
 * @param out_selected Pointer to store selected index, also used as initial.
 * @param allow_back Whether to allow the back option (Escape key).
 *
 * @return - `1` - Indicates user confirmed selection.
 * @return - `0` - Indicates user went back.
 */
int run_selection_step(
    WINDOW *modal, const char *title, int step_number, const char *description,
    const StepOption *options, int count, int *out_selected, int allow_back
);
