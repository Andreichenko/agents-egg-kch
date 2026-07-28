/**
 * @file test_metrics.h
 * @brief Unit tests for System Metrics Hardware Abstraction Layer (HAL).
 */

#include <stdio.h>
#include <assert.h>
#include "sys_metrics.h"

static void test_cpu_metrics(void) {
    sys_cpu_metrics_t cpu;
    int res = sys_get_cpu_metrics(&cpu);
    assert(res == 0);
    assert(cpu.core_count > 0);
    assert(cpu.overall_usage_pct >= 0.0f && cpu.overall_usage_pct <= 100.0f);
    printf("[PASS] test_cpu_metrics (detected %d cores, usage: %.2f%%)\n", 
           cpu.core_count, cpu.overall_usage_pct);
}

static void test_mem_metrics(void) {
    sys_mem_metrics_t mem;
    int res = sys_get_mem_metrics(&mem);
    assert(res == 0);
    assert(mem.total_bytes > 0);
    assert(mem.used_bytes <= mem.total_bytes);
    assert(mem.ram_usage_pct >= 0.0f && mem.ram_usage_pct <= 100.0f);
    printf("[PASS] test_mem_metrics (total: %llu MB, used: %llu MB, usage: %.2f%%)\n",
           (unsigned long long)(mem.total_bytes / 1024 / 1024),
           (unsigned long long)(mem.used_bytes / 1024 / 1024),
           mem.ram_usage_pct);
}

static void test_disk_metrics(void) {
    sys_disk_metrics_t disk;
    int res = sys_get_disk_metrics(&disk);
    assert(res == 0);
    assert(disk.count > 0);
    for (size_t i = 0; i < disk.count; i++) {
        assert(disk.disks[i].total_bytes > 0);
        assert(disk.disks[i].mount_point[0] != '\0');
    }
    printf("[PASS] test_disk_metrics (detected %zu mounted disks, root total: %llu GB)\n",
           disk.count,
           (unsigned long long)(disk.disks[0].total_bytes / 1024 / 1024 / 1024));
}

int main(void) {
    printf("Running System Metrics HAL unit tests...\n");
    assert(sys_metrics_init() == 0);
    test_cpu_metrics();
    test_mem_metrics();
    test_disk_metrics();
    printf("All System Metrics HAL unit tests passed successfully!\n");
    return 0;
}
