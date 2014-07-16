#ifndef ONION_CLIENT_H
#define ONION_CLIENT_H

#ifdef  __cplusplus
#include "OnionParams.h"
#include "OnionInterface.h"
#endif
#include <stdbool.h>
#include <stdint.h>

// ONION_MAX_PACKET_SIZE : Maximum packet size
#define ONION_MAX_PACKET_SIZE 	1420

// ONION CONNECT HEADER LENGTH
#define ONION_HEADER_CONNECT_LENGTH 6

// ONION_KEEPALIVE : keepAlive interval in Seconds
#define ONION_KEEPALIVE 			15
#define ONION_RETRY                 1

#define ONION_PROTOCOL_VERSION 	1
#define ONION_CONNECT     		1 << 4  // Client request to connect to Server
#define ONION_CONNACK     		2 << 4  // Connect Acknowledgment
#define ONION_PUBLISH     		3 << 4  // Publish message
#define ONION_PUBACK      		4 << 4  // Publish Acknowledgment
#define ONION_PUBREC      		5 << 4  // Publish Received (assured delivery part 1)
#define ONION_PUBREL      		6 << 4  // Publish Release (assured delivery part 2)
#define ONION_PUBCOMP     		7 << 4  // Publish Complete (assured delivery part 3)
#define ONION_SUBSCRIBE   		8 << 4  // Client Subscribe request
#define ONION_SUBACK      		9 << 4  // Subscribe Acknowledgment
#define ONION_UNSUBSCRIBE 		10 << 4 // Client Unsubscribe request
#define ONION_UNSUBACK    		11 << 4 // Unsubscribe Acknowledgment
#define ONION_PINGREQ     		12 << 4 // PING Request
#define ONION_PINGRESP    		13 << 4 // PING Response
#define ONION_DISCONNECT  		14 << 4 // Client is Disconnecting
#define ONION_RESERVED    		15 << 4 // Reserved

#define ONION_QOS0        		(0 << 1)
#define ONION_QOS1        		(1 << 1)
#define ONION_QOS2        		(2 << 1)

#define ONION_MFR_KEY_LOCATION "./MFR_KEY"
#define ONION_DEVICE_KEY_LOCATION "./DEVICE_KEY"

typedef void(*remoteFunction)(char**);

#ifdef __cplusplus
typedef struct subscription_t {
    uint8_t index;
    char* id;
    char** params;
    uint8_t param_count;
    subscription_t* next;
} subscription_t;
#endif

#ifdef __cplusplus
class OnionClient {

public:
	OnionClient(char*);
    ~OnionClient();
	void begin();
	uint16_t declare(char*, remoteFunction, char**, uint8_t);
    void declare(char*, char*);
    void update(char*, float);
    bool publish(char**, uint8_t);
	bool publish(char*, char*);
	bool publish(char*, int);
	bool publish(char*, bool);
	bool publish(char*, double);
	bool loop();

protected:
	void callback(uint8_t*, uint8_t*, unsigned int);
	void parsePublishData(OnionPacket*);
	void sendPingRequest(void);
	void sendPingResponse(void);
	bool connect(char*, char*);
    bool connect(char*);
	bool connected();
	bool subscribe();
    void logError(uint8_t);
	uint16_t readPacket();
	uint8_t readByte();
	
	uint8_t buffer[ONION_MAX_PACKET_SIZE];
	uint16_t nextMsgId;
	unsigned long lastOutActivity;
	unsigned long lastInActivity;
	bool pingOutstanding;

	// Static data for connecting to Onion
	static char domain[];
	static uint16_t port;
	static const uint8_t connectHeader[ONION_HEADER_CONNECT_LENGTH];

	// Array of functions registered as remote functions and length
	remoteFunction* remoteFunctions;
	subscription_t subscriptions;
	subscription_t* lastSubscription;
	uint8_t totalSubscriptions;
	unsigned int totalFunctions;
	OnionInterface* interface;

	char* deviceId;
	char* deviceKey;
}; 

extern "C" {
#else
typedef struct OnionClient OnionClient;
#endif // End of CPP Section


void onion_init(char*);
void onion_begin();
uint16_t onion_declare(char*, remoteFunction, char**, uint8_t);
bool onion_publish(char*, char*);
bool onion_publish_map(char**, uint8_t);
void onion_loop(void);


#ifdef __cplusplus
} // close the extern C
#endif

#endif // End of HEADER

