#ifndef GAMEOBJECTMANAGER_H
#define GAMEOBJECTMANAGER_H

#include "objhandle.h"

#include <atomic>
#include <mutex>

namespace evnt
{
class GameObjectManager
{
    using PUniqueObjPtr = std::unique_ptr<Object>;
    using PWeakHandle   = std::weak_ptr<ObjHandle>;

    struct ObjEntry
    {
        PUniqueObjPtr unique;
        PWeakHandle   handle;
    };

    std::atomic<uint32_t>                  mNextAvailableID = {1};   // 0 is not a valid ID
    std::unordered_map<uint32_t, ObjEntry> mObjects;                 // key = instance_id, ObjEntry

    mutable std::mutex mMutex;

public:
    GameObjectManager() = default;
    ~GameObjectManager();

    void releaseUnusedObjects();

    bool objectExists(uint32_t id) const;

    PObjHandle createDefaultObj(int32_t obj_type);
    PObjHandle registerObj(PUniqueObjPtr ob);
    PObjHandle getObject(uint32_t id);
    Object *   getObjectPtr(uint32_t id);

    template<typename type>
    PObjHandle createDefaultObj();

    void serialize(OutputMemoryStream & inMemoryStream) const;
    void deserialize(const InputMemoryStream & inMemoryStream, std::vector<PObjHandle> & objects);
    void dump() const;
};

template<typename type>
PObjHandle GameObjectManager::createDefaultObj()
{
    return createDefaultObj(type::GetClassIDStatic());
}
}   // namespace evnt

#endif   // GAMEOBJECTMANAGER_H
