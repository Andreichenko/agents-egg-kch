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
 * @brief Main TUI event loop step.
 * @param active_tab Pointer to currently active tab variable.
 * @return false if exit command (e.g. 'q') was issued, true otherwise.
 */
bool ui_handle_input(ui_tab_t *active_tab);

#endif /* UI_H */
