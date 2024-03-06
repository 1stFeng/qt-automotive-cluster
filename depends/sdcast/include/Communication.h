#ifndef __COMMUNICATE__H
#define __COMMUNICATE__H
#include <stddef.h>
#include <memory>
#include "Socket.h"

class Communication
{
public:
    Communication() {
        //TODO
        if(access("/dev/ivi",F_OK) == 0) {
            printf("-----------create  RpmsgSocket");
            mSock = std::make_shared<RpmsgSocket>();
        } else {
            printf("-----------create  NetSocket");
            mSock = std::make_shared<NetSocket>();
        }
    };
    virtual ~Communication(){};
    virtual int Init(int comm_type, int chn, const char *addr=nullptr) {
        return mSock->Init(comm_type, chn, addr);
    };
    virtual void DeInit() {
        mSock->DeInit();
    };
    virtual int Write(const void *buf, size_t len) {
        return mSock->Write(buf, len);
    };
    virtual int Read(void *buf, size_t len) {
        return mSock->Read(buf, len);
    }
    virtual int ReadFd(void *ptr, int nbytes, int *recv_fd) {
        return mSock->ReadFd(ptr, nbytes, recv_fd);
    }
    virtual int WriteFd(void *ptr, int nbytes, int send_fd) {
        return mSock->WriteFd(ptr, nbytes, send_fd);
    }
    virtual int WaitForClient() {
        return mSock->WaitForClient();
    }
    virtual int Connect() {
        return mSock->Connect();
    }
protected:
    std::shared_ptr<Socket> mSock;
};

#endif