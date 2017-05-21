// EPOS Cortex_M SPI Mediator Declarations

#include <system/config.h>

#if !defined(__cortex_spi_h__) && defined(__SPI_H)
#define __cortex_spi_h__

#include <machine.h>
#include <spi.h>

__BEGIN_SYS

class SSI_Engine : public Machine_Model
{
    friend class eMote3;
private:
    static const unsigned int UNITS = Traits<SPI>::UNITS;

public:
    enum Interrupt_Flag {
        TXIM    = Machine_Model::TXIM,
        RXIM    = Machine_Model::RXIM,
        RTIM    = Machine_Model::RTIM,
        RORIM   = Machine_Model::RORIM,
    };

    enum Clock_Source {
        IO      = 0x00,
        SYS     = 0x01,
    };

    using Machine_Model::SSI_Frame_Format;
    using Machine_Model::SSI_Mode;

    volatile Reg32 & ssi(unsigned int o) { return reinterpret_cast<volatile Reg32 *>(_base)[o / sizeof(Reg32)]; }

public:

    SSI_Engine(Reg32 unit = 0) :
	_base(reinterpret_cast<Log_Addr *>(unit ? SSI1_BASE : SSI0_BASE))
    {
    }

    SSI_Engine(Reg32 unit, Reg32 clock, SSI_Frame_Format protocol, SSI_Mode mode, Reg32 bit_rate, Reg32 data_width) :
    _base(reinterpret_cast<Log_Addr *>(unit ? SSI1_BASE : SSI0_BASE)) {
        assert(unit < UNITS);
        config(clock, protocol, mode, bit_rate, data_width);
    }

    //This method configures the synchronous serial interface.  It sets
    // the SSI protocol, mode of operation, bit rate, and data width.
    // data_width must be a value between 4 and 16, inclusive
    void config(Reg32 clock, SSI_Frame_Format protocol, SSI_Mode mode, Reg32 bit_rate, Reg32 data_width) {
        disable();
        Machine_Model::config_SSI(_base, clock, protocol, mode, bit_rate, data_width);
        enable();
    }

    //enable SSI interface
    void enable() {
        // Read-modify-write the enable bit
        ssi(SSI_CR1) |= SSE;
    }

    //disable SSI interface
    void disable() {
        // Read-modify-write the enable bit
        ssi(SSI_CR1) &= ~(SSE);
    }

    void int_enable(Interrupt_Flag flag) {
        ssi(SSI_IM) |= flag;
    }

    void int_disable(Interrupt_Flag flag) {
        ssi(SSI_IM) &= ~(flag);
    }

    void put_data(Reg32 data) {
        // Wait until there is space.
        while(!(ssi(SSI_SR) & TNF)) ;

        // Write the data to the SSI.
        ssi(SSI_DR) = data;
    }

    //returns 1 if the data has been written or 0 if an error occurred
    Reg32 put_data_non_blocking(Reg32 data) {
        // Check for space to write.
        if(ssi(SSI_SR) & TNF) {
            ssi(SSI_DR) = data;
            return 1;
        }
        return 0;
    }

    Reg32 get_data() {
        // Wait until there is data to be read.
        while(!(ssi(SSI_SR) & RNE)) ;

        return ssi(SSI_DR);
    }

    //returns the data read or 0 if an error occurred
    Reg32 get_data_non_blocking() {
        if(ssi(SSI_SR) & RNE) {
            return ssi(SSI_DR);
        }

        return 0;
    }

    bool is_busy() {
        // Determine if the SSI is busy.
        return((ssi(SSI_SR) & BSY) ? true : false);
    }

    void clock_source(Clock_Source source) {
        ssi(SSI_CC) = source;
    }

    Reg32 clock_source() {
        return ssi(SSI_CC);
    }

private:
    volatile Log_Addr * _base;
};


typedef SSI_Engine SPI_Engine;


class SPI: public SPI_Common, public SPI_Engine
{
private:
    typedef SPI_Engine Engine;

public:

    static const unsigned int CLOCK = Traits<CPU>::CLOCK;

    using SPI_Engine::SSI_Frame_Format;
    using Machine_Model::SSI_Mode;

    typedef SPI_Engine::SSI_Frame_Format SPI_Frame_Format;
    typedef Machine_Model::SSI_Mode SPI_Mode;

    SPI() : Engine(0) {
    }

    SPI(Reg32 unit, Reg32 clock, SPI_Frame_Format protocol, SPI_Mode mode, Reg32 bit_rate, Reg32 data_width) :
        Engine(unit, clock, protocol, mode, bit_rate, data_width)
    {
    }

    //This method configures the synchronous serial interface.  It sets
    // the SSI protocol, mode of operation, bit rate, and data width.
    // data_width must be a value between 4 and 16, inclusive
    void config(Reg32 clock, SPI_Frame_Format protocol, SPI_Mode mode, Reg32 bit_rate, Reg32 data_width) {
        Engine::config(clock, protocol, mode, bit_rate, data_width);
    }

    void disable() { Engine::disable(); }
    void enable() { Engine::enable(); }
    void int_enable(Interrupt_Flag flag) { Engine::int_enable(flag); }
    void int_disable(Interrupt_Flag flag) { Engine::int_disable(flag); }
    void put_data(Reg32 data) { Engine::put_data(data); }
    Reg32 put_data_non_blocking(Reg32 data) { return Engine::put_data_non_blocking(data); }
    Reg32 get_data() { return Engine::get_data(); }
    Reg32 get_data_non_blocking() { return Engine::get_data_non_blocking(); }
    bool is_busy() { return Engine::is_busy(); }
    void clock_source(Clock_Source source) { Engine::clock_source(source); }
    Reg32 clock_source() { return Engine::clock_source(); }
};

__END_SYS

#endif
