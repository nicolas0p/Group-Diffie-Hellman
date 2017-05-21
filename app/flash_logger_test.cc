#include <utility/flash_logger.h>
#include <utility/ostream.h>
#include <timer.h>

/**
* This class is properly documented at http://lisha.ufsc.br/Flash+Logger+Module
* The last update occured on 06/05/2016.
* The last change in this file is present at the svn version 3940.
* If any doubts are still present, email asantana@lisha.ufsc.br
**/

using namespace EPOS;

#define TESTED_DATA_COUNT 2
#define TESTED_DATA_AMMOUNT 3

OStream cout;

Flash_Logger<TESTED_DATA_COUNT> flash_logger;
unsigned int block_data[TESTED_DATA_COUNT];
unsigned int all_blocks_data[TESTED_DATA_COUNT*TESTED_DATA_AMMOUNT];

const bool test_simple_hydrology_logging();
const bool test_simple_hydrology_logging_with_block_read();
const bool test_rewrite_block();
const bool test_rewrite_block_in_offset();
const bool test_read_multiple_blocks();

void generate_data();
void generate_data_all_blocks();

void set_up(){
    flash_logger.reset();

    generate_data();

    for(int i = 0; i < TESTED_DATA_COUNT; ++i){
        for(int u = 0; u < TESTED_DATA_AMMOUNT; ++u){
            all_blocks_data[i*TESTED_DATA_COUNT+u] = 0;
        }
    }
}

void generate_data(){
    static unsigned int val;
    for(int i = 0; i < TESTED_DATA_COUNT; ++i){
        block_data[i] = val++;
    }
}

void generate_data_all_blocks(){
    for(int i = 0; i < TESTED_DATA_COUNT*TESTED_DATA_AMMOUNT; ++i)
        all_blocks_data[i] = i;
}


void test_case(const bool pass, const char * const test_name){
    static unsigned int case_count;
    const static char *suc = " succeded.";
    const static char *fail = " failed.";

    cout << "Test " << case_count << ": " << test_name;
    (pass)? cout << suc : cout << fail;
    cout << endl;

    ++case_count;
}

int main(){
    User_Timer_0::delay(6000000);

    cout << "Max block count: " << flash_logger.MAX_BLOCK_COUNT << endl;
    // Simple Hydrology logging function
    test_case(test_simple_hydrology_logging(), "Read/Write");
    test_case(test_simple_hydrology_logging_with_block_read(), "Block read");

    // Other Flavour Methods Testings
    test_case(test_rewrite_block(), "Rewrite Block");
    test_case(test_rewrite_block_in_offset(), "Rewrite Block in offset");
    test_case(test_read_multiple_blocks(), "Read multiple Blocks");

    while(true);
    return 0;
}

const bool test_simple_hydrology_logging(){
    set_up();
    bool is_equals= false;
    for(int i = 0; i < TESTED_DATA_AMMOUNT; ++i){
        flash_logger.write(block_data);
        for(int u = 0; u < TESTED_DATA_COUNT; ++u){
            is_equals = block_data[u] == flash_logger.read(i, u);
            if(!is_equals)
                return false;
        }
        generate_data();
    }

    return true;
}

const bool test_simple_hydrology_logging_with_block_read(){
    set_up();
    unsigned int read_block[TESTED_DATA_COUNT];

    bool is_equals = false;

    for(int i = 0; i < TESTED_DATA_AMMOUNT; ++i){
        flash_logger.write(block_data);

        flash_logger.read(i, read_block);

        for(int u = 0; u < TESTED_DATA_COUNT; ++u){
            is_equals = block_data[u] == read_block[u];
            if(!is_equals)
                return false;
        }
        generate_data();
    }

    return true;
}

const bool test_rewrite_block(){
    set_up();
    unsigned int read_block[TESTED_DATA_COUNT];
    bool is_equals = false;

    for(int i = 0; i < TESTED_DATA_AMMOUNT; ++i){
        flash_logger.write(block_data);
        generate_data();
    }

    flash_logger.write(block_data,TESTED_DATA_AMMOUNT-1);
    flash_logger.read(TESTED_DATA_AMMOUNT-1,read_block);

    for(int u = 0; u < TESTED_DATA_COUNT; ++u){
        is_equals = block_data[u] == read_block[u];
        if(!is_equals)
            return false;
    }

    return true;
}

const bool test_rewrite_block_in_offset(){
    set_up();
    unsigned int read_block[TESTED_DATA_COUNT];
    bool is_equals = false;

    for(int i = 0; i < TESTED_DATA_AMMOUNT; ++i){
        flash_logger.write(block_data);
        generate_data();
    }

    flash_logger.write(block_data, TESTED_DATA_COUNT-1, TESTED_DATA_AMMOUNT-1, 0);
    flash_logger.read(TESTED_DATA_AMMOUNT-1,read_block);

    for(int u = 0; u < TESTED_DATA_COUNT-1; ++u){
        is_equals = block_data[u] == read_block[u];
        if(!is_equals)
            return false;
    }

    return true;
}

const bool test_read_multiple_blocks(){
    set_up();
    unsigned int all_blocks_cache[TESTED_DATA_COUNT*TESTED_DATA_AMMOUNT];

    for(int i = 0; i < TESTED_DATA_AMMOUNT; ++i){
        flash_logger.write(block_data);
        for(int u = 0; u < TESTED_DATA_COUNT; ++u)
            all_blocks_cache[i*TESTED_DATA_COUNT+u] = block_data[u];
        generate_data();
    }

    flash_logger.read(0,TESTED_DATA_AMMOUNT,all_blocks_data);

    for(int i = 0; i < TESTED_DATA_COUNT*TESTED_DATA_AMMOUNT; ++i)
        if(all_blocks_cache[i] != all_blocks_data[i])
            return false;
    return true;
}
