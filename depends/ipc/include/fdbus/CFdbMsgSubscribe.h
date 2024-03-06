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

#ifndef __CFDBMSGSUBSCRIBEITEM_H__
#define __CFDBMSGSUBSCRIBEITEM_H__

#include <string>
#include "CFdbSimpleSerializer.h"
#include "IFdbMsgBuilder.h"
#include <vector>
#include <fdbus/CFdbSimpleMsgBuilder.h>

namespace ipc {
namespace fdbus {
class CFdbMsgSubscribeItem : public IFdbParcelable
{
public:
    CFdbMsgSubscribeItem()
        : mOptions(0)
    {
    }
    int32_t msg_code() const
    {
        return mCode;
    }
    void set_msg_code(int32_t msg_code)
    {
        mCode = msg_code;
    }
    bool has_topic() const
    {
        return !!(mOptions & mMaskFilter);
    }
    const std::string &topic() const
    {
        return mFilter;
    }
    void set_topic(const char *topic)
    {
        mFilter.assign(topic);
        mOptions |= mMaskFilter;
    }
    void serialize(CFdbSimpleSerializer &serializer) const
    {
        serializer << mCode << mOptions;
        if (mOptions & mMaskFilter)
        {
            serializer << mFilter;
        }
    }
    void deserialize(CFdbSimpleDeserializer &deserializer)
    {
        deserializer >> mCode >> mOptions;
        if (mOptions & mMaskFilter)
        {
            deserializer >> mFilter;
        }
    }
protected:
    void toString(std::ostringstream &stream) const
    {
        stream << "event : " << mCode
               << ", topic : " << mFilter;
    }
    
private:
    int32_t mCode;
    std::string mFilter;
    uint8_t mOptions;
        static const uint8_t mMaskFilter = 1 << 0;
};

class CFdbMsgTable : public IFdbParcelable
{
public:
    CFdbParcelableArray<CFdbMsgSubscribeItem> &subscribe_tbl()
    {
        return mSubscribeTbl;
    }
    CFdbMsgSubscribeItem *add_subscribe_tbl()
    {
        return mSubscribeTbl.Add();
    }

    void serialize(CFdbSimpleSerializer &serializer) const
    {
        serializer << mSubscribeTbl;
    }
    void deserialize(CFdbSimpleDeserializer &deserializer)
    {
        deserializer >> mSubscribeTbl;
    }
protected:
    void toString(std::ostringstream &stream) const
    {
        stream << "mName:"; mSubscribeTbl.format(stream);
    }
private:
    CFdbParcelableArray<CFdbMsgSubscribeItem> mSubscribeTbl;
};

}
}
#endif

