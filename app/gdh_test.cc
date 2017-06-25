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
    Random::seed(seed);

    // cout << "Testing mod_exp function" << endl;
    // Group_Diffie_Hellman mod_exp_test;
    // cout << "base = " << mod_exp_test.parameters().base() << endl;
    // cout << "q = " << mod_exp_test.parameters().q() << endl;
    // cout << "_private  = " << mod_exp_test.private_key() << endl;
    // Group_Diffie_Hellman::Round_Key mod_exp_1 = mod_exp_test.insert_key();
    // cout << "mod_exp_1 = pow(base, _private, q) # " << mod_exp_1 << endl;
    // Group_Diffie_Hellman::Round_Key mod_exp_2 = mod_exp_test.insert_key(mod_exp_1);
    // cout << "mod_exp_2 = pow(mod_exp_1, _private, q) # " << mod_exp_2 << endl;
    // Group_Diffie_Hellman::Round_Key mod_exp_3 = mod_exp_test.insert_key(mod_exp_2);
    // cout << "mod_exp_3 = pow(mod_exp_2, _private, q) # " << mod_exp_3 << endl;

    // cout << "Testing invert function" << endl;
    // Group_Diffie_Hellman invert_test;
    // cout << "base = " << invert_test.parameters().base() << endl;
    // cout << "q = " << invert_test.parameters().q() << endl;
    // cout << "_private  = " << invert_test.private_key() << endl;
    // Group_Diffie_Hellman::Round_Key * egcd = invert_test.egcd(invert_test.private_key(), invert_test.parameters().q());
    // cout << "egcd a = " << egcd[0] << ", b = " << egcd[1] << ", c = " << egcd[2] << endl;
    // Group_Diffie_Hellman::Round_Key inv = invert_test.mod_inv(invert_test.private_key(), invert_test.parameters().q());
    // cout << "inv = " << inv << ", inv*private = " << (invert_test.private_key()*inv)%invert_test.parameters().q() << endl;

    // cout << "Testing insert then remove" << endl;
    // Group_Diffie_Hellman test_i_r;
    // cout << "base = " << test_i_r.parameters().base() << endl;
    // cout << "q = " << test_i_r.parameters().q() << endl;
    // cout << "_private  = " << test_i_r.private_key() << endl;
    // Group_Diffie_Hellman::Round_Key public_i_r = test_i_r.insert_key();
    // cout << "public = " << public_i_r << endl;
    // Group_Diffie_Hellman::Round_Key _inverted = test_i_r.mod_inv(test_i_r.private_key(), test_i_r.parameters().q()-1);
    // cout << "_inverted = " << _inverted << endl;
    // Group_Diffie_Hellman::Round_Key removed_i_r = test_i_r.remove_key(public_i_r);
    // cout << "removed = " << removed_i_r << endl;
    

    cout << "EPOS Group Diffie-Hellman Test" << endl;
    cout << "Configuration: " << endl;
    cout << "sizeof(Group_Diffie_Hellman) = " << sizeof(Group_Diffie_Hellman) << endl;
    cout << "sizeof(Group_Diffie_Hellman::Public_Key) = " << sizeof(Group_Diffie_Hellman::Round_Key) << endl;
    cout << "sizeof(Group_Diffie_Hellman::Private_Key) = " << sizeof(Group_Diffie_Hellman::Private_Key) << endl;
    cout << "Random seed = " << seed << endl;
    cout << "Iterations = " << ITERATIONS << endl;
    
    unsigned int tests_failed = 0;
    
    for(unsigned int it = 0; it < ITERATIONS; it++) {
        cout << endl;
        cout << "Iteration " << it << endl;
    
		Group_Diffie_Hellman gateway;
		Group_Diffie_Hellman first;
        Group_Diffie_Hellman intermediate;
		Group_Diffie_Hellman last;
    
        Group_Diffie_Hellman::Round_Key first_round = first.insert_key(); //mod exp of the public key base to the power of its private key
		//first send its round key to intermediate
		Group_Diffie_Hellman::Round_Key intermediate_round = intermediate.insert_key(first_round); //mod exp of the number received to the power of its private key
		//intermediate sends its round key to last
		Group_Diffie_Hellman::Round_Key last_round = last.insert_key(intermediate_round);
		//intermediate sends its round key to all the nodes, including the gateway
		Group_Diffie_Hellman::Round_Key last_removed = intermediate_round; //the last node doesn't have to remove its private key because it already receives it ready from the intermediate node
		//intermediate sends the previous round key to the gateway
		Group_Diffie_Hellman::Round_Key gateway_final = gateway.insert_key(last_round);
		//first and intermediate remove their private keys from the round key
		Group_Diffie_Hellman::Round_Key first_removed = first.remove_key(last_round);
		Group_Diffie_Hellman::Round_Key intermediate_removed = intermediate.remove_key(last_round);
		//first and intermediate send them to the gateway
		Group_Diffie_Hellman::Round_Key first_missing_own = gateway.insert_key(first_removed);
		Group_Diffie_Hellman::Round_Key intermediate_missing_own = gateway.insert_key(intermediate_removed);
		Group_Diffie_Hellman::Round_Key last_missing_own = gateway.insert_key(last_removed);
		//the gateway sends the key without each node's private key to each of them so they can do the last exponentiation
		Group_Diffie_Hellman::Round_Key first_final = first.insert_key(first_missing_own);
		Group_Diffie_Hellman::Round_Key intermediate_final = intermediate.insert_key(intermediate_missing_own);
		Group_Diffie_Hellman::Round_Key last_final = last.insert_key(last_missing_own);
    
        cout << "first round " << first_round << endl;
        cout << "intermediate round " << intermediate_round << endl;
        cout << "last round " << last_round << endl;
    
        cout << "first removed " << first_removed << endl;
        cout << "intermediate removed " << intermediate_removed << endl;
    
        cout << "first missing_own " << first_missing_own << endl;
        cout << "intermediate missing_own " << intermediate_missing_own << endl;
        cout << "last missing_own " << last_missing_own << endl;
    
        cout << "first final " << first_final << endl;
        cout << "intermediate final " << intermediate_final << endl;
        cout << "last final " << last_final << endl;
        cout << "gateway final " << gateway_final << endl;
    
        bool ok = gateway_final == first_final && first_final == intermediate_final && intermediate_final == last_final;
        // bool ok1 = gateway_final.x == first_final.x && gateway_final.y == first_final.y && gateway_final.z == first_final.z;
        // bool ok2 = first_final.x == intermediate_final.x && first_final.y == intermediate_final.y && first_final.z == intermediate_final.z;
        // bool ok3 = intermediate_final.x == last_final.x && intermediate_final.y == last_final.y && intermediate_final.z == last_final.z;
        // bool ok = ok1 && ok2 && ok3;
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
