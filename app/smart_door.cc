#include <transducer.h>
#include <tstp.h>
#include <utility/ostream.h>

using namespace EPOS;

OStream cout;

// TODO
RFID_Sensor::Observed RFID_Sensor::_observed;
RFID_Sensor * RFID_Sensor::_dev[RFID_Sensor::MAX_DEVICES];
Switch_Sensor::Observed Switch_Sensor::_observed;
Switch_Sensor * Switch_Sensor::_dev[Switch_Sensor::MAX_DEVICES];

class Door_Transform
{
    static const unsigned int DOOR_OPEN_TIME = 15 * 1000000;
    typedef RFID_Sensor::Data Data;

public:
    Door_Transform(Switch * output) : _door_control_condition(), _door_control_thread(&door_control, output, &_door_control_condition) { }

    void apply(Switch * o, Switch * button, RFID * rfid) {
        Switch::Value b = *button;
        if(b) {
            _door_control_condition.signal();
        } else {
            unsigned int i = *rfid;
            Data value = i;

            if(value != 0) {
                // Update RFID authorization code based on cache
                if(rfid->location() == TSTP::absolute(TSTP::here())) { // RFID was updated by this node
                    // Check cache
                    for(unsigned int i = 0; i < list_size(); i++) {
                        Data d = read_cache(i);
                        if(d.uid() == value.uid()) {
                            if(d.code() != value.code()) {
                                value.code(d.code());
                                *rfid = value; // Update Smart Data authorization code
                            }
                            if(d.authorized())
                                value.open(true);
                            break;
                        }
                    }
                } else { // RFID was updated by a command from the network
                    // Update cache
                    unsigned int end = list_size();
                    bool updated = false;
                    for(unsigned int i = 0; i < end; i++) {
                        Data d = read_cache(i);
                        if(d.uid() == value.uid()) {
                            if(d.code() != value.code()) {
                                value.code(d.code());
                                update_cache(value, i);
                            }
                            updated = true;
                            break;
                        }
                    }
                    if((!updated) && value.authorized())
                        push_cache(value, end);
                }
            }

            if(value.open())
                _door_control_condition.signal();
        }
    }

private:
    static int door_control(Switch * output, Condition * condition) {
        *output = 1;
        while(true) {
            condition->wait();
            *output = 1;
            Alarm::delay(DOOR_OPEN_TIME);
            *output = 0;
        }

        return 0;
    }

    // Flash handling methods

    static const unsigned int SIZE_ALIGNED = (((sizeof(Data) + sizeof(Persistent_Storage::Word) - 1) / sizeof(Persistent_Storage::Word)) * sizeof(Persistent_Storage::Word));
    static const unsigned int FLASH_LIMIT = Persistent_Storage::SIZE / SIZE_ALIGNED - 1;

    unsigned int list_size() {
        unsigned int ret;
        Persistent_Storage::read(0, &ret, sizeof(unsigned int));
        if(ret > FLASH_LIMIT) {
            ret = 0;
            Persistent_Storage::write(0, &ret, sizeof(unsigned int));
        }
        return ret;
    }

    Data read_cache(unsigned int flash_block) {
        Data d;
        Persistent_Storage::read(sizeof(Persistent_Storage::Word) + flash_block * SIZE_ALIGNED, &d, sizeof(Data));
        return d;
    }

    void update_cache(const Data & d, unsigned int flash_block) {
        Persistent_Storage::write(sizeof(Persistent_Storage::Word) + flash_block * SIZE_ALIGNED, &d, sizeof(Data));
    }

    unsigned int push_cache(const Data & d, unsigned int flash_block) {
        bool pushed = true;
        unsigned int addr = sizeof(Persistent_Storage::Word) + flash_block * SIZE_ALIGNED;
        if(addr > FLASH_LIMIT) {
            flash_block = Random::random() % FLASH_LIMIT;
            addr = sizeof(Persistent_Storage::Word) + flash_block * SIZE_ALIGNED;
            pushed = false;
        }

        Persistent_Storage::write(addr, &d, sizeof(Data));

        if(pushed) {
            unsigned int b = flash_block + 1;
            Persistent_Storage::write(0, &b, sizeof(unsigned int));
        }

        return flash_block;
    }

private:
    Condition _door_control_condition;
    Thread _door_control_thread;
};

int main()
{
    cout << "Smart Door" << endl;
    cout << "Here = " << TSTP::here() << endl;

    Switch_Sensor open_door_sensor(0, 'A', 2, GPIO::IN, GPIO::UP, GPIO::RISING);
    Switch open_door(0, 100000, Switch::PRIVATE);

    GPIO in0('D', 2, GPIO::IN);
    GPIO in1('D', 4, GPIO::IN);
    RFID_Sensor rfid_sensor(1, 0, &in0, &in1);
    RFID rfid(1, 10 * 1000000, RFID::COMMANDED);

    Switch_Sensor door_control_actuator(2, 'D', 1, GPIO::OUT);
    Switch door_control(2, 100000, Switch::COMMANDED);

    Door_Transform door_transform(&door_control);
    Actuator<Switch, Door_Transform, Switch, RFID> door_actuator(&door_control, &door_transform, &open_door, &rfid);

    Thread::self()->suspend();

    return 0;
}
