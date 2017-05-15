#include <http.h>
#include <tstp.h>
#include <gpio.h>
#include <machine.h>
#include <smart_data.h>
#include <transducer.h>
#include <persistent_storage.h>
#include <utility/ostream.h>

using namespace EPOS;

const RTC::Microsecond INTEREST_PERIOD = 5 * 60 * 1000000;
const RTC::Microsecond INTEREST_EXPIRY = INTEREST_PERIOD;
const RTC::Microsecond HTTP_SEND_PERIOD = 20 * 60 * 1000000;
const char STATION_NAME[] = "f_99";

class DB_Entry
{
    public:
        DB_Entry(unsigned int i = 0) :
            _zero(0), _timestamp(i), _level(i), _turbidity(i), _pluviometer(i), _signal_level(i) {}
        DB_Entry(const char * station_name, unsigned int timestamp, unsigned short level, unsigned short turbidity, unsigned char pluviometer, char signal_level) :
            _zero(0), _timestamp(timestamp), _level(level), _turbidity(turbidity), _pluviometer(pluviometer), _signal_level(signal_level)
    {
        _station[0] = station_name[0];
        _station[1] = station_name[1];
        _station[2] = station_name[2];
        _station[3] = station_name[3];
    }

        void rssi(char r) { _signal_level = r; }
        void time(unsigned int ts) { _timestamp = ts; }

    private:
        char _station[4];
        char _zero;
        unsigned int _timestamp;
        unsigned short _level;
        unsigned short _turbidity;
        unsigned char _pluviometer;
        char _signal_level;
}__attribute__((packed));

typedef Persistent_Ring_FIFO<DB_Entry> Storage;

int http_send()
{
    OStream cout;
    DB_Entry e;

    M95 * m95 = M95::get(0);

    while(Periodic_Thread::wait_next()) {
        cout << "http_send()" << endl;
        CPU::int_disable();
        if(Storage::pop(&e)) {
            CPU::int_enable();
            cout << "Turning GPRS on" << endl;
            m95->on();
            bool popped = true;
            while(popped) {
                cout << "Popped" << endl;
                e.rssi(m95->rssi());
                int ret = Quectel_HTTP::post("http://150.162.62.3/data/hydro/put.php", &e, sizeof(DB_Entry));
                cout << "post = " << ret << endl;
                if(ret <= 0) {
                    CPU::int_disable();
                    Storage::push(e);
                    CPU::int_enable();
                    break;
                }
                CPU::int_disable();
                popped = Storage::pop(&e);
                CPU::int_enable();
            }
            TSTP::epoch(m95->now());
            cout << "Turning GPRS off" << endl;
            m95->off();
        } else
            CPU::int_enable();
    }
}

int tstp_work()
{
    OStream cout;

    M95 * m95 = M95::get(0);

    TSTP::Time t = m95->now();
    TSTP::epoch(t);

    m95->off();

    Coordinates center_station0(-6000,4500,0);
    Region region_station0(center_station0, 0, TSTP::now(), -1);

    Water_Level level(region_station0, INTEREST_EXPIRY, INTEREST_PERIOD);
    Water_Turbidity turbidity(region_station0, INTEREST_EXPIRY, INTEREST_PERIOD);
    Rain rain(region_station0, INTEREST_EXPIRY, INTEREST_PERIOD, Rain::CUMULATIVE);

    while(Periodic_Thread::wait_next()) {

        DB_Entry e(STATION_NAME, 0, level, turbidity, rain, 0);

        TSTP::Time last = level.time();
        if(turbidity.time() > last)
            last = turbidity.time();
        if(rain.time() > last)
            last = rain.time();
        last = TSTP::absolute(last);

        cout << "last = " << last << endl;
        cout << "level.time() = " << level.time() << endl;
        cout << "turbidity.time() = " << turbidity.time() << endl;
        cout << "rain.time() = " << rain.time() << endl;

        e.time(last / 1000000);

        CPU::int_disable();
        Storage::push(e);
        CPU::int_enable();
    }

    return 0;
}

int main()
{
    Storage::clear();

    Periodic_Thread * tstp_worker = new Periodic_Thread(INTEREST_EXPIRY, tstp_work);
    Periodic_Thread * http_sender = new Periodic_Thread(HTTP_SEND_PERIOD, http_send);
    tstp_worker->join();
    http_sender->join();

    return 0;
}
