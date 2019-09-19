#include "core/core.h"

#include <cstring>
#include <deque>
#include <future>
#include <iostream>
#include <list>
#include <numeric>
#include <thread>
#include <vector>

// namespace evnt

DECLARE_EVENT_TRAIT(evIncr, int, int);

int test(int i)
{
    return i++;
}

int main()
{
    evnt::ThreadPool pool;
    evnt::Core::startUp();

    auto fn = []() -> auto
    {
        static int i = 0;
        i++;

        return i;
    };

    // https://stackoverflow.com/questions/32840369/no-matching-function-error-when-passing-lambda-function-as-argument
    // ThreadPool
    auto res1 = pool.submit(fn);
    auto res2 = pool.submit(fn);
    auto res3 = pool.submit(fn);
    auto res4 = pool.submit(fn);

    auto fn2 = [](int t) -> auto
    {
        t++;

        return t;
    };

    // Event system
    evnt::Core::instance().registerEvent<evIncr>();
    evnt::Core::instance().addFunctor<evIncr>(fn2);
    auto h = evnt::Core::instance().addFunctor<evIncr>(test);

    auto res = evnt::Core::instance().raiseEvent<evIncr>(42);

    std::cout << res4.get() << std::endl;
    std::cout << res[0].get() << std::endl << std::endl;

    evnt::Core::instance().removeFunctor<evIncr>(h);

    // Oject system
    evnt::OutputMemoryStream out;
    {
        evnt::GameObjectManager g_mgr;

        auto goh = g_mgr.createDefaultObj<evnt::GameObject>();
        auto go  = evnt::dynamic_ohdl_cast<evnt::GameObject>(goh);
        {
            auto cmph    = g_mgr.createDefaultObj<evnt::Component>();
            auto cmp_ptr = evnt::dynamic_ohdl_cast<evnt::Component>(cmph);

            auto cid  = static_cast<ClassIDType>(cmp_ptr->getClassIDVirtual());
            auto fnct = [cmp_ptr](evnt::Component * rec, evnt::CmpMsgsTable::msg_id id, std::any msg_data) {
                cmp_ptr->onAddMessage(rec, id, msg_data);
            };
            evnt::GameObject::sMsgHandler.registerMessageCallback(evnt::CmpMsgsTable::mDidAddComponent, cid,
                                                                  fnct);

            go->addComponent(cmph);

            auto cmph2 = g_mgr.createDefaultObj<evnt::Component>();
            go->addComponent(cmph2);
        }

        g_mgr.dump();
        std::cout << std::endl;

        auto cp = go->getComponentPtr<evnt::Component>();
        auto cr = go->getComponent<evnt::Component>();

        g_mgr.serialize(out);
    }

    std::unique_ptr<int8_t[]> new_buf(new int8_t[out.getLength()]);
    std::memcpy(new_buf.get(), out.getBufferPtr(), out.getLength());
    evnt::InputMemoryStream in(std::move(new_buf), out.getLength());
    {
        evnt::GameObjectManager       g_mgr;
        std::vector<evnt::PObjHandle> new_obj;
        g_mgr.deserialize(in, new_obj);

        g_mgr.dump();
        std::cout << std::endl;
    }

    evnt::Core::shutDown();

    std::cout << std::numeric_limits<int64_t>::max() << "  " << evnt::GetMilisecFromStart() << std::endl;

    return 0;
}
