/*
** OnionClient for linux c++
*/

#include <stdio.h>
#include "OnionClient.h"

char* test(char** params){
    printf("on test: %s\n", params[0]);

    return "return test";
}

char* onStart(char** params) {
    printf("->Start\n");

    return "return OnStart";
}

char* onStop(char** params) {
    printf("->Stop\n");

    return "return onStop";
}

char* onUpdate(char** params) {
    printf("Update function: \n");
    printf("-> param 1: %s\n", params[0]);
    printf("-> param 2: %s\n", params[1]);

    return "return onUpdate";
}

char* testParams[] = {"status1", "status2"};

int main(int argc, char *argv[]) {
    printf("Starting C Test..\n");
    int i;
    // Setup onion client data:
    // Device Id, device_type, firmware_version and hardware_version
    onion_init("00C0CA75A715", "linux-c", "1.0", "0.1");
    onion_declare("start", onStart, 0, 0);
    onion_declare("stop", onStop, 0, 0);
    onion_declare("update", onUpdate, testParams, 2);
    onion_declare("test", onUpdate, testParams, 2);

    onion_begin();

    while (1) {
	   onion_loop();
    }

    printf("Done\r\n");

    return 0;
}
