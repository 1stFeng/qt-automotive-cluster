#ifndef __SURFACEINFO_PUBLIC_H
#define __SURFACEINFO_PUBLIC_H
#include <stdint.h>
namespace sdm {


class Rect {
public:
    Rect(): left(0), top(0), right(0), bottom(0)  {
    }
    Rect(int l, int t, int r, int b): left(l), top(t), right(r), bottom(b) {

    }
    Rect(Rect &r) {
        left = r.left;
        top = r.top;
        right = r.right;
        bottom = r.bottom;
    }

    Rect& operator=(const Rect& r) {
        if (this != &r) {
            left = r.left;
            top = r.top;
            right = r.right;
            bottom = r.bottom;
        }
        return *this;
    }

    inline int getWidth() const {return right - left;}
    inline int getHeight() const {return bottom - top;}
    int left;
    int top;
    int right;
    int bottom;
};


struct Surface {    
    uint64_t phy_addr;
    int width;
    int height;
    int stride;
    size_t size;
    Rect source;
    Rect display;
    int format;
    int prime_fd;
    int cmd;
    uint32_t fourcc_format;
    uint64_t usage;
    uint32_t pitches[4];
    uint32_t offsets[4];
    uint64_t modifiers[4];
    bool with_modifiers;
}__attribute__((__packed__));


enum {
    MSG_NOTIFY_SURFACE_INFO = 0,
	MSG_NOTIFY_HAS_CONTENT = 1,
	MSG_NOTIFY_NO_CONTENT,
        MSG_NOTIFY_HEART_BEAT,
	MSG_NOTIFY_MAX = 16,
};

}
#endif
