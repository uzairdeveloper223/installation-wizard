/**
 * This code is responsible for rendering installation progress
 * and handling the log viewer toggle functionality.
 */

#include "../../all.h"

/** A type representing the status of an installation step. */
typedef enum {
    PROGRESS_PENDING,
    PROGRESS_ACTIVE,
    PROGRESS_OK,
    PROGRESS_FAILED
} ProgressStatus;

/** The current visibility state of the log viewer. */
static int logs_visible = 0;

/** The modal window used for tick updates during command execution. */
static WINDOW *tick_modal = NULL;

/** The status of each installation phase. */
static ProgressStatus phase_status[INSTALL_PHASE_COUNT];

/** The error codes for each installation phase. */
static int phase_error_codes[INSTALL_PHASE_COUNT];

/** The current animation frame (0-2 for dot count). */
static int animation_frame = 0;

/** Tick counter for animation timing. */
static int animation_tick = 0;

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

void set_install_tick_modal(void *modal)
{
    tick_modal = (WINDOW *)modal;
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

static void render_all_phases(WINDOW *modal)
{
    const int col1 = 3;
    const int col2 = MODAL_WIDTH / 2;

    for (int i = 0; i < INSTALL_PHASE_COUNT; i++)
    {
        int row = 4 + (i < 5 ? i : i - 5);
        int col = (i < 5) ? col1 : col2;
        ProgressStatus status = phase_status[i];
        const char *name = install_phases[i].display_name;

        // Clear previous content at this position.
        mvwprintw(modal, row, col, "                       ");

        // Render step number and name with status suffix.
        wattron(modal, COLOR_PAIR(COLOR_PAIR_MAIN));
        switch (status)
        {
            case PROGRESS_PENDING:
                mvwprintw(modal, row, col, "%d. %s", i + 1, name);
                break;
            case PROGRESS_ACTIVE:
            {
                const char *dots[] = {".", "..", "..."};
                mvwprintw(modal, row, col, "%d. %s%s", i + 1, name, dots[animation_frame]);
                break;
            }
            case PROGRESS_OK:
                mvwprintw(modal, row, col, "%d. %s [OK]", i + 1, name);
                break;
            case PROGRESS_FAILED:
                mvwprintw(modal, row, col, "%d. %s [ERR %d]", i + 1, name, phase_error_codes[i]);
                break;
        }
        wattroff(modal, COLOR_PAIR(COLOR_PAIR_MAIN));
    }

    wrefresh(modal);
}

static void update_animation(WINDOW *modal)
{
    // Update animation every 6 ticks (300ms at 50ms intervals).
    animation_tick++;
    if (animation_tick < 6)
    {
        return;
    }
    animation_tick = 0;

    // Cycle through frames 0, 1, 2.
    animation_frame = (animation_frame + 1) % 3;

    // Re-render phases to show updated animation.
    render_all_phases(modal);

    // Refresh logs if visible.
    if (logs_visible)
    {
        render_background_logs(modal);
    }
}

void tick_install(void)
{
    if (!tick_modal) return;

    // Handle input (log toggle).
    check_toggle_input(tick_modal);

    // Update animation.
    update_animation(tick_modal);
}

static void render_install_start(WINDOW *modal)
{
    clear_modal(modal);
    mvwprintw(modal, 2, 3, "Installing LimeOS...");

    // Reset all phase statuses to pending.
    for (int i = 0; i < INSTALL_PHASE_COUNT; i++)
    {
        phase_status[i] = PROGRESS_PENDING;
        phase_error_codes[i] = 0;
    }

    // Render initial state.
    render_all_phases(modal);

    const char *footer[] = {"[~] Show logs", NULL};
    render_footer(modal, footer);
}

static void await_reboot_with_logs(WINDOW *modal)
{
    // Show success message.
    mvwprintw(modal, MODAL_HEIGHT - 4, 3, "Success! LimeOS has been installed.");

    // Update footer to show reboot option.
    const char *footer[] = {"[~] Show logs", "[Enter] Reboot", NULL};
    render_footer(modal, footer);
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

void handle_install_progress(
    InstallEvent event, int phase_index,
    int error_code, void *context
)
{
    WINDOW *modal = (WINDOW *)context;
    if (!modal) return;

    // Decide what to print / display based on the event type.
    switch (event)
    {
        case INSTALL_START:
            render_install_start(modal);
            break;

        case INSTALL_STEP_BEGIN:
            phase_status[phase_index] = PROGRESS_ACTIVE;
            render_all_phases(modal);
            break;

        case INSTALL_STEP_OK:
            phase_status[phase_index] = PROGRESS_OK;
            render_all_phases(modal);
            break;

        case INSTALL_STEP_FAIL:
            phase_status[phase_index] = PROGRESS_FAILED;
            phase_error_codes[phase_index] = error_code;
            render_all_phases(modal);
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
