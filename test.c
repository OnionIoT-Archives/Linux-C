/*
** OnionClient for linux c
*/

#include <stdio.h>
#include "OnionClient.h"

void test(char* params){
    printf("on test: %s\n", params);
}

int main(int argc, char *argv[]){
    printf("Starting..\n");
    int i;
    onion_connect("pbW078TG", "NgslJgIxOTUjBekI");
    onion_get("/test", test);
    onion_get("/test2", test);
    onion_post("/test3", test,"cmd,cmd2");
    while (1){
        onion_loop();
    }
    printf("Done\r\n");
    return 0;
}
