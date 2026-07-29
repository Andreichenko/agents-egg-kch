#include "sys_metrics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/statvfs.h>

int sys_metrics_init(void) {
    return 0;
}

int sys_get_cpu_metrics(sys_cpu_metrics_t *metrics) {
    if (!metrics) return -1;
    memset(metrics, 0, sizeof(sys_cpu_metrics_t));

    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) return -1;

    char line[256];
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;

    if (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "cpu %llu %llu %llu %llu %llu %llu %llu %llu",
                   &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal) >= 4) {
            unsigned long long total_idle = idle + iowait;
            unsigned long long total_non_idle = user + nice + system + irq + softirq + steal;
            unsigned long long total = total_idle + total_non_idle;

            if (total > 0) {
                metrics->overall_usage_pct = (float)total_non_idle / (float)total * 100.0f;
            }
        }
    }

    long nproc = sysconf(_SC_NPROCESSORS_ONLN);
    metrics->core_count = nproc > 0 ? (int)nproc : 1;

    fclose(fp);
    return 0;
}

int sys_get_mem_metrics(sys_mem_metrics_t *metrics) {
    if (!metrics) return -1;
    memset(metrics, 0, sizeof(sys_mem_metrics_t));

    FILE *fp = fopen("/proc/meminfo", "r");
    if (!fp) return -1;

    char key[64];
    uint64_t val;
    uint64_t mem_total = 0, mem_free = 0, buffers = 0, cached = 0, swap_total = 0, swap_free = 0;

    while (fscanf(fp, "%63s %llu kB\n", key, (unsigned long long *)&val) == 2) {
        if (strcmp(key, "MemTotal:") == 0) mem_total = val * 1024;
        else if (strcmp(key, "MemFree:") == 0) mem_free = val * 1024;
        else if (strcmp(key, "Buffers:") == 0) buffers = val * 1024;
        else if (strcmp(key, "Cached:") == 0) cached = val * 1024;
        else if (strcmp(key, "SwapTotal:") == 0) swap_total = val * 1024;
        else if (strcmp(key, "SwapFree:") == 0) swap_free = val * 1024;
    }
    fclose(fp);

    metrics->total_bytes = mem_total;
    uint64_t actual_free = mem_free + buffers + cached;
    metrics->used_bytes = mem_total > actual_free ? (mem_total - actual_free) : 0;
    metrics->free_bytes = mem_free;

    if (mem_total > 0) {
        metrics->ram_usage_pct = (float)metrics->used_bytes / (float)mem_total * 100.0f;
    }

    metrics->swap_total_bytes = swap_total;
    metrics->swap_used_bytes = swap_total > swap_free ? (swap_total - swap_free) : 0;

    return 0;
}

int sys_get_disk_metrics(sys_disk_metrics_t *metrics) {
    if (!metrics) return -1;
    memset(metrics, 0, sizeof(sys_disk_metrics_t));

    FILE *fp = fopen("/proc/mounts", "r");
    if (!fp) return -1;

    char dev[128], mount[256], fstype[64], opts[256];
    int dump, pass;
    size_t idx = 0;

    while (fscanf(fp, "%127s %255s %63s %255s %d %d\n", dev, mount, fstype, opts, &dump, &pass) == 6 && idx < MAX_DISKS) {
        if (strncmp(dev, "/dev/", 5) != 0) continue;

        struct statvfs vfs;
        if (statvfs(mount, &vfs) == 0) {
            sys_disk_info_t *d = &metrics->disks[idx++];
            strncpy(d->mount_point, mount, sizeof(d->mount_point) - 1);
            strncpy(d->device, dev, sizeof(d->device) - 1);

            d->total_bytes = (uint64_t)vfs.f_blocks * vfs.f_frsize;
            d->free_bytes = (uint64_t)vfs.f_bavail * vfs.f_frsize;
            d->used_bytes = d->total_bytes > d->free_bytes ? (d->total_bytes - d->free_bytes) : 0;
        }
    }
    fclose(fp);

    metrics->count = idx;
    return 0;
}

int sys_get_proc_metrics(sys_proc_metrics_t *metrics) {
    if (!metrics) return -1;
    memset(metrics, 0, sizeof(sys_proc_metrics_t));

    DIR *dir = opendir("/proc");
    if (!dir) return -1;

    struct dirent *entry;
    size_t idx = 0;

    while ((entry = readdir(dir)) != NULL && idx < MAX_PROCESSES) {
        if (!isdigit(entry->d_name[0])) continue;

        pid_t pid = (pid_t)atoi(entry->d_name);
        char stat_path[256];
        snprintf(stat_path, sizeof(stat_path), "/proc/%d/stat", pid);

        FILE *fp = fopen(stat_path, "r");
        if (!fp) continue;

        sys_proc_info_t *p = &metrics->procs[idx++];
        p->pid = pid;

        char comm[64];
        char state;
        if (fscanf(fp, "%*d (%63[^)]) %c", comm, &state) == 2) {
            strncpy(p->name, comm, sizeof(p->name) - 1);
            p->state = state;
        } else {
            snprintf(p->name, sizeof(p->name), "proc_%d", pid);
            p->state = 'S';
        }
        fclose(fp);
    }
    closedir(dir);

    metrics->count = idx;
    return 0;
}
