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

#ifndef __CFDINTERFACETOKEN_H__
#define __CFDINTERFACETOKEN_H__

#include <fdbus/CFdbSimpleSerializer.h>

namespace ipc {
namespace fdbus {
class FdbMsgTokens : public IFdbParcelable
{
public:
    void add_tokens(const std::string &token)
    {
        mTokens.Add(token);
    }
    void add_tokens(const char *token)
    {
        mTokens.Add(token);
    }
    void clear_tokens()
    {
        mTokens.clear();
    }
    CFdbParcelableArray<std::string> &tokens()
    {
        return mTokens;
    }
    void serialize(CFdbSimpleSerializer &serializer) const
    {
        serializer << mTokens;
    }
    void deserialize(CFdbSimpleDeserializer &deserializer)
    {
        deserializer >> mTokens;
    }
    
private:
    CFdbParcelableArray<std::string> mTokens;
};
}
}
#endif

