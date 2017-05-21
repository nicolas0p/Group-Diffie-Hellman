#include <machine.h>
#include <alarm.h>
#include <utility/ostream.h>
#include <spi.h>
#include <gpio.h>
#include <cpu.h>

using namespace EPOS;

#define SPI_MASTER 1 //change this to zero to test SPI as a slave
#define SPI_UNIT 0 //using SPI unit 0 (change to 1)

OStream cout;

void spi_master();
void spi_slave();

int main()
{
    for(volatile unsigned int t = 0; t < 0xfffff; t++)
                    ;
    for(volatile unsigned int t = 0; t < 0xfffff; t++)
                    ;
    for(volatile unsigned int t = 0; t < 0xfffff; t++)
                    ;
    if(SPI_MASTER) {
        spi_master();
    } else {
        spi_slave();
    }
    return 0;
}

void spi_master()
{
    cout << "SPI Master test\n";

    GPIO g('C', 3, GPIO::OUT);
    SPI spi(SPI_UNIT, Traits<CPU>::CLOCK, SPI::FORMAT_MOTO_1, SPI::MASTER, 1000000, 8);

    while(1) {
        g.set(1);
        spi.put_data_non_blocking('b');
        for(volatile unsigned int t = 0; t < 0xfffff; t++)
                ;
        g.clear();
        for(volatile unsigned int t = 0; t < 0xfffff; t++)
                        ;
        cout << "Test\n";
    }
}

void spi_slave()
{
    cout << "SPI Slave test\n";

    GPIO g('C', 3, GPIO::OUT);
    SPI spi(SPI_UNIT, Traits<CPU>::CLOCK, SPI::FORMAT_MOTO_1, SPI::SLAVE, 1000000, 8);

    while(1) {
        g.set(1);
        unsigned int a = spi.get_data();
        cout << "Data from SPI = " << a << "\n";
        for(volatile int t = 0; t < 0xfffff; t++)
            ;
        g.clear();
        for(volatile unsigned int t = 0; t < 0xfffff; t++)
            ;
    }
}

