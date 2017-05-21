#ifndef flash_logger_hdr
#define flash_logger_hdr

#include <flash.h>
#include <utility/string.h>

/**
* This class is properly documented at http://lisha.ufsc.br/Flash+Logger+Module
* The last update occured on 06/05/2016.
* The last change in this file is present at the svn version 3940.
* If any doubts are still present, email asantana@lisha.ufsc.br
**/

__BEGIN_UTIL

extern "C"{
    void * memcpy(void * d, const void * s, size_t n);
}

struct Flash_Address{
    static const unsigned int START_ADDRESS;
    static const unsigned int CURRENT_BLOCK_COUNT_MEMORY_MAP;
    static const unsigned int LAST_ADDRESS;
    static const unsigned int FIRST_WRITEABLE_ADDRESS;
};

template<unsigned int n>
class Flash_Logger{
protected:

    template<unsigned int m>
    struct Log_Data{
        static const unsigned int DATA_COUNT = m;
        static const unsigned int DATA_SIZE = DATA_COUNT*sizeof(unsigned int);
    };

public:
    static const unsigned int MAX_BLOCK_COUNT;

    const unsigned int block_size() const{
        return Log_Data<n>::DATA_SIZE;
    }

    const unsigned int data_count() const{
        return Log_Data<n>::DATA_COUNT;
    }

    const unsigned int get_current_block_count(){
        return current_block_count;
    }

    const unsigned int overwritten_block_count(){
        return (current_block_count <= MAX_BLOCK_COUNT)? 0 : current_block_count%MAX_BLOCK_COUNT;
    }


    void write(const unsigned int * const data){
        Flash::write(get_block_address(current_block_count), data, Log_Data<n>::DATA_SIZE);
        update_block_count();
    }


    void write(const unsigned int * const data, const unsigned int block){
        Flash::write(get_block_address(block), data, Log_Data<n>::DATA_SIZE);
    }

    void write(const unsigned int * const data, const unsigned int data_count, const unsigned int block,const unsigned int offset){
        unsigned int address, rewrite_data_size;
        rewrite_data_size = data_count*sizeof(unsigned int);

        if(data_count > Log_Data<n>::DATA_COUNT || data_count + offset >= Log_Data<n>::DATA_COUNT || offset >= Log_Data<n>::DATA_COUNT)
            return;

        address = get_block_address(block) + offset*sizeof(unsigned int);
        Flash::write(address, data, rewrite_data_size);
    }

    const unsigned int read(const unsigned int log_block, unsigned int offset) const{
        unsigned int address = get_block_address(log_block);
        address += offset%Log_Data<n>::DATA_COUNT * sizeof(unsigned int);

        return Flash::read(address);
    }

    void read(const unsigned int log_block, unsigned int * data_array) const{
        Flash::read(get_block_address(log_block), data_array, Log_Data<n>::DATA_SIZE);
    }

    void read(const unsigned int starting_block, const unsigned int block_count, unsigned int *data_array) const{
        unsigned int address = get_block_address(starting_block);
        unsigned int read_already;
        if(starting_block + block_count <= MAX_BLOCK_COUNT){
            Flash::read(address, data_array, Log_Data<n>::DATA_SIZE*block_count);
            return;
        }
        else{
            read_already = MAX_BLOCK_COUNT-starting_block;
            read(starting_block,read_already,data_array);
            read(0,block_count-read_already,data_array+read_already);
        }
    }

    Flash_Logger(){
        assert(MAX_BLOCK_COUNT > 0);
        current_block_count = Flash::read(Flash_Address::CURRENT_BLOCK_COUNT_MEMORY_MAP);
    }

    void reset(){
        current_block_count = -1;
        update_block_count();
    }

protected:
    unsigned int current_block_count;

    void update_block_count(){
        ++current_block_count;
        Flash::write(Flash_Address::CURRENT_BLOCK_COUNT_MEMORY_MAP, &current_block_count, sizeof(unsigned int));
    }

    const unsigned int get_block_address(const unsigned int block) const{
        return (block%MAX_BLOCK_COUNT)*Log_Data<n>::DATA_SIZE + (Flash_Address::FIRST_WRITEABLE_ADDRESS);
    }
};

const unsigned int Flash_Address::START_ADDRESS = Flash::size() / 2;
const unsigned int Flash_Address::LAST_ADDRESS = Flash::size() - 2048;
const unsigned int Flash_Address::CURRENT_BLOCK_COUNT_MEMORY_MAP = Flash_Address::START_ADDRESS;
const unsigned int Flash_Address::FIRST_WRITEABLE_ADDRESS = Flash_Address::CURRENT_BLOCK_COUNT_MEMORY_MAP + sizeof(unsigned int);

template <unsigned int T>
const unsigned int Flash_Logger<T>::MAX_BLOCK_COUNT = (Flash_Address::LAST_ADDRESS - Flash_Address::FIRST_WRITEABLE_ADDRESS)/ T*sizeof(unsigned int);
__END_UTIL

#endif
