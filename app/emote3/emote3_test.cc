#include <uart.h>
#include <gpio.h>
#include <nic.h>

using namespace EPOS;

const unsigned int DEFAULT_TEST_N_TIMES = 200;
const NIC::Protocol NIC_PROTOCOL = 42;

GPIO * led;

void busy_wait(unsigned int limit = 0x3fffff) { for(volatile unsigned int i=0; i<limit; i++); }

void blink_led(const int times = 10, const unsigned int delay = 0xffff, GPIO * _led = led)
{
    for(int i=0;i<times;i++)
    {
        _led->set(true);
        busy_wait(delay);
        _led->set(false);
        busy_wait(delay);
    }
}

void fail()
{
    led->set(true);
    while(1);
}

void test_uart()
{
    const unsigned int TEST_N_TIMES = DEFAULT_TEST_N_TIMES;

    UART uart;

    while(uart.has_data()) 
        uart.get();

    for(auto i = 0u; i < TEST_N_TIMES; i++)
    {
        for(char c = 'a'; c <= 'z'; c++)
        {
            uart.put(c);
            char d = uart.get();
            if(c != d)
                fail();
        }
    }
}

void test_gpio()
{
    const unsigned int TEST_N_TIMES = DEFAULT_TEST_N_TIMES;

    GPIO pb0('b', 0, GPIO::OUTPUT);
    GPIO pb1('b', 1, GPIO::OUTPUT);
    GPIO pb2('b', 2, GPIO::INPUT);
    GPIO pb3('b', 3, GPIO::INPUT);
    
    for(auto i = 0u; i < TEST_N_TIMES; i++)
    {
        bool val = true;
        pb0.set(val); 
        busy_wait(0xf);
        if(pb2.get() != val) fail();
        pb1.set(val); 
        busy_wait(0xf);
        if(pb3.get() != val) fail();

        val = false;
        pb0.set(val); 
        busy_wait(0xf);
        if(pb2.get() != val) fail();
        pb1.set(val); 
        busy_wait(0xf);
        if(pb3.get() != val) fail();
    }
}

void test_radio()
{
    const unsigned int TEST_N_TIMES = DEFAULT_TEST_N_TIMES;

    char message[] = "0 Hello, World!";
    auto sz = sizeof(message);
    NIC nic;

    for(auto i = 0u; i < TEST_N_TIMES; i++)
    {
        message[0] = ((message[0] - '0' + 1) % 10) + '0';
        if(nic.send(nic.broadcast(), NIC_PROTOCOL, message, sz) != sz)
            fail();
    }
}

void success()
{
    while(1)
        blink_led();
}

int main()
{
    led = new GPIO('c', 3, GPIO::OUTPUT);
    blink_led();
    
    test_uart();
    test_gpio();
    test_radio();

    success();

    return 0;
}
