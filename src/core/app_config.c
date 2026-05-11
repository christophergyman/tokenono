#include "app_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Keep CLI usage local to the parser so main.c only deals with valid config.
static void print_usage(const char *program_name)
{
    fprintf(stderr, "Usage: %s [--no-debug]\n", program_name);
}

// Debug checks run by default; --no-debug is an explicit opt-out for faster/manual runs.
AppConfig app_config_parse(int argc, char **argv)
{
    AppConfig config = {
        .debug_enabled = true,
    };

    const char *program_name = argc > 0 ? argv[0] : "tokenono";

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--no-debug") == 0) {
            config.debug_enabled = false;
            continue;
        }

        fprintf(stderr, "Unknown option: %s\n", argv[i]);
        print_usage(program_name);
        exit(1);
    }

    return config;
}
