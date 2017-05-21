// EPOS Hydrology Board Mediator Common Package

#ifndef __hydro_board_h
#define __hydro_board_h

#include <system/config.h>

__BEGIN_SYS

class Hydro_Board_Common
{
protected:
    Hydro_Board_Common() {}
};

__END_SYS

#ifdef __HYDRO_BOARD_H
#include __HYDRO_BOARD_H
#else
__BEGIN_SYS
class Hydro_Board: public Dummy {};
__END_SYS
#endif

#endif
