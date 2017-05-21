#include <utility/ostream.h>
#include <utility/string.h>
#include <secure_nic.h>
#include <gpio.h>
#include <machine/cortex_m/emote3_gptm.h>
#include <alarm.h>
#include <utility/key_database.h>
#include <nic.h>
#include "samsung_ac_message.h" //struct Message

using namespace EPOS;

#define MAX_MSG_SIZE 100 //bytes
OStream cout;
GPIO * led;
NIC * nic;
const char Traits<Build>::ID[Traits<Build>::ID_SIZE] = {'F','0'};
NIC::Address to;

class Samsung_AC_Definitions
{
public:
    Samsung_AC_Definitions () { }

    enum Mode {
        COOL    = 0x08,
        DRY     = 0x04,
        FAN     = 0x0C,
        HEAT    = 0x02,
        AUTO    = 0x00,
    };

    enum Fan {
        FAN1            = 0x02,
        FAN2            = 0x01,
        FAN3            = 0x06,
        FAN_AUTO1       = 0x00, //Dry, Hot, Cool
        FAN_AUTO2       = 0x03, //AUTO Mode
    };

    enum Turbo {
        TURBO_ON      = 0x03,
        TURBO_OFF     = 0x00,
    };

    enum Swing {
        SWING_OFF      = 0x7F,
        SWING_ON     = 0xF5,
    };

    enum Temperature {
        T30 = 0x07,
        T29 = 0x0B,
        T28 = 0x03,
        T27 = 0x0D,
        T26 = 0x05,
        T25 = 0x09,
        T24 = 0x01,
        T23 = 0x0E,
        T22 = 0x06,
        T21 = 0x0A,
        T20 = 0x02,
        T19 = 0x0C,
        T18 = 0x04,
        T17 = 0x08,
        T16 = 0x00,
    };
};

int map_temperature(int temp)
{
    if(temp < 16 || temp > 30)
        return -1;

    switch(temp) {
    case 16:
        return Samsung_AC_Definitions::T16;
    case 17:
        return Samsung_AC_Definitions::T17;
    case 18:
        return Samsung_AC_Definitions::T18;
    case 19:
        return Samsung_AC_Definitions::T19;
    case 20:
        return Samsung_AC_Definitions::T20;
    case 21:
        return Samsung_AC_Definitions::T21;
    case 22:
        return Samsung_AC_Definitions::T22;
    case 23:
        return Samsung_AC_Definitions::T23;
    case 24:
        return Samsung_AC_Definitions::T24;
    case 25:
        return Samsung_AC_Definitions::T25;
    case 26:
        return Samsung_AC_Definitions::T26;
    case 27:
        return Samsung_AC_Definitions::T27;
    case 28:
        return Samsung_AC_Definitions::T28;
    case 29:
        return Samsung_AC_Definitions::T29;
    case 30:
        return Samsung_AC_Definitions::T30;
    }
}

int map_mode(char mode)
{
    if(mode < '0' || mode > '4')
        return -1;

    switch(mode) {
    case '0':
        return Samsung_AC_Definitions::COOL;
    case '1':
        return Samsung_AC_Definitions::DRY;
    case '2':
        return Samsung_AC_Definitions::FAN;
    case '3':
        return Samsung_AC_Definitions::HEAT;
    case '4':
        return Samsung_AC_Definitions::AUTO;
    }
}

int map_fan(char mode)
{
    if(mode < '0' || mode > '4')
        return -1;
    switch(mode) {
    case '0':
        return Samsung_AC_Definitions::FAN1;
    case '1':
        return Samsung_AC_Definitions::FAN2;
    case '2':
        return Samsung_AC_Definitions::FAN3;
    case '3':
        return Samsung_AC_Definitions::FAN_AUTO1;
    case '4':
        return Samsung_AC_Definitions::FAN_AUTO2;
    }
}

int map_swing(char mode)
{
    if(mode < '0' || mode > '1')
        return -1;
    if(mode == '0')
        return Samsung_AC_Definitions::SWING_OFF;
    else
        return Samsung_AC_Definitions::SWING_ON;
}

int map_turbo(char mode)
{
    if(mode < '0' || mode > '1')
        return -1;

    if(mode == '0')
        return Samsung_AC_Definitions::TURBO_OFF;
    else
        return Samsung_AC_Definitions::TURBO_ON;
}

void get_id(char *data)
{
    int i;
    for(i = 0; i < strlen(data); i++) {
        if(data[i] == 'R' || data[i] == 'r') { //receiver ID - 2 characters
            i++;
            cout << "data[" << i << "] = " << data[i] << endl;
            to[0] = data[i++];
            cout << "data[" << i << "] = " << data[i] << endl;
            to[1] = data[i];
            cout << "to=" << to << endl;
            break;
        }
    }
}

void send_command(char *data)
{
    int i, j;
    char tmp[3];
    Message msg;

    get_id(data);

    cout << "Sending a message to " << to << endl;

    if(data[0] == 'L' || data[0] == 'l') { //turn on
        msg.command = '0';
    } else if(data[0] == 'D' || data[0] == 'd') { //turn off
        msg.command = '1';
    } else {
        msg.command = '2';
        for(i = 0; i < strlen(data); i++) {
            switch(data[i]) {
            case 'T': //temperature
            case 't':
                i++;
                for(j = 0; j < 2; j++)
                    tmp[j] = data[i+j];

                tmp[j] = '\0';

                msg.temp = map_temperature(atoi(tmp));

                if(msg.temp == -1) {
                    cout << "Temperature must be between 16 and 30 degrees!\n";
                    return;
                }

                cout << "Temp = " << msg.temp << endl;

                break;

            case 'M': //mode
            case 'm':
                i++;
                msg.mode = map_mode(data[i]);

                if(msg.mode == -1) {
                    cout << "Invalid mode!\n";
                    return;
                }
                cout << "Mode = " << msg.mode << endl;

                break;

            case 'F': //Fan
            case 'f':
                i++;
                msg.fan = map_fan(data[i]);

                if(msg.fan == -1) {
                    cout << "Invalid FAN mode!\n";
                    return;
                }
                cout << "Fan = " << msg.fan << endl;

                break;

            case 'S': //swing
            case 's':
                i++;
                msg.swing = map_swing(data[i]);

                if(msg.swing == -1) {
                    cout << "Invalid Swing mode!\n";
                    return;
                }
                cout << "Swing = " << msg.swing << endl;

                break;

            case 'U': //turbo
            case 'u':
                i++;
                msg.turbo = map_turbo(data[i]);

                if(msg.turbo == -1) {
                    cout << "Invalid Turbo mode!\n";
                    return;
                }
                cout << "Turbo = " << msg.turbo << endl;

                break;
            }
        }
    }

    nic->send(to, Traits<Secure_NIC>::PROTOCOL_ID, (const void *) &msg, sizeof(Message));
}

int main()
{
    cout << "Hello main" << endl;
    char data[MAX_MSG_SIZE];

    led = new GPIO('c',3, GPIO::OUTPUT);
    led->set(1);

    nic = new NIC();
    NIC::Address addr;
    addr[0] = Traits<Build>::ID[0];
    addr[1] = Traits<Build>::ID[1];
    nic->address(addr);

    cout << "My NIC Address: " << nic->address() << endl;
    cout << "I will wait for a command to be sent to the Samsung AC!" << endl;

    unsigned int i = 0;
    while(1)
    {
        /* Expected data format
         * Important: all commands must end with the '-' character
         * Turn AC on: DRXX- -> D or d is the command to turn AC on
         * R indicates the receiver ID (2 hexa characters)
         *
         * Turn AC off> LRXX- -> the same as before, but changing D or d to L or l
         *
         * Command: TXXMXXFXSXUXRXXXX-
         * T or t -> is the temperature (16 to 30)
         * M or m -> is the mode (0 COOL, 1 DRY, 2 FAN, 3 HEAT, 4 AUTO)
         * F or f -> is the fan mode (0 FAN1, 1 FAN2, 2 FAN3, 3 FAN_AUTO1, 4 FAN_AUTO2)
         * S or s -> swing mode (0 OFF, 1 ON)
         * U or u -> turbo mode (0 OFF, 1 ON)
         * R or -> the receiver ID (2 hexa characters)
         * - -> ending character
         * Example of commands:
         * t20m2f1s0u0rF1-
         * lrF1-
         * drF1-
         */

        data[i] = USB::get();
        if(data[i] == '-') {
            int size = i;
            data[i] = '\0';
            cout << "Data  = " << data << endl;
            send_command(data);
            i = -1;
        }
        i++;
    }

    return 0;
}
