// EPOS Diffie-Hellman Component Test Program

#include <group_diffie_hellman.h>
#include <utility/ostream.h>
#include <utility/random.h>

using namespace EPOS;

OStream cout;

static const unsigned int ITERATIONS = 50;

int main()
{
    unsigned int seed = Random::random();

    cout << "EPOS Group Diffie-Hellman Test" << endl;
    cout << "Configuration: " << endl;
    cout << "Group_Diffie_Hellman::SECRET_SIZE = " << Group_Diffie_Hellman::SECRET_SIZE << endl;
    cout << "Group_Diffie_Hellman::PUBLIC_KEY_SIZE = " << Group_Diffie_Hellman::PUBLIC_KEY_SIZE << endl;
    cout << "sizeof(Group_Diffie_Hellman) = " << sizeof(Group_Diffie_Hellman) << endl;
    cout << "sizeof(Group_Diffie_Hellman::Public_Key) = " << sizeof(Group_Diffie_Hellman::Round_Key) << endl;
    cout << "sizeof(Group_Diffie_Hellman::Private_Key) = " << sizeof(Group_Diffie_Hellman::Private_Key) << endl;
    cout << "Random seed = " << seed << endl;
    cout << "Iterations = " << ITERATIONS << endl;

    unsigned int tests_failed = 0;

    Random::seed(seed);

    for(unsigned int it = 0; it < ITERATIONS; it++) {
        cout << endl;
        cout << "Iteration " << it << endl;

		Group_Diffie_Hellman gateway;
		Group_Diffie_Hellman first;
        Group_Diffie_Hellman intermediate;
		Group_Diffie_Hellman last;

        // cout << "First's public key: " << first.parameters() << endl; //should be g and q
        // cout << "Intermediate's public key: " << intermediate.parameters() << endl;
        // cout << "Last's public key: " << last.parameters() << endl;
        // cout << "Gateway's public key: " << gateway.parameters() << endl;

        Group_Diffie_Hellman::Round_Key first_round = first.insert_key(); //mod exp of the public key base to the power of its private key
		//first send its round key to intermediate
		Group_Diffie_Hellman::Round_Key intermediate_round = intermediate.insert_key(first_round); //mod exp of the number received to the power of its private key
		//intermediate sends its round key to last
		Group_Diffie_Hellman::Round_Key last_round = last.insert_key(intermediate_round);
		//intermediate sends its round key to all the nodes, including the gateway
		auto last_removed = intermediate_round; //the last node doesn't have to remove its private key because it already receives it ready from the intermediate node
		//intermediate sends the previous round key to the gateway
		Group_Diffie_Hellman::Round_Key gateway_final = gateway.insert_key(last_round);
		//first and intermediate remove their private keys from the round key
		Group_Diffie_Hellman::Round_Key first_removed = first.remove_key(last_round);
		Group_Diffie_Hellman::Round_Key intermediate_removed = intermediate.remove_key(last_round);
		//first and intermediate send them to the gateway
		Group_Diffie_Hellman::Round_Key before_last_first = gateway.insert_key(first_removed);
		Group_Diffie_Hellman::Round_Key before_last_intermediate = gateway.insert_key(intermediate_removed);
		Group_Diffie_Hellman::Round_Key before_last_last = gateway.insert_key(last_removed); //great name, right? :)
		//the gateway sends the key without each node's private key to each of them so they can do the last exponentiation
		Group_Diffie_Hellman::Round_Key first_final = first.insert_key(before_last_first);
		Group_Diffie_Hellman::Round_Key intermediate_final = intermediate.insert_key(before_last_intermediate);
		Group_Diffie_Hellman::Round_Key last_final = last.insert_key(before_last_last);

        // bool ok = gateway_final == first_final && first_final == intermediate_final && intermediate_final == last_final;
        bool ok1 = gateway_final.x == first_final.x && gateway_final.y == first_final.y && gateway_final.z == first_final.z;
        bool ok2 = first_final.x == intermediate_final.x && first_final.y == intermediate_final.y && first_final.z == intermediate_final.z;
        bool ok3 = intermediate_final.x == last_final.x && intermediate_final.y == last_final.y && intermediate_final.z == last_final.z;
        bool ok = ok1 && ok2 && ok3;
        if(ok) {
            cout << "Shared key = " << gateway_final << endl;
            cout << "OK! The key shared among all members of the group is the same" << endl;
        }
        else {
            cout << "First's shared key: " << first_final << endl;
            cout << "Intermediate's shared key: " << intermediate_final << endl;
            cout << "Last's shared key: " << last_final << endl;
            cout << "Gateway's shared key: " << gateway_final << endl;
            cout << "ERROR! Shared keys do not match!" << endl;
        }

        tests_failed += !ok;
    }

    cout << endl;
    cout << "Tests finished with " << tests_failed << " error" << (tests_failed > 1 ? "s" : "") << " detected." << endl;
    cout << endl;

    return 0;
}
