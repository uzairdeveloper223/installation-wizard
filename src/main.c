/**
 * This code is responsible for the main entry point of the installation
 * wizard. It initializes the UI, runs through each installation step, and
 * executes the installation process.
 */

#include "all.h"

static const char *libraries[] = {
    "libncurses.so.6"
};

static const char *commands[] = {
    // Partitioning.
    "parted",
    "mkfs.ext4",
    "mkfs.vfat",
    "mkswap",
    "mount",
    "umount",
    "swapon",
    "swapoff",
    "mkdir",
    // Rootfs extraction.
    "tar",
    // Locale configuration.
    "sed"
};

int main(int argc, char *argv[])
{
    Store *store = get_store();

    // Ensure that the required libraries are available.
    const int library_count = sizeof(libraries) / sizeof(libraries[0]);
    for (int i = 0; i < library_count; i++)
    {
        if (!is_library_available(libraries[i]))
        {
            fprintf(stderr, "Missing library \"%s\".\n", libraries[i]);
            exit(EXIT_FAILURE);
        }
    }

    // Ensure that the required commands are available.
    const int command_count = sizeof(commands) / sizeof(commands[0]);
    for (int i = 0; i < command_count; i++)
    {
        if (!is_command_available(commands[i]))
        {
            fprintf(stderr, "Missing command \"%s\".\n", commands[i]);
            exit(EXIT_FAILURE);
        }
    }

    // Parse command-line arguments.
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--dry") == 0)
        {
            store->dry_run = 1;
        }
    }

    // Initialize ncurses UI.
    initialize_ui();

    // Create the centered modal window for wizard content.
    WINDOW *modal = create_modal("Installation Wizard");

    // A loop that runs throughout the entire wizard process and waits for user
    // input at each step, allowing back-and-forth navigation between steps.
    int step = 1;
    while (step <= 5)
    {
        int result = 0;

        switch (step)
        {
            case 1:
                result = run_locale_step(modal);
                break;
            case 2:
                result = run_user_step(modal);
                break;
            case 3:
                result = run_disk_step(modal);
                break;
            case 4:
                result = run_partition_step(modal);
                break;
            case 5:
                result = run_confirmation_step(modal);
                break;
        }

        if (result)
        {
            step++;
        }
        else if (step > 1)
        {
            step--;
        }
        // If step == 1 and result == 0, stay on step 1.
    }

    // Run installation using settings from global state.
    int result = run_install(ncurses_install_progress, modal);

    // Wait for final input before exiting.
    await_step_input(modal);

    // Cleanup ncurses resources before exit.
    destroy_modal(modal);
    cleanup_ui();

    return result;
}
