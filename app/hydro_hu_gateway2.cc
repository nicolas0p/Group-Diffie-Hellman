#include <http.h>
#include <tstp.h>
#include <gpio.h>
#include <machine.h>
#include <smart_data.h>
#include <transducer.h>
#include <persistent_storage.h>
#include <utility/ostream.h>

using namespace EPOS;

const RTC::Microsecond INTEREST_EXPIRY = 5ull * 60 * 1000000;
const RTC::Microsecond INTEREST_PERIOD = INTEREST_EXPIRY / 2;

const RTC::Microsecond HTTP_SEND_PERIOD = 30ull * 60 * 1000000;
const unsigned int HTTP_SEND_PERIOD_MULTIPLY = 4;//2 * 12;

typedef Smart_Data_Common::DB_Record DB_Record;
typedef Smart_Data_Common::DB_Series DB_Series;
typedef Persistent_Ring_FIFO<DB_Record> Storage;

int http_send()
{
    OStream cout;
    DB_Record e;

    M95 * m95 = M95::get(0);
    int ret;

    while(true) {
        for(unsigned int i = 0; i < HTTP_SEND_PERIOD_MULTIPLY; i++)
            Periodic_Thread::wait_next();

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

    cout << "epoch now = " << TSTP::absolute(TSTP::now()) / 1000000 << endl;

    TSTP::Coordinates center_station1(6400,-4800,0);
    TSTP::Coordinates center_station2(-4800,-6400,0);

    TSTP::Region region_station1(center_station1, 0, 0, -1);
    TSTP::Region region_station2(center_station2, 0, 0, -1);

    Water_Flow flow1(region_station1, INTEREST_EXPIRY, INTEREST_PERIOD, Water_Flow::CUMULATIVE);
    Water_Flow flow2(region_station2, INTEREST_EXPIRY, INTEREST_PERIOD, Water_Flow::CUMULATIVE);

    cout << "Going to sleep..." << endl;
    while(Periodic_Thread::wait_next()) {

        cout << "tstp_work()" << endl;
        cout << "sleeping 10 seconds..." << endl;
        Alarm::delay(10000000);
        cout << "...woke up" << endl;

        Smart_Data_Common::DB_Record r1 = flow1.db_record();
        cout << "r1 = " << r1 << endl;
        Smart_Data_Common::DB_Record r2 = flow2.db_record();
        cout << "r2 = " << r2 << endl;

        CPU::int_disable();
        Storage::push(r1);
        cout << "push r1 OK" << endl;
        Storage::push(r2);
        cout << "push r2 OK" << endl;
        CPU::int_enable();
        cout << "Going to sleep..." << endl;
    }

    return 0;
}

int main()
{
    OStream cout;

    cout << "main()" << endl;

    //Storage::clear(); cout << "storage cleared" << endl; while(true);

    Alarm::delay(5000000);

    M95 * m95 = M95::get(0);

    TSTP::Time t = m95->now();
    TSTP::epoch(t);

    m95->off();

    Periodic_Thread * tstp_worker = new Periodic_Thread(INTEREST_EXPIRY, tstp_work);
    Periodic_Thread * http_sender = new Periodic_Thread(HTTP_SEND_PERIOD, http_send);

    cout << "threads created. Joining." << endl;
    tstp_worker->join();
    http_sender->join();

    return 0;
}
