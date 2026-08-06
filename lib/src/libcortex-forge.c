#include "libcortex-forge.h"
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define CORTEX_FORGE_MAGIC 0xCF
#define CORTEX_FORGE_IOCTL_SUBMIT_TASK _IOWR(CORTEX_FORGE_MAGIC, 1, struct cortex_forge_task_desc)
#define CORTEX_FORGE_IOCTL_QUERY_TASK _IOWR(CORTEX_FORGE_MAGIC, 2, struct cortex_forge_task_status)
#define CORTEX_FORGE_IOCTL_CANCEL_TASK _IOW(CORTEX_FORGE_MAGIC, 3, uint32_t)
#define CORTEX_FORGE_IOCTL_GET_ACCEL_INFO                                                          \
    _IOR(CORTEX_FORGE_MAGIC, 4, struct cortex_forge_accel_info)
#define CORTEX_FORGE_IOCTL_SET_POWER _IOW(CORTEX_FORGE_MAGIC, 5, uint32_t)
#define CORTEX_FORGE_IOCTL_GET_VERSION _IOR(CORTEX_FORGE_MAGIC, 6, uint32_t)

struct cortex_forge_handle
{
    int             fd;
    pthread_mutex_t lock;
    char            path[256];
};

struct cortex_forge_handle* cortex_forge_open(const char* p)
{
    if (!p)
    {
        errno = EINVAL;
        return NULL;
    }
    int fd = open(p, O_RDWR);
    if (fd < 0)
        return NULL;
    struct cortex_forge_handle* h = calloc(1, sizeof(*h));
    if (!h)
    {
        close(fd);
        return NULL;
    }
    h->fd = fd;
    pthread_mutex_init(&h->lock, NULL);
    strncpy(h->path, p, 255);
    return h;
}

void cortex_forge_close(struct cortex_forge_handle* h)
{
    if (!h)
        return;
    pthread_mutex_destroy(&h->lock);
    close(h->fd);
    free(h);
}

int cortex_forge_submit_task(struct cortex_forge_handle* h, struct cortex_forge_task_desc* d)
{
    if (!h || !d)
        return -EINVAL;
    pthread_mutex_lock(&h->lock);
    int r = ioctl(h->fd, CORTEX_FORGE_IOCTL_SUBMIT_TASK, d);
    pthread_mutex_unlock(&h->lock);
    return r < 0 ? -errno : 0;
}

int cortex_forge_query_task(struct cortex_forge_handle* h, struct cortex_forge_task_status* s)
{
    if (!h || !s)
        return -EINVAL;
    pthread_mutex_lock(&h->lock);
    int r = ioctl(h->fd, CORTEX_FORGE_IOCTL_QUERY_TASK, s);
    pthread_mutex_unlock(&h->lock);
    return r < 0 ? -errno : 0;
}

int cortex_forge_cancel_task(struct cortex_forge_handle* h, uint32_t id)
{
    if (!h)
        return -EINVAL;
    pthread_mutex_lock(&h->lock);
    int r = ioctl(h->fd, CORTEX_FORGE_IOCTL_CANCEL_TASK, &id);
    pthread_mutex_unlock(&h->lock);
    return r < 0 ? -errno : 0;
}

int cortex_forge_get_accel_info(struct cortex_forge_handle* h, struct cortex_forge_accel_info* i)
{
    if (!h || !i)
        return -EINVAL;
    pthread_mutex_lock(&h->lock);
    int r = ioctl(h->fd, CORTEX_FORGE_IOCTL_GET_ACCEL_INFO, i);
    pthread_mutex_unlock(&h->lock);
    return r < 0 ? -errno : 0;
}

int cortex_forge_set_power(struct cortex_forge_handle* h, uint32_t m)
{
    if (!h)
        return -EINVAL;
    pthread_mutex_lock(&h->lock);
    int r = ioctl(h->fd, CORTEX_FORGE_IOCTL_SET_POWER, &m);
    pthread_mutex_unlock(&h->lock);
    return r < 0 ? -errno : 0;
}

int cortex_forge_get_version(struct cortex_forge_handle* h, uint32_t* v)
{
    if (!h || !v)
        return -EINVAL;
    pthread_mutex_lock(&h->lock);
    int r = ioctl(h->fd, CORTEX_FORGE_IOCTL_GET_VERSION, v);
    pthread_mutex_unlock(&h->lock);
    return r < 0 ? -errno : 0;
}

const char* cortex_forge_strerror(int e)
{
    switch (e)
    {
        case 0:
            return "Success";
        case -EINVAL:
            return "Invalid argument";
        case -EFAULT:
            return "Bad address";
        case -ENOMEM:
            return "Out of memory";
        case -ENOENT:
            return "Task not found";
        case -ENOTTY:
            return "Invalid ioctl command";
        case -EBUSY:
            return "Device busy";
        case -ETIMEDOUT:
            return "Operation timed out";
        case -E2BIG:
            return "Data too large";
        case -ECANCELED:
            return "Task cancelled";
        default:
            return strerror(-e);
    }
}
