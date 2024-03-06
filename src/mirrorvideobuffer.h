#ifndef MIRRORVIDEOBUFFER_H
#define MIRRORVIDEOBUFFER_H

#include <QObject>
#include <QAbstractVideoBuffer>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <QVariant>
#include "SurfaceInfo_public.h"
#include "drm/drm_fourcc.h"


#define IMAGE_WIDTH  1920
#define IMAGE_HEIGHT 720

class MirrorVideoBuffer : public QAbstractVideoBuffer
{

public:
    explicit MirrorVideoBuffer(sdm::Surface *info);
    ~MirrorVideoBuffer();

    QVariant handle() const override;

    void release() override;

    uchar* map(MapMode mode, int* numBytes, int* bytesPerLine) override;

    MapMode mapMode() const override
    {
        return NotMapped;
    }

    void unmap() override;

private:

    int m_bufferFD;
    EGLImage m_Image;
    EGLDisplay m_Display;
};

#endif // MIRRORVIDEOBUFFER_H
