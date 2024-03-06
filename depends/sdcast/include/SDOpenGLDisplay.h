#include <stdio.h>
#include <stdlib.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <GLES/gl.h>
#include <GLES/glext.h>

class SDOpenGLDisplay
{

public:

    SDOpenGLDisplay(); 
    void updateImage(int buffer_fd);
    bool initGL();
    void drawWindow();

    
private:
    PFNEGLCREATEIMAGEKHRPROC pfneglCreateImageKHR = NULL;
    PFNEGLDESTROYIMAGEKHRPROC pfneglDestroyImageKHR = NULL;
    PFNGLEGLIMAGETARGETTEXTURE2DOESPROC pfnglEGLImageTargetTexture2DOES = NULL;

    EGLDisplay m_EGLDisplay;
    EGLSurface m_EGLSurface;
    EGLContext m_EGLContext;
    EGLImageKHR m_eglImage;

    GLuint m_TextureId;
    GLuint m_preTextureId;
};
