#include <utility/ostream.h>
#include <machine.h>
#include <alarm.h>
#include <flash.h>
#include <machine/cortex_m/emote3_gptm.h>

using namespace EPOS;

#define START_ADDRESS 128*1024 //flash start address is 128k
//65536 is the maximum number of words to be written to the flash (256k of storage)
//+1 because the first addr contains the current flash address to be written (see app/hydrology_sender_with_flash.cc)
#define DATA_SIZE (256*1024/4)+1

//This application takes some minutes to finish
int main()
{
    OStream cout;
    eMote3_GPTM::delay(3000000);

    cout << "This program will erase all memory storage used by the hydrology station." << endl;
    cout << "It starts writing 0 at address=" << START_ADDRESS << " . The number of written words is " << DATA_SIZE << endl;

    unsigned int data_to_write = 0;

    for(unsigned int i = 0; i < DATA_SIZE; i++) {
        cout<<"Erasing"<<endl;
        Flash::write(START_ADDRESS + (i * 4), &data_to_write, sizeof(unsigned int));
    }

    cout << "Reading the flash addresses..." << endl;

    for(unsigned int i = 0; i < DATA_SIZE; i++) {
        cout << "Address=" << START_ADDRESS + (i * 4) << " data=" << Flash::read(START_ADDRESS + (i * 4)) << endl;
    }

    cout << "Program done.. this will not reboot." << endl;
    while(1);
}
