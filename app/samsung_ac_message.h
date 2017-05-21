#ifndef message_h__
#define message_h__

#include <machine.h>
#include <nic.h>
#include <secure_nic.h>
#include <utility/key_database.h>

typedef struct {
    unsigned char command; //0 turn on, 1 turn off, 2 command
    unsigned char temp;
    unsigned char mode;
    unsigned char fan;
    unsigned char turbo;
    unsigned char swing;
} Message;

#endif
