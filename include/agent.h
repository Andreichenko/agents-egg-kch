/**
 * @file agent.h
 * @brief Core agent definitions, version info, and common data structures.
 */

#ifndef AGENT_H
#define AGENT_H

#define AGENT_VERSION "0.1.0"
#define AGENT_NAME    "agent-egg-kch"

/**
 * @brief Agent execution mode flags.
 */
typedef struct {
    int interactive;           /* Run TUI mode if non-zero */
    int json_output;           /* Output metrics as JSON */
    int once;                  /* Output once and exit */
    int show_cpu;              /* Output CPU metrics in CLI mode */
    int show_mem;              /* Output Memory metrics in CLI mode */
    int show_disk;             /* Output Disk metrics in CLI mode */
    int show_ps;               /* Output process list in CLI mode */
    int show_top;              /* Output top resource-consuming processes */
    char filter_proc[64];      /* Optional process filter string */
} agent_config_t;

#endif /* AGENT_H */
