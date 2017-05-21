#include <utility/ostream.h>
#include <alarm.h>
#include <uart.h>
#include <gpio.h>
#include <machine/cortex_m/emote3_gprs.h>

using namespace EPOS;

const auto NCONNECTIONS = 30u;
const auto NDELAYS = 1u;
const auto DELAYTIME = 30000000u;

const auto server = "http://sv13.lisha.ufsc.br:5000/"; // change at will

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

    gprs.on();

    cout << "GPRS is on\n";
    cout << "status is " << status.get() << "\n";

    cout << "will start sending commands\n";

    gprs.use_dns();

    for (auto i = 0u; i < NCONNECTIONS; ++i) {
        auto send = gprs.send_http_post(server, "data=dontpanic", 14);
        cout << "post: " << send << "\n";
        for (auto m = 0u; m < NDELAYS; ++m) {
            Alarm::delay(DELAYTIME);
            cout << "up\n";
        }
    }

    gprs.off();

    cout << "end of program";

    while (true);
}
