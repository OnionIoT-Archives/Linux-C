/*
** OnionClient for linux c++
*/

#include <stdio.h>
#include <iostream>
#include "OnionClient.h"

#define ONION_MFR_KEY "test-manufacturer-key"

using namespace std;

// local functions to handel remote calls
void onStart(char** params) {
    cout << "> ON START" << endl;
}
void onStop(char** params) {
    cout << "> ON STOP" << endl;
}

void onPrint(char** params) {
    cout << "> Printing: " << params[0] << endl;
}

char* printParams[] = {"message1", "message2"};

int main(int argc, char *argv[]){
    cout << "Starting Cpp Test.." << endl;

    // login with device id and device key
    OnionClient *client = new OnionClient("8T5MF3eI");

    // register local functions to web endpoint
    client->declare("start", onStart, 0, 0);
    client->declare("stop", onStop, 0, 0);
    client->declare("print", onPrint, printParams, 2);

    // initilize the connection
    client->begin();

    // Call the loop function in the program's main loop
    while (1) {
        client->loop();
    }

    cout << "Done" << endl;
    return 0;
}

