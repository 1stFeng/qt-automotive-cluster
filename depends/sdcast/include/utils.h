#ifndef __UTILS_H__
#define __UTILS_H__

#include "debug.h"
typedef struct drm_buffer_t {
    uint32_t size;
    uint64_t phys_addr;
    uint64_t base;
    uint64_t virt_addr;
    uint64_t dma_addr;
    uint64_t attachment;
    uint64_t sgt;
    int32_t	 buf_handle;
} drm_buffer_t;

int drm_ioctl_open(void);
int drm_ioctl_get_phy_addr(int fd, int dmabuf, drm_buffer_t *data);
int drm_ioctl_release_phy_addr(int fd, drm_buffer_t *data);
int drm_ioctl_export_dmabuf(int fd, uint64_t addr, size_t size, int *dma_fd, int flags);

void set_color(void *vaddr, int w, int h, uint32_t color);

#endif //__UTILS_H__

