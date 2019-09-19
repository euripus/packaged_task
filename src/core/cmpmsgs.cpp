#include "cmpmsgs.h"
#include "component.h"
#include <cassert>

namespace evnt
{
void CmpMsgsTable::registerMessageCallback(msg_id mid, ClassIDType classid, call_ptr fnct)
{
    key_type k {mid, classid};
    mCallBackMap[k] = std::move(fnct);
}

bool CmpMsgsTable::hasMessageCallback(msg_id mid, ClassIDType classid) const
{
    key_type k {mid, classid};

    return mCallBackMap.find(k) != mCallBackMap.end();
}

void CmpMsgsTable::handleMessage(Component * rec, msg_id id, std::any msg_data) const
{
    assert(rec != nullptr);

    auto cid = static_cast<ClassIDType>(rec->getClassIDVirtual());
    key_type    k {id, cid};

    const auto & it = mCallBackMap.find(k);
    it->second(rec, id, std::move(msg_data));
}
}   // namespace evnt
