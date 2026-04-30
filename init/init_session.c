#include "init_session.h"

static char const *get_config_dir_path(void)
{
    static char *config_path = NULL;

    if (config_path)
        return config_path;

    config_path = getenv("XDG_CONFIG_HOME");
}
