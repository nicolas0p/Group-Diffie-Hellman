// EPOS Quectel M95 GPRS NIC Mediator Initialization

#include <system/config.h>
#if defined(__NIC_H) && defined(__mmod_emote3__)

#include <machine/cortex/machine.h>
#include <machine/cortex/m95.h>
#include <machine/cortex/gpio.h>

__BEGIN_SYS

M95::M95(unsigned int unit): _unit(unit), _http_data_mode(false), _last_send(0)
{
    db<M95>(TRC) << "M95(unit=" << unit << ") => " << this << endl;

    _uart = new (SYSTEM) UART(Traits<M95>::UART_UNIT, Traits<M95>::UART_BAUD_RATE, Traits<M95>::UART_DATA_BITS, Traits<M95>::UART_PARITY, Traits<M95>::UART_STOP_BITS);

    _pwrkey = new (SYSTEM) GPIO(Traits<M95>::PWRKEY_PORT, Traits<M95>::PWRKEY_PIN, GPIO::OUT);
    _status = new (SYSTEM) GPIO(Traits<M95>::STATUS_PORT, Traits<M95>::STATUS_PIN, GPIO::IN, GPIO::DOWN);

    // Leave pwrkey signal down for one second
    _pwrkey->clear();
    Machine::delay(1000000);

    reset();
}

M95::~M95()
{
    off();
}

void M95::check_timeout()
{
    if(TSC::time_stamp() > _init_timeout) {
        off();
        CPU::int_disable();
        Machine::delay(5000000);
        Machine::reboot();
    }
}

void M95::on()
{
    db<M95>(TRC) << "M95::on()" << endl;

    _init_timeout = TSC::time_stamp() + 150000000ull * (TSC::frequency() / 1000000);

    if(_status->get()) {
        db<M95>(WRN) << "M95::on() => was already on!" << endl;
        return;
    }

    db<M95>(TRC) << "M95::on(): powering up" << endl;

    // Leave pwrkey up until status is stable at 1.
    _pwrkey->set();
    do {
        while(!_status->get())
            check_timeout();
        Machine::delay(1000000);
    } while(!_status->get());
    _pwrkey->clear();

    // Board takes 4 to 5 seconds to get ready
    Machine::delay(5500000);

    // Set baudrate
    send_command("AT", 2);
    // Turn off echo. This could fail a few times, until the board detects the baudrate
    do {
        check_timeout();
        const char cmd[] = "ATE0";
        send_command(cmd, strlen(cmd));
    } while(!wait_response("ATE0\r\r\nOK\r\n",300000));

    db<M95>(TRC) << "M95::on(): checking SIM card" << endl;
    // Check SIM card
    do {
        check_timeout();
        const char cmd[] = "AT+CPIN?";
        send_command(cmd, strlen(cmd));
    } while(!wait_response("+CPIN: READY\r\n\r\nOK\r\n",5000000));

    db<M95>(TRC) << "M95::on(): waiting for network registration" << endl;
    // Check Network registration
    do {
        check_timeout();
        const char cmd[] = "AT+CREG?";
        send_command(cmd, strlen(cmd));
    } while(!wait_response("+CREG: 0,1\r\n\r\nOK\r\n",5000000));

    // Select UART1
    do {
        check_timeout();
        const char cmd[] = "AT+QIFGCNT=0";
        send_command(cmd, strlen(cmd));
    } while(!wait_response("OK\r\n",300000));

    // Enable DNS
    do {
        check_timeout();
        const char cmd[] = "AT+QIDNSIP=1";
        send_command(cmd, strlen(cmd));
    } while(!wait_response("OK\r\n",300000));

    // Select GPRS and setup APN parameters
    switch(Traits<M95>::PROVIDER) {
        case Traits<M95>::CLARO: {
            do {
                check_timeout();
                const char cmd[] = "AT+QICSGP=1,\"g.claro.com.br\",\"claro\",\"claro\"";
                send_command(cmd, strlen(cmd));
            } while(!wait_response("OK\r\n",300000));
        } break;
        case Traits<M95>::TIM: {
            do {
                const char cmd[] = "AT+QICSGP=1,\"tim.br\",\"tim\",\"tim\"";
                send_command(cmd, strlen(cmd));
            } while(!wait_response("OK\r\n",300000));
        } break;
        case Traits<M95>::OI: {
            do {
                const char cmd[] = "AT+QICSGP=1,\"gprs.oi.com.br\"";
                send_command(cmd, strlen(cmd));
            } while(!wait_response("OK\r\n",300000));
        } break;
    }

    // Startup TCP/IP with the set APN parameters
    do {
        check_timeout();
        const char cmd[] = "AT+QIREGAPP";
        send_command(cmd, strlen(cmd));
    } while(!wait_response("OK\r\n",300000));

    // Wait for TCP/IP stack to get to the "IP START" state
    do {
        check_timeout();
        const char cmd[] = "AT+QISTAT";
        send_command(cmd, strlen(cmd));
    } while(!wait_response("OK\r\n\r\nSTATE: IP START\r\n",300000));

    db<M95>(INF) << "M95::on(): activating GPRS. This could take up to 150s" << endl;
    // Activate GPRS
    do {
        check_timeout();
        const char cmd[] = "AT+QIACT";
        send_command(cmd, strlen(cmd));
    } while(!wait_response("OK\r\n",150000000));// {

    // Wait for activation to be effective
    do {
        check_timeout();
        const char cmd[] = "AT+QISTAT";
        send_command(cmd, strlen(cmd));
    } while(!wait_response("OK\r\n\r\nSTATE: IP GPRSACT\r\n",300000));

    // Disable time zone change event reporting
    do {
        check_timeout();
        const char cmd[] = "AT+CTZR=0";
        send_command(cmd, strlen(cmd));
    } while(!wait_response("OK\r\n",300000));

    // Enable time synchronization from GSM network (needs network support)
    do {
        check_timeout();
        const char cmd[] = "AT+QNITZ=1";
        send_command(cmd, strlen(cmd));
    } while(!wait_response("OK\r\n",300000));

    // Automatically update the RTC with network time
    do {
        check_timeout();
        const char cmd[] = "AT+CTZU=1";
        send_command(cmd, strlen(cmd));
    } while(!wait_response("OK\r\n",300000));

    while(true) {
        check_timeout();
        if(now() / 1000000 < 1451606400)
            Machine::delay(1000000);
        else
            break;
    }
}

void M95::off()
{
    _init_timeout = TSC::time_stamp() + 150000000ull * (TSC::frequency() / 1000000);

    if(!_status->get()) {
        db<M95>(WRN) << "M95::off() => was already off!" << endl;
        return;
    }

    db<M95>(TRC) << "M95::off(): deactivating GPRS" << endl;
    const char cmd[] = "AT+QIDEACT";
    send_command(cmd, strlen(cmd));
    wait_response("DEACT OK\r\n",40000000);

    db<M95>(TRC) << "M95::off(): powering down..." << endl;

    // A 1-second pulse is what the manual says is fine for
    // turning off the device (Hardware Manual, 3.4.2.1)
    _pwrkey->set();
    Machine::delay(1000000);
    _pwrkey->clear();
    while(_status->get())
        check_timeout();

    _http_data_mode = false;
    _last_send = 0;

    db<M95>(TRC) << "...done!" << endl;
}

void M95::init(unsigned int unit)
{
    db<Init, M95>(TRC) << "M95::init(unit=" << unit << ")" << endl;

    // Initialize the device
    M95 * dev = new (SYSTEM) M95(unit);

    // Register the device
    _devices[unit].device = dev;
}

__END_SYS

#endif
