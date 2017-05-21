#include <machine/cortex_m/bootloader.h>
#include <usb.h>

using namespace EPOS;

extern "C" { extern void _fini(); } // Defined in armv7_crtbegin.c

int main()
{
    {
        eMote3_Bootloader bl;
        do {
            bl.run();
        } while(!eMote3_Bootloader::vector_table_present());
    }

    USB::disable();
    _fini(); // Call global destructors

    for(volatile unsigned int i = 0; i < 0x1fffff; i++); // Delay

    eMote3_Bootloader::jump_to_epos();

    return 0;
}
