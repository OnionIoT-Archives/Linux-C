/*
** OnionClient for linux c++
*/

#include <stdio.h>
#include "OnionClient.h"

void test(char* params){
    printf("on test: %s\n", params);
}

void ledOn(OnionParams* params) {
    printf("LED->On!\n");
}

void ledOff(OnionParams* params) {
    printf("LED->Off!\n");
}

void print(OnionParams* params) {
    char* param1;
    char* param2;
    char* param3;
    param1 = params->getChar(0);
    param2 = params->getChar(1);
    param3 = params->getChar(2);
    printf("Print-> %s, %s, %s\n",param1,param2,param3);
}

char *printArgs[] = {"first","second","third"};
int main(int argc, char *argv[]){
    printf("Starting..\n");
    int i;
//    OnionClient *client = new OnionClient("PrXsXHFl","Nhw08qHw6kAdYzwo");
    OnionClient *client = new OnionClient("PrXsXHFl","Nhw08qHw6kAdYzwo");
    client->registerFunction("/on",ledOn,0,0);
    client->registerFunction("/off",ledOff,0,0);
    client->registerFunction("/print",print,printArgs,3);
    client->begin();
//    onion_connect("pbW078TG", "NgslJgIxOTUjBekI");
//    onion_get("/test", test);
//    onion_get("/test2", test);
//    onion_post("/test3", test,"cmd,cmd2");
    while (1){
        client->loop();
    }
    printf("Done\r\n");
    return 0;
}

