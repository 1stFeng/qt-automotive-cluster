#ifndef __FRAME_RECEIVER_H
#define __FRAME_RECEIVER_H
#include <stdio.h>
#include <unistd.h>
#include "debug.h"
#include "SurfaceInfo_public.h"
#include "DisplayController.h"
#include "Communication.h"
#include <memory.h>
#include <map>

#ifndef USE_VIDEO_DEV
#define USE_VIDEO_DEV
#include "VideoDisplay.h"
#endif

typedef void* update_data_t;
typedef void (*update_call_t)(sdm::Surface *, update_data_t);

class DrmInfo {
public:
    uint64_t addr;
    int fd;
    void *virtualAddr;
};

class FrameReceiver2: public MyThread
{
public:
    FrameReceiver2(): gThreadStop(false), mUpdateCall(nullptr), mUpdateData(nullptr)
    {}
    ~FrameReceiver2() {
        gThreadStop = true;
        join();
    }

    void onShowDone();
    void Init(DisplayController *dc);
    void run(void);
    void setUpdateCall(update_call_t func, update_data_t arg) {
        mUpdateCall = func;
        mUpdateData = arg;
    }
    void reconnect();
    
    uint32_t mScreenState;
    int drm_fd_;

    static void bufferFdUpdate(int fd,update_data_t data);
private:
    std::shared_ptr<Communication> mSock;
    bool gThreadStop;
    DisplayController *mDC;
    update_call_t mUpdateCall;
    update_data_t mUpdateData;
    std::map<uint64_t,DrmInfo *> mDrms;
    #ifdef USE_VIDEO_DEV
    std::shared_ptr<VideoDisplay> m_VideoDisplay;
    void notifyAppUpdate(int fd);
    
    #endif

};

#endif
