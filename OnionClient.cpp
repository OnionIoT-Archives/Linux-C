#include "OnionClient.h"
#include "OnionPacket.h"
#include "OnionPayloadData.h"
#include "OnionPayloadPacker.h"
#include "OnionInterface.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char OnionClient::domain[] = "zh.onion.io";
uint16_t OnionClient::port = 2721;

// A paired list of name value pairs to be published when device connects
static char* publishMap[] = {"ipAddr","192.168.137.1","mac","deadbeef"};
static uint16_t publishLength = 2; // This is the number of pairs in the map

OnionClient::OnionClient(char* deviceId, char* deviceKey) {
	this->deviceId = new char[strlen(deviceId) + 1];
	this->deviceId[0] = 0;
	strcpy(this->deviceId, deviceId);

	this->deviceKey = new char[strlen(deviceKey) + 1];
	this->deviceKey[0] = 0;
	strcpy(this->deviceKey, deviceKey);
	
	this->remoteFunctions = new remoteFunction[1];
	this->remoteFunctions[0] = NULL;
	this->totalFunctions = 1;
	this->lastSubscription = NULL;
	totalSubscriptions = 0;
	this->interface = new OnionInterface();
}

void OnionClient::begin() {
	if (connect(deviceId, deviceKey)) {
		subscribe();
	}
}

bool OnionClient::connect(char* id, char* key) {
    if (interface == 0) {
        return false;
    }
	if (!interface->connected()) {
		int result = interface->open(OnionClient::domain, OnionClient::port);

		if (result) {
            OnionPacket* pkt = new OnionPacket(128);
            pkt->setType(ONIONCONNECT);
            OnionPayloadPacker* pack = new OnionPayloadPacker(pkt);
            pack->packArray(3);
            pack->packInt(ONIONPROTOCOLVERSION);
            pack->packStr(id);
            pack->packStr(key);
            interface->send(pkt);
			lastInActivity = lastOutActivity = interface->getMillis();
            delete pack;
            OnionPacket *recv_pkt = interface->getPacket();
			while (recv_pkt == 0) {
				unsigned long t = interface->getMillis();
				if (t - lastInActivity > ONION_KEEPALIVE * 1000UL) {
					interface->close();
					return false;
				}
			    recv_pkt = interface->getPacket();
			}
			uint8_t pkt_type = recv_pkt->getType();
			uint16_t length = recv_pkt->getPayloadLength();
			uint8_t* payload = recv_pkt->getPayload();
			if ((pkt_type == ONIONCONNACK) && (length > 0)) {
			    if (payload[0] == 0) {
    				lastInActivity = interface->getMillis();
    				pingOutstanding = false;
    				delete recv_pkt;
                                printf("Connected to server...\n");
    				return true;
    			}
			}
			delete recv_pkt;
		}
		interface->close();
	}
	return false;
}


char* OnionClient::registerFunction(char * endpoint, remoteFunction function, char** params, uint8_t param_count) {
	remoteFunction* resized = new remoteFunction[totalFunctions + 1];
	if (lastSubscription == NULL) {
	    subscriptions.id = totalFunctions;
	    subscriptions.endpoint = endpoint;
	    subscriptions.params = params;
	    subscriptions.param_count = param_count;
	    lastSubscription = &subscriptions;
	} else {
	    subscription_t* new_sub = (subscription_t*)malloc(sizeof(subscription_t));
	    new_sub->id = totalFunctions;
	    new_sub->endpoint = endpoint;
	    new_sub->params = params;
	    new_sub->param_count = param_count;
	    new_sub->next = NULL;
	    lastSubscription->next = new_sub;
	    lastSubscription = new_sub; 
	}
	totalSubscriptions++;
	
	for (int i = 0; i < totalFunctions; i++) {
		resized[i] = remoteFunctions[i];
	}
	
	// Set the last element of resized as the new function
	resized[totalFunctions] = function;
	
	delete [] remoteFunctions;
	remoteFunctions = resized;
	
	
	
	char* idStr = new char[6];
	idStr[0] = 0;
	sprintf(idStr, "%d", totalFunctions);
	totalFunctions++;

	return idStr;
};


bool OnionClient::publish(char* key, char* value) {
	int key_len = strlen(key);
	int value_len = strlen(value);
	if (interface->connected()) {
        OnionPacket* pkt = new OnionPacket(128);
        pkt->setType(ONIONPUBLISH);
        OnionPayloadPacker* pack = new OnionPayloadPacker(pkt);
        pack->packMap(1);
        pack->packStr(key);
        pack->packStr(value);
        
	    interface->send(pkt);
        //pkt->send();
        delete pack;
        //delete pkt;
	}
	return false;
}

bool OnionClient::publish(char** dataMap, uint8_t count) {
    OnionPacket* pkt = new OnionPacket(128);
    pkt->setType(ONIONPUBLISH);
    OnionPayloadPacker* pack = new OnionPayloadPacker(pkt);
    pack->packMap(count);
    for (uint8_t x=0; x<count; x++) {
        pack->packStr(*dataMap++);
        pack->packStr(*dataMap++);
    }
    
    interface->send(pkt);
    delete pack;
}

bool OnionClient::subscribe() {
	if (interface->connected()) {
	    // Generate 
	    if (totalSubscriptions > 0) {
            OnionPacket* pkt = new OnionPacket(128);
            pkt->setType(ONIONSUBSCRIBE);
            OnionPayloadPacker* pack = new OnionPayloadPacker(pkt);
	        subscription_t *sub_ptr = &subscriptions;
	        pack->packArray(totalSubscriptions);
        	uint8_t string_len = 0;
        	uint8_t param_count = 0;
	        for (uint8_t i=0;i<totalSubscriptions;i++) {
	            param_count = sub_ptr->param_count;
	            pack->packArray(param_count+2);
	            pack->packStr(sub_ptr->endpoint);
	            pack->packInt(sub_ptr->id);
	            for (uint8_t j=0;j<param_count;j++) {
	                pack->packStr(sub_ptr->params[j]);
	            }
	            sub_ptr = sub_ptr->next;
	        }
	        // Send subscribe packet
	        interface->send(pkt);
	        return true;
	        // Since the packet is generated and sent we can delete the packer
	        delete pack;
	    }
	    
	}
	return false;
}

bool OnionClient::loop() {
	if (interface->connected()) {
		unsigned long t = interface->getMillis();
		if ((t - lastInActivity > ONION_KEEPALIVE * 1000UL) || (t - lastOutActivity > ONION_KEEPALIVE * 1000UL)) {
			if (pingOutstanding) {
				interface->close();
				return false;
			} else {
			    sendPingRequest();
				lastOutActivity = t;
				lastInActivity = t;
				pingOutstanding = true;
			}
		}
        OnionPacket* pkt = interface->getPacket();
		if (pkt != 0) {
			lastInActivity = t;
			uint8_t type = pkt->getType();
			if (type == ONIONPUBLISH) {
                //printf("Got publish data...\n");
			    parsePublishData(pkt);
			} else if (type == ONIONPINGREQ) {
			    // Functionize this
                //printf("Got ping req...\n");
				sendPingResponse();
				lastOutActivity = t;
			} else if (type == ONIONPINGRESP) {
                //printf("Got ping resp...\n");
				pingOutstanding = false;
			} else if (type == ONIONSUBACK) {
        	    //printf("Publishing Data\n");
        		publish(publishMap,publishLength);
				lastOutActivity = t;
			}
			delete pkt;
		}
		return true;
	} else {
	    unsigned long t = interface->getMillis();
		if (t - lastOutActivity > ONION_KEEPALIVE * 1000UL) {
		    this->begin();
		}
	}
}


void OnionClient::sendPingRequest(void) {
    OnionPacket* pkt = new OnionPacket(8);
    pkt->setType(ONIONPINGREQ);
    interface->send(pkt);
}

void OnionClient::sendPingResponse(void) {
    OnionPacket* pkt = new OnionPacket(8);
    pkt->setType(ONIONPINGRESP);
    interface->send(pkt);
}

void OnionClient::parsePublishData(OnionPacket* pkt) {
    
    uint16_t length = pkt->getBufferLength();
    uint8_t *ptr = pkt->getBuffer();
    OnionPayloadData* data = new OnionPayloadData(pkt);
    
    data->unpack();
    uint8_t count = data->getLength();
    uint8_t function_id = data->getItem(0)->getInt();
	OnionParams* params = new OnionParams(count-1);
    
	if (count > 1) {
	    // Get parameters
	    for (uint8_t i=0;i<(count-1);i++) {
	        OnionPayloadData* item = data->getItem(i+1);
	        uint8_t strLen = item->getLength();
	        // Test
	        char* buf_ptr = (char *)(item->getBuffer());
	        params->setStr(i,buf_ptr,strLen);
	    }
	}
	if (function_id < totalFunctions) {
	    if (remoteFunctions[function_id] != 0) {
	        remoteFunctions[function_id](params);
	    } else {
	        // if the remote function isn't called
	        // no one will delete params, so we have to
	        delete params;
	    }
	} else {
	    // We need to delete this here since no one else can
	    delete params;
	}
	delete data;
}
