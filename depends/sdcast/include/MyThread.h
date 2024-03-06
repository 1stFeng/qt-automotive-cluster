#ifndef __MY_THREAD_H
#define __MY_THREAD_H
#include "debug.h"

#if 0
#include <pthread.h>
class MyThread {
private:
    pthread_t id;
    static void *start_thread(void *arg) {
        MyThread *thread = (MyThread*)arg;
        thread->run();
        return (void *)0;
    }
public:
    virtual ~MyThread() {

    }
    virtual void run() {
        LOGE("success\n");
    }
    virtual void start() {
        LOGD("debug");
        pthread_create(&id, NULL, start_thread, (void *)this);
    }
    void join() {
        void* retval;
        pthread_join(id, &retval);
    }
};
#else
#include <thread>
class MyThread {
private:
    std::thread th_;
    static void *start_thread(void *arg) {
        MyThread *thread = (MyThread*)arg;
        thread->run();
        return (void *)0;
    }
public:
    virtual ~MyThread() {

    }
    virtual void run() {
        LOGE("success\n");
    }
    virtual void start() {
        LOGD("debug");
        th_ = std::thread(&MyThread::start_thread, (void *)this);
    }
    void join() {
        if (th_.joinable())
            th_.join();
    }
    void waite() {
        
    }
};
#endif
#endif