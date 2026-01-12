#pragma once

/**
 * Installation progress events.
 */
typedef enum {
    INSTALL_START,
    INSTALL_STEP_BEGIN,
    INSTALL_STEP_OK,
    INSTALL_STEP_FAIL,
    INSTALL_COMPLETE,
    INSTALL_AWAIT_REBOOT
} InstallEvent;

/**
 * Installation steps.
 */
typedef enum {
    STEP_PARTITIONS,
    STEP_ROOTFS,
    STEP_BOOTLOADER,
    STEP_LOCALE,
    STEP_USERS
} InstallStep;

/**
 * Progress callback function type.
 *
 * @param event The type of progress event.
 * @param step Which installation step (for STEP_* events).
 * @param error_code Error code (for INSTALL_STEP_FAIL).
 * @param context User-provided context data.
 */
typedef void (*install_progress_cb)(
    InstallEvent event,
    InstallStep step,
    int error_code,
    void *context
);

/**
 * Runs the full installation process using settings from the global store.
 *
 * @param progress_cb Callback for progress updates (can be NULL for silent 
 * mode).
 * @param context User data passed to callback.
 *
 * @return - `0` - on success.
 * @return - `-1` - if partitioning fails.
 * @return - `-2` - if rootfs extraction fails.
 * @return - `-3` - if bootloader setup fails.
 * @return - `-4` - if locale configuration fails.
 * @return - `-5` - if user configuration fails.
 */
int run_install(install_progress_cb progress_cb, void *context);
