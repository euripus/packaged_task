#ifndef OBJECT_H
#define OBJECT_H

#include "classids.h"
#include "memory_stream.h"

#include <functional>
#include <map>
#include <memory>

// https://stackoverflow.com/questions/34222703/how-to-override-static-method-of-template-class-in-derived-class
#define CLASS_IMPLEMENT(inClass, inBaseClass)                                                               \
    using Super = inBaseClass;                                                                              \
    static evnt::StaticTypeInit sm_class_register;                                                          \
                                                                                                            \
    void         dump(int indentLevel = 0) const override;                                                  \
    void         write(OutputMemoryStream & inMemoryStream, const GameObjectManager & gmgr) const override; \
    void         read(const InputMemoryStream & inMemoryStream, GameObjectManager & gmgr) override;         \
    void         link(GameObjectManager & gmgr, const std::map<uint32_t, uint32_t> & id_remap) override;    \
    int32_t      getClassIDVirtual() const override;                                                        \
    const char * getClassString() const override;                                                           \
                                                                                                            \
    static int32_t                 GetClassIDStatic();                                                      \
    static std::unique_ptr<Object> CreateInstance();                                                        \
    static void                    InitType();

#define IMPLEMENT_STRUCT(inClass, inBaseClass)                                                             \
    evnt::StaticTypeInit inClass::sm_class_register{inClass::InitType};                                    \
                                                                                                           \
    int32_t                 inClass::getClassIDVirtual() const { return ClassName(inClass); }              \
    const char *            inClass::getClassString() const { return #inClass; }                           \
    int32_t                 inClass::GetClassIDStatic() { return ClassName(inClass); }                     \
    std::unique_ptr<Object> inClass::CreateInstance() { return std::make_unique<inClass>(); }              \
    void                    inClass::InitType()                                                            \
    {                                                                                                      \
        evnt::Object::RegisterClass(ClassName(inClass), ClassName(inBaseClass), #inClass, sizeof(inClass), \
                                    inClass::CreateInstance);                                              \
    }

namespace evnt
{
class GameObjectManager;

struct StaticTypeInit
{
    StaticTypeInit(void (*init)()) { init(); }
};

class Object
{
    uint32_t mInstanceId{0};   // 0 - not initialized
    bool     is_del_it{false};

public:
    using CreateFunc = std::function<std::unique_ptr<Object>()>;

    struct RTTI
    {
        int32_t     base{0};     // base class ID
        int32_t     size{0};     // sizeof size
        std::string className;   // the name of the class
        CreateFunc  factory;     // the factory function of the class
    };

    static StaticTypeInit sm_class_register;

    static void InitType();

private:
    inline static std::unordered_map<int32_t, RTTI> s_mClassIDToRttiMap;

public:
    virtual ~Object() = default;

    uint32_t getInstanceId() const { return mInstanceId; }
    void     setInstanceId(uint32_t inNetworkId) { mInstanceId = inNetworkId; }

    bool isDeleted() const { return is_del_it; }
    void deleteObj() { is_del_it = true; }

    bool isDerivedFrom(int32_t classID) { return IsDerivedFromClassID(getClassIDVirtual(), classID); }

    virtual int32_t      getClassIDVirtual() const { return ClassName(Object); }
    virtual const char * getClassString() const { return "Object"; }

    virtual void dump(int indentLevel = 0) const { (void)indentLevel; }
    virtual void write(OutputMemoryStream & inMemoryStream, const GameObjectManager & gmgr) const {}
    virtual void read(const InputMemoryStream & inMemoryStream, GameObjectManager & gmgr) {}
    virtual void link(GameObjectManager & gmgr, const std::map<uint32_t, uint32_t> & id_remap) {}

    static int32_t                 GetClassIDStatic() { return ClassName(Object); }
    static std::unique_ptr<Object> CreateInstance() { return std::make_unique<Object>(); }

    /// Returns the RTTI information for a classID
    static RTTI & ClassIDToRTTI(int32_t classID);
    static void   RegisterClass(int32_t inClassID, int32_t inBaseClass, const std::string & inName,
                                int32_t size, CreateFunc inFunc);

    /// Finds out if classID is derived from compareClassID
    static bool IsDerivedFromClassID(int32_t classID, int32_t derivedFromClassID);

    /// Returns the super Class ID of classID.
    /// if classID doesnt have any super Class	it will return -1
    static int32_t GetSuperClassID(int32_t classID);
    /// Get the classID from the class name, returns -1 if no classID was found
    static int32_t StringToClassID(const std::string & classString);
    /// Get the class name from the classID
    static std::string ClassIDToString(int32_t classID);
};
}   // namespace evnt

#endif   // OBJECT_H
