#ifndef OBJHANDLE_H
#define OBJHANDLE_H

#include "object.h"
#include <memory>

namespace evnt
{
class ObjHandle
{
    Object * m_ptr {nullptr};

public:
    ObjHandle(Object * ptr) : m_ptr(ptr) {}
    ~ObjHandle()
    {
        if(m_ptr != nullptr)
            m_ptr->deleteObj();
    }

    ObjHandle & operator=(const ObjHandle &) = delete;
    ObjHandle(const ObjHandle &)             = delete;

    Object & operator*() const { return *m_ptr; }
    Object * operator->() const { return m_ptr; }
             operator Object *() const { return m_ptr; }
             operator const Object *() const { return m_ptr; }
             operator bool() const { return m_ptr != nullptr; }
    Object * getPtr() const { return m_ptr; }

    uint32_t getInstanceId() const { return m_ptr != nullptr ? m_ptr->getInstanceId() : 0; }

public: /* internal */
    uint32_t m_link_key {0};
    void     nullify()
    {
        if(m_ptr != nullptr)
            m_ptr->deleteObj();

        m_ptr = nullptr;
    }
};

using PObjHandle = std::shared_ptr<ObjHandle>;

template<class T>
T * dynamic_ohdl_cast(PObjHandle ptr)
{
    Object * o         = ptr->getPtr();
    int32_t  type_id   = ptr->getPtr()->getClassIDVirtual();
    T *      castedPtr = (T *)(o);

    if(castedPtr && Object::IsDerivedFromClassID(type_id, T::GetClassIDStatic()))
        return castedPtr;

    return nullptr;
}
}   // namespace evnt

#endif   // OBJHANDLE_H
