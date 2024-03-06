#ifndef VIDEO_DISPLAY
#define VIDEO_DISPLAY

#include <unistd.h>

#include "MyThread.h"

typedef void* notify_update_t;
typedef void (*update_callback_t)(int,notify_update_t);


class VideoDisplay: public MyThread
{
public: 
    VideoDisplay(const char* video_name);
    ~VideoDisplay();
    int init();
    int release();
    int startDisplay();
    int stopDisplay();
    void returnShowData();
    void setUpdateCall(update_callback_t func,notify_update_t arg) {
        mUpdateCallback = func;
        mUpdateObj = arg;
    }

private:
    int video_fd = -1;
    bool m_isDisplay = false;
    int prime_fds[3] = {-1,-1,-1};
    
    int setFormat(int pixelFormat,int width,int height);
    int getFrame();
    int getBufferFd(int index);
    bool isShow;
    void run();
    update_callback_t mUpdateCallback;
    notify_update_t mUpdateObj;
};

#endif //VIDEO_DISPLAY