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

#ifndef __CDATAPOOL_H__
#define __CDATAPOOL_H__

#include <list>
#include <map>
#include <functional>
#include <string>
#include <vector>
#include <fdbus/CNotificationCenter.h>
#include <fdbus/CFdbIfNameServer.h>
#include <fdbus/CFdbMsgDispatcher.h>
#include <fdbus/dp_common.h>

#define FDB_DP_DEFAULT_TOPIC_ID     (FDB_EVENT_ID_MASK - 1)
#define FDB_DP_DEFAULT_TOPIC_STR    "----"

namespace ipc {
namespace fdbus {

class CDpClient;
class CDpServer;
class CBaseWorker;
class CFdbBaseContext;
class CDataPool;
class CServiceMonitor;

struct CTopicAvailableInfo
{
    std::string mTopic;
    FdbMsgCode_t mTopicId;
    CDataPool *mPool;
    bool mAvailable;
};

typedef std::function<void(CTopicAvailableInfo &)> tTopicAvailableFn;
typedef std::function<void(CBaseJob::Ptr &, CDataPool *)> tDataPoolCallbackFn;
typedef void *tTopicAvailableHandler;

class CDataPool
{
public:
    CDataPool(FdbDomainId_t domain_id = FDB_DEFAULT_DOMAIN, const char *name = 0);
    bool start(CBaseWorker *handle_worker = 0,
               CFdbBaseContext *context = 0,
               bool topic_owner = false,
               bool topic_borrower = true);
    tTopicAvailableHandler registerTopicAvailableListener(tTopicAvailableFn listener);
    bool createData(const char *topic,
                    IFdbMsgBuilder &data,
                    tDataPoolCallbackFn modify_listener = 0);
    bool createData(const char *topic,
                    const uint8_t *data = 0,
                    int32_t size = 0,
                    tDataPoolCallbackFn modify_listener = 0);
    bool destroyData(const char *topic);
    bool publishData(const char *topic, IFdbMsgBuilder &data,
                     bool force_update = true, EFdbQOS qos = FDB_QOS_DEFAULT);
    bool publishData(const char *topic, const void *data = 0, int32_t size = 0,
                     bool force_update = true, EFdbQOS qos = FDB_QOS_DEFAULT);
    bool subscribeData(const char *topic, tDataPoolCallbackFn publish_listener);

    bool createData(FdbMsgCode_t topic_id,
                    IFdbMsgBuilder &data,
                    tDataPoolCallbackFn modify_listener = 0);
    bool createData(FdbMsgCode_t topic_id,
                    const uint8_t *data = 0,
                    int32_t size = 0,
                    tDataPoolCallbackFn modify_listener = 0);
    bool destroyData(FdbMsgCode_t topic_id);
    bool publishData(FdbMsgCode_t topic_id, IFdbMsgBuilder &data,
                     bool force_update = true, EFdbQOS qos = FDB_QOS_DEFAULT);
    bool publishData(FdbMsgCode_t topic_id, const void *data = 0, int32_t size = 0,
                     bool force_update = true, EFdbQOS qos = FDB_QOS_DEFAULT);
    bool subscribeData(FdbMsgCode_t topic_id, tDataPoolCallbackFn publish_listener);

    bool createData(FdbMsgCode_t topic_id,
                    const char *topic,
                    IFdbMsgBuilder &data,
                    tDataPoolCallbackFn modify_listener = 0);
    bool createData(FdbMsgCode_t topic_id,
                    const char *topic,
                    const uint8_t *data = 0,
                    int32_t size = 0,
                    tDataPoolCallbackFn modify_listener = 0);
    bool destroyData(FdbMsgCode_t topic_id, const char *topic);
    bool publishData(FdbMsgCode_t topic_id, const char *topic, IFdbMsgBuilder &data,
                     bool force_update = true, EFdbQOS qos = FDB_QOS_DEFAULT);
    bool publishData(FdbMsgCode_t topic_id, const char *topic, const void *data = 0, int32_t size = 0,
                     bool force_update = true, EFdbQOS qos = FDB_QOS_DEFAULT);
    bool subscribeData(FdbMsgCode_t topic_id, const char *topic, tDataPoolCallbackFn publish_listener);
    /*
     * subscribe all data from the pool without specifying one by one
     */
    bool subscribeData(tDataPoolCallbackFn publish_listener);
    /*
     * create all data from the pool without specifying one by one
     */
    bool createData(tDataPoolCallbackFn modify_listener = 0);
    bool destroyData();

    CBaseWorker *getHandleWorker() const
    {
        return mHandleWorker;
    }
    CFdbBaseContext *getContext() const
    {
        return mContext;
    }

    const std::string &name() const
    {
        return mName;
    }

    void name(const char *n)
    {
        if (n)
        {
            mName = n;
        }
    }

    void getTopicList(FdbMsgTopics *owned_topics, FdbMsgTopics *available_topics);

    FdbDomainId_t domainId() const
    {
        return mDomainId;
    }

private:
    typedef void (CDataPool::*tRemoteCallback)(CBaseJob::Ptr &msg_ref);
    typedef std::list<CDpClient *> tDpClientTbl;
    typedef std::map<FdbInstanceId_t, std::map<std::string, std::vector<tDataPoolCallbackFn> > > tSubscriptionHandleTbl;
    typedef std::map<CDpClient *, tSubscriptionHandleTbl> tClientSubscriptionHandleTbl;

    std::string mName;
    FdbDomainId_t mDomainId;
    tDpClientTbl mDpClientTbl;
    CDpServer *mDpServer;
    tSubscriptionHandleTbl mPendingSubscriptionHandle;
    tClientSubscriptionHandleTbl mClientSubscriptionHandle;
    CBaseWorker *mHandleWorker;
    CFdbBaseContext *mContext;
    CBaseNotificationCenter<CTopicAvailableInfo> mTopicAvailableNtfCenter;
    CServiceMonitor *mSvcMonitor;

    void connect(FdbMsgAddressList &msg_addr_list);
    CDpClient *findClient(const char *host_name, const char *svc_name, FdbInstanceId_t instance_id);
    void unregister(CDpClient *client);
    void callCreateData(CBaseWorker *worker, CMethodJob<CDataPool> *job, CBaseJob::Ptr &ref);
    void callDestroyData(CBaseWorker *worker, CMethodJob<CDataPool> *job, CBaseJob::Ptr &ref);
    void callSubscribeData(CBaseWorker *worker, CMethodJob<CDataPool> *job, CBaseJob::Ptr &ref);
    void callGetTopicList(CBaseWorker *worker, CMethodJob<CDataPool> *job, CBaseJob::Ptr &ref);

    void onTopicCreated(FdbMsgCode_t topic_id, const char *topic, CDpClient *client);
    void notifyTopicCreated(const char *topic, FdbMsgCode_t topic_id, bool created);

    bool doPublishData(CFdbMessage *msg);
    void doGetTopicList(FdbMsgTopics *owned_topics, FdbMsgTopics *available_topics);
    static bool topicCreated(const char *topic, FdbMsgCode_t topic_id, tTopicTbl &topic_tbl);
    void doSubscribeData(FdbMsgCode_t topic_id, const char *topic, tDataPoolCallbackFn publish_listener);
    void saveSubscribeHandleForClient(CDpClient *client,
                                      FdbMsgCode_t topic_id,
                                      const char *topic,
                                      tDataPoolCallbackFn publish_listener);
    static bool topicMatched(FdbMsgCode_t id_intended, FdbMsgCode_t it_stored);
    static bool topicMatched(const char *topic_intended, const std::string &topic_stored);

    friend class CServiceMonitor;
    friend class CDpClient;
    friend class CDpServer;
    friend class createDataJob;
    friend class destroyDataJob;
    friend class subscribeDataJob;
    friend class getTopicListJob;
};

}
}
#endif
