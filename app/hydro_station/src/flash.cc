#include "../include/flash.h"

void FlashMemory::verify_flash_and_send(eMote3_GPRS *gprs, char buf[], char aux[]) {
   /* unsigned int data[3];
    unsigned int zero = 0;
    bool data_not_sent = false;

    //there is no data to be sent
    if(isEmpty()) {
        cout << "verify_flash_and_send() no data to be sent, returning..\n";
        return;
    }

    for(unsigned int i = 0; i < FLASH_DATA_SIZE; i += (3 * 4)) {
        data[0] = Flash::read(FLASH_START_ADDRESS + i);
        if(data[0] != 0) {
            data[1] = Flash::read(FLASH_START_ADDRESS + (i + 4));
            data[2] = Flash::read(FLASH_START_ADDRESS + (i + 8));

            cout << "Data[0]=" << data[0] << " saved in the flash at address=" << FLASH_START_ADDRESS + (i) << endl;

            mount_message_to_send(buf, aux, data);

            cout << "Sending: " <<  buf << endl;

            auto send = send_data(gprs, buf, data);

            //if data has been sent, erase it from the flash
            if(send) {
                Flash::write(FLASH_START_ADDRESS + i, &zero, sizeof(unsigned int));
                Flash::write(FLASH_START_ADDRESS + (i + 4), &zero, sizeof(unsigned int));
                Flash::write(FLASH_START_ADDRESS + (i + 8), &zero, sizeof(unsigned int));
            } else
                data_not_sent = true;

            eMote3_GPTM::delay(5000000); //5 sec delay before sending again if needed

        } else {
            break;
        }
    }

    //all data in flash has been sent. Restart the counter
    if(!data_not_sent)
        current_flash_address = FLASH_START_ADDRESS;*/
}


bool FlashMemory::isEmpty(){
	if(current_flash_address == FLASH_START_ADDRESS) return true;
	else return false;
}

