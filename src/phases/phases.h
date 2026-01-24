#pragma once

/** A type representing installation progress events. */
typedef enum {
    INSTALL_START,
    INSTALL_STEP_BEGIN,
    INSTALL_STEP_OK,
    INSTALL_STEP_FAIL,
    INSTALL_AWAIT_REBOOT
} InstallEvent;

/** A type representing a phase execution function. */
typedef int (*PhaseFunction)(void);

/** A type representing an installation phase. */
typedef struct {
    const char *display_name;
    const char *log_header;
    PhaseFunction execute;
} Phase;

/** The number of installation phases. */
#define INSTALL_PHASE_COUNT 7

/** The registry of all installation phases. */
extern const Phase install_phases[INSTALL_PHASE_COUNT];

/**
 * A callback function type for reporting installation progress.
 *
 * @param event The type of progress event.
 * @param phase_index The index of the installation phase (0-based).
 * @param error_code The error code for failure events.
 * @param context User-provided context data.
 */
typedef void (*install_progress_cb)(
    InstallEvent event,
    int phase_index,
    int error_code,
    void *context
);

/**
 * Runs the full installation process using settings from the global store.
 *
 * @param progress_cb Callback for progress updates (can be NULL for silent
 *                    mode).
 * @param context User data passed to callback.
 *
 * @return - `0` - Indicates success.
 * @return - Negative values indicate phase failure (-(phase_index + 1)).
 */
int run_install(install_progress_cb progress_cb, void *context);
