#include <alarm.h>
#include <gpio.h>
#include <utility/ostream.h>
#include <uart.h>
#include <adc.h>
#include <machine/cortex_m/emote3_gprs.h>
#include <machine/cortex_m/emote3_gptm.h>
#include <flash.h>

// This application logs two ADC conversions on the flash memory every five
// minutes. On reset, it dumps the flash contents and the last position that
// was actually read on the current run.

using namespace EPOS;

const auto MINUTES = 5u;
//const auto MINUTES = 1u;
const auto MINUTE_IN_US = 60000000u;

const auto IDENTIFIER = "hydro_joi_1";
const auto DATA_SERVER = "http://150.162.216.118:80/hidro";
const auto ERROR_SERVER = "http://150.162.216.118:80/errors";

#define FLASH_CURRENT_VALUE 128*1024 //flash start address is 128k. This address will storage the current flash address that is being written
#define FLASH_START_ADDRESS FLASH_CURRENT_VALUE+sizeof(unsigned int) //flash start address for storage (128k + 4)
#define FLASH_DATA_SIZE 256*1024/4  //65536 is the maximum number of words to be written to the flash (256k of storage)

volatile unsigned int current_flash_address;

//number of messages that were not sent. If it is equal to 3, reboot the mote
volatile unsigned int unsent_msgs;

OStream cout;

int verify_and_set_current_flash_address();
void verify_flash_and_send(eMote3_GPRS *gprs, char buf[], char aux[]);
void mount_message_to_send(char msg[], char aux[], unsigned int data[]);
int send_data(eMote3_GPRS *gprs, const char msg[], unsigned int data[]);
int send_data_or_storage(eMote3_GPRS *gprs, const char msg[], unsigned int data[]);


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
    Sensor_Base(GPIO &relay):
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
    GPIO &relay;
};

class Analog_Sensor_Base: public Sensor_Base{
public:
    Analog_Sensor_Base(ADC &adc, GPIO &relay):
        Sensor_Base(relay),
        adc(adc)
    {}

protected:
    ADC &adc;
};

class Level_Sensor: public Analog_Sensor_Base {
public:
    Level_Sensor(ADC &adc, GPIO &relay):
        Analog_Sensor_Base{adc, relay}
    {}

    int sample()
    {
        return adc.read();
    }
};

class Turbidity_Sensor: public Analog_Sensor_Base {
public:
    Turbidity_Sensor(ADC &adc, GPIO &relay, GPIO &infrared):
        Analog_Sensor_Base{adc, relay},
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


//return the message number of the last sent message
int verify_and_set_current_flash_address() {
    int seq = 1;
    current_flash_address = Flash::read(FLASH_CURRENT_VALUE);
    cout << "Current flash address to be written=" << current_flash_address << endl;
    if(current_flash_address == 0) {
        current_flash_address = FLASH_START_ADDRESS;
        return seq;
    } else {
        for(unsigned int i = 0; i < FLASH_DATA_SIZE; i += (3 * 4)) {
            int tmp = Flash::read(FLASH_START_ADDRESS + i);
            if(tmp == 0)
                return seq + 1;
            seq = tmp;
        }
    }
    return seq + 1;
}

// This function verifies whether a data was saved into the flash or not.
// If all flash addresses contain zero, it means that none data has been saved.
// Otherwise, whenever a value different than zero is found, try to send it
void verify_flash_and_send(eMote3_GPRS *gprs, char buf[], char aux[]) {
    unsigned int data[3];
    unsigned int zero = 0;
    bool data_not_sent = false;

    //there is no data to be sent
    if(current_flash_address == FLASH_START_ADDRESS) {
        cout << "verify_flash_and_send() no data to be sent, returning..\n";
        return;
    }

    for(unsigned int i = 0; i < FLASH_DATA_SIZE; i += (3 * 4)) {
        data[0] = Flash::read(FLASH_START_ADDRESS + i);
        if(data[0] != 0) {
            data[1] = Flash::read(FLASH_START_ADDRESS + (i + 4));
            data[2] = Flash::read(FLASH_START_ADDRESS + (i + 8));

            cout << "Data[0]=" << data[0] << " saved in the flash at address=" << FLASH_START_ADDRESS + (i) << endl;

            mount_message_to_send(buf, aux, data);

            cout << "Sending: " <<  buf << endl;

            auto send = send_data(gprs, buf, data);

            //if data has been sent, erase it from the flash
            if(send) {
                Flash::write(FLASH_START_ADDRESS + i, &zero, sizeof(unsigned int));
                Flash::write(FLASH_START_ADDRESS + (i + 4), &zero, sizeof(unsigned int));
                Flash::write(FLASH_START_ADDRESS + (i + 8), &zero, sizeof(unsigned int));
            } else
                data_not_sent = true;

            eMote3_GPTM::delay(5000000); //5 sec delay before sending again if needed

        } else {
            break;
        }
    }

    //all data in flash has been sent. Restart the counter
    if(!data_not_sent)
        current_flash_address = FLASH_START_ADDRESS;
}

// Receives a buffer (msg) to be send by GPRS a sequence number and the data from sensors (level and turbidity).
// Then, mounts the message in the format to be sent
// TODO: if aux is locally declared, the mote restarts after a while
void mount_message_to_send(char msg[], char aux[], unsigned int data[]) {
    strcpy(msg, "data=");
    strcat(msg, IDENTIFIER);
    strcat(msg, ",");
    aux[utoa(data[0], (char *)aux)] = '\0';
    strcat(msg, (char *)aux);
    strcat(msg, ",");
    aux[utoa(data[1], (char *)aux)] = '\0';
    strcat(msg, (char *)aux);
    strcat(msg, ",");
    aux[utoa(data[2], (char *)aux)] = '\0';
    strcat(msg, (char *)aux);
}

int send_data(eMote3_GPRS *gprs, const char msg[], unsigned int data[])
{
    cout << "Sending [0]=" << data[0] << " [1]=" << data[1] << " [2]=" << data[2] << endl;
    auto send = gprs->send_http_post(DATA_SERVER, msg, (unsigned int) strlen(msg));

    //try to send again for 2 times
    if(!send) {
        for(unsigned int i = 0; i < 2; i++) {
            blink_led(5, 500000);

            gprs->off();

            dog_status = true;
            gprs->on();
            dog_status = false;

            gprs->use_dns(); // This parameter is (or should be) reset when the module resets.

            eMote3_GPTM::delay(5000000); // Make sure network is up.

            char error[40] = "error=";
            strcat(error, IDENTIFIER);
            strcat(error, ",");
            strcat(error, "needed_gprs_reset");

            cout << "Error: " << error << endl;
            cout << "Sending again: " << msg << endl;

            gprs->send_http_post(ERROR_SERVER, error, (unsigned int) strlen(error));
            send = gprs->send_http_post(DATA_SERVER, msg, (unsigned int) strlen(msg));
            if(send)
                break;
        }
    }

    return send;
}

int send_data_or_storage(eMote3_GPRS *gprs, const char msg[], unsigned int data[])
{
    auto send = send_data(gprs, msg, data);

    //data has not been sent after 3 attempts, store data into the flash
    if(!send) {
        // Write seq, level, turbidity to flash
        cout << "Storing [0]=" << data[0] << " [1]=" << data[1] << " [2]=" << data[2] << " starting at addr=" << current_flash_address << endl;
        Flash::write(current_flash_address, data, sizeof(unsigned int) * 3); //data has size of 3
        current_flash_address += (3 * sizeof(unsigned int));
        Flash::write(FLASH_CURRENT_VALUE, (unsigned int *) &current_flash_address, sizeof(unsigned int));
        unsent_msgs++;
    }

    return send;
}

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

    unsent_msgs = 0;

    level.disable();
    turbidity.disable();

    //[0] = message number, [1] = level sensor, [2] = turbidity sensor
    unsigned int data[3];
    char buf[100];
    char aux[32] = "";

    //auto pwrkey = GPIO{'d', 3, GPIO::OUTPUT};
    auto pwrkey = GPIO{'c', 4, GPIO::OUTPUT};
    //auto status = GPIO{'d', 5, GPIO::INPUT};
    auto status = GPIO{'c', 1, GPIO::INPUT};
    auto uart = UART{9600, 8, 0, 1, 1};

    //updates the current flash address that will be used when the GPRS fails to send a message
    auto seq = verify_and_set_current_flash_address();

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
        if(!status.get())
            gprs.on();
        cout << "send command AT+CGDCONT?\n";
        gprs.send_command("AT+CGDCONT?");
        res = gprs.await_response("OK", 3000000);
        eMote3_GPTM::delay(5000000);
    }

    res = false;
    while(!res) {
        cout << "send command AT+CGATT=1\n";
        gprs.send_command("AT+CGATT=1");
        res = gprs.await_response("OK", 3000000);
        cout << "res = " << res << endl;
        cout << "status is " << status.get() << "\n";
        if(status.get() == 0)
            gprs.on();
        eMote3_GPTM::delay(3000000);
    }

    res = false;
    while(!res) {
        cout << "send command AT+CGACT=1,1\n";
        gprs.send_command("AT+CGACT=1,1");
        res = gprs.await_response("OK", 300000);
        cout << "res = " << res << endl;
        cout << "status is " << status.get() << "\n";
        if(status.get() == 0)
            gprs.on();
        eMote3_GPTM::delay(3000000);
    }

    eMote3_GPTM timer(1);

    while (true) {

        timer.set(MINUTE_IN_US * 2);
        timer.enable();

        cout << "1\n";
        level.enable();
        turbidity.enable();

        eMote3_GPTM::delay(3000000);

        data[0] = seq;
        data[1] = level.sample();
        data[2] = turbidity.sample();

        cout << "2\n";

        level.disable();
        turbidity.disable();

        cout << "3\n";

        //needs to be before calling mount_message_to_send(), because buf and aux are global variables
        verify_flash_and_send(&gprs, buf, aux);

        buf[0] = '\0';
        aux[0] = '\0';

        cout << "4\n";

        mount_message_to_send(buf, aux, data);

        if (!status.get()) {
            dog_status = true;
            gprs.on();
            dog_status = false;

            gprs.use_dns(); // This parameter is (or should be) reset when the module resets.
        }

        cout << "Sending: " << buf << endl;

        auto send = send_data_or_storage(&gprs, buf, data);

        //tried to sent the message for 3 times and it couldn't, reboot the system
        if(unsent_msgs == 3) {
            cout << "3 Messages were not sent. Rebooting...\n";
            Machine::reboot();
        }

        ++seq;

        // Sleep until the end of the first two minutes
        while(timer.running());

        for (auto i = 0u; i < MINUTES - 2; ++i) {
            eMote3_GPTM::delay(MINUTE_IN_US);
        }
    }

    while (true);
}
