/*
*   THIS CLASS CONTAINS METHODS AND ATTRIBUTES TO BUILD THE MESSAGE TO BE SENT OVER THE NETWORK
*   
*
* --> it stores an array with the data to be sent
* --> it builds the message based on the stored data

----------------METHODS
*
* --> build(...): receives a string and build the message to be sent on it
* --> getLength(): receives nothing but returns the length of the stored data array
* --> *getData(): returns the stored data
*/

#ifndef MESSAGES_H_
#define MESSAGES_H_

#include <utility/ostream.h>
#include <machine.h>

using namespace EPOS;

class MessagesHandler{
    public:
        MessagesHandler(){
            seq = 0;
            dataLength = 0;
        }

        void build(char msg[]);

        unsigned int getLength(){return dataLength;};

        void setData(const unsigned int d[], unsigned int length);

        unsigned int *getData(){return data;};
        
        unsigned int seq;
    private:
        static const unsigned int DATA_MAX_LENGTH = 20;

        unsigned int data[DATA_MAX_LENGTH];
        unsigned int dataLength;
};

#endif
