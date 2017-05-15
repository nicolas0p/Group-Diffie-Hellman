// EPOS ARM Cortex-M Analog to Digital Converter (ADC) Mediator Declarations

#include <system/config.h>
#if !defined(__cortex_adc_h_) && defined(__ADC_H)
#define __cortex_adc_h_

#include <adc.h>
#include <machine.h>

__BEGIN_SYS

class ADC: private ADC_Common, private Machine_Model
{
public:
    enum Channel {
        SINGLE_ENDED_ADC0 = 0,
        SINGLE_ENDED_ADC1 = 1,
        SINGLE_ENDED_ADC2 = 2,
        SINGLE_ENDED_ADC3 = 3,
        SINGLE_ENDED_ADC4 = 4,
        SINGLE_ENDED_ADC5 = 5,
        SINGLE_ENDED_ADC6 = 6,
        SINGLE_ENDED_ADC7 = 7,
        DIFF8             = 8,
        DIFF9             = 9,
        DIFF10            = 10,
        DIFF11            = 11,
        GND               = 12,
        TEMPERATURE       = 14,
        AVDD5_3           = 15,
    };

    enum Reference {
        INTERNAL_REF   = 0,
        EXTERNAL_REF   = 1, // External reference on AIN7 pin
        SYSTEM_REF     = 2,
        EXTERNAL_DIFF  = 3
    };

    enum Resolution {
        BITS_7  = 0, //  7 bits resolution, 64  decimation rate
        BITS_9  = 1, //  9 bits resolution, 128 decimation rate
        BITS_10 = 2, // 10 bits resolution, 256 decimation rate
        BITS_12 = 3  // 12 bits resolution, 512 decimation rate
    };

    ADC(const Channel & channel = SINGLE_ENDED_ADC5, const Reference & reference = SYSTEM_REF, const Resolution & resolution = BITS_12)
    : _channel(channel), _reference(reference), _resolution(resolution) {
        Machine_Model::adc_config(_channel);
    }

    short read() {
        reg(ADCCON3) = (_reference * ADCCON3_EREF) | (_resolution * ADCCON3_EDIV) | (_channel * ADCCON3_ECH);
        while(!(reg(ADCCON1) & ADCCON1_EOC));
        short ret = (reg(ADCH) << 8) + reg(ADCL);
        switch(_resolution) {
            case BITS_7:  ret >>= 9; break;
            case BITS_9:  ret >>= 7; break;
            case BITS_10: ret >>= 6; break;
            case BITS_12: ret >>= 4; break;
        }
        return ret;
    }

    // returns the voltage corresponding to the reading, with three decimal places (e.g. 2534 means 2.534V)
    int convert(short raw_reading, int reference = 3300/*3.3V*/) {
        int limit;
        switch(_resolution) {
            case BITS_7:  limit =   63; break;
            case BITS_9:  limit =  255; break;
            case BITS_10: limit =  511; break;
            case BITS_12: limit = 2047; break;
        }
        return raw_reading * reference / limit;
    }

private:
    volatile CPU::Reg32 & reg(unsigned int o) { return reinterpret_cast<volatile CPU::Reg32*>(ADC_BASE)[o / sizeof(CPU::Reg32)]; }

    Channel _channel;
    Reference _reference;
    Resolution _resolution;
};

__END_SYS

#endif
