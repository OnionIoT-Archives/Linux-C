#include <stdio.h> /* printf, scanf, NULL */
#include <stdlib.h> /* malloc, free, rand */
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "OnionClient.h"

#define bool int
#define true 1
#define false 0

char buffer[MQTT_MAX_PACKET_SIZE];
int bufferLen;

uint16_t nextMsgId;
unsigned long lastOutActivity;
unsigned long lastInActivity;
bool pingOutstanding;

int sock;  

unsigned long millis(){
    return 0;
}

bool connected(){
    return true;
}

int socket_connect(){
    int sd, rc;
    struct sockaddr_in localAddr, servAddr;
    struct hostent *host;

    host = gethostbyname(MQTTSERVER);
    if(host==NULL) {
        printf("unknown host '%s'\n",MQTTSERVER);
        return 1;
    }

    servAddr.sin_family = host->h_addrtype;
    memcpy((char *) &servAddr.sin_addr.s_addr, host->h_addr_list[0], host->h_length);
    servAddr.sin_port = htons(MQTTPORT);

    /* create socket */
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if(sd<0) {
        perror("cannot open socket ");
        return 1;
    }

    /* bind any port number */
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddr.sin_port = htons(0);

    rc = bind(sd, (struct sockaddr *) &localAddr, sizeof(localAddr));
    if(rc<0) {
        printf("%s: cannot bind port TCP %u\n",MQTTSERVER,MQTTPORT);
        perror("error ");
        return 1;
    }

    /* connect to server */
    rc = connect(sd, (struct sockaddr *) &servAddr, sizeof(servAddr));
    if(rc<0) {
        perror("cannot connect ");
        return 1;
    }
    sock = sd;

    return 0;
}

int readByte(int  *index) {
    if((*index)<bufferLen)
        return buffer[(*index)++];
}

int readPacket(int* lengthLength) {
    int i;
    int index = 0;
    uint16_t len = 0;
    buffer[len++] = readByte(&index);
    uint8_t multiplier = 1;
    uint16_t length = 0;
    uint8_t digit = 0;
    do {
        digit = readByte(&index);
        buffer[len++] = digit;
        length += (digit & 127) * multiplier;
        multiplier *= 128;
    } while ((digit & 128) != 0);
    *lengthLength = len - 1;
    for (i = 0; i < length; i++) {
        if (len < MQTT_MAX_PACKET_SIZE) {
            buffer[len++] = readByte(&index);
        } else {
            readByte(&index);
            len = 0; // This will cause the packet to be ignored.
        }
    }

    return len;
}

writeString(char* string, uint8_t* buf, uint16_t pos) {
    char* idp = string;
    uint16_t i = 0;
    pos += 2;
    while (*idp) {
        buf[pos++] = *idp++;
        i++;
    }
    buf[pos - i - 2] = (i >> 8);
    buf[pos - i - 1] = (i & 0xFF);
    return pos;
}

write(uint8_t header, uint8_t* buf, uint16_t length) {
    uint8_t lenBuf[4];
    uint8_t llen = 0;
    uint8_t digit;
    uint8_t pos = 0;
    uint8_t rc;
    uint8_t len = length;
    int i;
    do {
        digit = len % 128;
        len = len / 128;
        if (len > 0) {
            digit |= 0x80;
        }
        lenBuf[pos++] = digit;
        llen++;
    } while(len > 0);

    buf[4-llen] = header;
    for (i = 0; i < llen; i++) {
        buf[5-llen+i] = lenBuf[i];
    }
    rc = send(sock, buf + (4 - llen), length + 1 + llen, 0);

    lastOutActivity = millis();
    return (rc == 1 + llen + length);
}

int mqtt_connect(char* id, char* user, char* pass) {
    if (socket_connect()==0) {
        nextMsgId = 1;
        uint8_t d[9] = { 0x00, 0x06, 'M', 'Q', 'I', 's', 'd', 'p', MQTTPROTOCOLVERSION };
        // Leave room in the buffer for header and variable length field
        uint16_t length = 5;
        unsigned int j;
        for (j = 0; j < 9; j++) {
            buffer[length++] = d[j];
        }

        // No WillMsg
        uint8_t v = 0x02;

        // User
        v = v | 0x80;

        // Password
        v = v | (0x80 >> 1);

        buffer[length++] = v;

        buffer[length++] = ((MQTT_KEEPALIVE) >> 8);
        buffer[length++] = ((MQTT_KEEPALIVE) & 0xFF);

        // Writing id, user and pass
        length = writeString(id, buffer, length);
        length = writeString(user, buffer, length);
        length = writeString(pass, buffer, length);

        write(MQTTCONNECT, buffer, length - 5);

        //lastInActivity = lastOutActivity = millis();

        //while (!_client->available()) {
        //    unsigned long t = millis();
        //    if (t - lastInActivity > MQTT_KEEPALIVE * 1000UL) {
        //        _client->stop();
        //        return false;
        //    }
        //}
        uint8_t llen;
        //uint16_t len = readPacket(&llen);

        //if (len == 4 && buffer[3] == 0) {
        //    //lastInActivity = millis();
        //    pingOutstanding = false;
        //    return true;
        //}
    }
    return false;
}
int mqtt_publish(char* topic, char* payload){

    int plength = strlen(payload);
    if (connected()) {
        // Leave room in the buffer for header and variable length field
        uint16_t length = 5;
        length = writeString(topic, buffer, length);
        uint16_t i;
        for (i = 0; i < plength; i++) {
            buffer[length++] = payload[i];
        }
        uint8_t header = MQTTPUBLISH;

        return write(header, buffer, length - 5);
    }
    return false;
}

int mqtt_subscribe(char* topic){
    if (connected()) {
        // Leave room in the buffer for header and variable length field
        uint16_t length = 5;
        nextMsgId++;
        if (nextMsgId == 0) {
            nextMsgId = 1;
        }
        buffer[length++] = (nextMsgId >> 8);
        buffer[length++] = (nextMsgId & 0xFF);
        length = writeString(topic, buffer, length);
        buffer[length++] = 0; // Only do QoS 0 subs
        return write(MQTTSUBSCRIBE | MQTTQOS1, buffer, length - 5);
    }
    return false;
}




/* Onion Client Functions */

void onion_connect(char* deviceId, char* deviceKey){
    char topic[MQTT_MAX_PACKET_SIZE];
    char cmd[MQTT_MAX_PACKET_SIZE];

    topic[0] = 0;
    strcat(topic, "/");
    strcat(topic, deviceId);

    cmd[0] = 0;
    strcat(cmd, deviceId);
    strcat(cmd, ";CONNECTED");

    if (mqtt_connect(deviceId, deviceId, deviceKey) == 0) {
        mqtt_publish("/register", cmd);
        mqtt_subscribe(topic);
    }
}

int onion_loop(){ 
    int i, len;
    bufferLen = recv(sock, buffer, MAXDATASIZE-1, MSG_DONTWAIT); 
    buffer[bufferLen] = '\0';

    if (connected()) {
        /* Update heart beat */
        unsigned long t = millis();
        //if ((t - lastInActivity > MQTT_KEEPALIVE * 1000UL) || (t - lastOutActivity > MQTT_KEEPALIVE * 1000UL)) {
        //    if (pingOutstanding) {
        //        //_client->stop();
        //        return false;
        //    } else {
        //        buffer[0] = MQTTPINGREQ;
        //        buffer[1] = 0;
        //        send(sock, buffer, 2, 0);
        //        lastOutActivity = t;
        //        lastInActivity = t;
        //        pingOutstanding = true;
        //    }
        //}

        if (bufferLen > 0) {
            lastInActivity = t;

            int llen = 0;
            int len = readPacket(&llen);

            uint8_t type = buffer[0] & 0xF0;
            if (type == MQTTPUBLISH) {
                uint16_t tl = (buffer[llen + 1] << 8) + buffer[llen + 2];
                char topic[tl + 1];
                for (i=0; i < tl; i++) {
                    topic[i] = buffer[llen + 3 + i];
                }
                topic[tl] = 0;
                // ignore msgID - only support QoS 0 subs
                uint8_t *payload = buffer + llen + 3 + tl;
                callback(topic, payload, len - llen - 3 - tl);
            } else if (type == MQTTPINGREQ) {
                buffer[0] = MQTTPINGRESP;
                buffer[1] = 0;
                send(sock, buffer, 2, 0);
            } else if (type == MQTTPINGRESP) {
                pingOutstanding = false;
            }
        }
        return true;
    } else {
        //this->connect(deviceId, deviceId, deviceKey);
        //TODO reconnect
    }
}

void callback(char* topic, char* payload, unsigned int length) {
    printf("callback\n");
    printf("topic: %s\n", topic);
    printf("payload: %s\n", payload);
    //// Get the function ID
    //char idStr[6] = "";
    //OnionParams* params = NULL;
    //bool hasParams = false;
    //                
    //int i = 0;
    //for(i; i < length && i < 5; i++) {
    //        idStr[i] = (char)payload[i];
    //        if(i < length - 1 && (int)payload[i + 1] == 59) {
    //                hasParams = true;
    //                break;
    //        }
    //}
    //// Add NULL to end of string
    //idStr[++i] = 0;
    //unsigned int functionId = atoi(idStr);
    //
    //if(hasParams) {
    //        // skip the first ';'
    //        int offset = ++i;
    //        char* rawParams = new char[length - offset + 1];
    //        rawParams[0] = 0;

    //        // Load elements into raw params
    //        for(i; i < length; i++) {
    //                rawParams[i - offset] = (char)payload[i];
    //        }
    //        rawParams[length - offset] = 0;
    //        
    //        params = new OnionParams(rawParams);
    //        delete[] rawParams;
    //}
    //
    //// Call remote function
    //if(functionId && functionId < totalFunctions) {
    //        remoteFunctions[functionId](params);
    //}

    //delete params;
}
