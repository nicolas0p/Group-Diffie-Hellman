#include <alarm.h>
#include <gpio.h>
#include <utility/ostream.h>
#include <uart.h>
#include <adc.h>
#include <machine/cortex_m/emote3_gprs.h>
#include <machine/cortex_m/emote3_gptm.h>

// This application logs two ADC conversions on the flash memory every five
// minutes. On reset, it dumps the flash contents and the last position that
// was actually read on the current run.

using namespace EPOS;

const auto MINUTES = 5u;
const auto MINUTE_IN_US = 60000000u;

const auto IDENTIFIER = "Joi01";
const auto DATA_SERVER = "http://ap3.lisha.ufsc.br/hidro_test";
const auto ERROR_SERVER = "http://ap3.lisha.ufsc.br/errors";

OStream cout;

class Watchdog {
    const static unsigned int BASE = 0x400d5000;

    enum Offset {
        SMWDTHROSC_WDCTL = 0x0
    };

    typedef unsigned int Reg32;

    static volatile Reg32 &reg(Offset o) {
        return *(reinterpret_cast<Reg32*>(BASE + o));
    }

public:
    static volatile Reg32 &wdctl() {
        return reg(SMWDTHROSC_WDCTL);
    }

    enum Mask {
        ENABLE_2MS = 0xB, // really 1.9ms
        ENABLE_15MS = 0xA, // really 15.625ms
        ENABLE_25MS = 0x9,
        ENABLE_1S = 0x8
    };

    Watchdog(Mask timeout = ENABLE_1S)
    {
        wdctl() = timeout;
    }

    void kick() const
    {
        wdctl() = (wdctl() & 0xF) | (0xA << 4);
        wdctl() = (wdctl() & 0xF) | (0x5 << 4);
    }
};

unsigned int from_flash(unsigned int address)
{
    return *reinterpret_cast<unsigned int*>(address);
}

void blink_led(unsigned int times, unsigned int period = 50000)
{
    auto led = GPIO{'c', 3, GPIO::OUTPUT};
    for (auto i = 0u; i < times; ++i) {
        led.set();
        eMote3_GPTM::delay(period);
        led.clear();
        eMote3_GPTM::delay(period);
    }
}

volatile bool dog_status = false;

const unsigned int WD_TIMEOUT = 30000000;

void alarm_handler()
{
    cout << "watchdog overflow! rebooting...\n";
    Machine::reboot();
}

Function_Handler reboot_handler{alarm_handler};

void software_watchdog()
{
    static Alarm *alarm = 0;
    if (dog_status) {
        if (!alarm) {
            cout << "watchdog started\n";
            alarm = new Alarm(WD_TIMEOUT, &reboot_handler);
        }
    } else {
        delete alarm;
        //cout << "watchdog deleted\n";
        alarm = 0;
    }
}

Function_Handler wdt_handler{software_watchdog};
Alarm watchdog{1000000, &wdt_handler, Alarm::INFINITE};

class Sensor_Base {
public:
    Sensor_Base(ADC &adc, GPIO &relay):
        adc(adc),
        relay(relay)
    {}

    int enable()
    {
        relay.set();
    }

    int disable()
    {
        relay.clear();
    }

protected:
    ADC &adc;
    GPIO &relay;
};

class Level_Sensor: public Sensor_Base {
public:
    Level_Sensor(ADC &adc, GPIO &relay):
        Sensor_Base{adc, relay}
    {}

    int sample()
    {
        return adc.read();
    }
};

class Turbidity_Sensor: public Sensor_Base {
public:
    Turbidity_Sensor(ADC &adc, GPIO &relay, GPIO &infrared):
        Sensor_Base{adc, relay},
        infrared(infrared)
    {}

    int sample()
    {
        // Filter daylight as directed by sensor manufacturer
        eMote3_GPTM::delay(250000); // Wait 250 ms before reading
        auto daylight = adc.read();
        eMote3_GPTM::delay(250000); // Wait more 250 ms because we've been told to
        infrared.set();
        eMote3_GPTM::delay(450000); // Wait 200+250 ms before reading again
        auto mixed = adc.read();
        infrared.clear();
        return mixed - daylight;
    }

private:
    GPIO &infrared;
};



int main()
{
    auto level_toggle = GPIO{'b', 0, GPIO::OUTPUT};
    auto turbidity_infrared = GPIO{'b', 2, GPIO::OUTPUT};
    auto turbidity_toggle = GPIO{'b', 3, GPIO::OUTPUT};

    auto level_adc = ADC{ADC::SINGLE_ENDED_ADC2};
    auto turbidity_adc = ADC{ADC::SINGLE_ENDED_ADC5};

    auto level = Level_Sensor{level_adc, level_toggle};
    auto turbidity = Turbidity_Sensor{turbidity_adc,
                                      turbidity_toggle,
                                      turbidity_infrared};

    blink_led(10);

    level.disable();
    turbidity.disable();

    unsigned int sensors[2];

    auto pwrkey = GPIO{'d', 3, GPIO::OUTPUT};
    auto status = GPIO{'d', 5, GPIO::INPUT};
    auto uart = UART{9600, 8, 0, 1, 1};

    dog_status = true;
    auto gprs = eMote3_GPRS{pwrkey, status, uart};
    dog_status = false;

    cout << "gprs created and status is " << status.get() << "\n";
    blink_led(4, 100000);

    dog_status = true;
    gprs.on();
    dog_status = false;

    cout << "gprs on and status is " << status.get() << "\n";
    blink_led(4, 100000);

    gprs.use_dns();

    //codigo de teste, deve ser removido na versao final
    cout << "send command AT+CPIN? (sim card ready?)\n";
    auto res = gprs.sim_card_ready();
    eMote3_GPTM::delay(1000000);

    while(!res) {
        cout << "send command AT+CGDCONT=1,\"IP\",\"gprs.oi.com.br\"\n";
        gprs.send_command("AT+CGDCONT=1,\"IP\",\"gprs.oi.com.br\"");
        res = gprs.await_response("OK", 300000);
        cout << "res = " << res << endl;
        cout << "status is " << status.get() << "\n";

        cout << "send command AT+CGDCONT?\n";
        gprs.send_command("AT+CGDCONT?");
        res = gprs.await_response("OK", 3000000);
        eMote3_GPTM::delay(5000000);
    }

    res = false;
    while(!res) {
        cout << "send command AT+CGACT=1,1\n";
        gprs.send_command("AT+CGACT=1,1");
        res = gprs.await_response("OK", 300000);
        cout << "res = " << res << endl;
        cout << "status is " << status.get() << "\n";
    }

    res = false;
    while(!res) {
        cout << "send command AT+CGATT=1\n";
        gprs.send_command("AT+CGATT=1");
        res = gprs.await_response("OK", 3000000);
        cout << "res = " << res << endl;
        cout << "status is " << status.get() << "\n";
    }

    auto seq = 0u;

    while (true) {
        level.enable();
        turbidity.enable();

        eMote3_GPTM::delay(3000000);

        sensors[0] = level.sample();
        sensors[1] = turbidity.sample();

        level.disable();
        turbidity.disable();

        char buf[100] = "data=";
        char aux[32] = "";
        strcat(buf, IDENTIFIER);
        strcat(buf, ",");
        aux[utoa(seq, aux)] = '\0';
        strcat(buf, aux);
        strcat(buf, ",");
        aux[utoa(sensors[0], aux)] = '\0';
        strcat(buf, aux);
        strcat(buf, ",");
        aux[utoa(sensors[1], aux)] = '\0';
        strcat(buf, aux);

        if (!status.get()) {
            dog_status = true;
            gprs.on();
            dog_status = false;

            gprs.use_dns(); // This parameter is (or should be) reset when the module resets.
        }

        cout << "Sending: " << buf << endl;
        auto send = gprs.send_http_post(DATA_SERVER, buf, strlen(buf));
        cout << "send: " << send << endl;

        if (!send) {
            blink_led(5, 500000);

            gprs.off();

            dog_status = true;
            gprs.on();
            dog_status = false;

            gprs.use_dns(); // This parameter is (or should be) reset when the module resets.

            eMote3_GPTM::delay(5000000); // Make sure network is up.

            char error[100] = "error=";
            strcat(error, IDENTIFIER);
            strcat(error, ",");
            strcat(error, "needed_gprs_reset");

            cout << "Error: " << error << endl;
            cout << "Sending again: " << buf << endl;

            gprs.send_http_post(ERROR_SERVER, error, strlen(error));
            send = gprs.send_http_post(DATA_SERVER, buf, strlen(buf));
        }

        blink_led(send ? 1 : 3, 1000000);
        cout << "send:" << send << "\n";

        ++seq;

        for (auto i = 0u; i < MINUTES; ++i) {
            eMote3_GPTM::delay(MINUTE_IN_US);
        }
    }

    while (true);
}
