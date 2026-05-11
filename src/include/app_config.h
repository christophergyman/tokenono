#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <stdbool.h>

// Runtime options parsed from CLI flags before the game starts.
typedef struct AppConfig {
    bool debug_enabled;
} AppConfig;

// Parses supported flags and exits with usage text on unknown input.
AppConfig app_config_parse(int argc, char **argv);

#endif
