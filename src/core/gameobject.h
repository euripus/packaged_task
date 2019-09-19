#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "cmpmsgs.h"
#include "component.h"
#include "objhandle.h"
#include <cassert>

namespace evnt
{
class GameObject : public Object
{
public:
    CLASS_IMPLEMENT(GameObject, Object)

    GameObject() = default;

    template<class T>
    T & getComponent() const;
    template<class T>
    PObjHandle getComponentPtr() const;
    void       addComponent(PObjHandle com);

    Component * queryComponentImplementation(int32_t classID) const;

    void sendMessage(ClassIDType sender, CmpMsgsTable::msg_id messageIdentifier, std::any msg_data);

    inline static CmpMsgsTable sMsgHandler;

private:
    std::unordered_map<int32_t, PObjHandle> mComponents;   // [type_id, obj_handle]
};

template<class T>
T & GameObject::getComponent() const
{
    auto com = queryComponentImplementation(T::GetClassIDStatic());

    assert(com != nullptr);
    return *static_cast<T *>(com);
}

template<class T>
PObjHandle GameObject::getComponentPtr() const
{
    auto id = mComponents.find(T::GetClassIDStatic());

    assert(id != mComponents.end());
    return id->second;
}
}   // namespace evnt

#endif   // GAMEOBJECT_H
