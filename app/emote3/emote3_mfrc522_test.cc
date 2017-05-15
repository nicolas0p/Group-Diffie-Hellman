#include <utility/ostream.h>
#include <rfid_reader.h>
#include <alarm.h>
#include <spi.h>
#include <gpio.h>

using namespace EPOS;

OStream cout;

const unsigned int BLOCK = 21;

void read(RFID_Reader & reader, RFID_Reader::UID & uid, const RFID_Reader::Key & key)
{
    unsigned char data[16];

    if(reader.read(uid, BLOCK, data, key)) {
        cout << "Block " << BLOCK << " data:" << endl;
        for(unsigned int i = 0; i < 16; i++)
            cout << hex << data[i] << " ";
        cout << endl;
    } else
        cout << "READ FAILED" << endl;
}

void write(RFID_Reader & reader, RFID_Reader::UID & uid, const RFID_Reader::Key & key) {
    unsigned char data[16];
    cout << "Random generated data:" << endl;
    for(unsigned int i = 0; i < 16; i++) {
        data[i] = Random::random();
        cout << hex << data[i] << " ";
    }
    cout << endl;

    if(reader.put(uid, BLOCK, data, key))
        cout << "Write operation done" << endl;
    else
        cout << "WRITE FAILED" << endl;
}

int main()
{
    Alarm::delay(5000000);

    IF<Traits<USB>::enabled, USB, UART>::Result input;

    SPI spi(0, 4000000, SPI::FORMAT_MOTO_0, SPI::MASTER, 500000, 8);
    GPIO select('B', 5, GPIO::OUT);
    GPIO reset('C', 6, GPIO::INOUT);
    RFID_Reader reader(&spi, &select, &reset);
    RFID_Reader::Key key("\xff\xff\xff\xff\xff\xff", RFID_Reader::Key::TYPE_A);

    cout << "Place a card next to the reader" << endl;

    while(true) {
        cout << endl;
        RFID_Reader::UID uid = reader.get();

        cout << "UID: " << uid << endl;

        cout << "Using block " << BLOCK << endl;
        cout << "Press 'w' to write or any other key to read" << endl;

        if(input.get() == 'w')
            write(reader, uid, key);
        else
            read(reader, uid, key);

        reader.halt(uid);
    }
}
