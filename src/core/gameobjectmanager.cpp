#include "gameobjectmanager.h"
#include "exception.h"
#include <cassert>
#include <limits>
#include <iostream>
#include <map>

namespace evnt
{
GameObjectManager::~GameObjectManager()
{
    for(auto & [key, obj_entry]: mObjects)
    {
        if(auto sp = obj_entry.handle.lock())
            sp->nullify();
    }
}

PObjHandle GameObjectManager::createDefaultObj(int32_t obj_type)
{
    PUniqueObjPtr u_ptr = Object::ClassIDToRTTI(obj_type).factory();

    return registerObj(std::move(u_ptr));
}

PObjHandle GameObjectManager::registerObj(PUniqueObjPtr ob)
{
    const uint32_t id = mNextAvailableID.fetch_add(1, std::memory_order_relaxed);
    assert(id != std::numeric_limits<uint32_t>::max());

    ob->setInstanceId(id);
    PObjHandle sp = std::make_shared<ObjHandle>(ob.get());

    ObjEntry new_entry;
    new_entry.unique = std::move(ob);
    new_entry.handle = sp;

    {
        std::lock_guard<std::mutex> lk(mMutex);
        mObjects[id] = std::move(new_entry);
    }

    return sp;
}

bool GameObjectManager::objectExists(uint32_t id) const
{
    std::lock_guard<std::mutex> lk(mMutex);
    return mObjects.find(id) != mObjects.end();
}

PObjHandle GameObjectManager::getObject(uint32_t id)
{
    std::lock_guard<std::mutex> lk(mMutex);

    const auto iterFind = mObjects.find(id);
    if(iterFind == mObjects.end())
        EV_EXCEPT("Trying to acquire not created object.");

    return iterFind->second.handle.lock();
}

Object * GameObjectManager::getObjectPtr(uint32_t id)
{
    std::lock_guard<std::mutex> lk(mMutex);

    const auto iterFind = mObjects.find(id);
    if(iterFind == mObjects.end())
        EV_EXCEPT("Trying to acquire not created object.");

    return iterFind->second.unique.get();
}

void GameObjectManager::releaseUnusedObjects()
{
    std::lock_guard<std::mutex> lk(mMutex);

    for(auto it = mObjects.begin(); it != mObjects.end();)
    {
        if(it->second.unique->isDeleted())
            it = mObjects.erase(it);
        else
            ++it;
    }
}

void GameObjectManager::dump() const
{
    std::lock_guard<std::mutex> lk(mMutex);

    for(const auto & [key, obj_entry]: mObjects)
    {
        if(!obj_entry.unique->isDeleted())
        {
            obj_entry.unique->dump();
            std::cout << std::endl;
        }
    }
}

void GameObjectManager::serialize(OutputMemoryStream & inMemoryStream) const
{
    std::lock_guard<std::mutex> lk(mMutex);

    for(const auto & [key, obj_entry]: mObjects)
    {
        if(!obj_entry.unique->isDeleted())
            obj_entry.unique->write(inMemoryStream, *this);
    }
}

void GameObjectManager::deserialize(const InputMemoryStream & inMemoryStream,
                                    std::vector<PObjHandle> & objects)
{
    objects.clear();
    std::map<uint32_t, uint32_t> istance_id_remap;   // [old_id, new_id]

    while(inMemoryStream.getRemainingDataSize() > 0)
    {
        auto      stream_cur_ptr = inMemoryStream.getCurPosPtr();
        int32_t * type_id        = (int32_t *)stream_cur_ptr;

        auto obj = Object::ClassIDToRTTI(*type_id).factory();
        obj->read(inMemoryStream, *this);
        auto old_id = obj->getInstanceId();

        auto obj_handler         = registerObj(std::move(obj));
        istance_id_remap[old_id] = obj_handler->getPtr()->getInstanceId();

        objects.push_back(obj_handler);
    }

    for(const auto & [key, obj_entry]: mObjects)
    {
        obj_entry.unique->link(*this, istance_id_remap);
    }
}

}   // namespace evnt
