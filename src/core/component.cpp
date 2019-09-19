#include "component.h"
#include "exception.h"
#include "gameobject.h"
#include "gameobjectmanager.h"

#include <iostream>

namespace evnt
{
IMPLEMENT_STRUCT(Component, Object)

Component::Component() {}

void Component::setGameObjectInternal(GameObject * go)
{
    mGameObj = go;
}

void Component::sendMessage(CmpMsgsTable::msg_id messageIdentifier, std::any msg_data)
{
    if(mGameObj)
    {
        ClassIDType cid = static_cast<ClassIDType>(getClassIDVirtual());
        mGameObj->sendMessage(cid, messageIdentifier, msg_data);
    }
}

void Component::onAddMessage(Component * rec, CmpMsgsTable::msg_id id, std::any msg_data)
{
    auto tt = static_cast<ClassIDType>(rec->getClassIDVirtual());

    switch(id)
    {
        case CmpMsgsTable::msg_id::mDidAddComponent: {
            std::cout << "OnAddComponent type:" << tt << " ID:" << rec->getInstanceId()
                      << " this ID:" << getInstanceId() << std::endl;
        }
    }
}

void Component::dump(int indentLevel) const
{
    std::cout << getClassString() << " { " << std::endl;
    std::cout << std::string(4 * (indentLevel + 1), ' ') << "type: " << getClassIDVirtual() << std::endl;
    std::cout << std::string(4 * (indentLevel + 1), ' ') << "ID: " << getInstanceId() << std::endl;

    std::cout << std::string(4 * (indentLevel + 1), ' ') << "mGameObj {" << std::endl;
    if(mGameObj == nullptr)
    {
        std::cout << "nullptr";
    }
    else
    {
        std::cout << std::string(4 * (indentLevel + 2), ' ');
        std::cout << "Ptr object ID: " << mGameObj->getInstanceId();
        std::cout << std::endl;
        std::cout << std::string(4 * (indentLevel + 1), ' ');
    }
    std::cout << "}" << std::endl;
    std::cout << std::string(4 * indentLevel, ' ') << "}";
}

void Component::write(OutputMemoryStream & inMemoryStream, const GameObjectManager & gmgr) const
{
    inMemoryStream.write(getClassIDVirtual());
    inMemoryStream.write(getInstanceId());

    if(mGameObj == nullptr)
    {
        inMemoryStream.write(0);
    }
    else
    {
        inMemoryStream.write(1);
        inMemoryStream.write(mGameObj->getInstanceId());
    }
}

void Component::read(const InputMemoryStream & inMemoryStream, GameObjectManager & gmgr)
{
    int32_t type_id {0};
    inMemoryStream.read(type_id);
    uint32_t inst_id {0};
    inMemoryStream.read(inst_id);

    setInstanceId(inst_id);

    int n_ptr {0};
    inMemoryStream.read(n_ptr);

    if(n_ptr != 0)
    {
        // read instance id
        inMemoryStream.read(n_ptr);
        mGameObj = (GameObject *)n_ptr;
    }
    else
    {
        mGameObj = nullptr;
    }
}

void Component::link(GameObjectManager & gmgr, const std::map<uint32_t, uint32_t> & id_remap)
{
    // https://stackoverflow.com/questions/22419063/error-cast-from-pointer-to-smaller-type-int-loses-information-in-eaglview-mm
    uint32_t inst_id = (uint32_t)(size_t)mGameObj;

    if(id_remap.find(inst_id) == id_remap.end())
        EV_EXCEPT("Trying linking not exist object");

    mGameObj = (GameObject *)gmgr.getObjectPtr(id_remap.at(inst_id));
}
}   // namespace evnt
