/**
 * This code is responsible for rendering installation progress
 * and handling the log viewer toggle functionality.
 */

#include "../../all.h"

static const char *get_install_step_name(InstallStep step)
{
    switch (step)
    {
        case STEP_PARTITIONS: return "Partitioning";
        case STEP_ROOTFS:     return "Extracting system files";
        case STEP_BOOTLOADER: return "Installing bootloader";
        case STEP_LOCALE:     return "Configuring locale";
        case STEP_USERS:      return "Configuring users";
        default:              return "Processing";
    }
}

static int logs_visible = 0;
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
        case STEP_USERS:      return 8;
        default:              return 4;
    }
}

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
    mvwprintw(modal, 11, 3, "Press Enter to reboot...");
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
            mvwprintw(modal, 10, 3, "Installation complete!");
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
