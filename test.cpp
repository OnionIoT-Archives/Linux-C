/*
** OnionClient for linux c++
*/

#include <stdio.h>
#include "OnionClient.h"

// local functions to handel remote calls
void onStart(char** params) {
    printf("> ON START\n");
}
void onStop(char** params) {
    printf("> ON STOP\n");
}

void onPrint(char** params) {
    printf("> Printing:%s\n",params[0]);
}
char* printParams[] = {"message"};
int main(int argc, char *argv[]){
    printf("Starting Cpp Test..\n");

    // login with device id and device key
    OnionClient *client = new OnionClient("8T5MF3eI","Niiw52IZVlLhkZHc");

    // register local functions to web endpoint
    client->registerFunction("/start",onStart,0,0);
    client->registerFunction("/stop",onStop,0,0);
    client->registerFunction("/print",onPrint,printParams,1);
    // initilize the connection
    client->begin();

    // Call the loop function in the program's main loop
    while (1){
        client->loop();
    }

    printf("Done\r\n");
    return 0;
}

