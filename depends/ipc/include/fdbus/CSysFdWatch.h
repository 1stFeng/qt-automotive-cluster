/*
 * Copyright (C) 2015   Jeremy Chen jeremy_cz@yahoo.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _CSYSFDWATCH_H_
#define _CSYSFDWATCH_H_

#include <list>
#include "common_defs.h"

namespace ipc {
namespace fdbus {
class CFdEventLoop;
class CSysFdWatch
{
public:
    /*
     * Create FD watch
     *
     * @iparam fd - the file descriptor to watch
     *     Warning!!! the fd should be non-block when reading from onInput().
     *     If you are not sure, non-block it manully as follows:
     *     fcntl(fd, F_SETFL, O_NONBLOCK)
     * @iparam flag - or-ed by
     *     POLLIN
     *     POLLOUT
     *     POLLHUP
     *     POLLERR
     *  @return - NA
     */
    CSysFdWatch(int fd, uint32_t flags);

    virtual ~CSysFdWatch();

    /*
     * query enable status.
     */
    bool enable()
    {
        return mEnable;
    }

    /*
     * enable the watch. Internally used only!!!
     */
    void enable(bool enb);

    /*
     * set polling flag of the watch. Internally used only!!!
     */
    uint32_t flags()
    {
        return mFlags;
    }

    /*
     * query flag.
     */
    void flags(uint32_t flgs)
    {
        mFlags = flgs;
    }

    /*
     * Get file descriptor of the watch.
     */
    int descriptor()
    {
        return mFd;
    }

    /*
     * Set file descriptor of the watch.
     */
    void descriptor(int fd)
    {
        mFd = fd;
    }

    bool fatalError() const
    {
        return mFatalError;
    }

    void fatalError(bool enb);

    uint32_t getPendingChunkSize() const
    {
        return (uint32_t)mOutputChunkList.size();
    }

protected:
    static const int32_t mDefaultInputChunkSize = ((1 << 15) - 0xbad);
    static const int32_t mMaxInputChunkSize = mDefaultInputChunkSize * 16;
    struct CInputDataChunk
    {
        uint8_t *mBuffer;
        int32_t mSize;
        int32_t mOffset;
        int32_t mConsumed;
        CInputDataChunk();
        ~CInputDataChunk();
        int32_t getTailSize()
        {
            return mSize - mOffset - mConsumed;
        }
        uint8_t *getBufferHead()
        {
            return mBuffer + mOffset;
        }
        uint8_t *getBufferTail()
        {
            return mBuffer + mOffset + mConsumed;
        }
        int32_t getExpectedSize(int32_t total_size)
        {
            return (total_size > mConsumed) ? (total_size - mConsumed) : 0;
        }
        int32_t extractData(uint8_t *output, int32_t size);
        bool checkBufferSize(int32_t size_expected);
        void enable(bool enb);
    };

    struct COutputDataChunk
    {
        uint8_t *mBuffer;
        int32_t mSize;
        int32_t mConsumed;
        uint8_t *mLogBuffer;
        int32_t mLogSize;
        COutputDataChunk()
            : mBuffer(0)
            , mSize(0)
            , mConsumed(0)
            , mLogBuffer(0)
            , mLogSize(0)
        {}
        COutputDataChunk(const uint8_t *msg_buffer, int32_t msg_size, int32_t consumed,
                         uint8_t *log_buffer, int32_t log_size);
        ~COutputDataChunk();
    };
    typedef std::list<COutputDataChunk *> tOutputChunkList;

    CInputDataChunk mInputChunk;
    tOutputChunkList mOutputChunkList;
    /*-----------------------------------------------------------------------------
     * The virtual function should be implemented by subclass
     *---------------------------------------------------------------------------*/
    /*
     * callback invoked when POLLIN is set after poll()
     * You MUST read from fd as follows:
     *     read(descriptor(), data, size)
     * Otherwise the poll() will not be blocked.
     * Also, the read() should be non-block!!!
     */
    virtual void onInput() {}

    /*
     * callback invoked when POLLOUT is set after poll()
     * You MUST write to fd as follows:
     *     write(descriptor(), data, size)
     * Otherwise the poll() will not be blocked.
     * Also, the write() should be non-block!!!
     */
    virtual void onOutput() {}

    /*
     * callback invoked when POLLHUP is set after poll()
     */
    virtual void onHup()
    {
        enable(false);
    }

    /*
     * callback invoked when POLLERROR is set after poll()
     */
    virtual void onError()
    {
        fatalError(true);
    }

    virtual int32_t convertRetEvents(int32_t revents)
    {
        return revents;
    }

    virtual int32_t onInputReady()
    {
        return 0;
    }

    virtual int32_t writeStream(const uint8_t *data, int32_t size)
    {
        return -1;
    }

    virtual int32_t readStream(uint8_t *data, int32_t size)
    {
        return -1;
    }

    virtual void freeOutputBuffer(uint8_t *buffer)
    {
        if (buffer)
        {
            delete[] buffer;
        }
    }

    void submitOutput(const uint8_t *msg_buffer, int32_t msg_size,
                      uint8_t *log_buffer, int32_t log_size);

    void updateFlags(uint32_t mask, uint32_t value);

private:
    void eventloop(CFdEventLoop *loop)
    {
        mEventLoop = loop;
    }

    CFdEventLoop *eventloop()
    {
        return mEventLoop;
    }

    void clearOutputChunkList();

    void processInput();
    void processOutput();

    int mFd;
    uint32_t mFlags;
    bool mEnable;
    bool mFatalError;
    CFdEventLoop *mEventLoop;
    int32_t mInputRecursiveDepth;
    
    friend class CFdEventLoop;
    friend class CNotifyFdWatch;
};

}
}
#endif
