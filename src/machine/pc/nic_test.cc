// EPOS NIC Test Programs

#include <utility/ostream.h>
#include <nic.h>
#include <alarm.h>

using namespace EPOS;

OStream cout;

int main()
{
    NIC nic;
    NIC::Address src, dst;
    NIC::Protocol prot;
    char data[nic.mtu()];

    NIC::Address self = nic.address();
    cout << "  MAC: " << self << endl;

    if(self[5] % 2) { // sender
        for(int i = 0; i < 10; i++) {
            memset(data, '0' + i, nic.mtu());
            data[nic.mtu() - 1] = '\n';
            nic.send(nic.broadcast(), 0x8888, data, nic.mtu());
        }
    } else { // receiver
        for(int i = 0; i < 10; i++) {
           nic.receive(&src, &prot, data, nic.mtu());
           cout << "  Data: " << data;
        }
    }

    NIC::Statistics stat = nic.statistics();
    cout << "Statistics\n"
	 << "Tx Packets: " << stat.tx_packets << "\n"
	 << "Tx Bytes:   " << stat.tx_bytes << "\n"
	 << "Rx Packets: " << stat.rx_packets << "\n"
	 << "Rx Bytes:   " << stat.rx_bytes << "\n";
}
