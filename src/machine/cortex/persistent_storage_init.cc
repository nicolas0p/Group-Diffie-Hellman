// EPOS ARM Cortex Persisistent Storage Mediator Initialization

#include <system/config.h>

#ifdef __mmod_emote3__

#include <persistent_storage.h>

__BEGIN_SYS

void Persistent_Storage::init()
{
    db<Init, Persistent_Storage>(TRC) << "Persistent_Storage::init()" << endl;
    
    for(unsigned int i = 0; i < N_PAGES; ++i)
        _metadata.write_count[i] = MAX_WRITE_COUNT;
}

__END_SYS

#endif
