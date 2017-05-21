#include <alarm.h>
#include <adc.h>

__USING_SYS

class AC_Control
{
private:
    char _temperature;
public:
    AC_Control(GPIO * p) : _pin(p), data0_mask(0), _temperature(16)
    {
        clear_pin();
    }
    ~AC_Control() { }

    char temperature()
    {
        return _temperature;
    }

    enum status
    {
        keep_on = 0x24, turn_off = 0x4
    };

    void set_pin()
    {
        _pin->set(1);
    }

    void clear_pin()
    {
        _pin->set(0);
    }

    void delay10u(volatile unsigned int count)
    {
        volatile unsigned int inner_count;
        for(volatile unsigned int i = 0; i < count; i++)
        {
            inner_count = 30;
            while(inner_count--) { };
        }
    }

    void fulfil_set_command(int interval=40)
    {
        set_pin();
        delay10u(interval);
        clear_pin();
    }

    void put_L(int calibration = 0)
    {
        set_pin();
        delay10u(380);
    }

    void put_0(int calibration = 0) {
        fulfil_set_command();
        delay10u(50);
    }

    void put_1(int calibration = 0)
    {
        fulfil_set_command();
        delay10u(136);
    }

    void put_either(char either)
    {
        if (either)
            put_1();
        else
            put_0();
    }

    void put_c(char c)
    {
        for(int i = 7; i >= 0; i--)
        {
            put_either(c & (1 << i));
        }
    }

    void put_c_inverted(char c)
    {
        for(int i = 0; i <= 7; i++)
        {
            put_either(c & (1 << i));
        }
    }

    void put_half(char c)
    {
        for(int i = 3; i >= 0; i--)
        {
            put_either(c & (1 << i));
        }
    }

    void put_half_inverted(char c)
    {
        for(int i = 0; i <= 3; i++)
        {
            put_either(c & (1 << i));
        }
    }

    void send_command(char on_off, char temperature)
    {
        put_L();
        clear_pin();
        delay10u(188);
        put_c(0xC4);
        put_c(0xD3);
        put_c(0x64);
        put_c(0x80);
        put_c(0x0);
        put_c(on_off);
        put_c(0xC0);
        put_half_inverted(31-temperature);
        put_half(0x0);
        put_c(0x1C);
        put_c(0x0);
        put_c(0x0);
        put_c(0x0);
        put_c(0x0);
        if(on_off == turn_off)
            put_c(0x9E);
        else
            put_c_inverted(147-temperature);
        put_0();
    }

    void turn_ac_off()
    {
        send_command(turn_off, 22);
    }

    void turn_ac_on()
    {
        send_command(keep_on, _temperature);
    }

    void set_temperature(char temp)
    {
        if(temp > 30) temp = 30;
        if(temp < 16) temp = 16;
        _temperature = temp;
        send_command(keep_on, _temperature);
    }

    //FIXME: These two methods below are possibly not needed
    void increase_temperature()
    {
        _temperature++;
        if(_temperature > 32)
            _temperature = 32;
        send_command(keep_on, _temperature);
    }

    void decrease_temperature()
    {
        _temperature--;
        if(_temperature < 16)
            _temperature = 16;
        send_command(keep_on, _temperature);
    }
private:
    GPIO * _pin;
    OStream cout;

    unsigned int data0_mask;
};
