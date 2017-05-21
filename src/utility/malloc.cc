// EPOS Application-level Dynamic Memory Utility Implementation

#include <system/config.h>
#include <utility/malloc.h>

// C++ dynamic memory deallocators
void operator delete(void * object) {
    return free(object);
}

void operator delete[](void * object) {
    return free(object);
}
