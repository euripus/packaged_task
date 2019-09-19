#ifndef CLASSIDS_H
#define CLASSIDS_H

#include <cstdint>

#define ClassName(name)              CLASS_##name
#define DefineClassID(name, classID) ClassName(name) = classID,

enum ClassIDType : int32_t
{
    DefineClassID(Undefined, -1)
    DefineClassID(Object, 1000)
    DefineClassID(Component, 1001)
    DefineClassID(GameObject, 1002)

    eLargestRuntimeClassID
};

// make sure people dont accidentally define classids in other files:
#undef DefineClassID

#endif   // CLASSIDS_H
