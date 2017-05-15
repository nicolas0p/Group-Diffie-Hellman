// EPOS ARM Cortex Smart Transducer Declarations

#ifndef __cortex_transducer_h
#define __cortex_transducer_h

#include <smart_data.h>

#include <keyboard.h>
#include <smart_plug.h>
#include <hydro_board.h>
#include <gpio.h>
#include <tstp.h>
#include <tsc.h>
#include <rfid_reader.h>
#include <persistent_storage.h>
#include <condition.h>
#include <spi.h>

__BEGIN_SYS

enum CUSTOM_UNITS
{
    UNIT_RFID = TSTP::Unit::DIGITAL | 1,
    UNIT_SWITCH = TSTP::Unit::DIGITAL | 2,
};

class Keyboard_Sensor: public Keyboard
{
public:
    static const unsigned int UNIT = TSTP::Unit::Acceleration;
    static const unsigned int NUM = TSTP::Unit::I32;
    static const int ERROR = 0; // Unknown

    static const bool INTERRUPT = true;
    static const bool POLLING = false;

    typedef Keyboard::Observer Observer;
    typedef Keyboard::Observed Observed;

public:
    Keyboard_Sensor() {}

    static void sense(unsigned int dev, Smart_Data<Keyboard_Sensor> * data) {
        if(ready_to_get())
            data->_value = get();
        else
            data->_value = -1;
    }

    static void actuate(unsigned int dev, Smart_Data<Keyboard_Sensor> * data, const Smart_Data<Keyboard_Sensor>::Value & command) {}
};

#ifdef __mmod_emote3__

class Water_Level_Sensor: public Hydro_Board
{
public:
    static const unsigned int UNIT = TSTP::Unit::Length;
    static const unsigned int NUM = TSTP::Unit::I32;
    static const int ERROR = 0; // Unknown

    static const bool INTERRUPT = false;
    static const bool POLLING = true;

public:
    static void sense(unsigned int dev, Smart_Data<Water_Level_Sensor> * data) {
        data->_value = level(dev);
    }

    static void actuate(unsigned int dev, Smart_Data<Water_Level_Sensor> * data, const Smart_Data<Water_Level_Sensor>::Value & command) {}
};

class Water_Turbidity_Sensor: public Hydro_Board
{
public:
    static const unsigned int UNIT = TSTP::Unit::Amount_of_Substance; // TODO
    static const unsigned int NUM = TSTP::Unit::I32;
    static const int ERROR = 0; // Unknown

    static const bool INTERRUPT = false;
    static const bool POLLING = true;

public:
    static void sense(unsigned int dev, Smart_Data<Water_Turbidity_Sensor> * data) {
        data->_value = turbidity(dev);
    }

    static void actuate(unsigned int dev, Smart_Data<Water_Turbidity_Sensor> * data, const Smart_Data<Water_Turbidity_Sensor>::Value & command) {}
};

class Water_Flow_Sensor: public Hydro_Board
{
public:
    static const unsigned int UNIT = (TSTP::Unit::SI) | (TSTP::Unit::DIR) | ((4 + 3) * TSTP::Unit::M) | ((4 - 1) * TSTP::Unit::S); // m^3/s
    static const unsigned int NUM = TSTP::Unit::I32;
    static const int ERROR = 0; // Unknown

    static const bool INTERRUPT = false;
    static const bool POLLING = true;

public:
    static void sense(unsigned int dev, Smart_Data<Water_Flow_Sensor> * data) {
        data->_value = water_flow();
    }

    static void actuate(unsigned int dev, Smart_Data<Water_Flow_Sensor> * data, const Smart_Data<Water_Flow_Sensor>::Value & command) {}
};

class Pluviometer: public Hydro_Board
{
public:
    static const unsigned int UNIT = TSTP::Unit::DIV | TSTP::Unit::Length; // TODO: we want mm, or mm/m^2
    static const unsigned int NUM = TSTP::Unit::I32;
    static const int ERROR = 0; // Unknown

    static const bool INTERRUPT = false;
    static const bool POLLING = true;

public:
    static void sense(unsigned int dev, Smart_Data<Pluviometer> * data) {
        data->_value = rain();
    }

    static void actuate(unsigned int dev, Smart_Data<Pluviometer> * data, const Smart_Data<Pluviometer>::Value & command) {}
};

class Current_Sensor: public Smart_Plug
{
public:
    static const unsigned int UNIT = TSTP::Unit::Current;
    static const unsigned int NUM = TSTP::Unit::I32;
    static const int ERROR = 0; // Unknown

    static const bool INTERRUPT = false;
    static const bool POLLING = true;

    typedef Smart_Plug::Observer Observer;
    typedef Smart_Plug::Observed Observed;

public:
    static void sense(unsigned int dev, Smart_Data<Current_Sensor> * data) {
        data->_value = current(dev);
    }

    static void actuate(unsigned int dev, Smart_Data<Current_Sensor> * data, const Smart_Data<Current_Sensor>::Value & command) {
        Smart_Plug::actuate(dev, command);
    }
};

class ADC_Sensor // TODO
{
public:
    static const unsigned int UNIT = TSTP::Unit::Luminous_Intensity;
    static const unsigned int NUM = TSTP::Unit::I32;
    static const int ERROR = 0; // Unknown

    static const bool INTERRUPT = false;
    static const bool POLLING = true;

    typedef _UTIL::Observer Observer;
    typedef _UTIL::Observed Observed;

public:
    ADC_Sensor() {}

    static void sense(unsigned int dev, Smart_Data<ADC_Sensor> * data) {
        ADC adc(static_cast<ADC::Channel>(dev));
        data->_value = adc.read();
    }

    static void actuate(unsigned int dev, Smart_Data<ADC_Sensor> * data, const Smart_Data<ADC_Sensor>::Value & command) {}

    static void attach(Observer * obs) { _observed.attach(obs); }
    static void detach(Observer * obs) { _observed.detach(obs); }

private:
    static bool notify() { return _observed.notify(); }

    static void init();

private:
    static Observed _observed;
};

class Temperature_Sensor // TODO
{
public:
    static const unsigned int UNIT = TSTP::Unit::Temperature;
    static const unsigned int NUM = TSTP::Unit::I32;
    static const int ERROR = 0; // Unknown

    static const bool INTERRUPT = false;
    static const bool POLLING = true;

    typedef _UTIL::Observer Observer;
    typedef _UTIL::Observed Observed;

public:
    Temperature_Sensor() {}

    static void sense(unsigned int dev, Smart_Data<Temperature_Sensor> * data) {
        ADC adc(static_cast<ADC::Channel>(dev));
        data->_value = adc.read();
    }

    static void actuate(unsigned int dev, Smart_Data<Temperature_Sensor> * data, const Smart_Data<Temperature_Sensor>::Value & command) {}

    static void attach(Observer * obs) { _observed.attach(obs); }
    static void detach(Observer * obs) { _observed.detach(obs); }

private:
    static bool notify() { return _observed.notify(); }

    static void init();

private:
    static Observed _observed;
};

class Switch_Sensor: public GPIO
{
    static const unsigned int MAX_DEVICES = 8;

public:
    static const unsigned int UNIT = CUSTOM_UNITS::UNIT_SWITCH;
    static const unsigned int NUM = TSTP::Unit::I32;
    static const int ERROR = 0; // Unknown

    static const bool INTERRUPT = true;
    static const bool POLLING = true;

    typedef _UTIL::Observer Observer;
    typedef _UTIL::Observed Observed;

public:
    Switch_Sensor(unsigned int dev, char port, unsigned int pin, const GPIO::Direction & dir, const GPIO::Pull & p = GPIO::UP)
    : GPIO(port, pin, dir, p) {
        assert(dev < MAX_DEVICES);
        _dev[dev] = this;
    }

    Switch_Sensor(unsigned int dev, char port, unsigned int pin, const GPIO::Direction & dir, const GPIO::Pull & p, const GPIO::Edge & int_edge)
    : GPIO(port, pin, dir, p, &int_handler, int_edge) {
        assert(dev < MAX_DEVICES);
        _dev[dev] = this;
    }

    static void sense(unsigned int dev, Smart_Data<Switch_Sensor> * data) {
        assert(dev < MAX_DEVICES);
        data->_value = _dev[dev]->get();
    }

    static void actuate(unsigned int dev, Smart_Data<Switch_Sensor> * data, const Smart_Data<Switch_Sensor>::Value & command) {
        assert(dev < MAX_DEVICES);
        _dev[dev]->set(command);
        data->_value = command;
    }

    static void attach(Observer * obs) { _observed.attach(obs); }
    static void detach(Observer * obs) { _observed.detach(obs); }

private:
    static void int_handler(const IC::Interrupt_Id & id) { notify(); }

    static bool notify() { return _observed.notify(); }

    static void init();

private:
    static Observed _observed;
    static Switch_Sensor * _dev[MAX_DEVICES];
};

class RFID_Sensor
{
    static const unsigned int MAX_DEVICES = 8;

public:
    static const unsigned int UNIT = CUSTOM_UNITS::UNIT_RFID;
    static const unsigned int NUM = TSTP::Unit::I32;
    static const int ERROR = 0; // Unknown

    static const bool INTERRUPT = true;
    static const bool POLLING = false;

    typedef RFID_Reader::Observer Observer;
    typedef RFID_Reader::Observed Observed;

public:
    class Data : private RFID_Reader::UID
    {
    public:
        enum Code {
            DENIED     = 0,
            OPEN_NOW   = 1 << 0,
            AUTHORIZED = 1 << 1,
        };

        Data() : _code(0) {}
        Data(unsigned int v) : RFID_Reader::UID(v >> 8), _code(v) {}
        Data(const RFID_Reader::UID & u) : RFID_Reader::UID(u) {}
        Data(const RFID_Reader::UID & u, unsigned char code) : RFID_Reader::UID(u), _code(code) {}

        operator unsigned int() const {
            unsigned int i = RFID_Reader::UID::operator unsigned int();
            return (i << 8) | _code;
        }

        const RFID_Reader::UID & uid() const { return *this; }

        unsigned char code() const { return _code; }
        void code(unsigned char c) { _code = c; }

        bool authorized() const { return _code & AUTHORIZED; }
        void authorized(bool a) { if(a) _code |= AUTHORIZED; else _code &= ~AUTHORIZED; }
        bool open() const { return _code & OPEN_NOW; }
        void open(bool a) { if(a) _code |= OPEN_NOW; else _code &= ~OPEN_NOW; }

    private:
        unsigned char _code;
    }__attribute__((packed));

    RFID_Sensor(unsigned int dev, SPI * reader_spi, GPIO * reader_gpio0, GPIO * reader_gpio1)
    : _device(reader_spi, reader_gpio0, reader_gpio1)
    {
        assert(dev < MAX_DEVICES);
        _dev[dev] = this;
    }

    ~RFID_Sensor() {
        for(unsigned int i = 0; i < MAX_DEVICES; i++)
            if(_dev[i] == this)
                _dev[i] = 0;
    }

    static void sense(unsigned int dev, Smart_Data<RFID_Sensor> * data) {
        assert(dev < MAX_DEVICES);
        if(_dev[dev])
            _dev[dev]->sense(data);
    }

    static void actuate(unsigned int dev, Smart_Data<RFID_Sensor> * data, const Smart_Data<RFID_Sensor>::Value & command) {
        data->_value = command;
    }

    static void attach(Observer * obs) { RFID_Reader::attach(obs); }
    static void detach(Observer * obs) { RFID_Reader::detach(obs); }

private:
    void sense(Smart_Data<RFID_Sensor> * data) {
        if(_device.ready_to_get()) {
            Data id_with_code = _device.get();
            data->_value = id_with_code;
        } else
            data->_value = 0;
    }

private:
    RFID_Reader _device;
    static Observed _observed;
    static RFID_Sensor * _dev[MAX_DEVICES];
};

typedef Smart_Data<Current_Sensor> Current;
typedef Smart_Data<ADC_Sensor> Luminous_Intensity;
typedef Smart_Data<Temperature_Sensor> Temperature;
typedef Smart_Data<Water_Flow_Sensor> Water_Flow;
typedef Smart_Data<Water_Level_Sensor> Water_Level;
typedef Smart_Data<Water_Turbidity_Sensor> Water_Turbidity;
typedef Smart_Data<Pluviometer> Rain;
typedef Smart_Data<RFID_Sensor> RFID;
typedef Smart_Data<Switch_Sensor> Presence;
typedef Smart_Data<Switch_Sensor> Switch;


#endif

typedef Smart_Data<Keyboard_Sensor> Acceleration;

__END_SYS

#endif
