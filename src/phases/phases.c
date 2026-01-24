/**
 * This code is responsible for orchestrating the full installation process
 * by invoking partitioning, rootfs extraction, bootloader setup, and locale
 * configuration in sequence.
 */

#include "../all.h"

/** The registry of all installation phases. */
const Phase install_phases[INSTALL_PHASE_COUNT] = {
    { "Partitions",   "Partitioning",            create_partitions   },
    { "System files", "Extracting system files", extract_rootfs      },
    { "Fstab",        "Generating fstab",        generate_fstab      },
    { "Bootloader",   "Installing bootloader",   setup_bootloader    },
    { "Locale",       "Configuring locale",      configure_locale    },
    { "Users",        "Configuring users",       configure_users     },
    { "Components",   "Installing components",   install_components  },
};

#define NOTIFY(event, phase_index, err) \
    if (progress_cb) progress_cb((event), (phase_index), (err), context)

int run_install(install_progress_cb progress_cb, void *context)
{
    // Initialize install log file.
    init_install_log();

    // Enable periodic tick updates during command execution.
    set_install_tick_modal(context);
    set_command_tick_callback(tick_install);

    NOTIFY(INSTALL_START, 0, 0);

    // Execute each installation phase in sequence.
    for (int i = 0; i < INSTALL_PHASE_COUNT; i++)
    {
        const Phase *phase = &install_phases[i];

        // Write phase header to install log.
        write_install_log_header(phase->log_header);
        write_install_log("Starting phase %d/%d: %s", i + 1, INSTALL_PHASE_COUNT, phase->display_name);

        // Notify phase start.
        NOTIFY(INSTALL_STEP_BEGIN, i, 0);

        // Execute phase.
        int result = phase->execute();
        if (result != 0)
        {
            write_install_log("Phase failed with error code: %d", result);
            NOTIFY(INSTALL_STEP_FAIL, i, result);
            cleanup_mounts();
            return -(i + 1);
        }

        // Notify phase success.
        write_install_log("Phase completed successfully");
        NOTIFY(INSTALL_STEP_OK, i, 0);
    }

    // Clean up mounts.
    cleanup_mounts();

    write_install_log("Installation completed successfully");
    NOTIFY(INSTALL_AWAIT_REBOOT, 0, 0);

    // Disable tick updates before reboot.
    set_command_tick_callback(NULL);

    run_command("reboot >>" CONFIG_INSTALL_LOG_PATH " 2>&1");
    close_dry_run_log();

    return 0;
}
