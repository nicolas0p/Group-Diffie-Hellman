// EPOS ARM Cortex I2C Mediator Declarations

#include <system/config.h>
#ifdef __I2C_H

#ifndef __cortex_i2c_h__
#define __cortex_i2c_h__

#include __MODEL_H

#include <i2c.h>

__BEGIN_SYS

// TODO: It looks like this class only implements Master operation
class I2C : private Machine_Model, private I2C_Common
{
public:
    I2C(Role r, char port_sda, unsigned int pin_sda, char port_scl, unsigned int pin_scl)
    : _base(r == I2C_Common::MASTER ? reinterpret_cast<Log_Addr*>(I2C_MASTER_BASE) : reinterpret_cast<Log_Addr*>(I2C_SLAVE_BASE)) {
        Machine_Model::i2c_config(port_sda, pin_sda, port_scl, pin_scl);
        if(r == I2C_Common::MASTER) {
            reg(I2C_CR) = I2C_CR_MFE;
            reg(I2C_TPR) = 0x3; // 400kHz, assuming a system clock of 32MHz
        } else {
            reg(I2C_CR) = I2C_CR_SFE;
        }
    }

    bool ready_to_put() { return !(reg(I2C_STAT) & I2C_STAT_BUSY); }
    bool ready_to_get() { return ready_to_put(); }

    bool put(unsigned char slave_address, const char * data, unsigned int size, bool stop) {
        bool ret = true;
        // Specify the slave address and that the next operation is a write (last bit = 0)
        reg(I2C_SA) = (slave_address << 1) & 0xFE;
        for(unsigned int i = 0; i < size; i++) {
            if(i == 0) //first byte to be sent
                ret = send_byte(data[i], I2C_CTRL_RUN | I2C_CTRL_START);
            else if(i + 1 == size)
                ret = send_byte(data[i], I2C_CTRL_RUN | (stop ? I2C_CTRL_STOP : 0));
            else
                ret = send_byte(data[i], I2C_CTRL_RUN);

            if(!ret)
                return false;
        }
        return ret;
    }

    bool put(unsigned char slave_address, char data, bool stop = true) {
        // Specify the slave address and that the next operation is a write (last bit = 0)
        reg(I2C_SA) = (slave_address << 1);
        return send_byte(data, I2C_CTRL_RUN | I2C_CTRL_START | (stop ? I2C_CTRL_STOP : 0));
    }

    bool get(char slave_address, char * data, bool stop = true) {
        // Specify the slave address and that the next operation is a read (last bit = 1)
        reg(I2C_SA) = (slave_address << 1) | 0x01;
        return get_byte(data, I2C_CTRL_RUN | I2C_CTRL_START | (stop ? I2C_CTRL_STOP : 0));

    }

    bool get(char slave_address, char * data, unsigned int size, bool stop) {
        unsigned int i;
        bool ret = true;
        // Specify the slave address and that the next operation is a read (last bit = 1)
        reg(I2C_SA) = (slave_address << 1) | 0x01;
        for(i = 0; i < size; i++, data++) {
            if(i == 0)
                ret = get_byte(data, I2C_CTRL_START | I2C_CTRL_RUN | I2C_CTRL_ACK);
            else if(i + 1 == size)
                ret = get_byte(data, I2C_CTRL_RUN | (stop ? I2C_CTRL_STOP : 0));
            else
                ret = get_byte(data, I2C_CTRL_RUN | I2C_CTRL_ACK);
            if(!ret)
                return ret;
        }
        return ret;
    }

private:
    bool send_byte(char data, int mode) {
        reg(I2C_DR) = data;
        reg(I2C_CTRL) = mode;
        while(!ready_to_put());
        return !(reg(I2C_STAT) & I2C_STAT_ERROR);
    }

    bool get_byte(char * data, int mode) {
        reg(I2C_CTRL) = mode;
        while(!ready_to_get());
        if(reg(I2C_STAT) & I2C_STAT_ERROR) {
            return false;
        } else {
            *data = reg(I2C_DR);
            return true;
        }
    }

private:
    volatile Log_Addr * _base;
    volatile Reg32 & reg(unsigned int o) { return reinterpret_cast<volatile Reg32 *>(_base)[o / sizeof(Reg32)]; }
};

// Si7020 Temperature and Humidity sensor
class I2C_Sensor_Engine
{
private:
    //Addresses
    enum {
        I2C_ADDR        = 0x40, // The 7-bit base slave address is 0x40
        RH_HOLD         = 0xE5, // Measure Relative Humidity, Hold Master Mode
        RH_NOHOLD       = 0xF5, // Measure Relative Humidity, No Hold Master Mode
        TEMP_HOLD       = 0xE3, // Measure Temperature, Hold Master Mode
        TEMP_NOHOLD     = 0xF3, // Measure Temperature, No Hold Master Mode
        TEMP_PREV       = 0xE0, // Read Temperature Value from Previous RH Measurement
        RESET_SI        = 0xFE, // Reset
        WREG            = 0xE6, // Write RH/T User Register 1
        RREG            = 0xE7, // Read RH/T User Register 1
        WHCR            = 0x51, // Write Heater Control Register
        RHCR            = 0x11, // Read Heater Control Register
    };

    //Resolution
    enum {
        RH_12B_TEMP_14B = 0x0, /// < 12 bits for RH, 14 bits for Temp
        RH_8B_TEMP_12B  = 0x1, /// < 8  bits for RH, 12 bits for Temp
        RH_10B_TEMP_13B = 0x2, /// < 10 bits for RH, 13 bits for Temp
        RH_11B_TEMP_11B = 0x3, /// < 11 bits for RH, 11 bits for Temp
    };

public:
    static bool reset(I2C * i2c) { 
        bool ret = i2c->put(I2C_ADDR, RESET_SI, true);
        if(ret)
            Machine::delay(15000); // Si7020 may take up to 15ms to power up after a soft reset
        return ret;
    }

    static int relative_humidity(I2C * i2c) {
        char data[2];
        i2c->put(I2C_ADDR, RH_HOLD, false);

        i2c->get(I2C_ADDR, data, 2, true);

        int ret = (data[0] << 8 ) | data[1];

        ret = (1250 * ret) / 65536 - 60;
        ret = (ret + 5) / 10;

        // the measured value of %RH may be slightly greater than 100
        // when the actual RH level is close to or equal to 100
        if(ret < 0) ret = 0;
        else if(ret > 100) ret = 100;
        return ret;
    }

    static int celsius(I2C * i2c) {
        char data[2];
        i2c->put(I2C_ADDR, TEMP_HOLD, false);

        i2c->get(I2C_ADDR, data, 2, true);

        int ret = (data[0] << 8) | data[1];
        ret = ((17572 * ret)) / 65536 - 4685;
        ret = ret + 50 / 100;
        return ret;
    }
};

class I2C_Temperature_Sensor: private I2C_Sensor_Engine
{
public:
    I2C_Temperature_Sensor(char port_sda = 'B', unsigned int pin_sda = 1, char port_scl = 'B', unsigned int pin_scl = 0) : _i2c(I2C_Common::MASTER, port_sda, pin_sda, port_scl, pin_scl) { I2C_Sensor_Engine::reset(&_i2c); }

    int get() { while(!_i2c.ready_to_get()); return I2C_Sensor_Engine::celsius(&_i2c); }

private:
    I2C _i2c;
};

class I2C_Humidity_Sensor: private I2C_Sensor_Engine
{
public:
    I2C_Humidity_Sensor(char port_sda = 'B', unsigned int pin_sda = 1, char port_scl = 'B', unsigned int pin_scl = 0) : _i2c(I2C_Common::MASTER, port_sda, pin_sda, port_scl, pin_scl) { I2C_Sensor_Engine::reset(&_i2c); }

    int get() { while(!_i2c.ready_to_get()); return I2C_Sensor_Engine::relative_humidity(&_i2c); }

private:
    I2C _i2c;
};

__END_SYS

#endif

#endif
