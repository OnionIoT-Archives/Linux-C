/*
** OnionClient for linux c++
*/

#include <stdio.h>
#include "OnionClient.h"

void test(char* params){
    printf("on test: %s\n", params);
}

int main(int argc, char *argv[]){
    printf("Starting..\n");
    int i;
    OnionClient *client = new OnionClient("PrXsXHFl","Nhw08qHw6kAdYzwo");
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
