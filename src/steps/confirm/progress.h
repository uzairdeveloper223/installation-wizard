#pragma once
#include "../../all.h"

/**
 * Handles installation progress events in an ncurses modal window.
 *
 * @param event The progress event type.
 * @param phase_index The installation phase index (0-based).
 * @param error_code The error code for failure events.
 * @param context A valid WINDOW* pointer.
 */
void handle_install_progress(
    InstallEvent event,
    int phase_index,
    int error_code,
    void *context
);

/** Sets the visibility of the installation logs viewer. */
void set_logs_visible(int visible);

/** Gets the current visibility state of the installation logs viewer. */
int get_logs_visible(void);

/** Toggles the visibility of the installation logs viewer. */
void toggle_logs_visible(void);

/** Sets the modal window used for tick updates during command execution. */
void set_install_tick_modal(void *modal);

/**
 * Periodic tick handler for installation progress.
 * Handles input checking and animation updates.
 */
void tick_install(void);
