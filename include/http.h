// EPOS Hypertext Transfer Protocol Declarations

#ifndef __http_h
#define __http_h

#include <nic.h>

__BEGIN_SYS

// Sends Quectel AT Commands. Used with a Quectel HTTP-enabled NIC (such as M95)
class Quectel_HTTP
{
    template<int unit> friend void call_init();
    template<typename Type, int unit> friend void call_init();

public:
    static int post(const char * url, const void * data, unsigned int data_size) {
        int ret;
        char command[25];

        strncpy(command, "AT+QHTTPURL=", 12);
        command[12 + utoa(strlen(url), command + 12)] = '\0';
        strcat(command, ",5");

        for(unsigned int i = 0; i < Traits<NIC>::UNITS; i++) {
            if(!_networks[i])
                continue;
            ret = _networks[i]->nic()->send(_networks[i]->nic()->broadcast(), 0, command, strlen(command));
            if(ret > 0) {
                ret = _networks[i]->nic()->send(_networks[i]->nic()->broadcast(), 0, url, strlen(url));
                if(ret > 0)
                    ret = post(data, data_size);
            }
            if(ret > 0)
                break;
        }

        return ret;
    }

    static int post(const void * data, unsigned int data_size) {
        int ret;
        char command[27];

        strncpy(command, "AT+QHTTPPOST=", 13);
        command[13 + utoa(data_size, command + 13)] = '\0';
        strcat(command, ",5,5");

        for(unsigned int i = 0; i < Traits<NIC>::UNITS; i++) {
            if(!_networks[i])
                continue;
            ret = _networks[i]->nic()->send(_networks[i]->nic()->broadcast(), 0, command, strlen(command));
            if(ret > 0)
                ret = _networks[i]->nic()->send(_networks[i]->nic()->broadcast(), 0, data, data_size);
            if(ret > 0)
                break;
        }

        return ret;
    }

protected:
    Quectel_HTTP(const NIC & nic) : _nic(nic) { }

public:
    ~Quectel_HTTP();

    NIC * nic() { return &_nic; }

    static Quectel_HTTP * get_by_nic(unsigned int unit) {
        if(unit >= Traits<NIC>::UNITS) {
            db<Quectel_HTTP>(WRN) << "Quectel_HTTP::get_by_nic: requested unit (" << unit << ") does not exist!" << endl;
            return 0;
        } else
            return _networks[unit];
    }

private:
    template<unsigned int UNIT>
    static void init(const NIC & nic);

protected:
    NIC _nic;

    static Quectel_HTTP * _networks[Traits<NIC>::UNITS];
};

__END_SYS

#endif
