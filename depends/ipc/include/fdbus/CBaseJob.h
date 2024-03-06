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

#ifndef _CBASEJOB_H_
#define _CBASEJOB_H_

#include <memory>
#include <functional>
#include <condition_variable>
#include <mutex>
#include "common_defs.h"

namespace ipc {
namespace fdbus {
class CBaseWorker;

class CBaseJob
{
#define JOB_FORCE_RUN           (1 << 0)
#define JOB_IS_URGENT           (1 << 1)
public:
    typedef std::shared_ptr<CBaseJob> Ptr;
    CBaseJob(uint32_t flag = 0);
    virtual ~CBaseJob();
    void terminate(Ptr &ref);
    void forceRun(bool force)
    {
        if (force)
        {
            mFlag |= JOB_FORCE_RUN;
        }
        else
        {
            mFlag &= ~JOB_FORCE_RUN;
        }
    }

    bool forceRun() const
    {
        return !!(mFlag & JOB_FORCE_RUN);
    }

    bool urgent() const
    {
        return !!(mFlag & JOB_IS_URGENT);
    }

    bool sync() const
    {
        return !!mSyncReq;
    }

protected:
    /*-----------------------------------------------------------------------------
     * The virtual function should be implemented by subclass
     *---------------------------------------------------------------------------*/
    /*
     * this is the function implementing task of the job
     *
     * @iparam worker: the worker(thread) where the job is running
     * @return None
     */
    virtual void run(CBaseWorker *worker, Ptr &ref) {}

private:
    struct CSyncRequest
    {
        CSyncRequest(long int init_shared_cnt)
            : mInitSharedCnt(init_shared_cnt)
        {
        }

        long int mInitSharedCnt;
        std::condition_variable_any mWakeupSignal;
    };

    void urgent(bool active)
    {
        if (active)
        {
            mFlag |= JOB_IS_URGENT;
        }
        else
        {
            mFlag &= ~JOB_IS_URGENT;
        }
    }

    int32_t mFlag;
    std::mutex mSyncLock;
    CSyncRequest *mSyncReq;
    friend class CBaseWorker;
};

template <typename T>
T castToMessage(CBaseJob::Ptr &job_ref)
{
    return fdb_dynamic_cast_if_available<T>(job_ref.get());
}

typedef std::function<void(CBaseJob::Ptr &)> tJobCallable;
class CFunctionJob : public CBaseJob
{
public:
    CFunctionJob(tJobCallable job_func, uint32_t job_flag)
        : CBaseJob(job_flag)
        , mJobFunc(job_func)
    {
    }
    CFunctionJob(uint32_t flag = 0)
        : CBaseJob(flag)
    {
    }
    tJobCallable &getCallable()
    {
        return mJobFunc;
    }
    void setCallable(tJobCallable fn)
    {
        mJobFunc = fn;
    }
protected:
    void run(CBaseWorker *worker, Ptr &ref)
    {
        if (mJobFunc)
        {
            try // catch exception to avoid missing post processing
            {
                mJobFunc(ref);
            }
            catch (...)
            {
            }
        }
    }
private:
    tJobCallable mJobFunc;
};
}
}
#endif
