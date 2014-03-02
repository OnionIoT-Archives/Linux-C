#ifndef ONION_CLIENT_H
#define ONION_CLIENT_H

//#include "OnionParams.h"

// MQTT_MAX_PACKET_SIZE : Maximum packet size
#define MQTT_MAX_PACKET_SIZE    128
#define MAXDATASIZE 128 

// MQTT_KEEPALIVE : keepAlive interval in Seconds
#define MQTT_KEEPALIVE                  15

#define MQTTPROTOCOLVERSION     3
#define MQTTCONNECT                     1 << 4  // Client request to connect to Server
#define MQTTCONNACK                     2 << 4  // Connect Acknowledgment
#define MQTTPUBLISH                     3 << 4  // Publish message
#define MQTTPUBACK                      4 << 4  // Publish Acknowledgment
#define MQTTPUBREC                      5 << 4  // Publish Received (assured delivery part 1)
#define MQTTPUBREL                      6 << 4  // Publish Release (assured delivery part 2)
#define MQTTPUBCOMP                     7 << 4  // Publish Complete (assured delivery part 3)
#define MQTTSUBSCRIBE                   8 << 4  // Client Subscribe request
#define MQTTSUBACK                      9 << 4  // Subscribe Acknowledgment
#define MQTTUNSUBSCRIBE                 10 << 4 // Client Unsubscribe request
#define MQTTUNSUBACK                    11 << 4 // Unsubscribe Acknowledgment
#define MQTTPINGREQ                     12 << 4 // PING Request
#define MQTTPINGRESP                    13 << 4 // PING Response
#define MQTTDISCONNECT                  14 << 4 // Client is Disconnecting
#define MQTTReserved                    15 << 4 // Reserved

#define MQTTQOS0                        (0 << 1)
#define MQTTQOS1                        (1 << 1)
#define MQTTQOS2                        (2 << 1)

// Static data for connecting to Onion
#define MQTTSERVER                      "mqtt.onion.io" // Onion MQTT server domain
#define MQTTPORT                        1883 // Onion MQTT port 

#define CLIENT_VERSION "v1c"

typedef void(*remoteFunction)(char*);

/*MQTT support functions*/
int readByte(int  *index);
int readPacket(int* lengthLength);


/*Onion client function*/
void callback(char*, char*, unsigned int);
void onion_connect(char*, char*);
int onion_loop();

//OnionClient(char*, char*);
//void begin();
//void get(char*, remoteFunction);
//void post(char*, remoteFunction, char*);
//void update(char*, float);
//boolean loop();
//
//char* registerFunction(remoteFunction);
//boolean connect(char*, char*, char*);
//boolean connected();
//boolean publish(char*, char*);
//boolean subscribe(char*);
//uint16_t readPacket(uint8_t *);
//uint8_t readByte();
//boolean write(uint8_t, uint8_t*, uint16_t);
//uint16_t writeString(char*, uint8_t*, uint16_t);
//
//uint8_t buffer[MQTT_MAX_PACKET_SIZE];
//uint16_t nextMsgId;
//unsigned long lastOutActivity;
//unsigned long lastInActivity;
//bool pingOutstanding;
//

//
//// Array of functions registered as remote functions and length
//remoteFunction* remoteFunctions;
//unsigned int totalFunctions;
////Client* _client;
//char* deviceId;
//char* deviceKey;

#endif

