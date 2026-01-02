#include "../all.h"

int is_library_available(const char *name)
{
    void *handle = dlopen(name, RTLD_NOW);
    if (handle == NULL)
    {
        return 0;
    }
    dlclose(handle);
    return 1;
}

int is_command_available(const char *name)
{
    char *path_env = getenv("PATH");
    if (path_env == NULL)
    {
        return 0;
    }

    char *path = strdup(path_env);
    if (path == NULL)
    {
        return 0;
    }

    char *dir = strtok(path, ":");
    while (dir != NULL)
    {
        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir, name);
        if (access(full_path, X_OK) == 0)
        {
            free(path);
            return 1;
        }
        dir = strtok(NULL, ":");
    }

    free(path);
    return 0;
}
