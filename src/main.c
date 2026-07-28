/**
 * @file main.c
 * @brief Entry point for the agent CLI/TUI application.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "agent.h"

static void print_usage(const char *prog_name) {
    printf("Usage: %s [OPTIONS]\n", prog_name);
    printf("High-performance CLI/TUI monitoring agent (%s v%s)\n\n", AGENT_NAME, AGENT_VERSION);
    printf("Options:\n");
    printf("  -h, --help       Show this help message and exit\n");
    printf("  -v, --version    Show version information and exit\n");
    printf("  -j, --json       Output metrics in JSON format\n");
    printf("  -1, --once       Run once, output metrics, and exit\n");
}

int main(int argc, char *argv[]) {
    agent_config_t config = {
        .interactive = 1,
        .json_output = 0,
        .once = 0
    };

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return EXIT_SUCCESS;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            printf("%s version %s\n", AGENT_NAME, AGENT_VERSION);
            return EXIT_SUCCESS;
        } else if (strcmp(argv[i], "-j") == 0 || strcmp(argv[i], "--json") == 0) {
            config.json_output = 1;
            config.interactive = 0;
        } else if (strcmp(argv[i], "-1") == 0 || strcmp(argv[i], "--once") == 0) {
            config.once = 1;
            config.interactive = 0;
        } else {
            fprintf(stderr, "Error: Unknown option '%s'\n", argv[i]);
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    if (config.json_output) {
        printf("{\"status\": \"ok\", \"agent\": \"%s\", \"version\": \"%s\"}\n", AGENT_NAME, AGENT_VERSION);
    } else {
        printf("%s v%s initialized.\n", AGENT_NAME, AGENT_VERSION);
    }

    return EXIT_SUCCESS;
}
