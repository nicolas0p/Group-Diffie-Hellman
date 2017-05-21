// EPOS SHT11 Sensor Implementation

#include <system/config.h>
#ifdef __TEMP_H

#include <machine/cortex/sht11.h>

__BEGIN_SYS

bool SHT11::enable()
{
    if(_in_use)
        return false;

    _in_use = true;

    // The sensor needs 11 ms to get to sleep state after power-up
    Alarm::delay(11000);

    soft_reset();

    return true;
}

void SHT11::disable()
{
    _in_use = false;
}

unsigned char SHT11::write_byte(unsigned char value)
{
    unsigned char i = 0x80;
    unsigned char error = 0;

    transmission_start();

    while(i) {
        if(i & value)
            _data->set();
        else
            _data->clear();

        Alarm::delay(T_SCK_US);
        _sck->set();
        Alarm::delay(T_SCK_US);
        _sck->clear();

        i = (i >> 1);
    }

    // TODO: Maybe we can put this part inside an if in the loop
    _data->direction(GPIO::IN);
    _data->pull(GPIO::UP);

    // Check for the ACK
    Alarm::delay(T_SCK_US);
    if (_data->get())
        error = 1;
    _sck->set();

    Alarm::delay(T_SCK_US);
    _sck->clear();

    Alarm::delay(T_SCK_US);
    // Check if the line was released
    if (!_data->get())
        error = 1;

    return error;
}

unsigned char SHT11::read_byte(unsigned char ack)
{
    unsigned char i   = 0x80;
    unsigned char val = 0;

    _data->direction(GPIO::IN);
    _data->pull(GPIO::UP);

    while(i) {
        if(_data->get())
            val = (val | i);

        _sck->set();
        Alarm::delay(T_SCK_US); // TODO:: replace by Machine::delay() ???
        _sck->clear();
        Alarm::delay(T_SCK_US);

        i= (i >> 1);
    }

    _data->direction(GPIO::OUT);

    if (ack)
        _data->clear();
    else
        _data->set();

    Alarm::delay(T_SCK_US);
    _sck->set();
    Alarm::delay(T_SCK_US);
    _data->direction(GPIO::IN);
    _data->pull(GPIO::UP);
    _sck->clear();
    Alarm::delay(T_SCK_US);

    return val;
}

void SHT11::transmission_start()
{
    _sck->direction(GPIO::OUT);
    _sck->clear();

    _data->direction(GPIO::OUT);
    _data->set();

    Alarm::delay(T_SCK_US);
    _sck->set();
    Alarm::delay(T_SCK_US);
    _data->clear();
    Alarm::delay(T_SCK_US);
    _sck->clear();
    Alarm::delay(T_SCK_US);
    _sck->set();
    Alarm::delay(T_SCK_US);
    _data->set();
    Alarm::delay(T_SCK_US);
    _sck->clear();
    Alarm::delay(T_SCK_US);
}

void SHT11::connection_reset()
{
    unsigned char i;

    _sck->direction(GPIO::OUT);
    _sck->clear();

    _data->direction(GPIO::OUT);
    _data->set();

    for (i = 0; i < 9; i++) {
        _sck->set();
        Alarm::delay(T_SCK_US);
        _sck->clear();
        Alarm::delay(T_SCK_US);
    }

    transmission_start();
}

unsigned char SHT11::soft_reset()
{
    connection_reset();

    return write_byte(RESET);
}

int SHT11::measure(unsigned char mode)
{
    unsigned char msb = 0;
    int p_value = 0;

    write_byte(mode);

    // Wait for the measurement to complete
    Alarm::delay(100000);
    if (!_data->get()) {
        msb = read_byte(ACK);
        // There's no need to ACK the second byte as we don't want the CRC
        p_value = msb;
        p_value = (((int)msb) << 8) | (int)read_byte(NO_ACK);
    }

    return p_value;
}

int SHT11_Humidity::sample()
{
    int raw_humidity;

    raw_humidity = measure(MEASURE_HUMIDITY);
    _humidity = compensate(raw_humidity);

    // According to the datasheet, values higher than 99% indicate fully
    // saturated air and should be displayed as 100%
    if(_humidity > 99)
        _humidity = 100;

    return _humidity;
}

// FIXME: For temperatures significantly different from 25°C, the humidity
// signal requires temperature compensation, the procedure is explained in the
// datasheet
int SHT11_Humidity::compensate(int value)
{
    long humidity;

    // FIXME: Reimplement it using fixed point arithmetic
    // The equation to compensate the sensor non-linearity is
    // -2.0468 + 0.0367*x - 0.0000015955*(x^2), we're poorly approximating the
    // fractional number multiplications by natural number divisions
    humidity = value/27;
    humidity -= value*value/626762;
    humidity -= 2;

    return humidity;
}

__END_SYS

#endif
