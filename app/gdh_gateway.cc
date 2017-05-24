#include <machine.h>
#include <alarm.h>
#include <smart_data.h>
#include <tstp.h>
#include <machine/cortex/cc2538.h>

using namespace EPOS;

OStream cout;

const unsigned int INTEREST_PERIOD = 1000000;
const unsigned int INTEREST_EXPIRY = 2 * INTEREST_PERIOD;

int main()
{
    cout << "GDH Gateway test" << endl;

    cout << "My machine ID is:";
    for(unsigned int i = 0; i < 8; i++)
        cout << " " << hex << Machine::id()[i];
    cout << endl;
    cout << "You can set this value at src/component/tstp_init.cc to set initial coordinates for this mote." << endl;

    cout << "My coordinates are " << TSTP::here() << endl;
    cout << "The time now is " << TSTP::now() << endl;
    cout << "I am" << (TSTP::here() == TSTP::sink() ? " " : " not ") << "the sink" << endl;

	typedef TSTP::Region::Space Space;

	//Lets say these nodes exist. We need to configure them in this way!
	Space first(Space::Center(1,1,1));
	Space intermediate(Space::Center(2,2,2));
	Space last(Space::Center(3,3,3));

	auto nodes = Simple_List<Space>();
	List_Elements::Singly_Linked<Space> el_first(&first);
	List_Elements::Singly_Linked<Space> el_intermediate(&intermediate);
	List_Elements::Singly_Linked<Space> el_last(&last);
	nodes.insert(&el_first);
	nodes.insert(&el_intermediate);
	nodes.insert(&el_last);

	int g_id = TSTP::GDH_Security::begin_group_diffie_hellman(nodes);

	cout << "Group id = " << g_id << endl;

    return 0;
}
