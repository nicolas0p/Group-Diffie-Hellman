#include <http.h>
#include <tstp.h>
#include <gpio.h>
#include <machine.h>
#include <smart_data.h>
#include <transducer.h>
#include <persistent_storage.h>
#include <utility/ostream.h>

using namespace EPOS;

const RTC::Microsecond INTEREST_PERIOD = 10ull * 60 * 1000000;
const RTC::Microsecond INTEREST_EXPIRY = 2 * INTEREST_PERIOD;
const RTC::Microsecond HTTP_SEND_PERIOD = 30ull * 60 * 1000000;

typedef Smart_Data_Common::DB_Record DB_Record;
typedef Smart_Data_Common::DB_Series DB_Series;
typedef Persistent_Ring_FIFO<DB_Record> Storage;

int http_send()
{
    OStream cout;
    DB_Record e;

    M95 * m95 = M95::get(0);
    int ret;

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
                ret = Quectel_HTTP::post("http://150.162.62.3/data/hydro/put_new.php", &e, sizeof(DB_Record));
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
            if(ret > 0) {
                RTC::Microsecond t = m95->now();
                if(t)
                    TSTP::epoch(t);
            }
            cout << "Turning GPRS off" << endl;
            m95->off();
        } else
            CPU::int_enable();
        cout << "Going to sleep..." << endl;
    }
}

int tstp_work()
{
    OStream cout;
    cout << "tstp_work() init" << endl;

    M95 * m95 = M95::get(0);

    TSTP::Time t = m95->now();
    TSTP::epoch(t);

    cout << "epoch now = " << TSTP::absolute(TSTP::now()) / 1000000 << endl;

    Coordinates center_station1(50,0,0);
    //Coordinates center_station2(-6000,4500,0);
    Region region_station1(center_station1, 0, 0, -1);

    Water_Flow flow0(0, INTEREST_EXPIRY, static_cast<Water_Flow::Mode>(Water_Flow::PRIVATE | Water_Flow::CUMULATIVE));
    Water_Flow flow1(region_station1, INTEREST_EXPIRY, INTEREST_PERIOD, Water_Flow::CUMULATIVE);

    DB_Series s0;
    s0.version = 0;
    s0.unit = flow0.unit();
    TSTP::Global_Coordinates c = TSTP::absolute(TSTP::here());
    s0.x = c.x;
    s0.y = c.y;
    s0.z = c.z;
    s0.r = 0;
    s0.t0 = 0;
    s0.t1 = -1ull;
    cout << "s0 = " << s0 << endl;
    if(Quectel_HTTP::post("http://150.162.62.3/data/hydro/put_new.php", &s0, sizeof(DB_Series)) <= 0) {
        cout << "Retrying post..." << endl;
        if(Quectel_HTTP::post("http://150.162.62.3/data/hydro/put_new.php", &s0, sizeof(DB_Series)) <= 0) {
            cout << "Retrying post..." << endl;
            if(Quectel_HTTP::post("http://150.162.62.3/data/hydro/put_new.php", &s0, sizeof(DB_Series)) <= 0) {
                CPU::int_disable();
                cout << "Post failed! Rebooting" << endl;
                m95->off();
                Machine::reboot();
            }
        }
    }

    cout << "posted s0" << endl;

    DB_Series s1 = flow1.db_series();
    cout << "s1 = " << s1 << endl;
    if(Quectel_HTTP::post("http://150.162.62.3/data/hydro/put_new.php", &s1, sizeof(DB_Series)) <= 0) {
        cout << "Retrying post..." << endl;
        if(Quectel_HTTP::post("http://150.162.62.3/data/hydro/put_new.php", &s1, sizeof(DB_Series)) <= 0) {
            cout << "Retrying post..." << endl;
            if(Quectel_HTTP::post("http://150.162.62.3/data/hydro/put_new.php", &s1, sizeof(DB_Series)) <= 0) {
                CPU::int_disable();
                cout << "Post failed! Rebooting" << endl;
                m95->off();
                Machine::reboot();
            }
        }
    }
    cout << "posted s1" << endl;

    m95->off();

    cout << "Going to sleep..." << endl;
    while(Periodic_Thread::wait_next()) {

        cout << "tstp_work()" << endl;
        cout << "sleeping 10 seconds..." << endl;
        Alarm::delay(10000000);
        cout << "...woke up" << endl;

        Smart_Data_Common::DB_Record r0 = flow0.db_record();
        cout << "r0 = " << r0 << endl;
        Smart_Data_Common::DB_Record r1 = flow1.db_record();
        cout << "r1 = " << r1 << endl;

        CPU::int_disable();
        Storage::push(r0);
        cout << "push r0 OK" << endl;
        Storage::push(r1);
        cout << "push r1 OK" << endl;
        CPU::int_enable();
        cout << "Going to sleep..." << endl;
    }

    return 0;
}

int main()
{
    OStream cout;
    cout << "main()" << endl;

    Alarm::delay(5000000);

    //Storage::clear(); cout << "storage cleared" << endl; while(true);

    Periodic_Thread * tstp_worker = new Periodic_Thread(INTEREST_PERIOD, tstp_work);
    Alarm::delay(INTEREST_PERIOD);
    Periodic_Thread * http_sender = new Periodic_Thread(HTTP_SEND_PERIOD, http_send);

    cout << "threads created. Joining." << endl;
    tstp_worker->join();
    http_sender->join();

    return 0;
}
