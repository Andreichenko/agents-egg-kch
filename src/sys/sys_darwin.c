/**
 * @file sys_darwin.c
 * @brief macOS (Darwin) backend implementation for system metrics collection HAL.
 */

#include "sys_metrics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/sysctl.h>
#include <sys/mount.h>
#include <mach/mach.h>
#include <mach/processor_info.h>
#include <mach/mach_host.h>

static processor_info_array_t prev_cpu_info = NULL;
static mach_msg_type_number_t prev_cpu_info_count = 0;

int sys_metrics_init(void) {
    return 0;
}

int sys_get_cpu_metrics(sys_cpu_metrics_t *metrics) {
    if (!metrics) return -1;
    memset(metrics, 0, sizeof(sys_cpu_metrics_t));

    processor_info_array_t cpu_info;
    mach_msg_type_number_t cpu_info_count;
    unsigned int num_cpus;

    host_t host = mach_host_self();
    kern_return_t kr = host_processor_info(host, PROCESSOR_CPU_LOAD_INFO, &num_cpus, &cpu_info, &cpu_info_count);
    if (kr != KERN_SUCCESS) {
        return -1;
    }

    metrics->core_count = (int)num_cpus > MAX_CPU_CORES ? MAX_CPU_CORES : (int)num_cpus;

    float total_usage_sum = 0.0f;
    for (int i = 0; i < metrics->core_count; i++) {
        float usage = 0.0f;
        if (prev_cpu_info) {
            unsigned long long user = cpu_info[(CPU_STATE_MAX * i) + CPU_STATE_USER] - prev_cpu_info[(CPU_STATE_MAX * i) + CPU_STATE_USER];
            unsigned long long system = cpu_info[(CPU_STATE_MAX * i) + CPU_STATE_SYSTEM] - prev_cpu_info[(CPU_STATE_MAX * i) + CPU_STATE_SYSTEM];
            unsigned long long nice = cpu_info[(CPU_STATE_MAX * i) + CPU_STATE_NICE] - prev_cpu_info[(CPU_STATE_MAX * i) + CPU_STATE_NICE];
            unsigned long long idle = cpu_info[(CPU_STATE_MAX * i) + CPU_STATE_IDLE] - prev_cpu_info[(CPU_STATE_MAX * i) + CPU_STATE_IDLE];

            unsigned long long total = user + system + nice + idle;
            if (total > 0) {
                usage = (float)(user + system + nice) / (float)total * 100.0f;
            }
        }
        metrics->core_usage_pct[i] = usage;
        total_usage_sum += usage;
    }

    if (prev_cpu_info) {
        vm_deallocate(mach_task_self(), (vm_address_t)prev_cpu_info, prev_cpu_info_count * sizeof(int));
    }

    prev_cpu_info = cpu_info;
    prev_cpu_info_count = cpu_info_count;

    metrics->overall_usage_pct = metrics->core_count > 0 ? (total_usage_sum / metrics->core_count) : 0.0f;

    return 0;
}

int sys_get_mem_metrics(sys_mem_metrics_t *metrics) {
    if (!metrics) return -1;
    memset(metrics, 0, sizeof(sys_mem_metrics_t));

    // Get total RAM
    int mib[2] = {CTL_HW, HW_MEMSIZE};
    uint64_t memsize = 0;
    size_t len = sizeof(memsize);
    if (sysctl(mib, 2, &memsize, &len, NULL, 0) == 0) {
        metrics->total_bytes = memsize;
    }

    // Get VM statistics
    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
    vm_statistics64_data_t vm_stat;
    host_t host = mach_host_self();

    if (host_statistics64(host, HOST_VM_INFO64, (host_info64_t)&vm_stat, &count) == KERN_SUCCESS) {
        long page_size = sysconf(_SC_PAGESIZE);
        uint64_t active = (uint64_t)vm_stat.active_count * page_size;
        uint64_t wire = (uint64_t)vm_stat.wire_count * page_size;
        uint64_t compressed = (uint64_t)vm_stat.compressor_page_count * page_size;
        
        metrics->used_bytes = active + wire + compressed;
        if (metrics->total_bytes > metrics->used_bytes) {
            metrics->free_bytes = metrics->total_bytes - metrics->used_bytes;
        } else {
            metrics->free_bytes = 0;
        }

        if (metrics->total_bytes > 0) {
            metrics->ram_usage_pct = (float)metrics->used_bytes / (float)metrics->total_bytes * 100.0f;
        }
    }

    // Swap usage
    struct xsw_usage swapusage;
    size_t swap_len = sizeof(swapusage);
    int swap_mib[2] = {CTL_VM, VM_SWAPUSAGE};
    if (sysctl(swap_mib, 2, &swapusage, &swap_len, NULL, 0) == 0) {
        metrics->swap_total_bytes = swapusage.xsu_total;
        metrics->swap_used_bytes = swapusage.xsu_used;
    }

    return 0;
}

int sys_get_disk_metrics(sys_disk_metrics_t *metrics) {
    if (!metrics) return -1;
    memset(metrics, 0, sizeof(sys_disk_metrics_t));

    struct statfs *mntbuf;
    int count = getmntinfo(&mntbuf, MNT_NOWAIT);
    if (count <= 0) return -1;

    size_t disk_idx = 0;
    for (int i = 0; i < count && disk_idx < MAX_DISKS; i++) {
        // Filter out pseudo / non-file filesystems
        if (strncmp(mntbuf[i].f_fstypename, "devfs", 5) == 0 ||
            strncmp(mntbuf[i].f_fstypename, "autofs", 6) == 0) {
            continue;
        }

        sys_disk_info_t *d = &metrics->disks[disk_idx++];
        strncpy(d->mount_point, mntbuf[i].f_mntonname, sizeof(d->mount_point) - 1);
        strncpy(d->device, mntbuf[i].f_mntfromname, sizeof(d->device) - 1);

        d->total_bytes = (uint64_t)mntbuf[i].f_blocks * mntbuf[i].f_bsize;
        d->free_bytes = (uint64_t)mntbuf[i].f_bavail * mntbuf[i].f_bsize;
        d->used_bytes = d->total_bytes > d->free_bytes ? (d->total_bytes - d->free_bytes) : 0;
        
        /* Calculated metrics placeholders */
        d->read_bytes_sec = 0;
        d->write_bytes_sec = 0;
        d->iops = 0;
    }

    metrics->count = disk_idx;
    return 0;
}

int sys_get_proc_metrics(sys_proc_metrics_t *metrics) {
    if (!metrics) return -1;
    memset(metrics, 0, sizeof(sys_proc_metrics_t));

    int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0};
    size_t len = 0;
    if (sysctl(mib, 4, NULL, &len, NULL, 0) < 0) {
        return -1;
    }

    struct kinfo_proc *procs = malloc(len);
    if (!procs) return -1;

    if (sysctl(mib, 4, procs, &len, NULL, 0) < 0) {
        free(procs);
        return -1;
    }

    size_t count = len / sizeof(struct kinfo_proc);
    size_t idx = 0;

    for (size_t i = 0; i < count && idx < MAX_PROCESSES; i++) {
        sys_proc_info_t *p = &metrics->procs[idx++];
        p->pid = procs[i].kp_proc.p_pid;
        strncpy(p->name, procs[i].kp_proc.p_comm, sizeof(p->name) - 1);

        switch (procs[i].kp_proc.p_stat) {
            case SRUN:  p->state = 'R'; break;
            case SSLEEP: p->state = 'S'; break;
            case SSTOP:  p->state = 'T'; break;
            case SZOMB:  p->state = 'Z'; break;
            default:     p->state = 'I'; break;
        }

        /* Basic defaults, detailed CPU/RAM % calculated in subsequent steps */
        p->cpu_pct = 0.0f;
        p->mem_pct = 0.0f;
        p->rss_bytes = 0;
    }

    free(procs);
    metrics->count = idx;
    return 0;
}
