#include <utility/ostream.h>
#include <alarm.h>
#include <gpio.h>
#include <uart.h>
#include <machine/cortex_m/emote3_gprs.h>

using namespace EPOS;

const auto NCONNECTIONS = 30u;
const auto NDELAYS = 1u;
const auto DELAYTIME = 60000000u;

const auto server = "sv13.lisha.ufsc.br";
const auto port = "5000";

int main()
{
    auto cout = OStream{};

    cout << "Starting GPRS test program\n";

    auto led = GPIO{'c', 3, GPIO::OUTPUT};
    auto pwrkey = GPIO{'d', 3, GPIO::OUTPUT};
    auto status = GPIO{'d', 5, GPIO::INPUT};

    auto uart = UART{115200, 8, 0, 1, 1};

    led.set();

    auto gprs = eMote3_GPRS{pwrkey, status, uart};

    cout << "GPRS is created and turned off\n";
    cout << "status is " << status.get() << "\n";

    Alarm::delay(5000000);

    gprs.on();

    cout << "GPRS is on\n";
    cout << "status is " << status.get() << "\n";

    cout << "will start sending commands\n";

    gprs.use_dns();

    for (auto i = 0u; i < NCONNECTIONS; ++i) {
        auto open = gprs.open_tcp(server, "5000");
        cout << "open: " << open << "\n";
        auto send = gprs.send_tcp("test");
        cout << "send: " << send << "\n";
        auto close = gprs.close_tcp();
        cout << "close: " << close << "\n";
        for (auto m = 0u; m < NDELAYS; ++m) {
            Alarm::delay(DELAYTIME);
            cout << "up\n";
        }
    }

    gprs.off();

    cout << "end of program";

    while (true);
}
