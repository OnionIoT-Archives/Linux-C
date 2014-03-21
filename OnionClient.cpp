#include "OnionClient.h"
#include "OnionPacket.h"
#include "OnionPayloadData.h"
#include "OnionPayloadPacker.h"
#include "OnionInterface.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char OnionClient::domain[] = "zh.onion.io";
//char OnionClient::domain[] = "pataelmo.com";
uint16_t OnionClient::port = 2721;
   
static char* publishMap[] = {"ipAddr","192.168.137.1","mac","deadbeef"};
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
    //Serial.begin(115200);
	//Serial.print("Start Connection\n");
	if (connect(deviceId, deviceKey)) {
	    //Serial.print("Sending Subscription Requests\n");
		subscribe();
	}
}

bool OnionClient::connect(char* id, char* key) {
    if (interface == 0) {
        //Serial.print("Tried to connect with no interface!");
        return false;
        //interface = new OnionInterface();
    }
	if (!interface->connected()) {
		int result = interface->open(OnionClient::domain, OnionClient::port);

		if (result) {
			//nextMsgId = 1;
            OnionPacket* pkt = new OnionPacket(128);
            pkt->setType(ONIONCONNECT);
            OnionPayloadPacker* pack = new OnionPayloadPacker(pkt);
            pack->packArray(3);
            pack->packInt(ONIONPROTOCOLVERSION);
            pack->packStr(id);
            pack->packStr(key);
            //pkt->send();
            interface->send(pkt);
			lastInActivity = lastOutActivity = interface->getMillis();
            //delete pkt;
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



//void OnionClient::callback(uint8_t* topic, byte* payload, unsigned int length) {
//	// Get the function ID
//	char idStr[6] = "";
//	OnionParams* params = NULL;
//	bool hasParams = false;
//			
//	int i = 0;
//	for(i; i < length && i < 5; i++) {
//		idStr[i] = (char)payload[i];
//		if(i < length - 1 && (int)payload[i + 1] == 59) {
//			hasParams = true;
//			break;
//		}
//	}
//	// Add NULL to end of string
//	idStr[++i] = 0;
//	unsigned int functionId = atoi(idStr);
//	
//	if(hasParams) {
//		// skip the first ';'
//		int offset = ++i;
//		char* rawParams = new char[length - offset + 1];
//		rawParams[0] = 0;
//
//		// Load elements into raw params
//		for(i; i < length; i++) {
//			rawParams[i - offset] = (char)payload[i];
//		}
//		rawParams[length - offset] = 0;
//		
//		//params = new OnionParams(rawParams);
//		delete[] rawParams;
//	}
//	
//	// Call remote function
//	if(functionId && functionId < totalFunctions) {
//		remoteFunctions[functionId](params);
//	}
//
//	delete params;
//}

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
	    //Serial.print("->Found ");
	    //Serial.print(totalSubscriptions);
	    //Serial.print(" Subscriptions\n");
	    if (totalSubscriptions > 0) {
            OnionPacket* pkt = new OnionPacket(128);
            pkt->setType(ONIONSUBSCRIBE);
            OnionPayloadPacker* pack = new OnionPayloadPacker(pkt);
	        subscription_t *sub_ptr = &subscriptions;
	        pack->packArray(totalSubscriptions);
        	uint8_t string_len = 0;
        	uint8_t param_count = 0;
        	//Serial.print("->Packing subs\n");
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
	        interface->send(pkt);
	        return true;
	        //pkt->send();
	        delete pack;
	        //delete pkt;
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
                            printf("Got publish data...\n");
			    parsePublishData(pkt);
			} else if (type == ONIONPINGREQ) {
			    // Functionize this
                                printf("Got ping req...\n");
				sendPingResponse();
				lastOutActivity = t;
			} else if (type == ONIONPINGRESP) {
                                printf("Got ping resp...\n");
				pingOutstanding = false;
			} else if (type == ONIONSUBACK) {
        	            printf("Publishing Data\n");
        		//publish("/onion","isAwesome");
        		publish(publishMap,2);
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
    //pkt->send();
    //delete pkt;
}

void OnionClient::sendPingResponse(void) {
    OnionPacket* pkt = new OnionPacket(8);
    pkt->setType(ONIONPINGRESP);
    interface->send(pkt);
    //pkt->send();
    //delete pkt;
}

void OnionClient::parsePublishData(OnionPacket* pkt) {
    
    uint16_t length = pkt->getBufferLength();
    uint8_t *ptr = pkt->getBuffer();
//    Serial.print("Publish Pkt Length = ");
//    Serial.print(length);
//    Serial.print("\n");
    OnionPayloadData* data = new OnionPayloadData(pkt);
    
//    Serial.print("Payload Raw Length = ");
//    Serial.print(data->getRawLength());
//    Serial.print("\n");
    data->unpack();
    uint8_t count = data->getLength();
    uint8_t function_id = data->getItem(0)->getInt();
//    Serial.print("Param Count=");
//    Serial.print(count-1);
//    Serial.print("\n");
//    Serial.print("Function Id=");
//    Serial.print(function_id);
//    Serial.print("\n");
	OnionParams* params = new OnionParams(count-1);
    
	if (count > 1) {
	    // Get parameters
	    for (uint8_t i=0;i<(count-1);i++) {
	        OnionPayloadData* item = data->getItem(i+1);
	        uint8_t strLen = item->getLength();
	        // Test
	        char* buf_ptr = (char *)(item->getBuffer());
//            Serial.print("param #");
//            Serial.print(i+1);
//            Serial.print(" = ");
//            Serial.print(buf_ptr);
//            Serial.print("\n");
//            delay(100);
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
	//delete pkt;
	//delete params;
	delete data;
}
