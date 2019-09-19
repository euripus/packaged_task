#include "gameobject.h"
#include "gameobjectmanager.h"
#include "exception.h"

#include <any>
#include <iostream>

namespace evnt
{
IMPLEMENT_STRUCT(GameObject, Object)

void GameObject::addComponent(PObjHandle com)
{
    assert(com->getPtr() != nullptr);

    auto cmp_ptr = dynamic_ohdl_cast<evnt::Component>(com);

    cmp_ptr->setGameObjectInternal(this);
    cmp_ptr->sendMessage(CmpMsgsTable::mDidAddComponent, {});
    mComponents[cmp_ptr->getClassIDVirtual()] = com;
}

Component * GameObject::queryComponentImplementation(int32_t classID) const
{
    assert(classID != -1);

    Component * res{nullptr};

    auto id = mComponents.find(classID);
    if(id != mComponents.end())
    {
        res = dynamic_ohdl_cast<evnt::Component>(id->second);
    }

    return res;
}

void GameObject::sendMessage(ClassIDType sender, CmpMsgsTable::msg_id messageIdentifier, std::any msg_data)
{
    assert(messageIdentifier != CmpMsgsTable::mUndefined);

    for(auto & [key, cmp]: mComponents)
    {
        auto cid = static_cast<ClassIDType>(key);
        if(sMsgHandler.hasMessageCallback(messageIdentifier, cid) && cid != sender)
        {
            auto cmp_ptr = dynamic_ohdl_cast<evnt::Component>(cmp);
            sMsgHandler.handleMessage(cmp_ptr, messageIdentifier, msg_data);
        }
    }
}

void GameObject::dump(int indentLevel) const
{
    std::cout << getClassString() << " { " << std::endl;
    std::cout << std::string(4 * (indentLevel + 1), ' ') << "type: " << getClassIDVirtual() << std::endl;
    std::cout << std::string(4 * (indentLevel + 1), ' ') << "ID: " << getInstanceId() << std::endl;

    std::cout << std::string(4 * (indentLevel + 1), ' ') << std::string("std::unique_ptr<mComponents>")
              << "{";
    if(mComponents.empty())
    {
        std::cout << "0}";
        std::cout << std::endl;
    }
    else
    {
        std::cout << std::endl;
        for(auto & [key, cmp]: mComponents)
        {
            auto c_ptr = cmp->getPtr();
            std::cout << std::string(4 * (indentLevel + 2), ' ') << "[type: ";
            std::cout << key;
            std::cout << ", ID: " << c_ptr->getInstanceId();
            std::cout << "]" << std::endl;
        }
        std::cout << std::string(4 * (indentLevel + 1), ' ') << "}";
        std::cout << std::endl;
    }
    std::cout << std::string(4 * indentLevel, ' ') << "}";
}

void GameObject::write(OutputMemoryStream & inMemoryStream, const GameObjectManager & gmgr) const
{
    inMemoryStream.write(getClassIDVirtual());
    inMemoryStream.write(getInstanceId());

    if(mComponents.empty())
        inMemoryStream.write(0);
    else
    {
        inMemoryStream.write<uint32_t>(mComponents.size());
        for(auto & [key, cmp]: mComponents)
        {
            auto c_ptr = cmp->getPtr();
            inMemoryStream.write(key);
            inMemoryStream.write(c_ptr->getInstanceId());
        }
    }
}

void GameObject::read(const InputMemoryStream & inMemoryStream, GameObjectManager & gmgr)
{
    int32_t type_id{0};
    inMemoryStream.read(type_id);
    uint32_t inst_id{0};
    inMemoryStream.read(inst_id);

    setInstanceId(inst_id);

    uint32_t size{0};
    inMemoryStream.read(size);
    if(size > 0)
    {
        while(size > 0)
        {
            int      key{0};
            uint32_t cmp_inst{0};

            inMemoryStream.read(key);
            inMemoryStream.read(cmp_inst);

            auto temp_handle        = std::make_shared<ObjHandle>(nullptr);
            temp_handle->m_link_key = cmp_inst;
            mComponents[key]        = temp_handle;

            --size;
        }
    }
}

void GameObject::link(GameObjectManager & gmgr, const std::map<uint32_t, uint32_t> & id_remap)
{
    if(mComponents.empty())
        return;

    for(auto & [key, cmp]: mComponents)
    {
        uint32_t c_inst = cmp->m_link_key;

        if(id_remap.find(c_inst) == id_remap.end())
            EV_EXCEPT("Trying linking not exist object");

        auto ptr = gmgr.getObject(id_remap.at(c_inst));

        mComponents[key] = ptr;
    }
}
}   // namespace evnt
