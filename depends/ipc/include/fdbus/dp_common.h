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

#ifndef __DP_COMMON_H__
#define __DP_COMMON_H__

#include <map>
#include <string>
#include <fdbus/CFdbSimpleSerializer.h>

namespace ipc {
namespace fdbus {

enum FdbDpObjId
{
    CTRL_OBJ = FDB_CUSTOM_OBJECT_BEGIN,
};

enum FdbDpMsgCode
{
    REQ_QUERY_TOPICS = 1,
    NTF_TOPIC_CREATED = 1,
};

struct CTopicInfo
{
    std::string mTopic;
    FdbMsgCode_t mTopicId;
    std::string mPoolName;
    FdbDomainId_t mDomainId;
};

typedef std::map<FdbMsgCode_t, std::map<std::string, CTopicInfo> > tTopicTbl;

class FdbMsgTopicItem : public IFdbParcelable
{
public:
    CTopicInfo mTopicInfo;

    std::string &topic()
    {
        return mTopicInfo.mTopic;
    }

    FdbMsgCode_t topic_id()
    {
        return mTopicInfo.mTopicId;
    }

    CTopicInfo &topic_info()
    {
        return mTopicInfo;
    }

    std::string &pool_name()
    {
        return mTopicInfo.mPoolName;
    }

    FdbDomainId_t domain_id()
    {
        return mTopicInfo.mDomainId;
    }

    void serialize(CFdbSimpleSerializer &serializer) const
    {
        serializer << mTopicInfo.mTopic
                   << mTopicInfo.mTopicId
                   << mTopicInfo.mPoolName
                   << mTopicInfo.mDomainId;
    }
    void deserialize(CFdbSimpleDeserializer &deserializer)
    {
        deserializer >> mTopicInfo.mTopic
                     >> mTopicInfo.mTopicId
                     >> mTopicInfo.mPoolName
                     >> mTopicInfo.mDomainId;
    }
};

class FdbMsgTopics : public IFdbParcelable
{
public:
    CFdbParcelableArray<FdbMsgTopicItem> &topic_list()
    {
        return mTopicList;
    }
    void fromTopicInfo(const tTopicTbl &info_list)
    {
        for (auto it_id = info_list.begin(); it_id != info_list.end(); ++it_id)
        {
            for (auto it_topic = it_id->second.begin(); it_topic != it_id->second.end(); ++it_topic)
            {
                auto topic_info = mTopicList.Add();
                topic_info->mTopicInfo = it_topic->second;
            }
        }
    }
    void toTopicInfo(tTopicTbl &info_list)
    {
        for (auto it = mTopicList.vpool().begin(); it != mTopicList.vpool().end(); ++it)
        {
            info_list[it->topic_id()][it->topic()] = it->topic_info();
        }
    }
    void serialize(CFdbSimpleSerializer &serializer) const
    {
        serializer << mTopicList;
    }
    void deserialize(CFdbSimpleDeserializer &deserializer)
    {
        deserializer >> mTopicList;
    }

private:
    CFdbParcelableArray<FdbMsgTopicItem> mTopicList;
};

class FdbMsgTopicsQuery : public IFdbParcelable
{
public:
    FdbMsgTopics mOwnedTopics;
    FdbMsgTopics mAvailableTopics;

    void serialize(CFdbSimpleSerializer &serializer) const
    {
        serializer << mOwnedTopics
                   << mAvailableTopics;
    }
    void deserialize(CFdbSimpleDeserializer &deserializer)
    {
        deserializer >> mOwnedTopics
                     >> mAvailableTopics;
    }
};

}
}

#endif

