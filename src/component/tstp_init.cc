// EPOS Trustful SpaceTime Protocol Initialization

#include <system/config.h>
#ifndef __no_networking__

#include <tstp.h>

__BEGIN_SYS

TSTP::TSTP()
{
    db<TSTP>(TRC) << "TSTP::TSTP()" << endl;
}

// TODO: we need a better way to define static locations
void TSTP::Locator::bootstrap()
{
    db<TSTP>(TRC) << "TSTP::Locator::bootstrap()" << endl;

    _confidence = 100;

    // This is used if your machine ID is unlisted below
    if(Traits<TSTP>::sink)
        _here = TSTP::sink();
    else
        _here = Coordinates(10,10,0); // Adjust this value to the coordinates of the sensor

    // You can edit the values below to define coordinates based on the machine ID
    if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x00\x00\x00\x00", 8)) // Adjust this value to the ID of the mote
        _here = TSTP::sink();
    else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x00\x00\x00\x00", 8)) // Adjust this value to the ID of the mote
        _here = Coordinates(10,10,0); // Adjust this value to the coordinates of the sensor

    // For the Global Coordinates, Latitude, Longitude and z were taken from Google Maps,
    // then transformed into x,y using The World Coordinate Converter (http://twcc.fr/)
    // Source coordinate system: "GPS (WGS84) (deg)"
    // Destination coordinate system: "EPSG:32633"
    // (EPSG:32633 is not on the default list in twcc. Click the green plus sign button, and type "EPSG:32633" into box number 2)

    // UFSC HU mesh
    //if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\xb3\x0e\x16\x06", 8)) { // Sink
    //    _here = TSTP::sink();
    //    TSTP::coordinates(Global_Coordinates(74494581, 694493840, 25300));
    //}
    //else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\xb0\x0e\x16\x06", 8)) // Water flow sensor 1
    //    _here = Coordinates(50,0,0);
    //else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x28\x0f\x16\x06", 8)) // Water flow sensor 2
    //    _here = Coordinates(-6000,4500,0);

    // LISHA Testbed
    //if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\xee\x0e\x16\x06", 8)) { // Sink
    //    _here = TSTP::sink();
    //    TSTP::coordinates(Global_Coordinates(741869040, 679816341, 25300));
    //}
    //else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x7f\x0e\x16\x06", 8)) // Dummy 0
    //    _here = Coordinates(10,5,0);
    //else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x69\x0e\x16\x06", 8)) // Dummy 1
    //    _here = Coordinates(10,10,0);
    //else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\xca\x0e\x16\x06", 8)) // Dummy 2
    //    _here = Coordinates(5,15,0);
    //else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x67\x83\x0d\x06", 8)) // Dummy 3
    //    _here = Coordinates(0,15,0);
    //else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\xec\x82\x0d\x06", 8)) // Dummy 4
    //    _here = Coordinates(-5,10,0);
    //else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x97\x0e\x16\x06", 8)) // Dummy 5
    //    _here = Coordinates(-5,5,0);
    //else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x11\x83\x0d\x06", 8)) // Outlet 0 (B0)
    //    _here = Coordinates(460-730, -250-80, -15);
    //else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x74\x82\x0d\x06", 8)) // Outlet 1 (B1)
    //    _here = Coordinates(-5-730, -30-80, -15);
    //else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x5e\x83\x0d\x06", 8)) // Lights 1 (A1)
    //    _here = Coordinates(305-730, -170-80, 220);
    //else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x0b\x0f\x16\x06", 8)) // Luminosity sensor
    //    _here = Coordinates(-720,-90, 0);
    //else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x1a\x84\x0d\x06", 8)) // Router 1 (corridor, green)
    //    _here = Coordinates(-225,40,0);
    //else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x65\x84\x0d\x06", 8)) // Router 2 (main door)
    //    _here = Coordinates(-210, 120, 140);
    //else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\xb7\x82\x0d\x06", 8)) // Router 3 (Guto's door)
    //    _here = Coordinates(-270, -110, 160);
    //else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x46\x5c\x3a\x06", 8)) // Door
    //    _here = Coordinates(-200, 150, 200);
    //else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x0d\x3e\x3a\x06", 8)) // Presence
    //    _here = Coordinates(-720, -100, 0);
    //else
    //    _confidence = 0;

    // SSB
    //if(Traits<TSTP>::sink)
    //    _here = Coordinates(0, 0, 0);
    //else {
    //    // Student's room 1
    //    if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x0b\x0f\x16\x06", 8)) { // Test Sink
    //        _here = Coordinates(0, 0, 0);
    //        TSTP::coordinates(Global_Coordinates(75270213, 696322550, 101500));
    //    }
    //    else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x1a\x83\x0d\x06", 8)) // Lights 0
    //        _here = Coordinates(430, 50, 250);
    //    else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\xd3\x82\x0d\x06", 8)) // NOT SURE Lights 1
    //        _here = Coordinates(150, 50, 250);
    //    else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\xcb\x82\x0d\x06", 8)) // Lights 2
    //        _here = Coordinates(-140, 50, 250);
    //    else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x04\x84\x0d\x06", 8)) // Lights 3
    //        _here = Coordinates(430, 300, 250);
    //    else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x1e\x83\x0d\x06", 8)) // Lights 4
    //        _here = Coordinates(150, 300, 250);
    //    else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x98\x83\x0d\x06", 8)) // Lights 5
    //        _here = Coordinates(-140, 300, 250);
    //    else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\xf6\x82\x0d\x06", 8)) // Lights 7
    //        _here = Coordinates(150, 500, 250);
    //    else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\xfa\x82\x0d\x06", 8)) // Lights 8
    //        _here = Coordinates(-140, 500, 250);
    //    else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x84\x82\x0d\x06", 8)) // NOT SURE Outlet 3
    //        _here = Coordinates(-140, -10, -40);
    //    else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x0b\x83\x0d\x06", 8)) // Outlet 4
    //        _here = Coordinates(-250, -10, -40);
    //    else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x4c\x83\x0d\x06", 8)) // Outlet 5
    //        _here = Coordinates(-260, 200, -40);
    //    else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x98\x82\x0d\x06", 8)) // Outlet 6
    //        _here = Coordinates(-260, 400, -40);
    //    else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x8b\x82\x0d\x06", 8)) // NOT SURE Outlet 9
    //        _here = Coordinates(5, 570, -40);
    //    else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x90\x82\x0d\x06", 8)) // NOT SURE Outlet 11
    //        _here = Coordinates(185, 570, -40);
    //    else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x03\x83\x0d\x06", 8)) // Outlet 12
    //        _here = Coordinates(345, 570, -40);
    //    else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x22\x83\x0d\x06", 8)) // Lux 0
    //        _here = Coordinates(300, 320, 40);
    //    else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\xa4\x82\x0d\x06", 8)) // Temperature 0
    //        _here = Coordinates(-130, 320, 40);
    //    else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x51\x82\x0d\x06", 8)) // Air Conditioner 0
    //        _here = Coordinates(430, -10, 220);
    //    else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x47\x83\x0d\x06", 8)) // Air Conditioner 1
    //        _here = Coordinates(150, -10, 220);
    //    else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x8e\x83\x0d\x06", 8)) // Water 0
    //        _here = Coordinates(1140, -688, 350);
    //    else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x31\x84\x0d\x06", 8)) // Water 1
    //        _here = Coordinates(1090, -688, 350);
    //    else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x7f\x82\x0d\x06", 8)) // Water 2 // NOT OK
    //        _here = Coordinates(-920, -870, 350);
    //    else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x1b\xb0\x0d\x06", 8)) // Water 3 // OK
    //        _here = Coordinates(-970, -970, 350);
    //    else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\xdc\xaf\x0d\x06", 8)) // Water 4 // OK
    //        _here = Coordinates(-1020, -870, 350);
    //    else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x02\x0b\x0d\x06", 8)) // Router 0 
    //        _here = Coordinates(610, 1700, 550);
    //    else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\xe7\x83\x0d\x06", 8)) // Water 5
    //        _here = Coordinates(710, 2800, 350);
    //    //else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\xXX\xXX\xXX\xXX", 8)) // Rain 0 // TODO
    //    //    _here = Coordinates(2727, 3500, 0);

    //    // Student's room 2
    //    else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\xd5\x3d\x3a\x06", 8)) // Lights 9
    //        _here = Coordinates(-1368, -613, 250);
    //    else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x75\x3a\x3a\x06", 8)) // Lights 10
    //        _here = Coordinates(-1510, -613, 250);
    //    else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\xae\xaa\x0d\x06", 8)) // Lights 11
    //        _here = Coordinates(-1652, -613, 250);
    //    else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x00\x00\x00\x00", 8)) // Lights 12
    //        _here = Coordinates(-1368, -763, 250);
    //    else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x00\x00\x00\x00", 8)) // Lights 13
    //        _here = Coordinates(-1510, -763, 250);
    //    else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x0c\x39\x3a\x06", 8)) // Lights 14
    //        _here = Coordinates(-1652, -763, 250);
    //    else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x00\x00\x00\x00", 8)) // Lights 15
    //        _here = Coordinates(-1368, -914, 250);
    //    else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x00\x00\x00\x00", 8)) // Lights 16
    //        _here = Coordinates(-1510, -914, 250);
    //    else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x8a\x38\x3a\x06", 8)) // Lights 17
    //        _here = Coordinates(-1652, -914, 250);

    //    else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\xa1\xaf\x0d\x06", 8)) // Outlet 13 (Door)
    //        _here = Coordinates(-1268, -823, -40);
    //    else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\x1c\xb0\x0d\x06", 8)) // Outlet 14
    //        _here = Coordinates(-1752, -603, -40);
    //    else if(!memcmp(Machine::id(), "\x00\x4b\x12\x00\xbe\xaf\x0d\x06", 8)) // Outlet 15
    //        _here = Coordinates(-1752, -613, -40);

    //    else
    //        _confidence = 0;
    //}

    if(Traits<Radio>::promiscuous) {
        _here = Coordinates(12,12,12);
        _confidence = 100;
    }

    TSTP::_nic->attach(this, NIC::TSTP);

    // TODO: This should be in TSTP::init(), but only here we know if we are the sink
    if((here() != sink()) && (!Traits<Radio>::promiscuous))
        new (SYSTEM) TSTP::Life_Keeper;

    // Wait for spatial localization
    while(_confidence < 80)
        Thread::self()->yield();
}

void TSTP::Timekeeper::bootstrap()
{
    db<TSTP>(TRC) << "TSTP::Timekeeper::bootstrap()" << endl;

    if(here() == sink())
        _next_sync = -1ull; // Just so that the sink will always have synchronized() return true

    TSTP::_nic->attach(this, NIC::TSTP);

    if((TSTP::here() != TSTP::sink()) && (!Traits<Radio>::promiscuous)) { // TODO
        // Wait for time synchronization
        while(!synchronized())
            Thread::self()->yield();
    }
}

void TSTP::Router::bootstrap()
{
    db<TSTP>(TRC) << "TSTP::Router::bootstrap()" << endl;
    TSTP::_nic->attach(this, NIC::TSTP);
}

void TSTP::GDH_Security::bootstrap()
{
    db<TSTP>(TRC) << "TSTP::GDH_Security::bootstrap()" << endl;

    TSTP::_nic->attach(this, NIC::TSTP);
}

void TSTP::Security::bootstrap()
{
    db<TSTP>(TRC) << "TSTP::Security::bootstrap()" << endl;

    TSTP::_nic->attach(this, NIC::TSTP);

    if((TSTP::here() != TSTP::sink()) && (!Traits<Radio>::promiscuous)) { // TODO
        Peer * peer = new (SYSTEM) Peer(_id, Region(TSTP::sink(), 0, 0, -1));
        _pending_peers.insert(peer->link());

        // Wait for key establishment
        while(_trusted_peers.size() == 0)
            Thread::self()->yield();
    }
}

template<unsigned int UNIT>
void TSTP::init(const NIC & nic)
{
    db<Init, TSTP>(TRC) << "TSTP::init(u=" << UNIT << ")" << endl;

    _nic = new (SYSTEM) NIC(nic);
    TSTP::Locator * locator = new (SYSTEM) TSTP::Locator;
    TSTP::Timekeeper * timekeeper = new (SYSTEM) TSTP::Timekeeper;
    TSTP::Router * router = new (SYSTEM) TSTP::Router;
    TSTP::GDH_Security * gdh_security = new (SYSTEM) TSTP::GDH_Security;
    //TSTP::Security * security = new (SYSTEM) TSTP::Security;
    TSTP * tstp = new (SYSTEM) TSTP;

    locator->bootstrap();
    timekeeper->bootstrap();
    router->bootstrap();
    gdh_security->bootstrap();
    //security->bootstrap();

    _nic->attach(tstp, NIC::TSTP);
}

template void TSTP::init<0>(const NIC & nic);
template void TSTP::init<1>(const NIC & nic);
template void TSTP::init<2>(const NIC & nic);
template void TSTP::init<3>(const NIC & nic);
template void TSTP::init<4>(const NIC & nic);
template void TSTP::init<5>(const NIC & nic);
template void TSTP::init<6>(const NIC & nic);
template void TSTP::init<7>(const NIC & nic);

__END_SYS

#endif
