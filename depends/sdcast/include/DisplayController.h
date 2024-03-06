#ifndef __DISPLAY_CONTROLL_H
#define __DISPLAY_CONTROLL_H
#include <stdio.h>
#include <unistd.h>
#include "debug.h"
#include "Surface.h"
#include "MyThread.h"
#include "DrmDisplay.h"

#include <pthread.h>
#include <mutex>
#include <condition_variable>

class Display;
class DisplayController;

enum {
    DISPLAY_STATE_HINT_DPMS = 0,
    DISPLAY_STATE_HINT_TRANSFORM,
    DISPLAY_STATE_HINT_DISPLAY_RECT,
    DISPLAY_STATE_HINT_CROP
};

class DisplayFlushThread: public MyThread
{
public:
    DisplayFlushThread() =default;
    ~DisplayFlushThread()
    {
        DeInit();
    }

    inline void Init(std::shared_ptr<Display> disp) {
        mDisplay = disp;
        gThreadStop = false;
    }
    void DeInit() {
        LOGD("call");
        gThreadStop = true;
        frame_ready_ = false;
        cv_.notify_one();
        join();
    }

    int PostFrame(std::shared_ptr<sdm::Surface> surface);
private:
    void run();

private:
    bool gThreadStop;
    std::mutex iomux_;
    std::condition_variable cv_;
    bool frame_ready_;
    std::shared_ptr<sdm::Surface> mSurface;
    std::shared_ptr<Display> mDisplay;
};
class DisplayState;

class DisplayController
{
public:
public:
    DisplayController()
    {
    }
    ~DisplayController()
    {
    }
    int display_sync(std::shared_ptr<sdm::Surface> surface);
	int post_frame(DrmFrame *frame);
    void setDisplay(std::shared_ptr<Display> display);
    void blank(bool enabled);
    int setCrop(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
    int setDisplayRectange(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
    std::shared_ptr<Display> mDisplay;
	sdm::Rect& getDisplayRectange();
private:
    DisplayFlushThread mFlushThread;

    int getDisplayWidth(sdm::Rect *);
    int getDisplayHeight(sdm::Rect *);

};
class DisplayState {
public:

    int state_changed; // bits:
    int dpms;
    int transform;
    sdm::Rect display;
    sdm::Rect crop;
};
class Display
{
public:
    virtual ~Display() {}
    virtual int Init() = 0;
    virtual int display(std::shared_ptr<sdm::Surface> surface) = 0;
    virtual int blank(bool enabled) = 0;
    DisplayState state;
protected:
    std::mutex refresh_mutex_;

};

class DisplayDrm : public Display
{
public:
	DisplayDrm() {}
    ~DisplayDrm();
    int Init();
	void DeInit();
    int display(std::shared_ptr<sdm::Surface> surface);
    int blank(bool enabled);
	inline int getFd() {
		return backend_.fd();
	}
	DrmBackend backend_;
	int crtc_; //crtc id
private:

    int getDisplayWidth(sdm::Rect *);
    int getDisplayHeight(sdm::Rect *);

    std::unique_ptr<DrmFrame> new_frame;
    std::unique_ptr<DrmFrame> on_screen;

};

class DisplayDummy : public Display
{
public:
    ~DisplayDummy();
    int Init();
    int display(std::shared_ptr<sdm::Surface> surface);
    int blank(bool enabled);

private:

    int getDisplayWidth(sdm::Rect *);
    int getDisplayHeight(sdm::Rect *);

};

#endif
