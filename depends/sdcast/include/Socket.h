#ifndef __SOCKET_H
#define __SOCKET_H
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <stddef.h>
#include <errno.h>
#include <string>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <memory>

#define UNIXSTR_PATH "/data/app-private/semidrive.server.socket"
#define AINET_PORT 6666
#define CLIENT_MAX 5
enum {
    COMM_TYPE_INVALID = -1,
    COMM_TYPE_SERVER,
    COMM_TYPE_CLIENT,
};

typedef struct sockaddr SA;

class FDTransport {
public:
    FDTransport(int fd): mSockFd(fd) {}
    virtual ~FDTransport(){}
    virtual int ReadFd(void *ptr, int nbytes, int *recv_fd) = 0;
    virtual int WriteFd(void *ptr, int nbytes, int send_fd) = 0;
protected:
    int mSockFd;

};

class UnixFDTransport: public FDTransport {
public:
    UnixFDTransport(int fd): FDTransport(fd) {}
    ~UnixFDTransport() {}
    int ReadFd(void *ptr, int nbytes, int *recv_fd);
    int WriteFd(void *ptr, int nbytes, int send_fd);
private:
    union {
        struct cmsghdr cm;
        char control[CMSG_SPACE(sizeof(int))];
    } control_un;
};

class DummyFDTransport: public FDTransport {
public:
    DummyFDTransport(int fd): FDTransport(fd) {}
    ~DummyFDTransport() {}
    int ReadFd(void *ptr, int nbytes, int *recv_fd);
    int WriteFd(void *ptr, int nbytes, int send_fd);
};

class Socket {
public:
    Socket();
    Socket(int sock): mSockFd(sock) {}
    virtual ~Socket();

    virtual void DeInit() = 0;
    virtual int Init(int comm_type, int chn, const char *addr=nullptr) = 0;
    ssize_t Write(const void *buf, size_t len);
    ssize_t Read(void *buf, size_t len);
    int ReadFd(void *ptr, int nbytes, int *recv_fd) {
        return mFDTransport->ReadFd(ptr, nbytes, recv_fd);
    }
    int WriteFd(void *ptr, int nbytes, int send_fd) {
        return mFDTransport->WriteFd(ptr, nbytes, send_fd);
    }
    virtual int WaitForClient() = 0;
    virtual int Connect() = 0;
    int Act();
    int WaitAct();
    int getSockFd() { return mSockFd;}
    inline void setFDTransportFunc(std::shared_ptr<FDTransport> fdt) {
        mFDTransport = fdt;
    }
    inline int FD() {
        return mSockFd;
    }

    static char* getUnilinkIp();
protected:
    int mType;
    int mSockFd;
    int mServerFd;
    int mChannel;
    bool mHasConnected;
    static char mUnilinkIp[128];
    std::shared_ptr<FDTransport> mFDTransport;
};

class UnixSocket: public Socket {
public:
    UnixSocket() {}
    ~UnixSocket() {}
    void DeInit(){
        close(mSockFd);
        close(mServerFd);
        unlink(mSocketPath.c_str());
    }
    int Init(int comm_type, int chn, const char *addr=nullptr);
    int WaitForClient();
    int Connect();
private:
    std::string mSocketPath;
    struct sockaddr_un serv_un;
};

class NetSocket: public Socket {
public:
    NetSocket() {}
    ~NetSocket() {}
    void DeInit(){
        close(mSockFd);
        close(mServerFd);
    }
    int Init(int comm_type, int chn, const char *addr=nullptr);
    int WaitForClient();
    int Connect();

private:
    struct sockaddr_in serv_in;
};


class RpmsgSocket: public Socket {
public:
    RpmsgSocket();
    ~RpmsgSocket();
    void DeInit();
    int Init(int comm_type, int chn, const char *addr=nullptr);
    int WaitForClient();
    int Connect();
};


#endif
