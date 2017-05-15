/*
*   THIS CLASS CONTAINS METHODS TO SEND THE DATA OVER THE NETWORK OR STORE IT IN THE FLASH IN CASE IT'S NEEDED
*  
* --> it uses the board port C4 as the power key to the GPRS module
* --> it uses the board port C1 as to get the status of the GPRS module
* --> it uses the UART 1 with a baud rate of 9600 to communicate with the GPRS module
*
----------------METHODS
*
* --> __init(): called to initiate the GPRS module and connect it to the network
* --> verifyFlashAndSend(): is checks if there is any data stored in the flash memory, if there is tries to send it
* --> sendData(...): receives the message to be sent and tries to send it 3 times
* --> trySendingAndStore(): tries to send the message using [sendData(...)], if trial is not successfull store it on the flash
* --> __initNetwork(): send commands to the GPRS module to initialize
* --> __initConfig(): send commands to the GPRS module to initialize
* --> verifyAndSetCurrentFlashAddress(): check what's the current flash address to be read or written
*/

#ifndef SENDER_H_
#define SENDER_H_

#include "flash.h"
#include "interface.h"
#include "messages.h"

#include <machine.h>
#include <gpio.h>
#include <uart.h>
#include <machine/cortex_m/emote3_gprs.h>
#include <machine/cortex_m/emote3_gptm.h>


const auto DATA_SERVER = "http://150.162.216.118:80/hidro";
const auto EMOTEGPTMSHORTDELAY = 2000000;
const auto EMOTEGPTMLONGDELAY = 5000000;
const auto RESPONSETIMEOUT = 3000000;

using namespace EPOS;

#define FLASH_CURRENT_VALUE 128*1024 //flash start address is 128k. This address will storage the current flash address that is being written
#define FLASH_START_ADDRESS FLASH_CURRENT_VALUE+sizeof(unsigned int) //flash start address for storage (128k + 4)
#define FLASH_DATA_SIZE 256*1024/4  //65536 is the maximum number of words to be written to the flash (256k of storage)

class Sender{
public:
    Sender(Interface *x, MessagesHandler *m){
    	iHandler = x;
        mHandler = m;

        verifyAndSetCurrentFlashAddress();
    	
    	pwrkey = new EPOS::GPIO{'c', 4, EPOS::GPIO::OUTPUT};				
    	status = new EPOS::GPIO{'c', 1, EPOS::GPIO::INPUT};
    	uart = new EPOS::UART{9600, 8, 0, 1, 1};
    	    	
    	auto a = EPOS::GPIO{'c',4,EPOS::GPIO::OUTPUT};
    	auto b = EPOS::GPIO{'c',1,EPOS::GPIO::INPUT};
    	auto c = EPOS::UART{9600,8,0,1,1};
    	gprs = new EPOS::eMote3_GPRS{a,b,c};
    	iHandler->printMessage(Interface::MESSAGE::GPRSCREATED, status->get()); 

        gprs->use_dns();   	    	
    }
    
    bool __init();    
    void verifyFlashAndSend();
    int trySendingAndStore();
private:
    EPOS::eMote3_GPRS *gprs;
    Interface *iHandler;
    MessagesHandler *mHandler;
        
    volatile unsigned int current_flash_address;

    int sendData(const char msg[]);
    char buf[];

    int unsentMessages;
        
    EPOS::GPIO *pwrkey;
    EPOS::GPIO *status;
    EPOS::UART *uart;
    
    bool __initNetwork();
    bool __initConfig();

    int verifyAndSetCurrentFlashAddress();
};

#endif

