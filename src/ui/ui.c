/**
 * @file ui.c
 * @brief Implementation of the ncurses TUI rendering engine.
 */

#include "ui.h"
#include "agent.h"
#include <ncurses.h>
#include <stdio.h>
#include <string.h>

bool ui_init(void) {
    if (initscr() == NULL) {
        return false;
    }

    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0); /* Hide cursor */
    nodelay(stdscr, TRUE); /* Non-blocking input */

    if (has_colors()) {
        start_color();
        use_default_colors();

        init_pair(COLOR_PAIR_DEFAULT, COLOR_WHITE, -1);
        init_pair(COLOR_PAIR_HEADER, COLOR_BLACK, COLOR_CYAN);
        init_pair(COLOR_PAIR_OK, COLOR_GREEN, -1);
        init_pair(COLOR_PAIR_WARN, COLOR_YELLOW, -1);
        init_pair(COLOR_PAIR_CRIT, COLOR_RED, -1);
        init_pair(COLOR_PAIR_BAR_FILL, COLOR_CYAN, -1);
    }

    return true;
}

void ui_shutdown(void) {
    endwin();
}

void ui_draw_header(ui_tab_t active_tab) {
    int max_x = getmaxx(stdscr);

    attron(COLOR_PAIR(COLOR_PAIR_HEADER) | A_BOLD);
    for (int i = 0; i < max_x; i++) {
        mvaddch(0, i, ' ');
    }

    char title[128];
    snprintf(title, sizeof(title), " %s v%s ", AGENT_NAME, AGENT_VERSION);
    mvprintw(0, 1, "%s", title);

    const char *tabs[] = {"[F1 Overview]", "[F2 Disks]", "[F3 Processes]", "[F4 Manual]"};
    int pos_x = (int)strlen(title) + 4;

    for (int t = 0; t < 4; t++) {
        if (t == (int)active_tab) {
            attron(A_REVERSE);
            mvprintw(0, pos_x, " %s ", tabs[t]);
            attroff(A_REVERSE);
            attron(COLOR_PAIR(COLOR_PAIR_HEADER) | A_BOLD);
        } else {
            mvprintw(0, pos_x, " %s ", tabs[t]);
        }
        pos_x += (int)strlen(tabs[t]) + 3;
    }

    attroff(COLOR_PAIR(COLOR_PAIR_HEADER) | A_BOLD);
}

static void draw_progress_bar(int y, int x, int width, float pct, const char *label) {
    mvprintw(y, x, "%-10s [", label);
    int bar_start_x = x + 12;
    int inner_width = width - 20;
    if (inner_width < 5) inner_width = 5;

    int filled = (int)((pct / 100.0f) * inner_width);

    short color = COLOR_PAIR_OK;
    if (pct > 85.0f) {
        color = COLOR_PAIR_CRIT;
    } else if (pct > 60.0f) {
        color = COLOR_PAIR_WARN;
    }

    attron(COLOR_PAIR(color) | A_BOLD);
    for (int i = 0; i < inner_width; i++) {
        if (i < filled) {
            mvaddch(y, bar_start_x + i, '|');
        } else {
            mvaddch(y, bar_start_x + i, ' ');
        }
    }
    attroff(COLOR_PAIR(color) | A_BOLD);

    mvprintw(y, bar_start_x + inner_width, "] %5.1f%%", pct);
}

void ui_draw_system_summary(const sys_cpu_metrics_t *cpu, const sys_mem_metrics_t *mem, int start_y, int start_x) {
    int max_x = getmaxx(stdscr);
    int bar_width = max_x - start_x - 4;
    if (bar_width > 80) bar_width = 80;

    /* CPU overall usage bar */
    draw_progress_bar(start_y, start_x, bar_width, cpu ? cpu->overall_usage_pct : 0.0f, "CPU Usage");

    /* RAM usage bar */
    draw_progress_bar(start_y + 1, start_x, bar_width, mem ? mem->ram_usage_pct : 0.0f, "RAM Usage");
}

void ui_draw_disks_view(const sys_disk_metrics_t *disk, int start_y, int start_x) {
    if (!disk || disk->count == 0) {
        mvprintw(start_y, start_x, "No disk metrics available.");
        return;
    }

    attron(A_BOLD | A_UNDERLINE);
    mvprintw(start_y, start_x, "%-20s %-25s %-10s %-10s %-10s %-8s",
             "Mount Point", "Device", "Total", "Used", "Free", "Use %");
    attroff(A_BOLD | A_UNDERLINE);

    int cur_y = start_y + 1;
    for (size_t i = 0; i < disk->count && cur_y < getmaxy(stdscr) - 2; i++) {
        const sys_disk_info_t *d = &disk->disks[i];
        float use_pct = d->total_bytes > 0 ? ((float)d->used_bytes / (float)d->total_bytes) * 100.0f : 0.0f;

        char total_str[16], used_str[16], free_str[16];
        snprintf(total_str, sizeof(total_str), "%.1f GB", (double)d->total_bytes / (1024.0 * 1024.0 * 1024.0));
        snprintf(used_str, sizeof(used_str), "%.1f GB", (double)d->used_bytes / (1024.0 * 1024.0 * 1024.0));
        snprintf(free_str, sizeof(free_str), "%.1f GB", (double)d->free_bytes / (1024.0 * 1024.0 * 1024.0));

        short color = COLOR_PAIR_OK;
        if (use_pct > 90.0f) {
            color = COLOR_PAIR_CRIT;
        } else if (use_pct > 75.0f) {
            color = COLOR_PAIR_WARN;
        }

        mvprintw(cur_y, start_x, "%-20.20s %-25.25s %-10s %-10s %-10s ",
                 d->mount_point, d->device, total_str, used_str, free_str);

        attron(COLOR_PAIR(color) | A_BOLD);
        printw("%6.1f%%", use_pct);
        attroff(COLOR_PAIR(color) | A_BOLD);

        cur_y++;
    }
}

void ui_draw_processes_view(const sys_proc_metrics_t *proc, int start_y, int start_x) {
    if (!proc || proc->count == 0) {
        mvprintw(start_y, start_x, "No process metrics available.");
        return;
    }

    attron(A_BOLD | A_UNDERLINE);
    mvprintw(start_y, start_x, "%-8s %-30s %-6s", "PID", "Command", "State");
    attroff(A_BOLD | A_UNDERLINE);

    int cur_y = start_y + 1;
    for (size_t i = 0; i < proc->count && cur_y < getmaxy(stdscr) - 2; i++) {
        const sys_proc_info_t *p = &proc->procs[i];
        mvprintw(cur_y, start_x, "%-8d %-30.30s %-6c", p->pid, p->name, p->state);
        cur_y++;
    }
}

bool ui_handle_input(ui_tab_t *active_tab) {
    int ch = getch();
    if (ch == 'q' || ch == 'Q') {
        return false;
    }

    if (ch == KEY_F(1) || ch == '1') *active_tab = UI_TAB_OVERVIEW;
    if (ch == KEY_F(2) || ch == '2') *active_tab = UI_TAB_DISKS;
    if (ch == KEY_F(3) || ch == '3') *active_tab = UI_TAB_PROCESSES;
    if (ch == KEY_F(4) || ch == '4') *active_tab = UI_TAB_MANUAL;

    return true;
}
