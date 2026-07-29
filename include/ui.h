/**
 * @file ui.h
 * @brief Terminal User Interface (TUI) engine definitions using ncurses.
 */

#ifndef UI_H
#define UI_H

#include <stdbool.h>
#include "sys_metrics.h"

/**
 * @brief Color pair identifiers for ncurses rendering.
 */
typedef enum {
    COLOR_PAIR_DEFAULT = 1,
    COLOR_PAIR_HEADER,
    COLOR_PAIR_OK,       /* Green for low/normal usage */
    COLOR_PAIR_WARN,     /* Yellow for moderate usage */
    COLOR_PAIR_CRIT,     /* Red for high usage */
    COLOR_PAIR_BAR_FILL  /* Highlighting bars */
} ui_color_pair_t;

/**
 * @brief Active UI tab view.
 */
typedef enum {
    UI_TAB_OVERVIEW = 0,
    UI_TAB_DISKS,
    UI_TAB_PROCESSES,
    UI_TAB_MANUAL
} ui_tab_t;

/**
 * @brief Initialize ncurses terminal UI mode.
 * @return true on success, false if terminal lacks required capabilities.
 */
bool ui_init(void);

/**
 * @brief Cleanup and restore standard terminal settings.
 */
void ui_shutdown(void);

/**
 * @brief Draw top header bar with system title, version and active tab indicator.
 * @param active_tab Current selected tab.
 */
void ui_draw_header(ui_tab_t active_tab);

/**
 * @brief Draw CPU & Memory usage widgets/bars.
 * @param cpu Pointer to CPU metrics structure.
 * @param mem Pointer to Memory metrics structure.
 * @param start_y Vertical start position.
 * @param start_x Horizontal start position.
 */
void ui_draw_system_summary(const sys_cpu_metrics_t *cpu, const sys_mem_metrics_t *mem, int start_y, int start_x);

/**
 * @brief Draw detailed disks summary table view.
 * @param disk Pointer to Disk metrics structure.
 * @param start_y Vertical start position.
 * @param start_x Horizontal start position.
 */
void ui_draw_disks_view(const sys_disk_metrics_t *disk, int start_y, int start_x);

/**
 * @brief Draw process manager table view.
 * @param proc Pointer to Process metrics structure.
 * @param selected_idx Currently highlighted process row index.
 * @param start_y Vertical start position.
 * @param start_x Horizontal start position.
 */
void ui_draw_processes_view(const sys_proc_metrics_t *proc, int selected_idx, int start_y, int start_x);

/**
 * @brief Draw manual task/process control panel.
 * @param selected_idx Index of highlighted process/task.
 * @param status_msg Optional status message or prompt.
 * @param start_y Vertical start position.
 * @param start_x Horizontal start position.
 */
void ui_draw_manual_view(int selected_idx, const char *status_msg, int start_y, int start_x);

/**
 * @brief Main TUI event loop step.
 * @param active_tab Pointer to currently active tab variable.
 * @param selected_proc_idx Pointer to currently selected process index variable.
 * @param max_proc_count Total number of processes available for navigation.
 * @return false if exit command (e.g. 'q') was issued, true otherwise.
 */
bool ui_handle_input(ui_tab_t *active_tab, int *selected_proc_idx, int max_proc_count);

#endif /* UI_H */
