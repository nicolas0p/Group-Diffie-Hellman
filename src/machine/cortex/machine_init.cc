// EPOS Cortex Mediator Initialization

#include <machine/cortex/machine.h>
#include <smart_plug.h>
#include <hydro_board.h>
#include <persistent_storage.h>

__BEGIN_SYS

void Machine::init()
{
    db<Init, Machine>(TRC) << "Machine::init()" << endl;

    Machine_Model::init();

    if(Traits<IC>::enabled)
        IC::init();
    if(Traits<Timer>::enabled)
        Timer::init();
#ifdef __USB_H
    if(Traits<USB>::enabled)
        USB::init();
#endif
#ifdef __PERSISTENT_STORAGE_H
    if(Traits<Persistent_Storage>::enabled)
        Persistent_Storage::init();
#endif
#ifdef __SMART_PLUG_H
    if(Traits<Smart_Plug>::enabled)
        Smart_Plug::init();
#endif
#ifdef __HYDRO_BOARD_H
    if(Traits<Hydro_Board>::enabled)
        Hydro_Board::init();
#endif
#ifdef __NIC_H
    if(Traits<NIC>::enabled)
        NIC::init();
#endif
}

__END_SYS
