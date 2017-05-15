#include "../include/sender.h"

int Sender::sendData(const char msg[]){    
    auto send = gprs->send_http_post(DATA_SERVER, msg, (unsigned int) strlen(msg));
    
    //try to send again for 2 times    
    if(!send) {
        for(unsigned int i = 0; i < 2; i++) {
            iHandler->blinkError(Interface::ERROR::TRYINGSENDAGAIN);

            gprs->off(); 
            gprs->on();            
                        
            gprs->use_dns(); // This parameter is (or should be) reset when the module resets.
            eMote3_GPTM::delay(EMOTEGPTMLONGDELAY); // Make sure network is up.            
            
            send = gprs->send_http_post(DATA_SERVER, msg, (unsigned int) strlen(msg));
            if(send){
                iHandler->blinkSuccess(Interface::SUCCESS::MESSAGESENT);
                break;
            }
        }
    }
    return send;
}

int Sender::trySendingAndStore(){
    char buf[100];
    mHandler->build(buf);

    auto send = sendData(buf);
    //data has not been sent after 3 attempts, store data into the flash
    if(!send) {
        Flash::write(current_flash_address,mHandler->getData(),sizeof(unsigned int)*mHandler->getLength());
        current_flash_address += (mHandler->getLength() * sizeof(unsigned int));
        Flash::write(FLASH_CURRENT_VALUE, (unsigned int *) &current_flash_address, sizeof(unsigned int));
        unsentMessages++;
    }

    return send;
}

void Sender::verifyFlashAndSend(){
    unsigned int data[5];
    unsigned int zero = 0;
    bool data_not_sent = false;

    char buf[100];

    if(current_flash_address == FLASH_START_ADDRESS) return;

    for(unsigned int i = 0; i < FLASH_DATA_SIZE; i += (3 * 4)) {
        data[0] = Flash::read(FLASH_START_ADDRESS + i);
        if(data[0] != 0) {
            for(unsigned int a = 1; a<mHandler->getLength(); a++){
                data[a] = Flash::read(FLASH_START_ADDRESS + (i + 4*a));
            }
    
            mHandler->build(buf);
            auto send = sendData(buf);
            
            //if data has been sent, erase it from the flash
            if(send) {
                for(unsigned int a = 0; a<mHandler->getLength(); a++){
                    Flash::write(FLASH_START_ADDRESS+(i + 4*a), &zero, sizeof(unsigned int));
                }
            } else data_not_sent = true;

            eMote3_GPTM::delay(EMOTEGPTMLONGDELAY); //5 sec delay before sending again if needed
        } else {
            break;
        }
    }
    //all data in flash has been sent. Restart the counter
    if(!data_not_sent) current_flash_address = FLASH_START_ADDRESS;
}

int Sender::verifyAndSetCurrentFlashAddress(){
    int seq = 1;
    current_flash_address = Flash::read(FLASH_CURRENT_VALUE);
    if(current_flash_address == 0) {
        current_flash_address = FLASH_START_ADDRESS;
        return seq;
    } else {
        for(unsigned int i = 0; i < FLASH_DATA_SIZE; i += (mHandler->getLength() * 4)) {
            int tmp = Flash::read(FLASH_START_ADDRESS + i);
            if(tmp == 0)
                return seq + 1;
            seq = tmp;
        }
    }
    return seq + 1;
}


bool Sender::__init(){	
    eMote3_GPTM::delay(EMOTEGPTMSHORTDELAY/2);
    if(__initNetwork()){
        if(__initConfig()){
            iHandler->blinkSuccess(Interface::SUCCESS::SIMCARDINITIALIZED);
            gprs->use_dns();
            return true;
        }else{          
            iHandler->blinkError(Interface::ERROR::NOGPRSCARD);         
            return false;
        }
    }else{
        iHandler->blinkError(Interface::ERROR::NONETWORK);      
        return false;
    }
    return false;
}


bool Sender::__initConfig(){
    bool res;
    res = false;
    iHandler->printMessage(Interface::MESSAGE::GPRSSETUP,0);

    eMote3_GPTM::delay(EMOTEGPTMSHORTDELAY);

    while(!res) {
        gprs->send_command("AT+CGATT=1");        
        res = gprs->await_response("OK", RESPONSETIMEOUT);
        
        iHandler->printMessage(Interface::MESSAGE::GPRSSTATUS, res);
                
        if(status->get()==0) gprs->on();
        
        eMote3_GPTM::delay(EMOTEGPTMSHORTDELAY);        
    }
    res = false;
    while(!res) {
        gprs->send_command("AT+CGACT=1,1");
        res = gprs->await_response("OK", RESPONSETIMEOUT);
        iHandler->printMessage(Interface::MESSAGE::GPRSSTATUS, res);
        if(status->get()==0) gprs->on();
        eMote3_GPTM::delay(EMOTEGPTMSHORTDELAY);
    }
    iHandler->printMessage(Interface::MESSAGE::GPRSSTATUS, status->get());
    
    return res;
}

bool Sender::__initNetwork(){	
    bool res = gprs->sim_card_ready();
    unsigned int tmp=1;
    
    iHandler->printMessage(Interface::MESSAGE::STARTINGNETWORK,0);
    eMote3_GPTM::delay(EMOTEGPTMSHORTDELAY);

    while(!res) {
        iHandler->printMessage(Interface::MESSAGE::SENDCOMMAND,tmp);
        gprs->send_command("AT+CGDCONT=1,\"IP\",\"gprs.oi.com.br\"");
        res = gprs->await_response("OK", RESPONSETIMEOUT);
        iHandler->printMessage(Interface::MESSAGE::WAITINGRESPONSE, tmp);
        if(!status->get()) gprs->on();
        
        gprs->send_command("AT+CGDCONT?");
        res = gprs->await_response("OK", RESPONSETIMEOUT);

        tmp++;
    }    
    return res;
}