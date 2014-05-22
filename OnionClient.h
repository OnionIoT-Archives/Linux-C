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

#define ONIONPROTOCOLVERSION 	1
#define ONIONCONNECT     		1 << 4  // Client request to connect to Server
#define ONIONCONNACK     		2 << 4  // Connect Acknowledgment
#define ONIONPUBLISH     		3 << 4  // Publish message
#define ONIONPUBACK      		4 << 4  // Publish Acknowledgment
#define ONIONPUBREC      		5 << 4  // Publish Received (assured delivery part 1)
#define ONIONPUBREL      		6 << 4  // Publish Release (assured delivery part 2)
#define ONIONPUBCOMP     		7 << 4  // Publish Complete (assured delivery part 3)
#define ONIONSUBSCRIBE   		8 << 4  // Client Subscribe request
#define ONIONSUBACK      		9 << 4  // Subscribe Acknowledgment
#define ONIONUNSUBSCRIBE 		10 << 4 // Client Unsubscribe request
#define ONIONUNSUBACK    		11 << 4 // Unsubscribe Acknowledgment
#define ONIONPINGREQ     		12 << 4 // PING Request
#define ONIONPINGRESP    		13 << 4 // PING Response
#define ONIONDISCONNECT  		14 << 4 // Client is Disconnecting
#define ONIONReserved    		15 << 4 // Reserved

#define ONIONQOS0        		(0 << 1)
#define ONIONQOS1        		(1 << 1)
#define ONIONQOS2        		(2 << 1)

typedef void(*remoteFunction)(char**);
#ifdef __cplusplus
typedef struct subscription_t {
    uint8_t id;
    char* endpoint;
    char** params;
    uint8_t param_count;
    subscription_t* next;
} subscription_t;
#endif

#ifdef __cplusplus
class OnionClient {

public:
	OnionClient(char*, char*);
	void begin();
	char* registerFunction(char*, remoteFunction, char** params, uint8_t param_count);
    void update(char*, float);
    bool publish(char** dataMap, uint8_t count);
	bool publish(char*, char*);
	bool publish(char*, int);
	bool publish(char*, bool);
	bool publish(char*, double);
	bool loop();

protected:
	void callback(uint8_t*, uint8_t*, unsigned int);
	void parsePublishData(OnionPacket* pkt);
	void sendPingRequest(void);
	void sendPingResponse(void);
	bool connect(char*, char*);
	bool connected();
	bool subscribe();
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


void onion_init (char* deviceId, char* deviceKey);
void onion_begin();
void onion_register (char *endpoint, remoteFunction function, char **params, uint8_t param_count);
bool onion_publish(char* key, char* value);
bool onion_publish_map(char** dataMap, uint8_t count);
void onion_periodic(void);


#ifdef __cplusplus
} // close the extern C
#endif

#endif // End of HEADER

