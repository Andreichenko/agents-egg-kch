/**
 * @file sys_linux.c
 * @brief Linux backend implementation placeholder for system metrics collection HAL.
 */

#include "sys_metrics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int sys_metrics_init(void) {
    return 0;
}

int sys_get_cpu_metrics(sys_cpu_metrics_t *metrics) {
    if (!metrics) return -1;
    memset(metrics, 0, sizeof(sys_cpu_metrics_t));
    /* TODO: Parse /proc/stat */
    metrics->core_count = 1;
    return 0;
}

int sys_get_mem_metrics(sys_mem_metrics_t *metrics) {
    if (!metrics) return -1;
    memset(metrics, 0, sizeof(sys_mem_metrics_t));
    /* TODO: Parse /proc/meminfo */
    return 0;
}

int sys_get_disk_metrics(sys_disk_metrics_t *metrics) {
    if (!metrics) return -1;
    memset(metrics, 0, sizeof(sys_disk_metrics_t));
    /* TODO: Parse statvfs and /proc/diskstats */
    return 0;
}

int sys_get_proc_metrics(sys_proc_metrics_t *metrics) {
    if (!metrics) return -1;
    memset(metrics, 0, sizeof(sys_proc_metrics_t));
    /* TODO: Parse /proc/[pid]/stat */
    return 0;
}
