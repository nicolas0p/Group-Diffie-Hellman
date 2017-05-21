#include "hydro_station/include/index.h"

using namespace EPOS;

inline void fillMessageData(MessagesHandler *m, Level_Sensor& level, Turbidity_Sensor & turb, Pluviometric_Sensor &pluv){
    unsigned int d[5];
    d[0] = ++m->seq;
    d[1] = level.sample();
    d[2] = turb.sample();
    d[3] = pluv.countAndReset();
    d[4] = 1;
    m->setData(d,5);    
}

int main(){
    Interface interface;
    interface.showLife();

    auto lToggle = GPIO{'b', 0, GPIO::OUTPUT};  
    auto lAdc = ADC{ADC::SINGLE_ENDED_ADC2};
    auto level = Level_Sensor{lAdc, lToggle};
    level.disable();

    auto tAdc = ADC{ADC::SINGLE_ENDED_ADC5};
    auto tInfrared = GPIO{'b', 2, GPIO::OUTPUT};
    auto tToggle = GPIO{'b', 3, GPIO::OUTPUT};
    auto turbidity = Turbidity_Sensor{tAdc,tToggle,tInfrared};
    turbidity.disable();
    
    auto pToggle = GPIO{'b', 1, GPIO::OUTPUT};
    auto pInput = GPIO{'a', 3, GPIO::INPUT};
    auto pluviometric = Pluviometric_Sensor{pInput, pToggle};

    MessagesHandler msg;

    Sender sender(&interface, &msg);    
    sender.__init();

    eMote3_GPTM timer(1);

    while(1){
        timer.set(MINUTE_IN_US*2);
        timer.enable();

        fillMessageData(&msg, level, turbidity, pluviometric);        
        
        sender.verifyFlashAndSend();
	    
        sender.trySendingAndStore();

        while(timer.running());

        for(int i = 0; i<3; i++){
            eMote3_GPTM::delay(MINUTE_IN_US);            
        }
    }

    return 0;
}
