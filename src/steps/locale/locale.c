/**
 * This code is responsible for detecting available system locales
 * and presenting them for user selection.
 */

#include "../../all.h"

static int get_locale_priority(const char *locale)
{
    // Return priority 1 for en_US as the most common default.
    if (strncmp(locale, "en_US", 5) == 0)
    {
        return 1;
    }

    // Return priority 2 for other English locales.
    if (strncmp(locale, "en_", 3) == 0)
    {
        return 2;
    }

    // Return priority 3 for major European locales.
    if (strncmp(locale, "de_", 3) == 0 ||
        strncmp(locale, "fr_", 3) == 0 ||
        strncmp(locale, "es_", 3) == 0 ||
        strncmp(locale, "it_", 3) == 0 ||
        strncmp(locale, "pt_", 3) == 0 ||
        strncmp(locale, "nl_", 3) == 0)
    {
        return 3;
    }

    // Return priority 4 for all other locales.
    return 4;
}

static int compare_locales(const void *a, const void *b)
{
    const StepOption *option_a = (const StepOption *)a;
    const StepOption *option_b = (const StepOption *)b;

    int priority_a = get_locale_priority(option_a->value);
    int priority_b = get_locale_priority(option_b->value);

    // Sort by priority first.
    if (priority_a != priority_b)
    {
        return priority_a - priority_b;
    }

    // Sort alphabetically within same priority.
    return strcmp(option_a->value, option_b->value);
}

static int is_technical_locale(const char *locale)
{
    // Filter out C locale variants.
    if (strncmp(locale, "C.", 2) == 0 || strcmp(locale, "C") == 0)
    {
        return 1;
    }

    // Filter out POSIX locale.
    if (strcmp(locale, "POSIX") == 0)
    {
        return 1;
    }

    return 0;
}

int populate_locale_options(StepOption *out_options, int max_count)
{
    Store *store = get_store();

    // Return stored locales if already populated.
    if (store->locale_count >= 0)
    {
        int count = store->locale_count;
        if (count > max_count) count = max_count;
        for (int i = 0; i < count; i++)
        {
            snprintf(out_options[i].value, sizeof(out_options[i].value),
                     "%s", store->locales[i].value);
            snprintf(out_options[i].label, sizeof(out_options[i].label),
                     "%s", store->locales[i].label);
        }
        return count;
    }

    // Run locale -a to get available system locales.
    FILE *pipe = popen("locale -a 2>/dev/null", "r");
    if (pipe == NULL)
    {
        // Use fallback locale if detection fails.
        snprintf(out_options[0].value, sizeof(out_options[0].value), "en_US.UTF-8");
        snprintf(out_options[0].label, sizeof(out_options[0].label), "en_US.UTF-8 (Default)");
        store->locale_count = 1;
        snprintf(store->locales[0].value, sizeof(store->locales[0].value), "en_US.UTF-8");
        snprintf(store->locales[0].label, sizeof(store->locales[0].label), "en_US.UTF-8 (Default)");
        return 1;
    }

    // Read locales line by line and populate options array.
    int count = 0;
    int limit = max_count < MAX_OPTIONS ? max_count : MAX_OPTIONS;
    char line[256];
    while (fgets(line, sizeof(line), pipe) != NULL && count < limit)
    {
        // Remove trailing newline from locale name.
        line[strcspn(line, "\n")] = '\0';

        // Skip empty lines.
        if (strlen(line) == 0)
        {
            continue;
        }

        // Skip non-UTF-8 locales for cleaner list.
        if (strstr(line, "UTF-8") == NULL && strstr(line, "utf8") == NULL)
        {
            continue;
        }

        // Skip technical locales like C.utf8 and POSIX.
        if (is_technical_locale(line))
        {
            continue;
        }

        // Add locale to options array.
        snprintf(out_options[count].value, sizeof(out_options[count].value), "%s", line);
        snprintf(out_options[count].label, sizeof(out_options[count].label), "%s", line);
        count++;
    }

    // Close the previously opened pipe used for reading locales.
    pclose(pipe);

    // Ensure at least one fallback option exists.
    if (count == 0)
    {
        snprintf(out_options[0].value, sizeof(out_options[0].value), "en_US.UTF-8");
        snprintf(out_options[0].label, sizeof(out_options[0].label), "en_US.UTF-8 (Default)");
        store->locale_count = 1;
        snprintf(store->locales[0].value, sizeof(store->locales[0].value), "en_US.UTF-8");
        snprintf(store->locales[0].label, sizeof(store->locales[0].label), "en_US.UTF-8 (Default)");
        return 1;
    }

    // Sort locales by priority then alphabetically.
    qsort(out_options, count, sizeof(StepOption), compare_locales);

    // Store the sorted results.
    store->locale_count = count;
    for (int i = 0; i < count; i++)
    {
        snprintf(store->locales[i].value, sizeof(store->locales[i].value),
                 "%s", out_options[i].value);
        snprintf(store->locales[i].label, sizeof(store->locales[i].label),
                 "%s", out_options[i].label);
    }

    return count;
}

int run_locale_step(WINDOW *modal, int step_index)
{
    Store *store = get_store();
    StepOption options[STEPS_MAX_OPTIONS];

    // Populate options with available locales.
    int count = populate_locale_options(options, STEPS_MAX_OPTIONS);

    // Mark previously selected locale if any.
    int selected = 0;
    if (store->locale[0] != '\0')
    {
        for (int i = 0; i < count; i++)
        {
            if (strcmp(options[i].value, store->locale) == 0)
            {
                selected = i;
                // Append "*" to the label.
                size_t len = strlen(options[i].label);
                if (len + 3 < sizeof(options[i].label))
                {
                    strcat(options[i].label, " *");
                }
                break;
            }
        }
    }

    // Run selection step for locale choice.
    int result = run_selection_step(
        modal,                                    // Modal window.
        wizard_steps[step_index].display_name,    // Step title.
        step_index + 1,                           // Step number (1-indexed).
        "Select your system locale:",             // Step prompt.
        options,                                  // Options array.
        count,                                    // Number of options.
        &selected,                                // Selected index pointer.
        0                                         // Allow back navigation.
    );
    if (result)
    {
        // Store the selected locale in global store.
        snprintf(store->locale, sizeof(store->locale), "%s", options[selected].value);
    }

    return result;
}
