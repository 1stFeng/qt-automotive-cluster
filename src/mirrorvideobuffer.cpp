#include "mirrorvideobuffer.h"
#include <QOpenGLContext>
#include <QtPlatformHeaders/qeglnativecontext.h>
#include <QDateTime>
#include <QDebug>

#define MAX_ATTRIBUTES_COUNT 30

static void checkGlError(const char* op) {
    for (GLint error = glGetError(); error; error= glGetError()) {
        qDebug("after %s() glError (0x%x)\n", op, error);
    }
}

const char * GetEGLErrorString(EGLint error_code)
{
    switch(error_code)
    {
#define X(error) case error: return #error;
    X(EGL_SUCCESS)
            X(EGL_NOT_INITIALIZED)
            X(EGL_BAD_ACCESS)
            X(EGL_BAD_ALLOC)
            X(EGL_BAD_ATTRIBUTE)
            X(EGL_BAD_CONFIG)
            X(EGL_BAD_CONTEXT)
            X(EGL_BAD_CURRENT_SURFACE)
            X(EGL_BAD_DISPLAY)
            X(EGL_BAD_MATCH)
            X(EGL_BAD_NATIVE_PIXMAP)
            X(EGL_BAD_NATIVE_WINDOW)
            X(EGL_BAD_PARAMETER)
            X(EGL_BAD_SURFACE)
            X(EGL_CONTEXT_LOST)
        #undef X
            default: return "unknown";
    }
}

static void handle_egl_error(const char *name)
{
    EGLint error_code = eglGetError();

    qDebug("'%s' returned egl error '%s' (0x%llx)\n",
           name,
           GetEGLErrorString(error_code),
           (unsigned long long)error_code);
}


MirrorVideoBuffer::MirrorVideoBuffer(sdm::Surface *info)
    : QAbstractVideoBuffer(HandleType::EGLImageHandle)
{
    if(info->prime_fd <= 0) {
        return;
    }

    m_bufferFD = info->prime_fd;
//    qDebug("-----MirrorVideoBuffer    m_bufferFD  is %d",m_bufferFD);
    PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR = reinterpret_cast<PFNEGLCREATEIMAGEKHRPROC>(eglGetProcAddress("eglCreateImageKHR"));
    int idx = 0;

    EGLint aiImageAttribs[25] =
    {
        EGL_WIDTH, (EGLint)IMAGE_WIDTH,
        EGL_HEIGHT, (EGLint)IMAGE_HEIGHT,
        EGL_LINUX_DRM_FOURCC_EXT, (EGLint)DRM_FORMAT_ABGR8888,//DRM_FORMAT_ABGR8888  EGL_BAD_ALLOC------- DRM_FORMAT_RGBA8888 EGL_BAD_MATCH
        EGL_DMA_BUF_PLANE0_FD_EXT, (EGLint)m_bufferFD,//mirror_fd,     //TODO????
        EGL_DMA_BUF_PLANE0_OFFSET_EXT, (EGLint)0,
        EGL_DMA_BUF_PLANE0_PITCH_EXT, (EGLint)IMAGE_WIDTH * 4, //TODO??? no * 4 EGL_BAD_ACCESS
        EGL_NONE
    };

    qDebug("-----MirrorVideoBuffer    info->with_modifiers = %d, info->offsets[0] = %d, info->pitches[0] = %d, info->modifiers[0] = 0x%llx",
        info->with_modifiers,
        info->offsets[0],
        info->pitches[0],
        info->modifiers[0]);
    /* TODO should check format */
    if (info->with_modifiers) {
        aiImageAttribs[8]  = EGL_DMA_BUF_PLANE0_OFFSET_EXT;
        aiImageAttribs[9]  = info->offsets[0];
        aiImageAttribs[10] = EGL_DMA_BUF_PLANE0_PITCH_EXT;
        aiImageAttribs[11] = info->pitches[0];
        aiImageAttribs[12] = EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT;
        aiImageAttribs[13] = (info->modifiers[0]) & 0xFFFFFFFF;
        aiImageAttribs[14] = EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT;
        aiImageAttribs[15] = (info->modifiers[0]) >> 32;
        aiImageAttribs[16] = EGL_NONE;
    }

    auto m_qOpenGLContext = QOpenGLContext::currentContext();
    QEGLNativeContext qEglContext = qvariant_cast<QEGLNativeContext>(m_qOpenGLContext->nativeHandle());

    m_Display = qEglContext.display();

    if(m_Display ==EGL_NO_DISPLAY) {
        EGLint error_code = eglGetError();
        qDebug("EGLDisplay  is EGL_NO_DISPLAY  error_cord is %d",error_code);
    }

    m_Image = eglCreateImageKHR(m_Display, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, NULL, aiImageAttribs);
//    qDebug("-----MirrorVideoBuffer  eglCreateImageKHR");

    if(m_Image == EGL_NO_IMAGE_KHR) {
        checkGlError("eglCreateImageKHR");
        qDebug("m_Image  is EGL_NO_IMAGE_KHR m_bufferFD  is %d",m_bufferFD);
        handle_egl_error("eglCreateImageKHR");
    }
}

MirrorVideoBuffer::~MirrorVideoBuffer()
{
    qDebug("-----~MirrorVideoBuffer start");

    if((m_Display!=EGL_NO_IMAGE_KHR)&&(m_Display!=EGL_NO_DISPLAY)) {
        PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR =
                reinterpret_cast<PFNEGLDESTROYIMAGEKHRPROC>(
                    eglGetProcAddress("eglDestroyImageKHR"));
        qDebug("-----MirrorVideoBuffer  eglDestroyImageKHR");
        eglDestroyImageKHR(m_Display,m_Image);
    }

    m_bufferFD = -1;
    m_Display = EGL_NO_DISPLAY;
    m_Image = EGL_NO_IMAGE_KHR;
}


QVariant MirrorVideoBuffer::handle() const
{
    //qDebug("-----MirrorVideoBuffer::handle");
    return QVariant::fromValue<EGLImage>(m_Image);
}

void MirrorVideoBuffer::release()
{
    qDebug("-----MirrorVideoBuffer::release");
}

void MirrorVideoBuffer::unmap()
{
    qDebug("-----MirrorVideoBuffer::unmap");
}

uchar* MirrorVideoBuffer::map(MapMode mode, int* numBytes, int* bytesPerLine)
{
    qDebug("-----MirrorVideoBuffer::map");
    return nullptr;
}
