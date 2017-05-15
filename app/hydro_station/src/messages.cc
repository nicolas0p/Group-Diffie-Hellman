#include "../include/messages.h"



void MessagesHandler::setData(const unsigned int d[], unsigned int length){    
    for(unsigned int i = 0; i<length; i++){
        data[i] = d[i];
        dataLength = i+1;
        if(i==(DATA_MAX_LENGTH-1)) break;
    }
}

void MessagesHandler::build(char msg[]){
    char aux[32] = "";

    strcpy(msg,"data=");
    strcat(msg,"hydro_joi_1,");
    for(unsigned int i = 0; i<dataLength; i++){
        aux[utoa(data[i],(char *)aux)] = '\0';
        strcat(msg,(char *)aux);
        if(i<dataLength-1) strcat(msg, ",");
    }
}