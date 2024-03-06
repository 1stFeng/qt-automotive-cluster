#ifndef __DEBUG_H__
#define __DEBUG_H__
#include <stdio.h>
#include <string>
#include <errno.h>

#define DEBUG 1

#if DEBUG
#define LOGD(fmt, args...) printf("[::%s]  " fmt "\r\n", __func__, ##args)
#else
#define LOGD(fmt, args...)
#endif
#define LOGI(fmt, args...) printf("[::%s]  " fmt "\r\n", __func__, ##args)

#define LOGE(fmt, args...) printf("[::%s]  Error: " fmt "\r\n", __func__, ##args)
#define DDBG(x) LOGD(#x " = %d", x)
#define XDBG(x) LOGD(#x " = 0x%x", x)
#define SDBG(x) LOGD(#x " = %s", x)
#define PDBG(x) LOGD(#x " = %p", x)

#include <chrono>

class CostTime
{
public:
    CostTime(const char *func_name)
    {
        _name.append(func_name);
        _start_time = std::chrono::steady_clock::now();
    }
    ~CostTime()
    {
        LOGD("%s cost: %f ms", _name.c_str(), elapsed());
    }

    inline double elapsed()
    {
        _end_time = std::chrono::steady_clock::now();
        std::chrono::duration<double> diff = _end_time - _start_time;
        return diff.count() * 1000;
    }

private:
    std::chrono::steady_clock::time_point _start_time;
    std::chrono::steady_clock::time_point _end_time;
    std::string _name;
};

#endif
