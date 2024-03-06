#ifndef __SURFACE_CONTROL_H
#define __SURFACE_CONTROL_H
#include "debug.h"
#include "SurfaceInfo_public.h"
#include <linux/dma-buf.h>
#include <unistd.h>
#include <memory>

class SurfaceControl {
public:
    SurfaceControl(sdm::Surface *surface);
    SurfaceControl(std::shared_ptr<sdm::Surface> surface);
    ~SurfaceControl() {
    }
    int setPos(int x, int y);
    int setCrop(int w, int h);
    int setTransform(int transform);
    std::shared_ptr<sdm::Surface> getSurface() {
        return mSurface;
    }
private:
    std::shared_ptr<sdm::Surface> mSurface;
};
#endif