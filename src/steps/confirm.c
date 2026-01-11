/**
 * This code is responsible for displaying a confirmation summary
 * of all user selections before proceeding with installation,
 * and rendering installation progress.
 */

#include "../all.h"

/** Tracks whether installation logs viewer is visible. */
static int logs_visible = 0;

/** Modal window used for polling during command execution. */
static WINDOW *poll_modal = NULL;

void set_logs_visible(int visible)
{
    logs_visible = visible;
}

int get_logs_visible(void)
{
    return logs_visible;
}

void toggle_logs_visible(void)
{
    logs_visible = !logs_visible;
}

void set_install_poll_modal(void *modal)
{
    poll_modal = (WINDOW *)modal;
}

static void render_background_logs(WINDOW *modal)
{
    int screen_height, screen_width;
    getmaxyx(stdscr, screen_height, screen_width);

    int line_count = 0;
    char **lines = read_install_log_lines(screen_height, &line_count);

    // Clear stdscr and render log lines with dim attribute.
    werase(stdscr);
    if (lines)
    {
        wattron(stdscr, A_DIM);
        for (int i = 0; i < line_count; i++)
        {
            mvwaddnstr(stdscr, i, 0, lines[i], screen_width);
        }
        wattroff(stdscr, A_DIM);
        free_install_log_lines(lines, line_count);
    }

    // Refresh stdscr first, then touch and refresh modal to keep it on top.
    wnoutrefresh(stdscr);
    touchwin(modal);
    wnoutrefresh(modal);
    doupdate();
}

static void clear_background_logs(WINDOW *modal)
{
    werase(stdscr);
    wnoutrefresh(stdscr);
    touchwin(modal);
    wnoutrefresh(modal);
    doupdate();
}

static void check_toggle_input(WINDOW *modal)
{
    timeout(0); // Non-blocking.
    int key = getch();
    timeout(-1); // Restore blocking.

    if (key == '`')
    {
        toggle_logs_visible();
        if (logs_visible)
        {
            render_background_logs(modal);
        }
        else
        {
            clear_background_logs(modal);
        }
    }
}

void install_poll_callback(void)
{
    if (!poll_modal) return;
    check_toggle_input(poll_modal);
}

static int get_step_row(InstallStep step)
{
    switch (step)
    {
        case STEP_PARTITIONS: return 4;
        case STEP_ROOTFS:     return 5;
        case STEP_BOOTLOADER: return 6;
        case STEP_LOCALE:     return 7;
        default:              return 4;
    }
}

// =========================================================================
// Progress Display Helpers
// =========================================================================

static void render_install_start(WINDOW *modal)
{
    clear_modal(modal);
    mvwprintw(modal, 2, 3, "Installing LimeOS...");

    const char *footer[] = {"[~] Show logs", NULL};
    render_footer(modal, footer);
}

static void render_step_line(
    WINDOW *modal, int row, InstallStep step,
    const char *disk, const char *status, int error_code
)
{
    const char *step_name = get_install_step_name(step);

    if (step == STEP_PARTITIONS && disk != NULL)
    {
        if (status == NULL)
        {
            mvwprintw(modal, row, 3, "%s %s...", step_name, disk);
        }
        else if (error_code != 0)
        {
            mvwprintw(modal, row, 3, "%s %s... %s (%d)", step_name, disk, status, error_code);
        }
        else
        {
            mvwprintw(modal, row, 3, "%s %s... %s", step_name, disk, status);
        }
    }
    else
    {
        if (status == NULL)
        {
            mvwprintw(modal, row, 3, "%s...", step_name);
        }
        else if (error_code != 0)
        {
            mvwprintw(modal, row, 3, "%s... %s (%d)", step_name, status, error_code);
        }
        else
        {
            mvwprintw(modal, row, 3, "%s... %s", step_name, status);
        }
    }
}

static void await_reboot_with_logs(WINDOW *modal)
{
    mvwprintw(modal, 10, 3, "Press Enter to reboot...");
    wrefresh(modal);

    while (1)
    {
        int key = getch();
        if (key == '\n')
        {
            break;
        }
        if (key == '`')
        {
            toggle_logs_visible();
            if (logs_visible)
            {
                render_background_logs(modal);
            }
            else
            {
                clear_background_logs(modal);
            }
        }
    }
}

void ncurses_install_progress(
    InstallEvent event, InstallStep step,
    int error_code, void *context
)
{
    WINDOW *modal = (WINDOW *)context;
    if (!modal) return;

    Store *store = get_store();
    int row = get_step_row(step);

    // Decide what to print / display based on the event type.
    switch (event)
    {
        case INSTALL_START:
            render_install_start(modal);
            break;

        case INSTALL_STEP_BEGIN:
            render_step_line(modal, row, step, store->disk, NULL, 0);
            break;

        case INSTALL_STEP_OK:
            render_step_line(modal, row, step, store->disk, "OK", 0);
            break;

        case INSTALL_STEP_FAIL:
            render_step_line(modal, row, step, store->disk, "FAILED", error_code);
            break;

        case INSTALL_COMPLETE:
            mvwprintw(modal, 9, 3, "Installation complete!");
            break;

        case INSTALL_AWAIT_REBOOT:
            await_reboot_with_logs(modal);
            return;
    }

    // Check for backtick input to toggle logs.
    check_toggle_input(modal);

    // Refresh logs if visible, then refresh modal.
    if (logs_visible)
    {
        render_background_logs(modal);
    }
    else
    {
        wrefresh(modal);
    }
}

// =========================================================================
// Confirmation Step Helpers
// =========================================================================

static int has_root_partition(Store *store)
{
    for (int i = 0; i < store->partition_count; i++)
    {
        if (strcmp(store->partitions[i].mount_point, "/") == 0)
        {
            return 1;
        }
    }
    return 0;
}

static int has_duplicate_mount_points(Store *store)
{
    for (int i = 0; i < store->partition_count; i++)
    {
        if (store->partitions[i].mount_point[0] == '[') continue;
        for (int j = i + 1; j < store->partition_count; j++)
        {
            if (store->partitions[j].mount_point[0] == '[') continue;
            if (strcmp(
                store->partitions[i].mount_point,
                store->partitions[j].mount_point
            ) == 0)
            {
                return 1;
            }
        }
    }
    return 0;
}

static int has_required_boot_partition(Store *store, int is_uefi)
{
    for (int i = 0; i < store->partition_count; i++)
    {
        if (is_uefi && store->partitions[i].flag_esp)
        {
            return 1;
        }
        if (!is_uefi && store->partitions[i].flag_bios_grub)
        {
            return 1;
        }
    }
    return 0;
}

static int is_boot_partition_too_small(Store *store, int is_uefi)
{
    for (int i = 0; i < store->partition_count; i++)
    {
        if (is_uefi && store->partitions[i].flag_esp)
        {
            // ESP must be at least 512MB.
            return store->partitions[i].size_bytes < 512ULL * 1000000;
        }
        if (!is_uefi && store->partitions[i].flag_bios_grub)
        {
            // BIOS GRUB partition must be at least 2MB.
            return store->partitions[i].size_bytes < 2ULL * 1000000;
        }
    }
    return 0;
}

static void render_config_summary(WINDOW *modal, Store *store)
{
    // Display summary of selected options.
    mvwprintw(modal, 4, 3, "Ready to install LimeOS with the following settings:");
    mvwprintw(modal, 6, 3, "  Locale: %s", store->locale);
    mvwprintw(modal, 7, 3, "  Disk: %s", store->disk);

    // Display partition summary.
    unsigned long long disk_size = get_disk_size(store->disk);
    unsigned long long used = sum_partition_sizes(store->partitions, store->partition_count);
    unsigned long long free_space = (disk_size > used) ? disk_size - used : 0;
    char free_str[32];
    format_disk_size(free_space, free_str, sizeof(free_str));

    if (store->partition_count > 0)
    {
        mvwprintw(
            modal, 8, 3,
            "  Partitions: %d partitions, %s left",
            store->partition_count, free_str
        );
    }
    else
    {
        mvwprintw(modal, 8, 3, "  Partitions: (none)");
    }
}

static void render_duplicate_error(WINDOW *modal)
{
    render_error(modal, 10, 3,
        "Multiple partitions share the same mount point.\n"
        "Go back and fix the configuration."
    );
    const char *footer[] = {"[Esc] Back", NULL};
    render_footer(modal, footer);
}

static void render_no_root_error(WINDOW *modal)
{
    render_error(modal, 10, 3,
        "A root (/) partition is required.\n"
        "Go back and add one to continue."
    );
    const char *footer[] = {"[Esc] Back", NULL};
    render_footer(modal, footer);
}

static void render_no_boot_error(WINDOW *modal, int is_uefi)
{
    if (is_uefi)
    {
        render_error(modal, 10, 3,
            "UEFI boot requires an EFI System Partition.\n"
            "Add: Size=512MB, Mount=/boot, Flags=esp"
        );
    }
    else
    {
        render_error(modal, 10, 3,
            "GPT on BIOS requires a BIOS Boot Partition.\n"
            "Add: Size=8MB, Mount=none, Flags=bios_grub"
        );
    }
    const char *footer[] = {"[Esc] Back", NULL};
    render_footer(modal, footer);
}

static void render_boot_too_small_error(WINDOW *modal, int is_uefi)
{
    if (is_uefi)
    {
        render_error(modal, 10, 3,
            "EFI System Partition must be at least 512MB.\n"
            "Go back and resize it."
        );
    }
    else
    {
        render_error(modal, 10, 3,
            "BIOS GRUB partition must be at least 2MB.\n"
            "Go back and resize it."
        );
    }
    const char *footer[] = {"[Esc] Back", NULL};
    render_footer(modal, footer);
}

static void render_ready_message(WINDOW *modal, Store *store)
{
    if (store->dry_run)
    {
        render_info(modal, 10, 3,
            "Dry run mode enabled.\n"
            "No changes will be made to disk."
        );
    }
    else
    {
        char warning_text[128];
        snprintf(
            warning_text, sizeof(warning_text),
            "All data on %s will be erased!\n"
            "This action cannot be undone.", store->disk
        );
        render_warning(modal, 10, 3, warning_text);
    }
    const char *footer[] = {"[Enter] Install", "[Esc] Back", NULL};
    render_footer(modal, footer);
}

int run_confirmation_step(WINDOW *modal)
{
    Store *store = get_store();

    // Clear and draw step header.
    clear_modal(modal);
    wattron(modal, A_BOLD | COLOR_PAIR(CUSTOM_COLOR_PAIR_MAIN));
    mvwprintw(modal, 2, 3, "Step 4: Confirm Installation");
    wattroff(modal, A_BOLD);

    // Render configuration summary.
    render_config_summary(modal, store);

    // Perform validations.
    int has_root = has_root_partition(store);
    int has_duplicate = has_duplicate_mount_points(store);
    int is_uefi = (access("/sys/firmware/efi", F_OK) == 0);
    int has_boot = has_required_boot_partition(store, is_uefi);
    int boot_too_small = is_boot_partition_too_small(store, is_uefi);
    int can_install = has_root && !has_duplicate && has_boot && !boot_too_small;

    // Render the appropriate message based on validation.
    if (has_duplicate)
    {
        render_duplicate_error(modal);
    }
    else if (!has_root)
    {
        render_no_root_error(modal);
    }
    else if (!has_boot)
    {
        render_no_boot_error(modal, is_uefi);
    }
    else if (boot_too_small)
    {
        render_boot_too_small_error(modal, is_uefi);
    }
    else
    {
        render_ready_message(modal, store);
    }

    wrefresh(modal);

    // Wait for user confirmation or back.
    int key;
    while ((key = getch()) != 27 && (key != '\n' || !can_install))
    {
        // Ignore other input.
    }

    return key == '\n' && can_install;
}
