#ifndef CORE_H
#define CORE_H

#include <boost/property_tree/ptree.hpp>

#include "../fs/file_system.h"
#include "event.h"
#include "gameobject.h"
#include "gameobjectmanager.h"
#include "module.h"

namespace pt = boost::property_tree;

namespace evnt
{
int64_t GetMilisecFromStart();

class Core : public Module<Core>
{
    pt::ptree                    m_root_config;
    std::unique_ptr<ThreadPool>  m_thread_pool;
    std::unique_ptr<EventSystem> m_event_system;
    std::unique_ptr<FileSystem>  m_file_system;

public:
    Core();

    ~Core() = default;

    // Events
    template<typename EventTrait>
    void registerEvent()
    {
        m_event_system->registerEvent<EventTrait>();
    }

    template<typename EventTrait>
    EvntHandle addFunctor(typename EventTrait::DelegateType fn)
    {
        return m_event_system->subscribeToEvent<EventTrait>(std::move(fn));
    }

    template<typename EventTrait>
    void removeFunctor(EvntHandle evh)
    {
        m_event_system->unSubscribeFromEvent<EventTrait>(evh);
    }

    template<typename EventTrait, typename... Args>
    auto raiseEvent(Args &&... args)
    {
        static_assert(EventTrait::numArgs == sizeof...(Args), "Incorrect arguments number!");

        return m_event_system->raiseEvent<EventTrait>(std::forward<Args>(args)...);
    }

    // getters
    ThreadPool &      getThreadPool() { return *m_thread_pool; }
    FileSystem &      getFileSystem() { return *m_file_system; }
    const pt::ptree & getRootConfig() const { return m_root_config; }
};
}   // namespace evnt
#endif   // CORE_H
