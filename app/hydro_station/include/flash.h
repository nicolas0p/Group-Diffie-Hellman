#ifndef FLASH_H_
#define FLASH_H_

#include <flash.h>
#include "messages.h"
#include <machine/cortex_m/emote3_gptm.h>
#include <machine/cortex_m/emote3_gprs.h>

#define FLASH_CURRENT_VALUE 128*1024 //flash start address is 128k. This address will storage the current flash address that is being written
#define FLASH_START_ADDRESS FLASH_CURRENT_VALUE+sizeof(unsigned int) //flash start address for storage (128k + 4)
#define FLASH_DATA_SIZE 256*1024/4  //65536 is the maximum number of words to be written to the flash (256k of storage)

class FlashMemory{
	public:
		FlashMemory(){
			current_flash_address = 0;			
			data_not_sent = false;
		}
		
		void verify_flash_and_send(eMote3_GPRS *gprs, char buf[], char aux[]);		
		bool isEmpty();
	private:
		bool data_not_sent;
		volatile unsigned int current_flash_address;

};

#endif
