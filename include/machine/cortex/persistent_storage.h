// EPOS ARM Cortex Persisistent Storage Mediator Declarations

#ifndef __cortex_flash_h
#define __cortex_flash_h

#include <utility/math.h>
#include <persistent_storage.h>
#include <machine.h>

__BEGIN_SYS

#ifdef __mmod_emote3__

// Flash memory
class Flash_Engine: private Machine_Model
{
public:
    typedef unsigned int Address;
    typedef unsigned int Word;

    static const unsigned int BASE = Traits<Machine>::FLASH_BASE;
    static const unsigned int TOP = Traits<Machine>::FLASH_TOP;
    static const unsigned int SIZE_OF_PAGE = 2048;
    static const unsigned int N_PAGES = 256;
    static const unsigned int WORDS_IN_PAGE = SIZE_OF_PAGE / sizeof(Word);
    static const unsigned int MAX_WRITE_COUNT = 8;

private:
    enum {              // Description                                    Type  Value after reset
        FCTL    = 0x08, // Flash control                                  RW    0x00000004
        FADDR   = 0x0C, // Sets the address to be written in flash memory RW    0x00000000
        FWDATA  = 0x10, // Flash data                                     RW    0x00000000
        DIECFG0 = 0x14, // FLASH information page bit settings            RO    0xB9640580
        DIECFG1 = 0x18, // FLASH information page bit settings            RO    0x00000000
        DIECFG2 = 0x1C, // FLASH information page bit settings            RO    0x00002000
    };

    // Useful bits in FCTL register
    enum {             // Description                             Type  Value after reset
        BUSY = 1 << 7, // Set when the WRITE or ERASE bit is set  RO    0
    };

public:
    static bool write(const void * data, const Address & dst, unsigned int size) {
        db<Persistent_Storage>(TRC) << "Flash_Engine::write(d=" << data << ",a=" << hex << dst << ",s=" << dec << size << ")" << endl;

        typedef int (* volatile Program_Flash) (const unsigned int*, unsigned int, unsigned int);
        Program_Flash rom_function_write = *reinterpret_cast<Program_Flash*>(ROM_API_PROG_FLASH);

        while(reg(FCTL) & BUSY);

        // Write the last bytes to make the remaining size multiple of sizeof(Word)
        if(size % sizeof(Word)) {
            unsigned int bytes = size % sizeof(Word);
            // Pad the remaining bytes with flash content
            Word w = read(dst + size - bytes);
            memcpy(&w, data, bytes);

            // The ROM function simply won't write everything. We have to check the result and call it multiple times.
            do
                if(rom_function_write(&w, dst + size - bytes, bytes))
                    return false;
            while(read(dst + size - bytes) != w);

            size -= size % sizeof(Word);
        }

        // The ROM function simply won't write everything. We have to check the result and call it multiple times.
        for(unsigned int i = 0; i < size; ) {
            const Word * dt = reinterpret_cast<const Word *>(data);
            if(rom_function_write(dt + i / sizeof(Word), dst + i, size - i))
                return false;
            for(; (i < size) && ((dt[i / sizeof(Word)]) == read(dst + i)); i += sizeof(Word));
        }

        return true;
    }

    static Word read(const Address & address) { return *reinterpret_cast<volatile Word*>(address); }

    static bool erase(const Address & addr, unsigned int size) {
        db<Persistent_Storage>(TRC) << "Flash_Engine::erase(a=" << hex << addr << ",s=" << dec << size << ")" << endl;
        assert(!(size % sizeof(Word)));

        typedef int (* volatile Erase) (unsigned int, unsigned int);
        Erase rom_function_erase = *reinterpret_cast<Erase*>(ROM_API_PAGE_ERASE);

        while(reg(FCTL) & BUSY);

        // The ROM function simply won't erase everything. We have to check the result and call it multiple times.
        for(unsigned int i = 0; i < size; ) {
            if(rom_function_erase(addr + i, size - i))
                return false;
            for(; (i < size) && (0xFFFFFFFF == read(addr + i)); i += sizeof(Word));
        }

        return true;
    }

private:
    static volatile Reg32 & reg(unsigned int o) { return reinterpret_cast<volatile Reg32 *>(FLASH_CTRL_BASE)[o / sizeof(Reg32)]; }
};

#endif

// Flash memory
class Persistent_Storage: public Persistent_Storage_Common, private Flash_Engine
{
    friend class Machine;

    static const unsigned int BASE = Flash_Engine::BASE;
    static const unsigned int TOP = Flash_Engine::TOP;
    static const unsigned int STORAGE_BASE = Traits<Machine>::FLASH_STORAGE_BASE;
    static const unsigned int STORAGE_TOP = Traits<Machine>::FLASH_STORAGE_TOP;

    typedef Flash_Engine Engine;

public:
    static const unsigned int SIZE = STORAGE_TOP - STORAGE_BASE;

    typedef Engine::Word Word;
    typedef Engine::Address Address;

    static Address write(Address address, const void * data, unsigned int size) {
        db<Persistent_Storage>(TRC) << "Persistent_Storage::write(a=" << hex << address << ",d=" << data << ",s=" << dec << size << ")" << endl;

        address += STORAGE_BASE;
        Address end_address = address + size;

        if(end_address > STORAGE_TOP) {
            db<Persistent_Storage>(ERR) << "Persistent_Storage::write(a=" << hex << address << ",d=" << data << ",s=" << dec << size << ") ERROR: write would pass the end of flash storage, aborting!" << endl;
            return address - STORAGE_BASE;
        }

        if(address % sizeof(Word)) {
            db<Persistent_Storage>(ERR) << "Persistent_Storage::write(a=" << hex << address << ",d=" << data << ",s=" << dec << size << ") ERROR: write must start at a word-aligned address!" << endl;
            return address - STORAGE_BASE;
        }

        unsigned int first_page = address_to_page(address);
        unsigned int last_page = address_to_page(end_address - 1);

        for(unsigned int page = first_page; page <= last_page; ++page) {
            bool erase_needed = false;
            Address next_page_boundary = page_to_address(page+1);
            Address write_end = min(end_address, next_page_boundary);
            unsigned int bytes = (write_end - address);

            if(_metadata.write_count[page] >= MAX_WRITE_COUNT)
                erase_needed = true;
            else {
                // Check if a simple write would leave the flash with the desired content
                const Word * d = reinterpret_cast<const Word *>(data);
                unsigned int b = bytes;
                unsigned int addr = address;
                while(b >= sizeof(Word)) {
                    if(((*d) & (Engine::read(addr))) != (*d)) {
                        erase_needed = true;
                        break;
                    }
                    addr += sizeof(Word);
                    b -= sizeof(Word);
                    d++;
                }
                if(!erase_needed) {
                    Word dt = Engine::read(addr);
                    for(unsigned int i = 0; i < b; i++) {
                        char dti = reinterpret_cast<char *>(&dt)[i];
                        char datai = reinterpret_cast<const char *>(data)[i];
                        if((dti & datai) != dti) {
                            erase_needed = true;
                            break;
                        }
                    }
                }
            }

            if(erase_needed) {
                Address page_begin = page_to_address(page);
                Address page_end = next_page_boundary;

                // If we're not writing a whole page, save that page
                if((address != page_begin) || (end_address != page_end)) {
                    Address address = page_to_address(page);
                    read(address - STORAGE_BASE, _metadata.page_backup, SIZE_OF_PAGE);
                }

                Engine::erase(page_to_address(page), SIZE_OF_PAGE);
                _metadata.write_count[page] = 0;

                unsigned int write_delta = (address - page_begin) / 4;
                memcpy(&(_metadata.page_backup[write_delta]), data, bytes);

                Engine::write(_metadata.page_backup, page_begin, SIZE_OF_PAGE);
            } else
                Engine::write(data, address, bytes);

            ++_metadata.write_count[page];
            data = reinterpret_cast<const char *>(data) + bytes;
            address += bytes;
        }

        return address - STORAGE_BASE;
    }

    static void read(Address address, void * data, unsigned int size) {
        db<Persistent_Storage>(TRC) << "Persistent_Storage::read(a=" << hex << address << ",d=" << data << ",s=" << dec << size << ")" << endl;

        address += STORAGE_BASE;

        Word * d = reinterpret_cast<Word *>(data);
        while(size > sizeof(Word)) {
            *d = Engine::read(address);
            address += sizeof(Word);
            size -= sizeof(Word);
            d++;
        }
        Word word = Engine::read(address);
        memcpy(d, &word, size);
    }

    static Word read(const Address & address) { return Engine::read(address + STORAGE_BASE); }

private:
    static void init();

    static unsigned int address_to_page(const Address & address) { return (address - Engine::BASE) / SIZE_OF_PAGE; }
    static Address page_to_address(unsigned int page) { return (page * SIZE_OF_PAGE) + Engine::BASE; }

    struct Metadata {
        Word page_backup[WORDS_IN_PAGE];
        unsigned char write_count[N_PAGES];
    };

    static Metadata _metadata;
};

template<typename T, unsigned int N = Persistent_Storage::SIZE / (((sizeof(T) + sizeof(Persistent_Storage::Word) - 1) / sizeof(Persistent_Storage::Word)) * sizeof(Persistent_Storage::Word))>
class Persistent_Ring_FIFO
{
private:
    typedef Persistent_Storage::Address Address;

    static const Address START_ADDR = 0;
    static const Address END_ADDR = START_ADDR + sizeof(Address);
    static const Address DATA_ADDR = END_ADDR + sizeof(Address);
    static const Address EMPTY = 1 << (8 * sizeof(Address) - 1);

public:
    static const unsigned int SIZEOF_T = ((sizeof(T) + sizeof(Persistent_Storage::Word) - 1) / sizeof(Persistent_Storage::Word)) * sizeof(Persistent_Storage::Word);
    static const unsigned int SIZE = N * SIZEOF_T < Persistent_Storage::SIZE / SIZEOF_T * SIZEOF_T ?
        N * SIZEOF_T : Persistent_Storage::SIZE / SIZEOF_T * SIZEOF_T;

    static void clear() {
        Address start = DATA_ADDR;
        Address end = DATA_ADDR | EMPTY;

        Persistent_Storage::write(START_ADDR, &start, sizeof(Address));
        Persistent_Storage::write(END_ADDR, &end, sizeof(Address));
    }

    static void push(const T & t) {
        Address end = read_end();
        if(end & EMPTY)
            push_first(t, end);
        else {
            Address start = read_start();

            Address new_end;
            Address new_start = start;
            if(end + SIZEOF_T > SIZE)
                new_end = DATA_ADDR + SIZEOF_T;
            else
                new_end = end + SIZEOF_T;

            if(((end <= start) && (new_end > start)) ||
                    ((new_end < end) && ((new_end > start) || (end <= start)))) {
                if(start + SIZEOF_T > SIZE)
                    new_start = DATA_ADDR + SIZEOF_T;
                else
                    new_start += start + SIZEOF_T;
            }

            Persistent_Storage::write(new_end - SIZEOF_T, &t, SIZEOF_T);

            if(new_start != start)
                Persistent_Storage::write(START_ADDR, &new_start, sizeof(Address));

            Persistent_Storage::write(END_ADDR, &new_end, sizeof(Address));
        }
    }

    static bool pop(T * t) {
        Address end = read_end();
        if(end & EMPTY)
            return false;

        Address new_end = end;
        Address new_start;

        Address start = read_start();
        if(start + SIZEOF_T > SIZE)
            new_start = DATA_ADDR + SIZEOF_T;
        else
            new_start = start + SIZEOF_T;

        if((start < end) && (new_start >= end) ||
                ((new_start < start) && (end > start) || (new_start >= end))) {
            new_end = end | EMPTY;
        }

        Persistent_Storage::read(start, t, sizeof(T));
        if(new_end != end)
            Persistent_Storage::write(END_ADDR, &new_end, sizeof(Address));

        Persistent_Storage::write(START_ADDR, &new_start, sizeof(Address));

        return true;
    }

private:
    static Address read_start() { return Persistent_Storage::read(START_ADDR); }
    static Address read_end() { return Persistent_Storage::read(END_ADDR); }

    static void push_first(const T & t, Address end) {
        end = end & ~EMPTY;
        if(end + SIZEOF_T > SIZE)
            end = DATA_ADDR + SIZEOF_T;
        else
            end += SIZEOF_T;

        Persistent_Storage::write(end - SIZEOF_T, &t, SIZEOF_T);
        Persistent_Storage::write(END_ADDR, &end, sizeof(Address));
    }
};

__END_SYS

#endif
