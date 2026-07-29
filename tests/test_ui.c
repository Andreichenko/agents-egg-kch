/**
 * @file test_ui.c
 * @brief Mock unit tests for UI helper routines.
 */

#include <stdio.h>
#include <assert.h>
#include "ui.h"

static void test_ui_enums(void) {
    ui_tab_t tab = UI_TAB_OVERVIEW;
    assert(tab == 0);
    tab = UI_TAB_DISKS;
    assert(tab == 1);
    printf("[PASS] test_ui_enums\n");
}

static void test_ui_disk_view_struct(void) {
    sys_disk_metrics_t disk = {
        .count = 1,
        .disks = {
            {
                .mount_point = "/",
                .device = "/dev/disk1s1",
                .total_bytes = 100000000000ULL,
                .used_bytes = 50000000000ULL,
                .free_bytes = 50000000000ULL
            }
        }
    };
    assert(disk.count == 1);
    assert(disk.disks[0].total_bytes == 100000000000ULL);
    printf("[PASS] test_ui_disk_view_struct\n");
}

int main(void) {
    printf("Running UI module unit tests...\n");
    test_ui_enums();
    test_ui_disk_view_struct();
    printf("All UI module unit tests passed successfully!\n");
    return 0;
}
