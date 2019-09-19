#ifndef EVENT_H
#define EVENT_H

#include "threadpool.h"

#include <algorithm>
#include <atomic>
#include <functional>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>

#define DECLARE_EVENT_TRAIT(eventTrait, result, ...)                                                   \
    struct eventTrait                                                                                  \
    {                                                                                                  \
        typedef std::tuple<__VA_ARGS__>                 ParamsTuple;                                   \
        typedef result                                  result_type;                                   \
        typedef std::function<result_type(__VA_ARGS__)> DelegateType;                                  \
        static const std::size_t                        numArgs = std::tuple_size<ParamsTuple>::value; \
        static const char *                             name() { return #eventTrait; }                 \
    };

namespace evnt
{
template<typename T>
inline std::size_t get_event_trait_hash()
{
    static std::size_t result = 0;
    if(!result)
    {
        result = std::hash<std::string>{}(std::string(T::name()));
    }
    return result;
}

struct FlagLock
{
    FlagLock(std::atomic_bool & f) : m_flag(f)
    {
        while(m_flag.exchange(true))
            ;
    }

    ~FlagLock() { m_flag.store(false); }

private:
    std::atomic_bool & m_flag;
};

using EvntHandle = std::size_t;

struct BasicEvent
{
    BasicEvent()          = default;
    virtual ~BasicEvent() = default;

    BasicEvent(BasicEvent &)  = delete;
    BasicEvent(BasicEvent &&) = delete;
    BasicEvent & operator=(const BasicEvent &) = delete;
};

template<typename EventTrait>
class SpecEvent : public BasicEvent
{
public:
    SpecEvent() : m_access_flag(false) {}

    EvntHandle bind(typename EventTrait::DelegateType fn)
    {
        FlagLock   lk(m_access_flag);
        EvntHandle evh = ++m_next_available_ID;
        m_delegates.push_back(DelegateHolder{fn, evh});

        return evh;
    }

    void unbind(EvntHandle evh)
    {
        FlagLock lk(m_access_flag);
        m_delegates.erase(std::remove_if(m_delegates.begin(), m_delegates.end(),
                                         [evh](const DelegateHolder & d) { return d._object == evh; }),
                          m_delegates.end());
    }

    template<typename... Args>
    auto call(ThreadPool & pool, Args &&... args)
    {
        std::vector<std::future<typename EventTrait::result_type>> res;
        {
            FlagLock lk(m_access_flag);
            for(auto & d : m_delegates)
            {
                std::function<typename EventTrait::result_type()> fn =
                    std::bind(d._delegate, std::forward<Args>(args)...);
                res.push_back(pool.submit(fn));
            }
        }
        return res;
    }

private:
    struct DelegateHolder
    {
        typename EventTrait::DelegateType _delegate;
        std::size_t                       _object;
    };

    std::atomic_bool          m_access_flag;
    std::list<DelegateHolder> m_delegates;
    mutable uint32_t          m_next_available_ID = {0};
};

class EventSystem
{
public:
    EventSystem(const EventSystem &) = delete;
    EventSystem & operator=(const EventSystem &) = delete;

    EventSystem(ThreadPool & pool) : m_access_flag(false), m_threadpool(pool) {}

    template<typename EventTrait>
    void registerEvent()
    {
        const std::size_t eventHash = get_event_trait_hash<EventTrait>();
        FlagLock          lk(m_access_flag);
        m_events[eventHash] = std::make_unique<SpecEvent<EventTrait>>();
    }

    template<typename EventTrait>
    EvntHandle subscribeToEvent(typename EventTrait::DelegateType fn)
    {
        FlagLock                lk(m_access_flag);
        EvntHandle              evh = 0;
        SpecEvent<EventTrait> * evt = find_spec_event<EventTrait>();
        if(nullptr != evt)
        {
            evh = evt->bind(std::move(fn));
        }

        return evh;
    }

    template<typename EventTrait>
    void unSubscribeFromEvent(EvntHandle evh)
    {
        FlagLock                lk(m_access_flag);
        SpecEvent<EventTrait> * evt = find_spec_event<EventTrait>();
        if(nullptr != evt)
        {
            evt->unbind(evh);
        }
    }

    template<typename EventTrait, typename... Args>
    auto raiseEvent(Args &&... args)
    {
        std::vector<std::future<typename EventTrait::result_type>> res;
        FlagLock                                                   lk(m_access_flag);
        SpecEvent<EventTrait> *                                    evt = find_spec_event<EventTrait>();
        if(nullptr != evt)
        {
            res = evt->call(m_threadpool, std::forward<Args>(args)...);
        }

        return res;
    }

private:
    template<typename EventTrait>
    SpecEvent<EventTrait> * find_spec_event()
    {
        const std::size_t       eventHash = get_event_trait_hash<EventTrait>();
        auto                    it        = m_events.find(eventHash);
        SpecEvent<EventTrait> * ptr       = nullptr;

        if(it != m_events.end())
        {
            ptr = static_cast<SpecEvent<EventTrait> *>(it->second.get());
        }

        return ptr;
    }

private:
    using EventsMap = std::unordered_map<std::size_t, std::unique_ptr<BasicEvent>>;

    std::atomic_bool m_access_flag;   // spinlock access
    EventsMap        m_events;
    ThreadPool &     m_threadpool;
};
}   // namespace evnt

#endif   // EVENT_H
