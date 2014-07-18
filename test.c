/*
** OnionClient for linux c++
*/

#include <stdio.h>
#include "OnionClient.h"

void test(char** params){
    printf("on test: %s\n", params[0]);
}

void onStart(char** params) {
    printf("->Start\n");
}

void onStop(char** params) {
    printf("->Stop\n");
}

void onUpdate(char** params) {
    printf("->Update: %s\n", params[0]);
    onion_publish("/updateValue", params[0]);
}

char* testParams[] = {"status"};
int main(int argc, char *argv[]) {
    printf("Starting C Test..\n");
    int i;
    // Setup onion client data:
    // Device Id, device_type, firmware_version and hardware_version
    onion_init("8T5MF3eI","linux-c","1.0","0.1");
    onion_declare("start", onStart, 0, 0);
    onion_declare("stop", onStop, 0, 0);
    onion_declare("update", onUpdate, testParams, 1);
    onion_declare("test", onUpdate, testParams, 1);

    onion_begin();

    while (1) {
	   onion_loop();
    }

    printf("Done\r\n");

    return 0;
}
