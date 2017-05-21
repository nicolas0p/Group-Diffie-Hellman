#ifndef _KEY_DATABASE_H
#define _KEY_DATABASE_H

#include <system/config.h>
#include <flash.h>

__BEGIN_UTIL

template <typename Address>
class Key_Database
{
	const static unsigned int ID_SIZE = Traits<Build>::ID_SIZE;
	const static unsigned int KEY_SIZE = Traits<Diffie_Hellman>::SECRET_SIZE;
	const static unsigned int N_ENTRIES = Traits<Secure_NIC>::MAX_PEERS;
	public:
	Key_Database();
	bool validate_peer(const char *sn, const char *ms, const char *auth, const Address a);
	bool insert_peer(const char *ms, const Address a);
	bool remove_peer(const char *ms, const Address a);
	bool insert_peer(const char *sn, const char *auth);
	bool remove_peer(const char *sn, const char *auth);

    bool auth_to_sn(char *serial_number, const char *auth, const Address addr) const;
    bool sn_to_addr(const char* serial_number, Address & addr) const;
//  bool sn_to_ms(char *master_secret, const char *serial_number, Address addr);
    bool addr_to_ms(char *master_secret, const Address addr) const;
    bool ms_to_sn(char *serial_number, const char *master_secret) const;

    //void load(const Flash::Address start_addr);
    //void save(const Flash::Address start_addr) const;

    private:
    typedef struct
    {
        char serial_number[ID_SIZE];
        char auth[16];
        bool free;
        bool validated;
    } Known_Node;// __attribute__((aligned (4)));

    typedef struct
    {
        char master_secret[KEY_SIZE];
        Address addr;
        unsigned int node_index; // Won't use a pointer because this value can be saved/loaded from flash
        bool free;
    } Authenticated_Peer;// __attribute__((aligned (4)));

    typedef struct
    {
        char master_secret[KEY_SIZE];
        Address addr;
        bool free;
    } Weak_Peer;// __attribute__((aligned (4)));

    struct Database {
        Weak_Peer _weak_peers[N_ENTRIES];
        Known_Node _known_nodes[N_ENTRIES];
        Authenticated_Peer _peers[N_ENTRIES];
    } _database;

    void dump()
    {        
        for(auto i=0u;i<N_ENTRIES;i++)
            if(!_database._peers[i].free)
            {
                kout << "_database._peers["<<i<<"] = "<< endl << "[";
                for(auto j=0u;j<KEY_SIZE;j++)                
                    kout << (int)_database._peers[i].master_secret[j] << (j==KEY_SIZE-1?"]":",");
                kout << endl << _database._peers[i].addr
                     << endl << _database._peers[i].node_index << endl;
            }            
        for(auto i=0u;i<N_ENTRIES;i++)
            if(!_database._weak_peers[i].free)
            {
                kout << "_database._weak_peers["<<i<<"] = "<< endl << "[";
                for(auto j=0u;j<KEY_SIZE;j++)                
                    kout << (int)_database._weak_peers[i].master_secret[j] << (j==KEY_SIZE-1?"]":",");
                kout << endl << _database._weak_peers[i].addr << endl;
            }            
        for(auto i=0u;i<N_ENTRIES;i++)
            if(!_database._known_nodes[i].free && _database._known_nodes[i].validated)
            {
                kout << "_database._known_nodes["<<i<<"] = "<< &_database._known_nodes[i] << endl << "[";
                for(auto j=0u;j<ID_SIZE;j++)                
                    kout << (int)_database._known_nodes[i].serial_number[j] << (j==ID_SIZE-1?"]":",");
                kout << endl << "[";
                for(auto j=0u;j<16;j++)                
                    kout << (int)_database._known_nodes[i].auth[j] << (j==16-1?"]":",");
                kout << endl << _database._known_nodes[i].validated << endl;;
            }            
    }

    bool equals(const char *a, int sza, const char *b, int szb) const
    {
        while((sza > 0) && ((a[sza-1] == 0) || (a[sza-1] == '0'))) sza--;
        while((szb > 0) && ((b[szb-1] == 0) || (b[szb-1] == '0'))) szb--;
        if(sza != szb) return false;
        for(int i=0;i<sza;i++)
            if(a[i] != b[i]) return false;
        return true;
    }
};

/*
template<typename Address>
void Key_Database<Address>::load(const Flash::Address start_addr)
{
    Flash::read(start_addr, reinterpret_cast<unsigned int *>(&_database), sizeof(Database));
}

template<typename Address>
void Key_Database<Address>::save(const Flash::Address start_addr) const
{
    Flash::write(start_addr, reinterpret_cast<const unsigned int *>(&_database), sizeof(Database));
}
*/

template<typename Address>
Key_Database<Address>::Key_Database()
{
    for(auto i=0u;i<N_ENTRIES;i++)
    {
        _database._peers[i].free = true;
        _database._weak_peers[i].free = true;
        _database._known_nodes[i].free = true;
        _database._known_nodes[i].validated = false;
    }
}

template<typename Address>
bool Key_Database<Address>::insert_peer(const char *sn, const char *auth)
{
    // Check if this serial number is already there
    for(auto i=0u;i<N_ENTRIES;i++) { 
        if(!_database._known_nodes[i].free && equals(_database._known_nodes[i].serial_number, ID_SIZE, sn, ID_SIZE)) { // It is
            if(equals(_database._known_nodes[i].auth, 16, auth, 16)) { // auth is the same. Do nothing.
                return false;
            }
            else { // auth is different, so change it to the newly-inserted one
                for(auto j=0u; j<16u; j++) {
                    _database._known_nodes[i].auth[j] = auth[j];
                }
                return true;
            }
        }
    }

    for(auto i=0u;i<N_ENTRIES;i++) {
        if(_database._known_nodes[i].free) {
            _database._known_nodes[i].free = false;
            for(auto j=0u;j<ID_SIZE;j++) {
                _database._known_nodes[i].serial_number[j] = sn[j];
            }
            for(auto j=0u;j<16u;j++) {
                _database._known_nodes[i].auth[j] = auth[j];          
            }
            //dump();
            return true;
        }
    }
    return false;
}

template<typename Address>
bool Key_Database<Address>::remove_peer(const char *sn, const char *auth)
{
    //kout << "Key_Database::remove_peer" << endl;
    for(auto i=0u;i<N_ENTRIES;i++)
    {
        if(!_database._known_nodes[i].free && equals(_database._known_nodes[i].serial_number, ID_SIZE, sn, ID_SIZE))
        {
            _database._known_nodes[i].free = true;
            //dump();
            return true;
        }
    }
    return false;
}

template<typename Address>
bool Key_Database<Address>::insert_peer(const char *ms, const Address a)
{
    // Check if this master secret is already there
    for(auto i=0u;i<N_ENTRIES;i++) { 
        if(!_database._weak_peers[i].free && equals(_database._weak_peers[i].master_secret, KEY_SIZE, ms, KEY_SIZE)) { // It is
            if(_database._weak_peers[i].addr == a) { // address is the same. Do nothing.
                return false;
            }
            else { // address is different, so change it to the newly-inserted one
                _database._weak_peers[i].addr = a;
                return true;
            }
        }
    }

    //kout << "Key_Database::insert_peer" << endl;
    for(auto i=0u;i<N_ENTRIES;i++) {
        if(_database._weak_peers[i].free) {
            _database._weak_peers[i].free = false;
            _database._weak_peers[i].addr = a;
            for(auto j=0u;j<KEY_SIZE;j++) {
                _database._weak_peers[i].master_secret[j] = ms[j];
            }
            //dump();
            return true;
        }
    }
    return false;
}

template<typename Address>
bool Key_Database<Address>::remove_peer(const char *ms, const Address a)
{
    //kout << "Key_Database::remove_peer" << endl;
    for(auto i=0u;i<N_ENTRIES;i++)
    {
        if(!_database._weak_peers[i].free && (a == _database._weak_peers[i].addr))
        {
            _database._weak_peers[i].free = true;
            //dump();
            return true;
        }
    }
    return false;
}

template<typename Address>
bool Key_Database<Address>::sn_to_addr(const char* serial_number, Address & addr) const
{
    //kout << "Key_Database::sn_to_addr" << endl;
    for(auto i=0u;i<N_ENTRIES;i++)
    {
        if(!_database._peers[i].free && equals(_database._known_nodes[_database._peers[i].node_index].serial_number, ID_SIZE, serial_number, ID_SIZE))
        {
            addr = _database._peers[i].addr;
            //dump();
            return true;
        }
    }
    return false;
}

template<typename Address>
bool Key_Database<Address>::auth_to_sn(char *serial_number, const char *auth, const Address addr) const
{
    //kout << "Key_Database::auth_to_sn" << endl;
    for(auto i=0u;i<N_ENTRIES;i++)
    {
        if(!_database._known_nodes[i].free && equals(_database._known_nodes[i].auth, 16, auth, 16))
        {
            for(auto j=0u;j<ID_SIZE;j++)
                serial_number[j] = _database._known_nodes[i].serial_number[j];
            //dump();
            return true;
        }
    }
    return false;
}

template<typename Address>
bool Key_Database<Address>::addr_to_ms(char *master_secret, const Address addr) const
{
    //kout << "Key_Database::addr_to_ms" << endl;
    for(auto i=0u;i<N_ENTRIES;i++)
    {
        if(!_database._peers[i].free && (_database._peers[i].addr == addr))
        {
            for(auto j=0u;j<KEY_SIZE;j++)
                master_secret[j] = _database._peers[i].master_secret[j];
            //dump();
            return true;
        }
        if(!_database._weak_peers[i].free && (_database._weak_peers[i].addr == addr))
        {
            for(auto j=0u;j<KEY_SIZE;j++)
                master_secret[j] = _database._weak_peers[i].master_secret[j];
            //dump();
            return true;
        }
    }
    return false;
}

template<typename Address>
bool Key_Database<Address>::ms_to_sn(char *serial_number, const char *master_secret) const
{
    //kout << "Key_Database::ms_to_sn" << endl;
    for(auto i=0u;i<N_ENTRIES;i++)
        if(!_database._peers[i].free && equals(_database._peers[i].master_secret, KEY_SIZE, master_secret, KEY_SIZE))
        {
            for(auto j=0u;j<ID_SIZE;j++)
                serial_number[j] = _database._known_nodes[_database._peers[i].node_index].serial_number[j];
            //dump();
            return true;
        }
    return false;
}

template<typename Address>
bool Key_Database<Address>::validate_peer(const char *sn, const char *ms, const char *auth, const Address a)
{
    //kout << "Key_Database::validate_peer" << endl;
    for(auto i=0u;i<N_ENTRIES;i++)
    {
        if(!_database._weak_peers[i].free && (_database._weak_peers[i].addr == a) && (equals(_database._weak_peers[i].master_secret, KEY_SIZE, ms, KEY_SIZE)))
        {
            // Revoke any other authenticated node under the same ID
            if(Traits<Secure_NIC>::ALLOW_MULTIPLE_NODES_WITH_SAME_ID)
            {
                for(auto j=0u;j<N_ENTRIES;j++)
                {
                    if((!_database._peers[j].free) && equals(_database._known_nodes[_database._peers[j].node_index].serial_number, ID_SIZE, sn, ID_SIZE))
                        _database._peers[j].free = true;
                }
            }
            for(auto j=0u;j<N_ENTRIES;j++)
            {
                if(!_database._known_nodes[j].free 
                        && ( Traits<Secure_NIC>::ALLOW_MULTIPLE_NODES_WITH_SAME_ID || !_database._known_nodes[j].validated )
                        && equals(_database._known_nodes[j].serial_number, ID_SIZE, sn, ID_SIZE))
                {
                    for(auto k=0u;k<N_ENTRIES;k++)
                    {
                        if(_database._peers[k].free)
                        {
                            _database._peers[k].free = false;
                            _database._peers[k].addr = a;
                            _database._peers[k].node_index = j;
                            for(auto l=0u;l<KEY_SIZE;l++)
                                _database._peers[k].master_secret[l] = ms[l];
                            _database._weak_peers[i].free = true;
                            _database._known_nodes[j].validated = true;
                            //dump();
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}
/*
template<typename Address>
bool Key_Database<Address>::sn_to_ms(char *master_secret, const char *serial_number, Address addr)
{
    for(int i=0;i<N_ENTRIES;i++)
        if(!_peers[i].free && (_peers[i].addr == addr) && !strcmp(_peers[i].node->serial_number, serial_number))
        {
            _peers[i].addr = addr;
            master_secret = _peers[i].master_secret;
            return true;
        }
    return false;
}
*/
__END_UTIL
#endif
