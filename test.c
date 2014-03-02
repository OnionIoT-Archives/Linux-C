/*
** OnionClient for linux c
*/

#include <stdio.h>
#include "OnionClient.h"

int main(int argc, char *argv[]){
    int i;
    onion_connect("pbW078TG", "NgslJgIxOTUjBekI");
    while (1){
        onion_loop();
    }
    printf("Done\r\n");
    return 0;
}
