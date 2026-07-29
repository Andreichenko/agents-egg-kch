/**
 * @file sys_metrics.h
 * @brief Hardware Abstraction Layer (HAL) for system metrics collection.
 * 
 * Provides unified interface for CPU, Memory, Disk, and Process metrics
 * on both Linux and macOS platforms.
 */

#ifndef SYS_METRICS_H
#define SYS_METRICS_H

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>

#define MAX_CPU_CORES 128
#define MAX_DISKS     32
#define MAX_PROCESSES 1024

/**
 * @brief CPU usage metrics.
 */
typedef struct {
    float overall_usage_pct;                 /* Total CPU usage percentage (0.0 to 100.0) */
    int core_count;                          /* Number of logical CPU cores */
    float core_usage_pct[MAX_CPU_CORES];     /* Per-core usage percentage */
} sys_cpu_metrics_t;

/**
 * @brief Memory usage metrics (bytes).
 */
typedef struct {
    uint64_t total_bytes;
    uint64_t used_bytes;
    uint64_t free_bytes;
    uint64_t swap_total_bytes;
    uint64_t swap_used_bytes;
    float ram_usage_pct;
} sys_mem_metrics_t;

/**
 * @brief Individual disk partition metrics.
 */
typedef struct {
    char mount_point[256];
    char device[128];
    uint64_t total_bytes;
    uint64_t used_bytes;
    uint64_t free_bytes;
    uint64_t read_bytes_sec;
    uint64_t write_bytes_sec;
    uint32_t iops;
} sys_disk_info_t;

/**
 * @brief Aggregate disk subsystem metrics.
 */
typedef struct {
    size_t count;
    sys_disk_info_t disks[MAX_DISKS];
} sys_disk_metrics_t;

/**
 * @brief Process information entry.
 */
typedef struct {
    pid_t pid;
    char name[64];
    char user[32];
    float cpu_pct;
    float mem_pct;
    uint64_t rss_bytes;
    char state; /* 'R' (running), 'S' (sleeping), etc. */
} sys_proc_info_t;

/**
 * @brief Aggregate process metrics list.
 */
typedef struct {
    size_t count;
    sys_proc_info_t procs[MAX_PROCESSES];
} sys_proc_metrics_t;

/**
 * @brief Initialize the system metrics collector HAL.
 * @return 0 on success, non-zero on failure.
 */
int sys_metrics_init(void);

/**
 * @brief Collect current CPU metrics.
 * @param metrics Pointer to output structure.
 * @return 0 on success, non-zero on failure.
 */
int sys_get_cpu_metrics(sys_cpu_metrics_t *metrics);

/**
 * @brief Collect current Memory metrics.
 * @param metrics Pointer to output structure.
 * @return 0 on success, non-zero on failure.
 */
int sys_get_mem_metrics(sys_mem_metrics_t *metrics);

/**
 * @brief Collect current Disk metrics.
 * @param metrics Pointer to output structure.
 * @return 0 on success, non-zero on failure.
 */
int sys_get_disk_metrics(sys_disk_metrics_t *metrics);

/**
 * @brief Collect current running processes list.
 * @param metrics Pointer to output structure.
 * @return 0 on success, non-zero on failure.
 */
int sys_get_proc_metrics(sys_proc_metrics_t *metrics);

#endif /* SYS_METRICS_H */
