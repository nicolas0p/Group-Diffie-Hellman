#include <transducer.h>
#include <smart_data.h>
#include <utility/ostream.h>
#include <gpio.h>

using namespace EPOS;

IF<Traits<USB>::enabled, USB, UART>::Result io;

typedef Smart_Data_Common::DB_Series DB_Series;
typedef Smart_Data_Common::DB_Record DB_Record;
typedef TSTP::Coordinates Coordinates;
typedef TSTP::Region Region;

const unsigned int INTEREST_PERIOD = 5 * 60 * 1000000;
const unsigned int INTEREST_EXPIRY = 2 * INTEREST_PERIOD;

void print(const DB_Series & d)
{
    for(unsigned int i = 0; i < sizeof(DB_Series); i++)
        io.put(reinterpret_cast<const char *>(&d)[i]);
    io.put('X'); io.put('X'); io.put('X'); io.put('Z');
}

void print(const DB_Record & d)
{
    //CPU::int_disable();
    for(unsigned int i = 0; i < sizeof(Smart_Data_Common::DB_Record); i++)
        io.put(reinterpret_cast<const char *>(&d)[i]);
    io.put('X'); io.put('X'); io.put('X'); io.put('Z');
    //CPU::int_enable();
}

int main()
{
    // Get epoch time from serial
    TSTP::Time epoch = 0;
    char c = io.get();
    if(c != 'X') {
        epoch += c - '0';
        c = io.get();
        while(c != 'X') {
            epoch *= 10;
            epoch += c - '0';
            c = io.get();
        }
        TSTP::epoch(epoch);
    }

    Alarm::delay(5000000);
    io.put('X'); io.put('X'); io.put('X'); io.put('Z');



    // Interest center points
    //Coordinates center_lights0(430,50,250);
    //Coordinates center_lights1(150,50,250);
    Coordinates center_lights2(-140,50,250);
    //Coordinates center_lights3(430,300,250);
    //Coordinates center_lights4(150,300,250);
    Coordinates center_lights5(-140,300,250);
    ////Coordinates center_lights6(430,500,250);
    //Coordinates center_lights7(150,500,250);
    Coordinates center_lights8(-140,500,250);
    //Coordinates center_outlet3(-140,-10,-40);
    //////Coordinates center_outlet4(-250,-10,-40);
    ////Coordinates center_outlet6(-260,400,-40);
    //Coordinates center_outlet9(5,570,-40);
    ////Coordinates center_outlet11(185,570,-40);
    //Coordinates center_outlet12(345,570,-40);
    //Coordinates center_lux0(300,320,40);
    //Coordinates center_temperature0(-130,320,40);
    ////Coordinates center_ac1(150,-10,220);
    //Coordinates center_water0(1140,-688,350);
    //Coordinates center_water1(1090,-688,350);
    //Coordinates center_water2(-750,-870,350);
    Coordinates center_water3(-800,-970,350);
    Coordinates center_water4(-850,-870,350);
    ////Coordinates center_water5(,,350); // TODO



    // Regions of interest
    //Region region_lights0(center_lights0, 0, 0, -1);
    //Region region_lights1(center_lights1, 0, 0, -1);
    Region region_lights2(center_lights2, 0, 0, -1);
    //Region region_lights3(center_lights3, 0, 0, -1);
    //Region region_lights4(center_lights4, 0, 0, -1);
    Region region_lights5(center_lights5, 0, 0, -1);
    //Region region_lights6(center_lights6, 0, 0, -1);
    //Region region_lights7(center_lights7, 0, 0, -1);
    Region region_lights8(center_lights8, 0, 0, -1);
    //Region region_outlet3(center_outlet3, 0, 0, -1);
    //////Region region_outlet4(center_outlet4, 0, 0, -1);
    ////Region region_outlet6(center_outlet6, 0, 0, -1);
    //Region region_outlet9(center_outlet9, 0, 0, -1);
    ////Region region_outlet11(center_outlet11, 0, 0, -1);
    //Region region_outlet12(center_outlet12, 0, 0, -1);
    //Region region_lux0(center_lux0, 0, 0, -1);
    //Region region_temperature0(center_temperature0, 0, 0, -1);
    ////Region region_ac1(center_ac1, 0, 0, -1);
    //Region region_water0(center_water0, 0, 0, -1);
    //Region region_water1(center_water1, 0, 0, -1);
    //Region region_water2(center_water2, 0, 0, -1);
    Region region_water3(center_water3, 0, 0, -1);
    Region region_water4(center_water4, 0, 0, -1);
    ////Region region_water5(center_water5, 0, 0, -1); // TODO



    // Data of interest
    //Current data_lights0(region_lights0, INTEREST_EXPIRY, INTEREST_PERIOD);
    //Current data_lights1(region_lights1, INTEREST_EXPIRY, INTEREST_PERIOD);
    Current data_lights2(region_lights2, INTEREST_EXPIRY, INTEREST_PERIOD);
    //Current data_lights3(region_lights3, INTEREST_EXPIRY, INTEREST_PERIOD);
    //Current data_lights4(region_lights4, INTEREST_EXPIRY, INTEREST_PERIOD);
    Current data_lights5(region_lights5, INTEREST_EXPIRY, INTEREST_PERIOD);
    //Current data_lights6(region_lights6, INTEREST_EXPIRY, INTEREST_PERIOD);
    //Current data_lights7(region_lights7, INTEREST_EXPIRY, INTEREST_PERIOD);
    Current data_lights8(region_lights8, INTEREST_EXPIRY, INTEREST_PERIOD);
    ////Current data_outlet3(region_outlet3, INTEREST_EXPIRY, INTEREST_PERIOD);
    ////////Current data_outlet4(region_outlet4, INTEREST_EXPIRY, INTEREST_PERIOD);
    //////Current data_outlet6(region_outlet6, INTEREST_EXPIRY, INTEREST_PERIOD);
    ////Current data_outlet9(region_outlet9, INTEREST_EXPIRY, INTEREST_PERIOD);
    //////Current data_outlet11(region_outlet11, INTEREST_EXPIRY, INTEREST_PERIOD);
    ////Current data_outlet12(region_outlet12, INTEREST_EXPIRY, INTEREST_PERIOD);
    ////Luminous_Intensity data_lux0(region_lux0, INTEREST_EXPIRY, INTEREST_PERIOD);
    ////Temperature data_temperature0(region_temperature0, INTEREST_EXPIRY, INTEREST_PERIOD);
    //////Current data_ac1(region_ac1, INTEREST_EXPIRY, INTEREST_PERIOD);
    ////Water_Flow data_water0(region_water0, INTEREST_EXPIRY, INTEREST_PERIOD, Water_Flow::CUMULATIVE);
    ////Water_Flow data_water1(region_water1, INTEREST_EXPIRY, INTEREST_PERIOD, Water_Flow::CUMULATIVE);
    ////Water_Flow data_water2(region_water2, INTEREST_EXPIRY, INTEREST_PERIOD, Water_Flow::CUMULATIVE);
    Water_Flow data_water3(region_water3, INTEREST_EXPIRY, INTEREST_PERIOD, Water_Flow::CUMULATIVE);
    Water_Flow data_water4(region_water4, INTEREST_EXPIRY, INTEREST_PERIOD, Water_Flow::CUMULATIVE);
    //////Water_Flow data_water5(region_water5, INTEREST_EXPIRY, INTEREST_PERIOD, Water_Flow::CUMULATIVE); // TODO



    //// Output interests to serial
    //print(data_lights0.db_series());
    //print(data_lights1.db_series());
    print(data_lights2.db_series());
    //print(data_lights3.db_series());
    //print(data_lights4.db_series());
    print(data_lights5.db_series());
    //print(data_lights6.db_series());
    //print(data_lights7.db_series());
    print(data_lights8.db_series());
    ////print(data_outlet3.db_series());
    ////////print(data_outlet4.db_series());
    //////print(data_outlet6.db_series());
    ////print(data_outlet9.db_series());
    //////print(data_outlet11.db_series());
    ////print(data_outlet12.db_series());
    ////print(data_lux0.db_series());
    ////print(data_temperature0.db_series());
    //////print(data_ac1.db_series());
    ////print(data_water0.db_series());
    ////print(data_water1.db_series());
    ////print(data_water2.db_series());
    print(data_water3.db_series());
    print(data_water4.db_series());
    //////print(data_water5.db_series());



    // Output data to serial
    while(true) {
        Alarm::delay(INTEREST_PERIOD);
        //kout << data_lights0 << " " << data_lights0.time() << " " << endl;
        //kout << "Lights 1: " << data_lights1 << " " << data_lights1.time() << " " << endl;
        //kout << "Lights 2: " << data_lights2 << " " << data_lights2.time() << " " << endl;
        //kout << "Lights 3: " << data_lights3 << " " << data_lights3.time() << " " << endl;
        //kout << "Lights 4: " << data_lights4 << " " << data_lights4.time() << " " << endl;
        //kout << "Lights 5: " << data_lights5 << " " << data_lights5.time() << " " << endl;
        //kout << "Lights 6: " << data_lights6 << " " << data_lights6.time() << " " << endl;
        //kout << "Lights 7: " << data_lights7 << " " << data_lights7.time() << " " << endl;
        //kout << "Lights 8: " << data_lights8 << " " << data_lights8.time() << " " << endl;
        //print(data_lights0.db_record());
        //print(data_lights1.db_record());
        print(data_lights2.db_record());
        //print(data_lights3.db_record());
        //print(data_lights4.db_record());
        print(data_lights5.db_record());
        ////print(data_lights6.db_record());
        //print(data_lights7.db_record());
        print(data_lights8.db_record());
        //print(data_outlet3.db_record());
        //////print(data_outlet4.db_record());
        ////print(data_outlet6.db_record());
        //print(data_outlet9.db_record());
        ////print(data_outlet11.db_record());
        //print(data_outlet12.db_record());
        //print(data_lux0.db_record());
        //print(data_temperature0.db_record());
        ////print(data_ac1.db_record());
        //print(data_water0.db_record());
        //print(data_water1.db_record());
        //print(data_water2.db_record());
        print(data_water3.db_record());
        print(data_water4.db_record());
        ////print(data_water5.db_record());
    }

    return 0;
}
