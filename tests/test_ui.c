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

int main(void) {
    printf("Running UI module unit tests...\n");
    test_ui_enums();
    printf("All UI module unit tests passed successfully!\n");
    return 0;
}
