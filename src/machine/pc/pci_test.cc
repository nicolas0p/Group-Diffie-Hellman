// EPOS PC PCI Test Program

#include <utility/ostream.h>
#include <pci.h>

using namespace EPOS;

int main()
{
    OStream cout;

    cout << "PCI test" << endl;

    PCI pci;

    for(int bus = 0; bus <= Traits<PCI>::MAX_BUS; bus++) {
        for(int dev_fn = 0; dev_fn <= Traits<PCI>::MAX_DEV_FN; dev_fn++) {
            PCI::Header hdr;

            pci.header(PCI::Locator(bus, dev_fn), &hdr);
            if(hdr) {
        	cout << (int)bus << ":" << (dev_fn >> 3) << "." << (dev_fn & 7)
        	     << " Class " << hdr.class_id << " " << hdr.vendor_id
        	     << ":" << hdr.device_id
        	     << " (rev " << (int)hdr.revision_id
        	     << ")";
        	for(int i = 0; i < PCI::Region::N; i++)
        	    if(hdr.region[i].size)
        		cout << " #" << i
        		     << "(" << (void *)hdr.region[i].phy_addr
        		     << "," << (void *)hdr.region[i].size << ")";
        	cout << "" << endl;
            }
        }
    }

    PCI::Locator loc = pci.scan(32902, 28945, 0);

    PCI::Header hdr;
    pci.header(loc, &hdr);
    if(hdr) {
        cout << hdr.locator.bus << ":" << (hdr.locator.dev_fn >> 3)
             << "." << (hdr.locator.dev_fn & 7)
             << " Class " << hdr.class_id << " " << hdr.vendor_id
             << ":" << hdr.device_id
             << " (rev " << (int)hdr.revision_id
             << ")";
        for(int i = 0; i < 6; i++)
            if(hdr.region[i].size)
        	cout << " #" << i
        	     << "(" << (void *)hdr.region[i].phy_addr
        	     << "," << (void *)hdr.region[i].size << ")";
        cout << "" << endl;
    }

    cout << "Finish!" << endl;

    return 0;
}
