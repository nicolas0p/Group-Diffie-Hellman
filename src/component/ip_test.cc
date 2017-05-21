// EPOS IP Protocol Test Program

#include <utility/ostream.h>
#include <utility/random.h>
#include <communicator.h>

using namespace EPOS;

const int ITERATIONS = 3;
const int PDU = 2000;

OStream cout;

int icmp_test()
{
    cout << "ICMP Test" << endl;

    Port<ICMP> * com;
    IP * ip = IP::get_by_nic(0);

    cout << "  IP: " << ip->address() << endl;
    cout << "  MAC: " << ip->nic()->address() << endl;

    if(ip->address()[3] % 2) { // sender
        cout << "Sender:" << endl;

        IP::Address peer_ip = ip->address();
        peer_ip[3]--;
        ICMP::Packet packet;
        com = new Port<ICMP>(0);
        unsigned int id = Random::random();

        for(int i = 0; i < ITERATIONS; i++) {
            new (&packet) ICMP::Packet(ICMP::ECHO, 0, id, i);

            int sent = com->send(peer_ip, &packet, sizeof(ICMP::Packet));
            if(sent == sizeof(ICMP::Packet))
                cout << "  Data: " << &packet << endl;
            else
                cout << "  Data was not correctly sent. It was " << sizeof(ICMP::Packet) << " bytes long, but only " << sent << " bytes were sent!"<< endl;
            Delay(100000);
        }
    } else { // receiver
        cout << "Receiver:" << endl;

        IP::Address peer_ip = ip->address();
        peer_ip[3]++;
        ICMP::Packet packet;
        com = new Port<ICMP>(0);

        for(int i = 0; i < ITERATIONS; i++) {
            ICMP::Address from;
            int received = com->receive(&from, &packet, sizeof(ICMP::Packet));
            if(received == sizeof(ICMP::Packet))
                cout << "  Data: " << &packet << endl;
            else
                cout << "  Data was not correctly received. It was " << sizeof(ICMP::Packet) << " bytes long, but " << received << " bytes were received!"<< endl;

            if(packet.type() == ICMP::ECHO) {
                db<ICMP>(WRN) << "ICMP::update: echo request from " << from << endl;

                ICMP::Packet * reply = new (&packet) ICMP::Packet(ICMP::ECHO_REPLY, 0);
                com->send(from, reply, sizeof(packet));
            } else if(packet.type() == ICMP::ECHO_REPLY)
                db<ICMP>(WRN) << "ICMP::update: echo reply to " << from << endl;
        }
    }

    delete com;

    NIC::Statistics stat = ip->nic()->statistics();
    cout << "Statistics\n"
         << "Tx Packets: " << stat.tx_packets << "\n"
         << "Tx Bytes:   " << stat.tx_bytes << "\n"
         << "Rx Packets: " << stat.rx_packets << "\n"
         << "Rx Bytes:   " << stat.rx_bytes << endl;

    return stat.tx_bytes + stat.rx_bytes;
}

int udp_test()
{
    cout << "UDP Test" << endl;

    char data[PDU];
    Link<UDP> * com;

    IP * ip = IP::get_by_nic(0);

    cout << "  IP: " << ip->address() << endl;
    cout << "  MAC: " << ip->nic()->address() << endl;

    if(ip->address()[3] % 2) { // sender
        cout << "Sender:" << endl;

        IP::Address peer_ip = ip->address();
        peer_ip[3]--;

        com = new Link<UDP>(8000, Link<UDP>::Address(peer_ip, UDP::Port(8000)));

        for(int i = 0; i < ITERATIONS; i++) {
            data[0] = '\n';
            data[1] = ' ';
            data[2] = '0' + i;
            data[3] = '0' + i;
            data[4] = '0' + i;
            data[5] = '0' + i;
            data[6] = '0' + i;
            data[7] = '0' + i;

            for(int j = 8; j < sizeof(data) - 8; j += 8) {
                data[j+0] = ' ';
                data[j+1] = '0' + i + (j / 1000000 % 10);
                data[j+2] = '0' + (j / 100000 % 10);
                data[j+3] = '0' + (j / 10000 % 10);
                data[j+4] = '0' + (j / 1000 % 10);
                data[j+5] = '0' + (j / 100 % 10);
                data[j+6] = '0' + (j / 10 % 10);
                data[j+7] = '0' + (j % 10);
            }

            data[sizeof(data) - 8] = ' ';
            data[sizeof(data) - 7] = '0' + i;
            data[sizeof(data) - 6] = '0' + i;
            data[sizeof(data) - 5] = '0' + i;
            data[sizeof(data) - 4] = '0' + i;
            data[sizeof(data) - 3] = '0' + i;
            data[sizeof(data) - 2] = '\n';
            data[sizeof(data) - 1] = 0;

            int sent = com->send(&data, sizeof(data));
            if(sent == sizeof(data))
                cout << "  Data: " << data << endl;
            else
                cout << "  Data was not correctly sent. It was " << sizeof(data) << " bytes long, but only " << sent << " bytes were sent!"<< endl;
        }
    } else { // receiver
        cout << "Receiver:" << endl;

        IP::Address peer_ip = ip->address();
        peer_ip[3]++;

        com = new Link<UDP>(8000, Link<UDP>::Address(peer_ip, UDP::Port(8000)));

        for(int i = 0; i < ITERATIONS; i++) {
            int received = com->receive(&data, sizeof(data));
            if(received == sizeof(data))
                cout << "  Data: " << data << endl;
            else
                cout << "  Data was not correctly received. It was " << sizeof(data) << " bytes long, but " << received << " bytes were received!"<< endl;
        }
    }

    delete com;

    NIC::Statistics stat = ip->nic()->statistics();
    cout << "Statistics\n"
         << "Tx Packets: " << stat.tx_packets << "\n"
         << "Tx Bytes:   " << stat.tx_bytes << "\n"
         << "Rx Packets: " << stat.rx_packets << "\n"
         << "Rx Bytes:   " << stat.rx_bytes << endl;

    return stat.tx_bytes + stat.rx_bytes;
}

int tcp_test()
{
    cout << "TCP Test" << endl;

    char data[PDU];
    Link<TCP> * com;

    IP * ip = IP::get_by_nic(0);

    cout << "  IP: " << ip->address() << endl;
    cout << "  MAC: " << ip->nic()->address() << endl;

    if(ip->address()[3] % 2) { // sender
        cout << "Sender:" << endl;

        IP::Address peer_ip = ip->address();
        peer_ip[3]--;

        com = new Link<TCP>(8000, Link<TCP>::Address(peer_ip, TCP::Port(8000))); // connect

        for(int i = 0; i < ITERATIONS; i++) {
            data[0] = '\n';
            data[1] = ' ';
            data[2] = '0' + i;
            data[3] = '0' + i;
            data[4] = '0' + i;
            data[5] = '0' + i;
            data[6] = '0' + i;
            data[7] = '0' + i;

            for(int j = 8; j < sizeof(data) - 8; j += 8) {
                data[j+0] = ' ';
                data[j+1] = '0' + i + (j / 1000000 % 10);
                data[j+2] = '0' + (j / 100000 % 10);
                data[j+3] = '0' + (j / 10000 % 10);
                data[j+4] = '0' + (j / 1000 % 10);
                data[j+5] = '0' + (j / 100 % 10);
                data[j+6] = '0' + (j / 10 % 10);
                data[j+7] = '0' + (j % 10);
            }

            data[sizeof(data) - 8] = ' ';
            data[sizeof(data) - 7] = '0' + i;
            data[sizeof(data) - 6] = '0' + i;
            data[sizeof(data) - 5] = '0' + i;
            data[sizeof(data) - 4] = '0' + i;
            data[sizeof(data) - 3] = '0' + i;
            data[sizeof(data) - 2] = '\n';
            data[sizeof(data) - 1] = 0;

            int sent = com->write(&data, sizeof(data));
            if(sent == sizeof(data))
                cout << "  Data: " << data << endl;
            else
                cout << "  Data was not correctly sent. It was " << sizeof(data) << " bytes long, but only " << sent << "bytes were sent!"<< endl;
        }
    } else { // receiver
        cout << "Receiver:" << endl;

        IP::Address peer_ip = ip->address();
        peer_ip[3]++;

        com = new Link<TCP>(TCP::Port(8000)); // listen

        for(int i = 0; i < ITERATIONS; i++) {
            int received = com->read(&data, sizeof(data));
            if(received == sizeof(data))
                cout << "  Data: " << data << endl;
            else
                cout << "  Data was not correctly received. It was " << sizeof(data) << " bytes long, but " << received << " bytes were received!"<< endl;
        }
    }

    delete com;

    NIC::Statistics stat = ip->nic()->statistics();
    cout << "Statistics\n"
         << "Tx Packets: " << stat.tx_packets << "\n"
         << "Tx Bytes:   " << stat.tx_bytes << "\n"
         << "Rx Packets: " << stat.rx_packets << "\n"
         << "Rx Bytes:   " << stat.rx_bytes << endl;

    return stat.tx_bytes + stat.rx_bytes;
}

#include <network.h>

int main()
{
    cout << "IP Test Program" << endl;
    cout << "Sizes:" << endl;
    cout << "  NIC::Header => " << sizeof(NIC::Header) << endl;
    cout << "  IP::Header => " << sizeof(IP::Header) << endl;
    cout << "  UDP::Header => " << sizeof(UDP::Header) << endl;

    icmp_test();
    Alarm::delay(2000000);
    udp_test();
    Alarm::delay(2000000);
    tcp_test();

    return 0;
}
