/*
** OnionClient for linux c++
*/

#include <stdio.h>
#include "OnionClient.h"

// local functions to handel remote calls
void onStart(OnionParams* params) {
    printf("> ON START\n");
}
void onStop(OnionParams* params) {
    printf("> ON STOP\n");
}

int main(int argc, char *argv[]){
    printf("Starting..\n");

    // login with device id and device key
    OnionClient *client = new OnionClient("pbW078TG","NgslJgIxOTUjBekI");

    // register local functions to web endpoint
    client->registerFunction("/start",onStart,0,0);
    client->registerFunction("/stop",onStop,0,0);
    // initilize the connection
    client->begin();

    // Call the loop function in the program's main loop
    while (1){
        client->loop();
    }

    printf("Done\r\n");
    return 0;
}

