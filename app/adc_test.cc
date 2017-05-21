#include <adc.h>
#include <utility/ostream.h>

using namespace EPOS;

int main()
{
    auto cout = OStream{};
    auto adc1 = ADC{ADC::SINGLE_ENDED_ADC5};
    auto adc2 = ADC{ADC::SINGLE_ENDED_ADC7};

    while (true) {
        cout << adc1.read() << ", " << adc2.read() << endl;
    }
}
