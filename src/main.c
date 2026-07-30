/**
 * @file main.c
 * @brief Entry point for the agent CLI/TUI application.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "agent.h"
#include "sys_metrics.h"
#include "ui.h"

static void print_usage(const char *prog_name) {
    printf("Usage: %s [OPTIONS]\n", prog_name);
    printf("High-performance CLI/TUI monitoring agent (%s v%s)\n\n", AGENT_NAME, AGENT_VERSION);
    printf("Options:\n");
    printf("  -h, --help           Show this help message and exit\n");
    printf("  -v, --version        Show version information and exit\n");
    printf("  -j, --json           Output metrics in JSON format\n");
    printf("  -1, --once           Run once, output metrics, and exit\n");
    printf("  -c, --cpu            Show CPU metrics and exit\n");
    printf("  -m, --mem            Show Memory metrics and exit\n");
    printf("  -d, --disk           Show Disk metrics and exit\n");
    printf("  -p, --ps[=NAME]      Show process list (optional filter by NAME) and exit\n");
    printf("  -t, --top            Show top resource-consuming processes and exit\n");
}

int main(int argc, char *argv[]) {
    agent_config_t config = {
        .interactive = 1,
        .json_output = 0,
        .once = 0,
        .show_cpu = 0,
        .show_mem = 0,
        .show_disk = 0,
        .show_ps = 0,
        .show_top = 0,
        .filter_proc = ""
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
        } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--cpu") == 0) {
            config.show_cpu = 1;
            config.interactive = 0;
        } else if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--mem") == 0) {
            config.show_mem = 1;
            config.interactive = 0;
        } else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--disk") == 0) {
            config.show_disk = 1;
            config.interactive = 0;
        } else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--top") == 0) {
            config.show_top = 1;
            config.interactive = 0;
        } else if (strcmp(argv[i], "-p") == 0 || strncmp(argv[i], "--ps", 4) == 0) {
            config.show_ps = 1;
            config.interactive = 0;
            if (strncmp(argv[i], "--ps=", 5) == 0) {
                strncpy(config.filter_proc, argv[i] + 5, sizeof(config.filter_proc) - 1);
            } else if (i + 1 < argc && argv[i + 1][0] != '-') {
                strncpy(config.filter_proc, argv[i + 1], sizeof(config.filter_proc) - 1);
                i++;
            }
        } else {
            fprintf(stderr, "Error: Unknown option '%s'\n", argv[i]);
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    if (config.show_cpu) {
        sys_cpu_metrics_t cpu;
        sys_metrics_init();
        sys_get_cpu_metrics(&cpu);
        printf("\033[1;36m=== CPU Metrics ===\033[0m\n");
        printf("Cores: \033[1;33m%d\033[0m\n", cpu.core_count);
        printf("Usage: \033[1;32m%5.1f%%\033[0m [", cpu.overall_usage_pct);
        int filled = (int)((cpu.overall_usage_pct / 100.0f) * 30);
        for (int i = 0; i < 30; i++) {
            if (i < filled) printf("\033[32m█\033[0m");
            else printf("\033[90m░\033[0m");
        }
        printf("]\n");
        return EXIT_SUCCESS;
    }

    if (config.show_mem) {
        sys_mem_metrics_t mem;
        sys_metrics_init();
        sys_get_mem_metrics(&mem);
        printf("\033[1;35m=== Memory Metrics ===\033[0m\n");
        printf("Total: \033[1;37m%.2f GB\033[0m\n", (double)mem.total_bytes / (1024.0 * 1024.0 * 1024.0));
        printf("Used:  \033[1;31m%.2f GB\033[0m (%.2f%%)\n", (double)mem.used_bytes / (1024.0 * 1024.0 * 1024.0), mem.ram_usage_pct);
        printf("Free:  \033[1;32m%.2f GB\033[0m\n", (double)mem.free_bytes / (1024.0 * 1024.0 * 1024.0));
        printf("RAM:   [");
        int filled = (int)((mem.ram_usage_pct / 100.0f) * 30);
        for (int i = 0; i < 30; i++) {
            if (i < filled) printf("\033[35m█\033[0m");
            else printf("\033[90m░\033[0m");
        }
        printf("]\n");
        return EXIT_SUCCESS;
    }

    if (config.show_top) {
        sys_proc_metrics_t proc;
        sys_metrics_init();
        sys_get_proc_metrics(&proc);

        printf("\033[1;33m=== Top System Processes ===\033[0m\n");
        printf("\033[1;36m%-8s %-30s %-8s\033[0m\n", "PID", "COMMAND", "STATE");
        printf("\033[90m--------------------------------------------------\033[0m\n");

        size_t limit = proc.count < 10 ? proc.count : 10;
        for (size_t i = 0; i < limit; i++) {
            sys_proc_info_t *p = &proc.procs[i];
            const char *state_color = "\033[32m";
            if (p->state == 'S') state_color = "\033[34m";
            if (p->state == 'T') state_color = "\033[33m";
            if (p->state == 'Z') state_color = "\033[31m";

            printf("\033[1;37m%-8d\033[0m %-30.30s %s%-8c\033[0m\n", p->pid, p->name, state_color, p->state);
        }
        printf("\033[90m--------------------------------------------------\033[0m\n");
        return EXIT_SUCCESS;
    }

    if (config.show_ps) {
        sys_proc_metrics_t proc;
        sys_metrics_init();
        sys_get_proc_metrics(&proc);

        printf("\033[1;36m%-8s %-30s %-8s\033[0m\n", "PID", "COMMAND", "STATE");
        printf("\033[90m--------------------------------------------------\033[0m\n");

        size_t matched = 0;
        for (size_t i = 0; i < proc.count; i++) {
            sys_proc_info_t *p = &proc.procs[i];
            if (config.filter_proc[0] != '\0' && strcasestr(p->name, config.filter_proc) == NULL) {
                continue;
            }
            matched++;
            const char *state_color = "\033[32m"; /* Green running */
            if (p->state == 'S') state_color = "\033[34m"; /* Blue sleeping */
            if (p->state == 'T') state_color = "\033[33m"; /* Yellow stopped */
            if (p->state == 'Z') state_color = "\033[31m"; /* Red zombie */

            printf("\033[1;37m%-8d\033[0m %-30.30s %s%-8c\033[0m\n", p->pid, p->name, state_color, p->state);
        }

        printf("\033[90m--------------------------------------------------\033[0m\n");
        printf("\033[1;33mTotal matched processes:\033[0m %zu / %zu\n", matched, proc.count);
        return EXIT_SUCCESS;
    }

    if (config.show_disk) {
        sys_disk_metrics_t disk;
        sys_metrics_init();
        sys_get_disk_metrics(&disk);
        printf("%-20s %-25s %-12s %-12s %-8s %-8s\n", "Mount Point", "Device", "Total", "Used", "Use %", "IOPS");
        for (size_t i = 0; i < disk.count; i++) {
            sys_disk_info_t *d = &disk.disks[i];
            float use_pct = d->total_bytes > 0 ? ((float)d->used_bytes / (float)d->total_bytes) * 100.0f : 0.0f;
            char total_str[32], used_str[32];
            snprintf(total_str, sizeof(total_str), "%.1f GB", (double)d->total_bytes / (1024.0 * 1024.0 * 1024.0));
            snprintf(used_str, sizeof(used_str), "%.1f GB", (double)d->used_bytes / (1024.0 * 1024.0 * 1024.0));
            printf("%-20.20s %-25.25s %-12s %-12s %6.1f%% %8u\n",
                   d->mount_point, d->device, total_str, used_str, use_pct, d->iops);
        }
        return EXIT_SUCCESS;
    }

    if (config.json_output) {
        sys_cpu_metrics_t cpu;
        sys_mem_metrics_t mem;
        sys_metrics_init();
        sys_get_cpu_metrics(&cpu);
        sys_get_mem_metrics(&mem);

        printf("{\"status\": \"ok\", \"agent\": \"%s\", \"version\": \"%s\", "
               "\"cpu\": {\"cores\": %d, \"usage_pct\": %.2f}, "
               "\"memory\": {\"total_bytes\": %llu, \"used_bytes\": %llu, \"usage_pct\": %.2f}}\n", 
               AGENT_NAME, AGENT_VERSION, 
               cpu.core_count, cpu.overall_usage_pct,
               (unsigned long long)mem.total_bytes, (unsigned long long)mem.used_bytes, mem.ram_usage_pct);
        return EXIT_SUCCESS;
    }

    if (config.interactive) {
        if (!ui_init()) {
            fprintf(stderr, "Error: Failed to initialize terminal UI mode.\n");
            return EXIT_FAILURE;
        }

        sys_metrics_init();
        ui_tab_t active_tab = UI_TAB_OVERVIEW;
        int selected_proc_idx = 0;
        char status_msg[128] = "";
        sys_cpu_metrics_t cpu;
        sys_mem_metrics_t mem;
        sys_disk_metrics_t disk;
        sys_proc_metrics_t proc;

        while (true) {
            sys_get_cpu_metrics(&cpu);
            sys_get_mem_metrics(&mem);
            sys_get_disk_metrics(&disk);
            sys_get_proc_metrics(&proc);

            erase();
            ui_draw_header(active_tab);

            pid_t current_pid = 0;
            if (proc.count > 0 && selected_proc_idx < (int)proc.count) {
                current_pid = proc.procs[selected_proc_idx].pid;
            }

            if (active_tab == UI_TAB_DISKS) {
                ui_draw_disks_view(&disk, 2, 2);
            } else if (active_tab == UI_TAB_PROCESSES) {
                ui_draw_processes_view(&proc, selected_proc_idx, 2, 2);
                if (status_msg[0] != '\0') {
                    mvprintw(getmaxy(stdscr) - 2, 2, "Status: %s", status_msg);
                }
                mvprintw(getmaxy(stdscr) - 1, 2, "Use UP/DOWN/j/k to navigate. Press 'x' for SIGTERM, 'X' for SIGKILL, 'q' to exit.");
            } else if (active_tab == UI_TAB_MANUAL) {
                ui_draw_manual_view(selected_proc_idx, status_msg[0] != '\0' ? status_msg : "Ready for manual commands", 2, 2);
                mvprintw(getmaxy(stdscr) - 1, 2, "Press 'q' or 'Q' to exit.");
            } else {
                ui_draw_system_summary(&cpu, &mem, 2, 2);
                mvprintw(getmaxy(stdscr) - 1, 2, "Press 'q' or 'Q' to exit. Press F1-F4 or 1-4 to switch tabs.");
            }

            refresh();

            if (!ui_handle_input(&active_tab, &selected_proc_idx, current_pid, (int)proc.count, status_msg, sizeof(status_msg))) {
                break;
            }

            napms(100); /* 100ms refresh rate */
        }

        ui_shutdown();
    } else {
        printf("%s v%s initialized.\n", AGENT_NAME, AGENT_VERSION);
    }

    return EXIT_SUCCESS;
}
