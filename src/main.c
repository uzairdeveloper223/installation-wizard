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
    int step_index = 0;
    while (step_index < WIZARD_STEP_COUNT)
    {
        int result = wizard_steps[step_index].run(modal, step_index);

        if (result)
        {
            step_index++;
        }
        else if (step_index > 0)
        {
            step_index--;
        }
        // If step_index == 0 and result == 0, stay on first step.
    }

    // Run installation using settings from global state.
    int result = run_install(handle_install_progress, modal);

    // Clear any buffered input before waiting.
    flushinp();

    // Wait for final input before exiting.
    await_step_input(modal);

    // Cleanup ncurses resources before exit.
    destroy_modal(modal);
    cleanup_ui();

    return result;
}
